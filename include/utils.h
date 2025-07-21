#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Compute the Percentage
double compute_usage_percentage(unsigned long long idle, unsigned long long total);

// get Single Line content
int read_line_from_file(const char *file_path, char *buffer, size_t buffer_size, int target_line);

// parse Line data
int parse_line_value(const char *file_path, int target_line, const char *format, void *output);

#endif 
