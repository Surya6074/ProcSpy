#include "main.h"

int main(){

    list_all_process();
    
    while (1) {

        CpuStats stats_before = get_cpu_stats();
        sleep(1);
        CpuStats stats_after = get_cpu_stats();

        unsigned long long total_delta = stats_after.total - stats_before.total;
        unsigned long long idle_delta  = stats_after.idle - stats_before.idle;

        if (total_delta == 0) {
            fprintf(stderr, "Warning: total delta is zero. Skipping...\n");
            continue;
        }

        printf("CPU Usage: %.2f%%\n", compute_usage_percentage( idle_delta, total_delta));

         get_memory_usage_stats();
    }

    return 0;
}