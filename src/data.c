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

int get_memory_usage_stats(void)
{
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        perror("/proc/meminfo");
        return -1;
    }

    char buf[256];
    unsigned long long mem_total = 0;
    unsigned long long mem_available = 0;

    while (fgets(buf, sizeof(buf), fp)) {
        if (sscanf(buf, "MemTotal: %llu kB", &mem_total) == 1)
            continue;

        if (sscanf(buf, "MemAvailable: %llu kB", &mem_available) == 1)
            break;
    }

    fclose(fp);

    if (mem_total == 0 || mem_available == 0) {
        fprintf(stderr, "Error: Failed to parse MemTotal or MemAvailable\n");
        return -1;
    }

    unsigned long long mem_used = mem_total - mem_available;

    printf("Memory Usage: %.2f%%\n", percent(mem_used, mem_total));
    printf("Memory Free : %.2f%%\n", percent(mem_available, mem_total));

    return 0;
}

void list_all_process(void)
{
    DIR *proc = opendir("/proc");
    if (!proc) {
        perror("opendir /proc");
        return;
    }

    struct dirent *entry;

    // Dynamic array setup
    struct process_info *processes = malloc(PROCESS_LIST_INITIAL_CAPACITY * sizeof(*processes));
    if (!processes) {
        perror("malloc");
        closedir(proc);
        return;
    }

    size_t capacity = PROCESS_LIST_INITIAL_CAPACITY;
    size_t count = 0;

    while ((entry = readdir(proc)) != NULL) {
        if (!isdigit(entry->d_name[0]))
            continue;

        char file_path[512];
        snprintf(file_path, sizeof(file_path), "/proc/%s/status", entry->d_name);

        struct process_info p = {0};

        if (parse_key_value(file_path, "Pid:", "Pid:\t%llu", &p.pid) != 0)
            continue;
        if (parse_key_value(file_path, "PPid:", "PPid:\t%llu", &p.ppid) != 0)
            continue;
        if (parse_key_value(file_path, "Threads:", "Threads:\t%d", &p.threads) != 0)
            continue;
        if (parse_key_value(file_path, "State:", "State:\t%s", &p.state) != 0)
            continue;
        if (parse_key_value(file_path, "VmSize:", "VmSize:\t%lu kB", &p.vm_size) != 0)
            continue;
        if (parse_key_value(file_path, "VmRSS:",  "VmRSS:\t%lu kB", &p.vm_rss) != 0)
            continue;
        
        // Get cmdline
        snprintf(file_path, sizeof(file_path), "/proc/%s/comm", entry->d_name);

        FILE *f = fopen(file_path, "r");
        if (f) {
            if (fgets(p.comm, sizeof(p.comm), f) != NULL) {
                // Remove trailing newline
                size_t len = strlen(p.comm);
                if (len > 0 && p.comm[len - 1] == '\n') {
                    p.comm[len - 1] = '\0';
                }
            } else {
                strcpy(p.comm, "N/A");
            }
            fclose(f);
        } else {
            strcpy(p.comm, "N/A");
        }

        // Store in array
        if (count >= capacity) {
            capacity *= 2;
            struct process_info *tmp = realloc(processes, capacity * sizeof(*processes));
            if (!tmp) {
                perror("realloc");
                free(processes);
                closedir(proc);
                return;
            }
            processes = tmp;
        }
        processes[count++] = p;
    }

    closedir(proc);

    printf("%-8s %-8s %-8s %-8s\t %-12s\t %-12s\t %-20s\n",
        "PID", "PPID", "Threads", "State", "VmSize", "VmRSS", "Comm");

    for (size_t i = 0; i < count; ++i) {
        printf("%-8llu %-8llu %-8d %-8s %10lu KB %10lu KB\t\t %-20s\n",
            processes[i].pid,
            processes[i].ppid,
            processes[i].threads,
            processes[i].state,
            processes[i].vm_size,
            processes[i].vm_rss,
            processes[i].comm);
    }

    free(processes);
}
