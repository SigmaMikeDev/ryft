/*
 * ryft.h - Public API header for ryft
 *
 * This header exposes key functions for external use,
 * enabling future vim/neovim plugin development.
 */

#ifndef RYFT_API_H
#define RYFT_API_H

#include <stdbool.h>

/* Runtime options for processing */
typedef struct {
    bool backup;               /* create backup before overwriting */
    bool backup_timestamp;     /* use timestamp in backup name (vs sequential) */
    int  backup_limit;         /* max backups to keep (0=unlimited) */
    bool dry_run;              /* don't write files, just show what would happen */
    bool verbose;              /* extra output during processing */
    bool summary;              /* print summary at end */
    bool strict_mode;          /* fail on warnings instead of continuing */
} RyftOptions;

/* Process a markdown file, extracting code blocks to output files.
 *
 * filepath: Path to the markdown file to process
 * opts: Runtime options (may be NULL for defaults)
 *
 * Returns 0 on success, non-zero on error.
 */
int ryft_process_file(const char *filepath, RyftOptions *opts);

#endif /* RYFT_API_H */
