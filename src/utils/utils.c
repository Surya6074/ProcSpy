#include "utils.h"
#include "main.h"

double percent(unsigned long long used, unsigned long long total)
{
    if (total == 0)
        return 0.0;
    return ((double)used / total) * 100.0;
}

int read_first_line(const char *path, char *buf, size_t size)
{
    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror(path);
        return -1;
    }

    if (!fgets(buf, size, fp)) {
        fclose(fp);
        fprintf(stderr, "Error: could not read first line of %s\n", path);
        return -1;
    }

    fclose(fp);
    return 0;
}

int parse_key_value(const char *file_path, const char *key, const char *format, void *output)
{
    FILE *file = fopen(file_path, "r");
    if (!file) {
        // This is normal — process may have exited. Just skip.
        return -1;
    }

    char buffer[256];
    int found = 0;

    while (fgets(buffer, sizeof(buffer), file)) {
        if (strncmp(buffer, key, strlen(key)) == 0) {
            if (sscanf(buffer, format, output) == 1) {
                found = 1;
            }
            break;
        }
    }

    fclose(file);

    if (!found) {
        return -1; // Not found is OK — skip
    }

    return 0;
}