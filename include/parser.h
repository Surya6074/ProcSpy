#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

#define CPU_FIELD_COUNT 10

// Cpu Usage Parser
void parse_cpu_line(const char *line, unsigned long long cpu_fields[CPU_FIELD_COUNT]);

#endif 
