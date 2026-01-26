/*
 * main.c - CLI entry point for ryft
 *
 * Literate programming tool that tangles markdown code fences
 * into source/config files.
 */

#include "types.h"
#include "config.h"
#include "process.h"

#include <stdio.h>
#include <string.h>

#ifndef RYFT_VERSION
#define RYFT_VERSION "dev"
#endif

static void usage(const char *prog)
{
    fprintf(stderr, "usage: %s [options] <markdown-file>\n", prog);
    fprintf(stderr, "\nExtracts code blocks from markdown files.\n");
    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, "  -b, --backup     Create timestamped backup before overwriting\n");
    fprintf(stderr, "  -n, --dry-run    Show what would be done without writing files\n");
    fprintf(stderr, "  -s, --summary    Print detailed summary after processing\n");
    fprintf(stderr, "  -S, --strict     Strict mode: fail on warnings\n");
    fprintf(stderr, "  -v, --verbose    Verbose output (includes summary)\n");
    fprintf(stderr, "  -V, --version    Show version information\n");
    fprintf(stderr, "  -h, --help       Show this help message\n");
}

int main(int argc, char *argv[])
{
    const char *input_file = NULL;
    RyftOptions cli_options = {0};  /* Track what CLI explicitly set */

    /* Parse arguments first so we know if verbose is set */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--backup") == 0) {
            g_options.backup = true;
            cli_options.backup = true;
        } else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--dry-run") == 0) {
            g_options.dry_run = true;
            cli_options.dry_run = true;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--summary") == 0) {
            g_options.summary = true;
            cli_options.summary = true;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            g_options.verbose = true;
            cli_options.verbose = true;
        } else if (strcmp(argv[i], "-S") == 0 || strcmp(argv[i], "--strict") == 0) {
            g_options.strict_mode = true;
            cli_options.strict_mode = true;
        } else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("ryft %s\n", RYFT_VERSION);
            return 0;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            return 0;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "error: unknown option '%s'\n", argv[i]);
            usage(argv[0]);
            return 1;
        } else {
            if (input_file) {
                fprintf(stderr, "error: multiple input files not supported\n");
                return 1;
            }
            input_file = argv[i];
        }
    }

    if (!input_file) {
        usage(argv[0]);
        return 1;
    }

    /* Load global config from ~/.config/ryft/config */
    char global_config_path[MAX_PATH];
    get_global_config_path(global_config_path, sizeof(global_config_path));

    if (global_config_path[0]) {
        RyftConfig global_config = {0};
        if (load_config_file(global_config_path, &global_config, &g_options, g_options.verbose)) {
            apply_config(&global_config, &cli_options, &g_options);
        }
    }

    return process_file(input_file, &cli_options);
}
