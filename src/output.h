/*
 * output.h - Output file management and backups
 */

#ifndef RYFT_OUTPUT_H
#define RYFT_OUTPUT_H

#include "types.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/* Create backup of existing file with timestamp */
bool create_backup(const char *path, char *output_backup, size_t backup_size,
                   RyftOptions *options, RyftStats *stats);

/* Find or create output file entry */
int get_output_file(OutputState *state, const char *path);

/* Open output file for writing (or simulate in dry-run mode) */
FILE *open_output(OutputState *state, int idx, const char *lang,
                  RyftOptions *options, RyftStats *stats);

/* Close all output files */
void close_all_outputs(OutputState *state);

/* Print warnings about output state
 * Returns true if there were warnings, false otherwise
 */
bool print_warnings(OutputState *state, RyftOptions *options);

/* Print detailed summary */
void print_summary(OutputState *state, RyftOptions *options, RyftStats *stats);

#endif /* RYFT_OUTPUT_H */
