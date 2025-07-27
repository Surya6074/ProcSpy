#include "main.h"

void version(void) {
    printf("ProcSpy Version 1.0.0\n");
}

struct cpu_stats get_cpu_stats(void)
{
    struct cpu_stats stats = {0};
    char buf[256];
    unsigned long long fields[CPU_FIELDS] = {0};

    if (read_first_line("/proc/stat", buf, sizeof(buf)) < 0)
        return stats;

    parse_cpu_fields(buf, fields);

    for (int i = 0; i < CPU_FIELDS; i++)
        stats.total += fields[i];

    stats.idle = fields[3] + fields[4]; // idle + iowait

    return stats;
}

double usage_percent(struct cpu_stats *before, struct cpu_stats *after)
{
    unsigned long long total = after->total - before->total;
    unsigned long long idle  = after->idle  - before->idle;

    if (!total)
        return 0.0;

    return (1.0 - (double)idle / total) * 100.0;
}

struct mem_stats get_memory_usage_stats(void)
{    
    struct mem_stats stats = {0};

    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        perror("Could not open /proc/meminfo");
        return stats;
    }

    char buf[256];
    unsigned long long mem_total = 0;
    unsigned long long mem_available = 0;

    while (fgets(buf, sizeof(buf), fp)) {
        if (strncmp(buf, "MemTotal:", 9) == 0) {
            sscanf(buf + 9, "%llu", &mem_total);
        } else if (strncmp(buf, "MemAvailable:", 13) == 0) {
            sscanf(buf + 13, "%llu", &mem_available);
        }
        if (mem_total && mem_available) break;
    }

    fclose(fp);

    if (mem_total == 0) {
        fprintf(stderr, "Failed to read MemTotal from /proc/meminfo\n");
        return stats;
    }

    if (mem_available > mem_total) mem_available = mem_total;

    unsigned long long mem_used = mem_total - mem_available;

    stats.free = percent(mem_available, mem_total);
    stats.usage = percent(mem_used, mem_total);

    return stats;
}
struct process_list *list_all_process(void) {
    DIR *proc = opendir("/proc");
    if (!proc) {
        perror("opendir /proc");
        return NULL;
    }

    struct dirent *entry;
    size_t capacity = PROCESS_LIST_INITIAL_CAPACITY;
    size_t count = 0;

    struct process_info *processes = malloc(capacity * sizeof(*processes));
    if (!processes) {
        perror("malloc");
        closedir(proc);
        return NULL;
    }

    // Get system constants
    long clk_tck = sysconf(_SC_CLK_TCK);

    // Get boot time from /proc/stat
    FILE *bootf = fopen("/proc/stat", "r");
    time_t boot_time = 0;
    if (bootf) {
        char line[256];
        while (fgets(line, sizeof(line), bootf)) {
            if (sscanf(line, "btime %ld", &boot_time) == 1)
                break;
        }
        fclose(bootf);
    }

    while ((entry = readdir(proc)) != NULL) {
        if (!isdigit(entry->d_name[0])) continue;

        struct process_info p = {0};
        char path[512];

        // Read /proc/[pid]/status
        snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);
        if (parse_key_value(path, "Pid:",    "Pid:\t%llu",   &p.pid)    != 0) continue;
        if (parse_key_value(path, "PPid:",   "PPid:\t%llu",  &p.ppid)   != 0) continue;
        if (parse_key_value(path, "Threads:","Threads:\t%d", &p.threads)!= 0) continue;
        if (parse_key_value(path, "State:",  "State:\t%s",   &p.state)  != 0) continue;
        if (parse_key_value(path, "VmSize:", "VmSize:\t%lu kB", &p.vm_size) != 0) continue;
        if (parse_key_value(path, "VmRSS:",  "VmRSS:\t%lu kB", &p.vm_rss)  != 0) continue;
        if (parse_key_value(path, "Uid:", "Uid:\t%u", &p.uid) != 0) continue;

        // Convert UID to username
        struct passwd *pw = getpwuid(p.uid);
        strncpy(p.username, pw ? pw->pw_name : "unknown", sizeof(p.username));

        // Read command name
        snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);
        FILE *f = fopen(path, "r");
        if (f && fgets(p.comm, sizeof(p.comm), f)) {
            size_t len = strlen(p.comm);
            if (len > 0 && p.comm[len - 1] == '\n')
                p.comm[len - 1] = '\0';
            fclose(f);
        } else {
            strncpy(p.comm, "N/A", sizeof(p.comm));
            if (f) fclose(f);
        }

        // Read /proc/[pid]/stat for priority, nice, starttime, utime+stime
        snprintf(path, sizeof(path), "/proc/%s/stat", entry->d_name);
        FILE *statf = fopen(path, "r");
        if (statf) {
            unsigned long utime = 0, stime = 0;
            unsigned long long starttime = 0;
            int prio = 0, nice = 0;
            char dummy[1024], comm[256];
            int dummy_int;
            fscanf(statf, "%d %s", &dummy_int, comm); // pid and comm

            for (int i = 0; i < 11; i++) fscanf(statf, "%s", dummy); // skip to 14

            fscanf(statf, "%lu %lu", &utime, &stime);     // 14, 15
            fscanf(statf, "%s %s", dummy, dummy);         // 16, 17
            fscanf(statf, "%d %d", &prio, &nice);         // 18, 19

            for (int i = 0; i < 2; i++) fscanf(statf, "%s", dummy); // 20, 21
            fscanf(statf, "%llu", &starttime);            // 22

            fclose(statf);

            p.priority = prio;
            p.nice = nice;
            p.cpu_time_sec = (utime + stime) / (double)clk_tck;
            p.start_time = boot_time + (starttime / clk_tck);
        }

        // Store
        if (count >= capacity) {
            capacity *= 2;
            struct process_info *tmp = realloc(processes, capacity * sizeof(*tmp));
            if (!tmp) {
                perror("realloc");
                free(processes);
                closedir(proc);
                return NULL;
            }
            processes = tmp;
        }
        processes[count++] = p;
    }

    closedir(proc);

    struct process_list *result = malloc(sizeof(*result));
    if (!result) {
        perror("malloc process_list");
        free(processes);
        return NULL;
    }

    result->data = processes;
    result->count = count;
    return result;
}

void help(void) {
    printf("\n\033[1;36mProcess Table Column Descriptions\033[0m\n\n");
    printf(" %-10s : %s\n", "PID",     "Process ID — Unique ID for each process.");
    printf(" %-10s : %s\n", "PPID",    "Parent PID — ID of the parent process.");
    printf(" %-10s : %s\n", "THR",     "Threads — Number of threads the process uses.");
    printf(" %-10s : %s\n", "STAT",    "State — Process state (R=Running, S=Sleeping, Z=Zombie, etc.).");
    printf(" %-10s : %s\n", "PRI",     "Priority — Kernel scheduling priority.");
    printf(" %-10s : %s\n", "NI",      "Nice — User-defined priority (-20 to 19, lower = higher priority).");
    printf(" %-10s : %s\n", "VmSize",  "Virtual Memory — Total virtual memory used (in KB).");
    printf(" %-10s : %s\n", "VmRSS",   "Resident Set — Actual physical memory used (in KB).");
    printf(" %-10s : %s\n", "USER",    "Username — Owner of the process.");
    printf(" %-10s : %s\n", "CMD",     "Command — Command or executable that launched the process.");

    printf("\nUse arrow keys to navigate, 'q' to quit.\n");
}