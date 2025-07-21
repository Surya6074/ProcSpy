#include "utils.h"
#include "main.h"

// Compute the Usage in Percentage
double compute_usage_percentage(unsigned long long idle, unsigned long long total) {
    if (total == 0) {
        return 0.0;
    }
    return 100.0 * (1.0 - ((double)idle / total));
}

// read specific Line from the file
int read_line_from_file(const char *file_path, char *buffer, size_t buffer_size, int target_line) {

    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    int current_line = 1;

    while (fgets(buffer, buffer_size, file) != NULL) {
        if (current_line == target_line) {
            fclose(file);
            return 0;
        }
        current_line++;
    }

    fclose(file);
    return -1;
}


int parse_line_value(const char *file_path, int target_line, const char *format, void *output){
    char line_buffer[1024];

    if (read_line_from_file(file_path, line_buffer, sizeof(line_buffer), target_line) != 0) {
        fprintf(stderr, "Failed to read line\n");
        return -1;
    }

    if (sscanf(line_buffer, format, &output) != 1) {
        fprintf(stderr, "Failed to parse data\n");
        return -1;
    }

    return 0;
}