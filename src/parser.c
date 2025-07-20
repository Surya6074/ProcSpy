#include "parser.h"

void parse_cpu_line(const char *line, unsigned long long cpu_fields[CPU_FIELD_COUNT]) {
    int fields_parsed = sscanf(
        line,
        "cpu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
        &cpu_fields[0], &cpu_fields[1], &cpu_fields[2], &cpu_fields[3], &cpu_fields[4],
        &cpu_fields[5], &cpu_fields[6], &cpu_fields[7], &cpu_fields[8], &cpu_fields[9]);

    if (fields_parsed < 4) {
        fprintf(stderr, "Error: Failed to parse CPU fields from line.\n");
    }
}
