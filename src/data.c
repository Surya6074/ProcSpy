#include "main.h"

CpuStats get_cpu_stats(void) {
    CpuStats stats = {0};
    char line_buffer[256];

    if (read_line_from_file("/proc/stat", line_buffer, sizeof(line_buffer), 1) != 0) {
        fprintf(stderr, "Error: Could not read /proc/stat\n");
        return stats;
    }

    unsigned long long cpu_fields[CPU_FIELD_COUNT] = {0};
    parse_cpu_line(line_buffer, cpu_fields);

    for (int i = 0; i < CPU_FIELD_COUNT; i++) {
        stats.total += cpu_fields[i];
    }

    // By convention, idle = idle + iowait
    stats.idle = cpu_fields[3] + cpu_fields[4];

    return stats;
}

// get memory usage
int get_memory_usage_stats(){

    char line_buffer[256];

    if (read_line_from_file("/proc/meminfo", line_buffer, sizeof(line_buffer), 1) != 0) {
        fprintf(stderr, "Failed to read MemTotal line\n");
        return -1;
    }

    unsigned long long mem_total = 0;
    if (sscanf(line_buffer, "MemTotal: %llu kB", &mem_total) != 1) {
        fprintf(stderr, "Failed to parse MemTotal\n");
        return -1;
    }

    if (read_line_from_file("/proc/meminfo", line_buffer, sizeof(line_buffer), 3) != 0) {
        fprintf(stderr, "Failed to read MemAvailable line\n");
        return -1;
    }

    unsigned long long mem_available = 0;
    if (sscanf(line_buffer, "MemAvailable: %llu kB", &mem_available) != 1) {
        fprintf(stderr, "Failed to parse MemAvailable\n");
        return -1;
    }

    unsigned long long mem_used = mem_total - mem_available;

    printf("mem Usage : %.2f%% \n",compute_usage_percentage(mem_available, mem_total));

    return 0;
}