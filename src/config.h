/*
 * config.h - Config parsing and management
 */

#ifndef RYFT_CONFIG_H
#define RYFT_CONFIG_H

#include "types.h"
#include <stdbool.h>
#include <stddef.h>

/* Parse a single config line: key = value */
bool parse_config_line(const char *line, RyftConfig *config, RyftOptions *options);

/* Apply document config to global options */
void apply_config(RyftConfig *config, RyftOptions *cli_options, RyftOptions *g_options);

/* Load config from a file */
bool load_config_file(const char *path, RyftConfig *config, RyftOptions *options, bool verbose);

/* Get path to global config file */
void get_global_config_path(char *out, size_t out_size);

#endif /* RYFT_CONFIG_H */
