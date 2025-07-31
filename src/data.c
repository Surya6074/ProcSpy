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
struct process_list *list_all_process(unsigned long long pid_filter) {
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

    // Get total memory from /proc/meminfo
    unsigned long total_memory_kb = 0;
    FILE *meminfo = fopen("/proc/meminfo", "r");
    if (meminfo) {
        char line[256];
        while (fgets(line, sizeof(line), meminfo)) {
            if (sscanf(line, "MemTotal: %lu kB", &total_memory_kb) == 1)
                break;
        }
        fclose(meminfo);
    }

    // Get system uptime
    double system_uptime = 0.0;
    FILE *uptimef = fopen("/proc/uptime", "r");
    if (uptimef) {
        fscanf(uptimef, "%lf", &system_uptime);
        fclose(uptimef);
    }

    while ((entry = readdir(proc)) != NULL) {
        if (!isdigit(entry->d_name[0])) continue;

        unsigned long long pid = strtoull(entry->d_name, NULL, 10);
        if (pid_filter && pid != pid_filter) continue;

        struct process_info p = {0};
        p.pid = pid;

        char path[512];

        snprintf(path, sizeof(path), "/proc/%llu/status", pid);
        if (parse_key_value(path, "PPid:",   "PPid:\t%llu",  &p.ppid)   != 0) continue;
        if (parse_key_value(path, "Threads:","Threads:\t%d", &p.threads)!= 0) continue;
        if (parse_key_value(path, "State:",  "State:\t%s",   &p.state)  != 0) continue;
        if (parse_key_value(path, "VmSize:", "VmSize:\t%lu kB", &p.vm_size) != 0) continue;
        if (parse_key_value(path, "VmRSS:",  "VmRSS:\t%lu kB", &p.vm_rss)  != 0) continue;
        if (parse_key_value(path, "Uid:", "Uid:\t%u", &p.uid) != 0) continue;

        struct passwd *pw = getpwuid(p.uid);
        strncpy(p.username, pw ? pw->pw_name : "unknown", sizeof(p.username));

        snprintf(path, sizeof(path), "/proc/%llu/comm", pid);
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

        // /proc/[pid]/stat
        snprintf(path, sizeof(path), "/proc/%llu/stat", pid);
        FILE *statf = fopen(path, "r");
        if (statf) {
            unsigned long utime = 0, stime = 0;
            unsigned long long starttime = 0;
            int prio = 0, nice = 0;
            char dummy[1024], comm[256];
            int dummy_int;

            fscanf(statf, "%d %s", &dummy_int, comm); // pid & comm
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

            // Calculate CPU usage
            double elapsed_time = system_uptime - (p.start_time - boot_time);
            p.cpu_usage = (elapsed_time > 0) ? (p.cpu_time_sec / elapsed_time) * 100.0 : 0.0;
        }

        // Calculate memory usage percent
        p.mem_usage_percent = (total_memory_kb > 0) ?
            (p.vm_rss / (double)total_memory_kb) * 100.0 : 0.0;

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

        if (pid_filter) break;
    }

    closedir(proc);

    if (pid_filter && count == 0) {
        free(processes);
        return NULL;
    }

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
    printf("\n\033[1;36m┌───────────────────────────────────────────────────────────┐\033[0m\n");
    printf(  "\033[1;36m│                  🔍 ProcSpy - Process Monitor             │\033[0m\n");
    printf(  "\033[1;36m└───────────────────────────────────────────────────────────┘\033[0m\n");
    printf("  A terminal-based, real-time Linux process viewer with\n");
    printf("  sortable columns and detailed resource statistics.\n");

    printf("\n\033[1;33m🛠️  Usage:\033[0m\n");
    printf("  procspy [--help] [--version] [--log [filename]]\n");

    printf("\n\033[1;33m🚩 Flags:\033[0m\n");
    printf("  --help        Show this help screen and exit.\n");
    printf("  --version     Display application version.\n");
    printf("  --log [file]  Save current process list to a file (default: procspy.log).\n");
    printf("  --web         Web Based Process viewer\n");

    printf("\n\033[1;36m📋 Process Table Columns:\033[0m\n");
    printf("  %-10s : %s\n", "PID",     "Process ID.");
    printf("  %-10s : %s\n", "PPID",    "Parent Process ID.");
    printf("  %-10s : %s\n", "USER",    "Username (owner of the process).");
    printf("  %-10s : %s\n", "CMD",     "Command or executable.");
    printf("  %-10s : %s\n", "THR",     "Thread count.");
    printf("  %-10s : %s\n", "STAT",    "Process state (e.g. R, S, Z).");
    printf("  %-10s : %s\n", "PRI",     "Priority (scheduling).");
    printf("  %-10s : %s\n", "NI",      "Nice value (user-set priority).");
    printf("  %-10s : %s\n", "CPU%%",    "CPU usage (per-process).");
    printf("  %-10s : %s\n", "MEM%%",    "Memory usage as percent of total.");
    printf("  %-10s : %s\n", "TIME",    "Total CPU time consumed.");
    printf("  %-10s : %s\n", "VmSize",  "Total virtual memory (in KB).");
    printf("  %-10s : %s\n", "VmRSS",   "Resident memory (RAM used, in KB).");

    printf("\n\033[1;33m⌨️ Controls:\033[0m\n");
    printf("  Arrow Keys   Move selection (↑↓) or scroll (←→).\n");
    printf("  Enter        Open detailed view for selected process.\n");
    printf("  Backspace    Return to the process list from details.\n");
    printf("  Q            Quit the application.\n");

    printf("\n\033[1;35m↕ Sorting:\033[0m\n");
    printf("  Press \033[1mF1\033[0m followed by a letter to sort the table:\n");
    printf("    \033[1;32mp\033[0m  Sort by PID\n");
    printf("    \033[1;32mc\033[0m  Sort by CPU usage (%%)\n");
    printf("    \033[1;32mm\033[0m  Sort by memory usage (%%)\n");
    printf("    \033[1;32mt\033[0m  Sort by total CPU time\n");
    printf("    \033[1;32mn\033[0m  Sort by command name\n");
    printf("  \033[3mPressing the same key again will re-sort.\033[0m\n");

    printf("\n\033[1;34m💡 Tip:\033[0m Resize the terminal window if the layout breaks.\n");
    printf("        Add `--log` to dump process info for analysis.\n");

    printf("\n\033[1;36m─────────────────────────────────────────────────────────────\033[0m\n\n");
}


int logger(const char *filename) {

    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        perror("Error opening log file");
        return 1;
    }

    time_t now = time(NULL);
    char *timestamp = ctime(&now);
    if (timestamp) {
        timestamp[strcspn(timestamp, "\n")] = '\0';
        fprintf(file, "\n===== ProcSpy Snapshot - %s =====\n", timestamp);
    }

    // CPU usage calculation
    struct cpu_stats before = get_cpu_stats();
    sleep(1);
    struct cpu_stats after = get_cpu_stats();
    double cpu_usage = usage_percent(&before, &after);

    // Memory usage
    struct mem_stats mem = get_memory_usage_stats();

    fprintf(file, "CPU Usage: %.2f%%\n", cpu_usage);
    fprintf(file, "Memory Usage: %.2f%%, Free: %.2f MB\n\n", mem.usage, mem.free);

    // Get all processes
    struct process_list *plist = list_all_process(0);
    if (!plist || plist->count == 0) {
        fprintf(file, "No processes found.\n");
        if (plist) {
            free(plist->data);
            free(plist);
        }
        fclose(file);
        return -1;
    }

    // Print table headers
    fprintf(file,
        "%-8s %-8s %-5s %-5s %-5s %-5s %-10s %-10s %-10s %-25s %-8s %-8s %-8s\n",
        "PID", "PPID", "THR", "STAT", "PRI", "NI", "VmSize", "VmRSS", "USER", "CMD", "CPU%", "MEM%", "TIME");

    // Print process data
    for (size_t i = 0; i < plist->count; ++i) {
        struct process_info *p = &plist->data[i];

        // Format CPU time (MM:SS)
        char time_buf[16];
        time_t total_seconds = (time_t)p->cpu_time_sec;
        snprintf(time_buf, sizeof(time_buf), "%02lu:%02lu",
                 total_seconds / 60, total_seconds % 60);

        // Convert KB to MB
        unsigned long vm_size_mb = p->vm_size / 1024;
        unsigned long vm_rss_mb  = p->vm_rss / 1024;

        // Truncate user and cmd fields
        char truncated_user[12];
        char truncated_cmd[18];
        snprintf(truncated_user, sizeof(truncated_user), "%-11.11s", p->username);
        snprintf(truncated_cmd, sizeof(truncated_cmd), "%-17.17s", p->comm);

        fprintf(file,
            "%-8llu %-8llu %-5d %-5s %-5d %-5d %-10lu %-10lu %-10s %-20s %8.1f%% %8.1f%% %8s\n",
            p->pid, p->ppid, p->threads, p->state,
            p->priority, p->nice, vm_size_mb, vm_rss_mb,
            truncated_user, truncated_cmd, p->cpu_usage, p->mem_usage_percent, time_buf);
    }

    free(plist->data);
    free(plist);
    fclose(file);

    return 0;
}
int compare_by_pid(const void *a, const void *b) {
    return ((struct process_info *)a)->pid - ((struct process_info *)b)->pid;
}

int compare_by_cpu(const void *a, const void *b) {
    double diff = ((struct process_info *)b)->cpu_usage - ((struct process_info *)a)->cpu_usage;
    return (diff > 0) - (diff < 0);
}

int compare_by_mem(const void *a, const void *b) {
    double diff = ((struct process_info *)b)->mem_usage_percent - ((struct process_info *)a)->mem_usage_percent;
    return (diff > 0) - (diff < 0);
}

int compare_by_time(const void *a, const void *b) {
    return ((struct process_info *)b)->cpu_time_sec - ((struct process_info *)a)->cpu_time_sec;
}

int compare_by_name(const void *a, const void *b) {
    return strncmp(((struct process_info *)a)->comm, ((struct process_info *)b)->comm, 16);
}

void sort_process_list(struct process_list *plist, SortMode mode) {
    switch (mode) {
        case SORT_BY_CPU:
            qsort(plist->data, plist->count, sizeof(struct process_info), compare_by_cpu);
            break;
        case SORT_BY_MEM:
            qsort(plist->data, plist->count, sizeof(struct process_info), compare_by_mem);
            break;
        case SORT_BY_TIME:
            qsort(plist->data, plist->count, sizeof(struct process_info), compare_by_time);
            break;
        case SORT_BY_NAME:
            qsort(plist->data, plist->count, sizeof(struct process_info), compare_by_name);
            break;
        case SORT_BY_PID:
        default:
            qsort(plist->data, plist->count, sizeof(struct process_info), compare_by_pid);
            break;
    }
}


char *web_response_json() {
    static char json[BUFFER_SIZE_RESPONSE];
    char *ptr = json;

    struct cpu_stats before = get_cpu_stats();
    sleep(1);
    struct cpu_stats after = get_cpu_stats();
    double cpu_usage = usage_percent(&before, &after);

    struct mem_stats mem = get_memory_usage_stats();

    ptr += sprintf(ptr,
        "{\n"
        "  \"usages\": {\n"
        "    \"cpu\": %.2f,\n"
        "    \"memory\": {\n"
        "      \"usage\": %.2f,\n"
        "      \"free_mb\": %.2f\n"
        "    }\n"
        "  },\n"
        "  \"processes\": [\n",
        cpu_usage, mem.usage, mem.free);

    struct process_list *plist = list_all_process(0);
    if (!plist || plist->count == 0) {
        ptr += sprintf(ptr, "  ]\n}\n");
        return json;
    }

    for (size_t i = 0; i < plist->count; ++i) {
        struct process_info *p = &plist->data[i];

        char time_buf[16];
        time_t total_seconds = (time_t)p->cpu_time_sec;
        snprintf(time_buf, sizeof(time_buf), "%02lu:%02lu",
                 total_seconds / 60, total_seconds % 60);

        ptr += sprintf(ptr,
            "    {\n"
            "      \"pid\": %llu,\n"
            "      \"ppid\": %llu,\n"
            "      \"threads\": %d,\n"
            "      \"state\": \"%s\",\n"
            "      \"priority\": %d,\n"
            "      \"nice\": %d,\n"
            "      \"vmsize_mb\": %lu,\n"
            "      \"vmrss_mb\": %lu,\n"
            "      \"user\": \"%s\",\n"
            "      \"command\": \"%s\",\n"
            "      \"cpu_usage\": %.1f,\n"
            "      \"mem_usage\": %.1f,\n"
            "      \"time\": \"%s\"\n"
            "    }%s\n",
            p->pid, p->ppid, p->threads, p->state,
            p->priority, p->nice,
            p->vm_size / 1024, p->vm_rss / 1024,
            p->username, p->comm,
            p->cpu_usage, p->mem_usage_percent, time_buf,
            (i < plist->count - 1) ? "," : "");
    }

    ptr += sprintf(ptr, "  ]\n}\n");

    free(plist->data);
    free(plist);

    return json;
}
