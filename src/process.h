/*
 * process.h - Main processing logic
 */

#ifndef RYFT_PROCESS_H
#define RYFT_PROCESS_H

#include "types.h"

/* Global options - defined in process.c */
extern RyftOptions g_options;

/* Global stats - defined in process.c */
extern RyftStats g_stats;

/* Process a markdown file, extract code blocks to files */
int process_file(const char *filepath, RyftOptions *cli_options);

#endif /* RYFT_PROCESS_H */
