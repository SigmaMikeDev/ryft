/*
 * ryft.h - Public API header for ryft
 *
 * This header exposes key functions for external use,
 * enabling future vim/neovim plugin development.
 */

#ifndef RYFT_API_H
#define RYFT_API_H

#include "../src/types.h"

/* Process a markdown file, extracting code blocks to output files.
 *
 * filepath: Path to the markdown file to process
 * opts: Runtime options (may be NULL for defaults)
 *
 * Returns 0 on success, non-zero on error.
 */
int ryft_process_file(const char *filepath, RyftOptions *opts);

#endif /* RYFT_API_H */
