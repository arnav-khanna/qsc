#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

#include "qsc3.h"

static void print_usage(const char *prog) {
    printf("QSC v3: Production Compression Engine (C)\n\n");
    printf("Usage:\n");
    printf("  %s compress   <input> <output.qsc>\n", prog);
    printf("  %s decompress <input.qsc> <output_dir>\n", prog);
    printf("  %s benchmark  <input_file>\n", prog);
    printf("\n");
}

static uint64_t get_total_size(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;

    if (S_ISREG(st.st_mode)) {
        return (uint64_t)st.st_size;
    }

    /* Directory — recurse */
    uint64_t total = 0;
    DIR *d = opendir(path);
    if (!d) return 0;

    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (ent->d_name[0] == '.') continue;
        char child[4096];
        snprintf(child, sizeof(child), "%s/%s", path, ent->d_name);
        total += get_total_size(child);
    }
    closedir(d);
    return total;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    const char *mode  = argv[1];
    const char *input = argv[2];

    if (strcmp(mode, "benchmark") == 0) {
        qsc_benchmark(input);
        return 0;
    }

    if (argc < 4) {
        fprintf(stderr, "Error: Output path required\n\n");
        print_usage(argv[0]);
        return 1;
    }

    const char *output = argv[3];

    if (strcmp(mode, "compress") == 0) {
        struct timespec ts_start, ts_end;
        clock_gettime(CLOCK_MONOTONIC, &ts_start);

        int rc = qsc_pack(input, output);
        if (rc != 0) return rc;

        clock_gettime(CLOCK_MONOTONIC, &ts_end);
        double elapsed = (ts_end.tv_sec - ts_start.tv_sec) +
                         (ts_end.tv_nsec - ts_start.tv_nsec) / 1e9;

        uint64_t orig_size = get_total_size(input);
        struct stat st;
        stat(output, &st);
        uint64_t comp_size = (uint64_t)st.st_size;

        printf("\n✅ Compressed %s → %s\n", input, output);
        if (orig_size > 0) {
            printf("   %llu → %llu bytes (%.1f%%)\n",
                   (unsigned long long)orig_size,
                   (unsigned long long)comp_size,
                   (double)comp_size / orig_size * 100.0);
        }
        printf("   Time: %.2fs\n", elapsed);

    } else if (strcmp(mode, "decompress") == 0) {
        struct timespec ts_start, ts_end;
        clock_gettime(CLOCK_MONOTONIC, &ts_start);

        int rc = qsc_unpack(input, output);
        if (rc != 0) return rc;

        clock_gettime(CLOCK_MONOTONIC, &ts_end);
        double elapsed = (ts_end.tv_sec - ts_start.tv_sec) +
                         (ts_end.tv_nsec - ts_start.tv_nsec) / 1e9;

        printf("\n✅ Decompressed %s → %s\n", input, output);
        printf("   Time: %.2fs\n", elapsed);

    } else {
        fprintf(stderr, "Error: Unknown mode '%s'. Use compress, decompress, or benchmark.\n", mode);
        return 1;
    }

    return 0;
}
