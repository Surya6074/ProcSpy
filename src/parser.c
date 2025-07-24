#include "parser.h"

void parse_cpu_fields(const char *line, unsigned long long *fields)
{
    int n = sscanf(line,
        "cpu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
        &fields[0], &fields[1], &fields[2], &fields[3], &fields[4],
        &fields[5], &fields[6], &fields[7], &fields[8], &fields[9]);

    if (n < 4) {
        fprintf(stderr, "Error: failed to parse CPU line\n");
        exit(1);
    }
}
