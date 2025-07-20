#ifndef DATA_H
#define DATA_H


// CPU Usage
typedef struct {
    unsigned long long total;
    unsigned long long idle;
} CpuStats;

CpuStats get_cpu_stats(void);

// Memory Usage
int get_memory_usage_stats(void);

#endif 
