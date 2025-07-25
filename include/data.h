#ifndef DATA_H
#define DATA_H

#include <dirent.h>
#include <ctype.h>

/* ===========================
 * CPU usage data and helpers
 * ===========================
 */

/**
 * @brief Holds total and idle CPU time from /proc/stat.
 */
struct cpu_stats {
    unsigned long long total;
    unsigned long long idle;
};

/**
 * @brief Read current CPU stats from /proc/stat.
 *
 * @return Struct with total and idle time.
 */
struct cpu_stats get_cpu_stats(void);

/**
 * @brief Compute CPU usage percentage between two samples.
 *
 * @param before Stats before interval.
 * @param after Stats after interval.
 * @return Usage percentage.
 */
double usage_percent(struct cpu_stats *before, struct cpu_stats *after);

/* ===========================
 * Memory usage
 * ===========================
 */

/**
 * @brief Holds available memory and memory usage information from /proc/meminfo.
 */
struct mem_stats {
    double free; 
    double usage;
};

/**
 * @brief Read memory statistics from /proc/meminfo.
 *
 * Reads MemTotal and MemAvailable to calculate usage and free percentage.
 *
 * @return struct mem_stats containing usage and free percentage.
 */
struct mem_stats get_memory_usage_stats(void);

/* ===========================
 * Process info and listing
 * ===========================
 */

#define PROCESS_LIST_INITIAL_CAPACITY 128

/**
 * @brief Holds basic process information.
 */
struct process_info {
    unsigned long long pid;
    unsigned long long ppid;
    int threads;
    char state[16];
    unsigned long vm_size;
    unsigned long vm_rss;
    char comm[1024];
};

/**
 * @brief List all processes and print basic info.
 */
void list_all_process(void);

#endif
