#include "main.h"

int main(){

    printf("===================================\n");
    printf("         🕵️  ProcSpy Monitor         \n");
    printf("===================================\n");
    printf("  Monitoring system processes...\n");
    printf("  Press Ctrl+C to exit.\n");
    printf("===================================\n\n");
    
    list_all_process();

    printf("\n----------- CPU and Memory Stats -----------\n\n");

    while (1) {
        struct cpu_stats before = get_cpu_stats();
        sleep(1);
        struct cpu_stats after = get_cpu_stats();

        double cpu_usage = usage_percent(&before, &after);
        printf("CPU Usage   : %.2f%%\n", cpu_usage);

        if (get_memory_usage_stats() < 0) {
            fprintf(stderr, "Warning: Failed to get memory usage\n");
        }

        printf("\n");
    }

    return 0;
}