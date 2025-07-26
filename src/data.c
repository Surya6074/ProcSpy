#include "main.h"

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

    while ((entry = readdir(proc)) != NULL) {
        // Skip non-numeric entries
        if (!isdigit(entry->d_name[0]))
            continue;

        struct process_info p = {0};

        char status_path[512];
        snprintf(status_path, sizeof(status_path), "/proc/%s/status", entry->d_name);

        // Parse all required fields
        if (parse_key_value(status_path, "Pid:",    "Pid:\t%llu",   &p.pid)    != 0) continue;
        if (parse_key_value(status_path, "PPid:",   "PPid:\t%llu",  &p.ppid)   != 0) continue;
        if (parse_key_value(status_path, "Threads:","Threads:\t%d", &p.threads)!= 0) continue;
        if (parse_key_value(status_path, "State:",  "State:\t%s",   &p.state)  != 0) continue;
        if (parse_key_value(status_path, "VmSize:", "VmSize:\t%lu kB", &p.vm_size) != 0) continue;
        if (parse_key_value(status_path, "VmRSS:",  "VmRSS:\t%lu kB", &p.vm_rss)  != 0) continue;

        // Read command name
        char comm_path[512];
        snprintf(comm_path, sizeof(comm_path), "/proc/%s/comm", entry->d_name);

        FILE *f = fopen(comm_path, "r");
        if (f && fgets(p.comm, sizeof(p.comm), f)) {
            size_t len = strlen(p.comm);
            if (len > 0 && p.comm[len - 1] == '\n') {
                p.comm[len - 1] = '\0';
            }
            fclose(f);
        } else {
            strncpy(p.comm, "N/A", sizeof(p.comm));
            if (f) fclose(f);
        }

        // Resize array if needed
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