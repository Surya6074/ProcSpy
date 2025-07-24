#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>

/* ===========================
 * Parser helpers for /proc
 * ===========================
 */

#define CPU_FIELDS 10  /**< Number of expected CPU fields in /proc/stat line */

/**
 * @brief Parse CPU fields from a single line of /proc/stat.
 *
 * Expected line format: "cpu  <user> <nice> <system> <idle> ..."
 *
 * @param line Input line.
 * @param fields Array to store parsed fields. Must have at least CPU_FIELDS elements.
 */
void parse_cpu_fields(const char *line, unsigned long long *fields);

#endif /* PARSER_H */
