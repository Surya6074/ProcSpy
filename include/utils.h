#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Calculate the percentage of `used` over `total`.
 *
 * Example: percent(50, 200) = 25.0
 *
 * @param used The used portion.
 * @param total The total value.
 * @return Percentage as double. Returns 0.0 if total is zero.
 */
double percent(unsigned long long used, unsigned long long total);

/**
 * @brief Read the first line from a file into buffer.
 *
 * @param path Path to the file.
 * @param buf Buffer to store the line.
 * @param size Size of the buffer.
 * @return 0 on success, -1 on failure.
 */
int read_first_line(const char *path, char *buf, size_t size);

/**
 * @brief Parse a specific key's value from a key-value file.
 *
 * Searches for a line that starts with `key` and parses its value using the given format.
 *
 * Example: parse_key_value("/proc/123/status", "Pid:", "Pid:\t%llu", &pid);
 *
 * @param file_path Path to the file.
 * @param key Key prefix to match (e.g., "Pid:").
 * @param format sscanf format to extract the value.
 * @param output Pointer to store parsed value.
 * @return 0 if found and parsed, -1 if not found or failed.
 */
int parse_key_value(const char *file_path, const char *key, const char *format, void *output);

/**
 * @brief Generate a simple ASCII progress bar.
 *
 * Builds a progress bar string with '#' characters based on a percentage,
 * filling the remaining width with spaces. The result is written into the
 * caller-provided output buffer.
 *
 * Example usage:
 *   char bar[21];
 *   make_bar(65.5, 20, bar);
 *   printf("[%s] %.2f%%\n", bar, 65.5);
 *
 * @param percent    Percentage to represent (0.0 to 100.0).
 * @param bar_width  Total width of the bar (number of characters).
 * @param output     Buffer to store the resulting null-terminated bar string.
 *                   Must be at least (bar_width + 1) bytes.
 */
void make_bar(double percent, int bar_width, char *output);


#endif /* UTILS_H */
