/*
 * output.c - Output file management and backups
 */

#include "output.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

/* Create backup of existing file with timestamp
 * Format: filename.ext.YYYYMMDD_HHMMSS.bak
 * Stores backup path in output_backup if provided
 */
bool create_backup(const char *path, char *output_backup, size_t backup_size,
                   RyftOptions *options, RyftStats *stats)
{
    if (!file_exists(path)) {
        return true;  /* Nothing to backup */
    }

    /* Generate timestamp */
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", tm_info);

    /* Build backup filename */
    char backup_path[MAX_PATH];
    snprintf(backup_path, sizeof(backup_path), "%s.%s.bak", path, timestamp);

    /* Copy file contents */
    FILE *src = fopen(path, "rb");
    if (!src) {
        fprintf(stderr, "error: cannot read '%s' for backup: %s\n",
                path, strerror(errno));
        return false;
    }

    FILE *dst = fopen(backup_path, "wb");
    if (!dst) {
        fclose(src);
        fprintf(stderr, "error: cannot create backup '%s': %s\n",
                backup_path, strerror(errno));
        return false;
    }

    /* Copy in chunks */
    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
        if (fwrite(buf, 1, n, dst) != n) {
            fprintf(stderr, "error: failed writing backup '%s': %s\n",
                    backup_path, strerror(errno));
            fclose(src);
            fclose(dst);
            return false;
        }
    }

    fclose(src);
    fclose(dst);

    /* Store backup path if requested */
    if (output_backup && backup_size > 0) {
        strncpy(output_backup, backup_path, backup_size - 1);
        output_backup[backup_size - 1] = '\0';
    }

    if (options->verbose) {
        printf("  backup: %s -> %s\n", path, backup_path);
    }

    stats->backups_created++;
    return true;
}

/* Find or create output file entry */
int get_output_file(OutputState *state, const char *path)
{
    /* Expand the path first */
    char expanded[MAX_PATH];
    if (!expand_path(path, expanded, sizeof(expanded))) {
        return -1;
    }

    /* Check if already exists */
    for (int i = 0; i < state->count; i++) {
        if (strcmp(state->files[i].path, expanded) == 0) {
            return i;
        }
    }

    /* Create new entry */
    if (state->count >= MAX_OUTPUT_FILES) {
        fprintf(stderr, "error: too many output files (max %d)\n", MAX_OUTPUT_FILES);
        return -1;
    }

    int idx = state->count++;
    strncpy(state->files[idx].path, expanded, MAX_PATH - 1);
    state->files[idx].path[MAX_PATH - 1] = '\0';
    state->files[idx].fp = NULL;
    state->files[idx].block_count = 0;
    state->files[idx].unnamed_block_count = 0;

    if (state->count > 1) {
        state->multiple_files = true;
    }

    return idx;
}

/* Open output file for writing (or simulate in dry-run mode) */
FILE *open_output(OutputState *state, int idx, const char *lang,
                  RyftOptions *options, RyftStats *stats)
{
    if (idx < 0 || idx >= state->count) {
        return NULL;
    }

    OutputFile *of = &state->files[idx];

    /* Track language if not already set */
    if (lang && lang[0] && !of->lang[0]) {
        strncpy(of->lang, lang, MAX_LANG - 1);
        of->lang[MAX_LANG - 1] = '\0';
    }

    /* Already opened (or simulated open in dry-run) */
    if (of->opened) {
        return of->fp;  /* May be NULL in dry-run mode */
    }
    of->opened = true;

    /* Check if file exists before we would write */
    of->existed = file_exists(of->path);

    /* Dry-run mode: simulate what would happen */
    if (options->dry_run) {
        /* Track stats */
        if (of->existed) {
            stats->files_overwritten++;
        } else {
            stats->files_created++;
        }

        /* Would backup be created? */
        if (options->backup && of->existed) {
            /* Generate backup path for display */
            time_t now = time(NULL);
            struct tm *tm_info = localtime(&now);
            char timestamp[20];
            strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", tm_info);
            snprintf(of->backup_path, sizeof(of->backup_path), "%s.%s.bak", of->path, timestamp);
            of->backed_up = true;
            stats->backups_created++;

            if (options->verbose) {
                printf("  [dry-run] would backup: %s -> %s\n", of->path, of->backup_path);
            }
        }

        if (options->verbose) {
            if (of->existed) {
                printf("  [dry-run] would overwrite: %s\n", of->path);
            } else {
                printf("  [dry-run] would create: %s\n", of->path);
            }
        }

        /* Mark as "opened" by incrementing block_count later */
        return NULL;  /* No actual file in dry-run */
    }

    /* Normal mode: actually create files */

    /* Ensure parent directory exists */
    char dir[MAX_PATH];
    if (get_directory(of->path, dir, sizeof(dir)) && dir[0]) {
        if (!ensure_directory(dir)) {
            return NULL;
        }
    }

    /* Create backup if enabled and file exists */
    if (options->backup && of->existed) {
        if (!create_backup(of->path, of->backup_path, sizeof(of->backup_path), options, stats)) {
            return NULL;
        }
        of->backed_up = true;
    }

    of->fp = fopen(of->path, "w");
    if (!of->fp) {
        fprintf(stderr, "error: cannot create '%s': %s\n", of->path, strerror(errno));
        return NULL;
    }

    /* Track stats */
    if (of->existed) {
        stats->files_overwritten++;
    } else {
        stats->files_created++;
    }

    if (options->verbose) {
        if (of->existed) {
            printf("  overwriting: %s\n", of->path);
        } else {
            printf("  creating: %s\n", of->path);
        }
    }

    return of->fp;
}

/* Close all output files */
void close_all_outputs(OutputState *state)
{
    for (int i = 0; i < state->count; i++) {
        if (state->files[i].fp) {
            fclose(state->files[i].fp);
            state->files[i].fp = NULL;
        }
    }
}

/* Print warnings about output state
 * Returns true if there were warnings, false otherwise
 */
bool print_warnings(OutputState *state, RyftOptions *options)
{
    bool had_warnings = false;

    /* Warn about mixed named/unnamed blocks with multiple files */
    if (state->multiple_files && state->has_unnamed_blocks) {
        had_warnings = true;
        if (options->strict_mode) {
            fprintf(stderr, "\nerror: multiple output files with some unnamed blocks (strict mode)\n");
        } else {
            fprintf(stderr, "\nwarning: multiple output files with some unnamed blocks\n");
        }
        fprintf(stderr, "  For clarity, specify filename in each code fence.\n");
        fprintf(stderr, "  Example: ```c filename.c instead of just ```c\n");

        for (int i = 0; i < state->count; i++) {
            if (state->files[i].unnamed_block_count > 0) {
                fprintf(stderr, "  %s: %d unnamed block(s)\n",
                        state->files[i].path,
                        state->files[i].unnamed_block_count);
            }
        }
    }

    return had_warnings;
}

/* Print detailed summary */
void print_summary(OutputState *state, RyftOptions *options, RyftStats *stats)
{
    printf("\n");
    if (options->dry_run) {
        printf("=== Summary (dry-run) ===\n");
    } else {
        printf("=== Summary ===\n");
    }
    printf("Blocks found:     %d\n", stats->total_blocks);
    printf("  Extracted:      %d\n", stats->extracted_blocks);
    printf("  Display only:   %d (4+ backticks)\n", stats->display_blocks);
    printf("  Config:         %d (ryft.config)\n", stats->config_blocks);
    printf("\n");
    printf("Files%s:\n", options->dry_run ? " (would be written)" : "");

    for (int i = 0; i < state->count; i++) {
        OutputFile *of = &state->files[i];
        printf("  %s\n", of->path);
        printf("    Blocks:   %d\n", of->block_count);
        if (of->lang[0]) {
            printf("    Language: %s\n", of->lang);
        }
        if (options->dry_run) {
            if (of->existed) {
                printf("    Status:   would overwrite\n");
            } else {
                printf("    Status:   would create (new)\n");
            }
            if (of->backed_up) {
                printf("    Backup:   would create %s\n", of->backup_path);
            }
        } else {
            if (of->existed) {
                printf("    Status:   overwritten\n");
            } else {
                printf("    Status:   created (new)\n");
            }
            if (of->backed_up) {
                printf("    Backup:   %s\n", of->backup_path);
            }
        }
    }

    printf("\n");
    printf("Totals%s:\n", options->dry_run ? " (would be)" : "");
    printf("  New files:      %d\n", stats->files_created);
    printf("  Overwritten:    %d\n", stats->files_overwritten);
    if (stats->backups_created > 0) {
        printf("  Backups:        %d\n", stats->backups_created);
    }
}
