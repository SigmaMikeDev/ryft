/*
 * process.c - Main processing logic
 */

#include "process.h"
#include "config.h"
#include "markdown.h"
#include "output.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

/* Default options */
RyftOptions g_options = {
    .backup = false,
    .backup_timestamp = true,
    .backup_limit = 10,
    .dry_run = false,
    .verbose = false,
    .summary = false,
    .strict_mode = false,
};

/* Global stats */
RyftStats g_stats = {0};

/* Process a markdown file, extract code blocks to files */
int process_file(const char *filepath, RyftOptions *cli_options)
{
    FILE *f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "error: cannot open '%s'\n", filepath);
        return 1;
    }

    OutputState state = {0};
    state.current = -1;

    /* Document-level config */
    RyftConfig doc_config = {0};

    /* Reset stats */
    memset(&g_stats, 0, sizeof(g_stats));

    /* Get default output basename from input file */
    char default_basename[MAX_FILENAME];
    get_basename_no_ext(filepath, default_basename, sizeof(default_basename));

    char line[MAX_LINE];
    bool in_block = false;
    FenceInfo current = {0};

    if (g_options.verbose) {
        printf("processing: %s\n", filepath);
    }

    while (fgets(line, sizeof(line), f)) {
        if (!in_block) {
            /* Check for opening fence */
            if (count_backticks(line) >= 3) {
                current = parse_fence(line);
                in_block = true;
                g_stats.total_blocks++;

                /* Handle display blocks */
                if (current.is_display) {
                    g_stats.display_blocks++;
                    if (g_options.verbose) {
                        printf("  [block %d] display-only (skipped)\n", g_stats.total_blocks);
                    }
                    continue;
                }

                /* Handle config blocks */
                if (current.is_config) {
                    g_stats.config_blocks++;
                    if (g_options.verbose) {
                        printf("  [block %d] ryft.config\n", g_stats.total_blocks);
                    }
                    continue;
                }

                /* Track default language from first code block */
                if (!state.default_lang[0] && current.lang[0]) {
                    strncpy(state.default_lang, current.lang, MAX_LANG - 1);
                }

                /* Determine output target */
                if (current.filename[0]) {
                    /* Explicit filename - switch to this target */
                    int idx = get_output_file(&state, current.filename);
                    if (idx >= 0) {
                        state.current = idx;
                        state.has_named_blocks = true;
                        if (g_options.verbose) {
                            printf("  [block %d] lang=%s -> %s\n",
                                   g_stats.total_blocks,
                                   current.lang[0] ? current.lang : "(none)",
                                   current.filename);
                        }
                    }
                } else if (state.current < 0) {
                    /* No current target, create fallback */
                    char filename[MAX_FILENAME];
                    char fallback[MAX_PATH];
                    const char *ext = lang_to_ext(current.lang);

                    /* Build filename from basename + extension */
                    if (current.lang[0]) {
                        snprintf(filename, sizeof(filename), "%s%s", default_basename, ext);
                    } else {
                        snprintf(filename, sizeof(filename), "%s", default_basename);
                    }

                    /* Apply config output path if set, or use config filename */
                    bool using_config_filename = false;
                    if (doc_config.filename[0]) {
                        strncpy(fallback, doc_config.filename, sizeof(fallback) - 1);
                        fallback[sizeof(fallback) - 1] = '\0';
                        using_config_filename = true;
                    } else {
                        build_output_path(doc_config.output, filename, fallback, sizeof(fallback));
                    }

                    /* Warn or error about fallback (but not if filename came from config) */
                    if (!using_config_filename) {
                        if (current.lang[0]) {
                            if (g_options.strict_mode) {
                                fprintf(stderr, "error: no output filename specified (strict mode)\n");
                                fclose(f);
                                close_all_outputs(&state);
                                return 1;
                            }
                            fprintf(stderr, "warning: no output filename specified, assuming '%s' from ```%s\n",
                                    fallback, current.lang);
                        } else {
                            if (g_options.strict_mode) {
                                fprintf(stderr, "error: no language specified (strict mode)\n");
                                fclose(f);
                                close_all_outputs(&state);
                                return 1;
                            }
                            fprintf(stderr, "warning: no language specified, outputting as plaintext '%s'\n",
                                    fallback);
                        }
                    }

                    int idx = get_output_file(&state, fallback);
                    if (idx >= 0) {
                        state.current = idx;
                        if (g_options.verbose) {
                            printf("  [block %d] lang=%s -> %s (fallback)\n",
                                   g_stats.total_blocks,
                                   current.lang[0] ? current.lang : "(none)",
                                   fallback);
                        }
                    }
                } else {
                    /* Continuation block - append to current */
                    state.has_unnamed_blocks = true;
                    if (state.current >= 0) {
                        state.files[state.current].unnamed_block_count++;
                        if (g_options.verbose) {
                            printf("  [block %d] lang=%s -> %s (continuation)\n",
                                   g_stats.total_blocks,
                                   current.lang[0] ? current.lang : "(none)",
                                   state.files[state.current].path);
                        }
                    }
                }
            }
        } else {
            /* Check for closing fence */
            int closing_backticks = get_closing_fence_backticks(line, current.backtick_count);
            if (closing_backticks > 0) {
                in_block = false;

                /* Apply config after parsing config block */
                if (current.is_config) {
                    apply_config(&doc_config, cli_options, &g_options);
                }

                /* Handle extracted blocks */
                if (!current.is_display && !current.is_config && state.current >= 0) {
                    OutputFile *of = &state.files[state.current];
                    of->block_count++;
                    g_stats.extracted_blocks++;

                    /* Add blank line after block if closing fence has 4+ backticks */
                    if (closing_backticks >= 4 && of->fp) {
                        fputs("\n", of->fp);
                    }
                }

                current = (FenceInfo){0};
            } else {
                /* Parse config block content */
                if (current.is_config) {
                    parse_config_line(line, &doc_config, &g_options);
                }
                /* Output regular block content */
                else if (!current.is_display && state.current >= 0) {
                    FILE *out = open_output(&state, state.current, current.lang,
                                            &g_options, &g_stats);
                    if (out) {
                        fputs(line, out);
                    }
                }
            }
        }
    }

    if (in_block) {
        if (g_options.strict_mode) {
            fprintf(stderr, "error: unclosed code block at end of file (strict mode)\n");
            fclose(f);
            close_all_outputs(&state);
            return 1;
        }
        fprintf(stderr, "warning: unclosed code block at end of file\n");
    }

    fclose(f);
    close_all_outputs(&state);

    /* Use config output path if set and no explicit filenames were given */
    (void)doc_config;  /* Mark as used - output path handling comes later */

    /* Print summary or brief output */
    if (g_options.summary || g_options.verbose) {
        print_summary(&state, &g_options, &g_stats);
    } else {
        /* Brief output */
        if (g_options.dry_run) {
            printf("[dry-run] would extract %d block(s) to %d file(s)\n",
                   g_stats.extracted_blocks, state.count);
        } else {
            printf("extracted %d block(s) to %d file(s)\n",
                   g_stats.extracted_blocks, state.count);
        }
    }

    bool had_warnings = print_warnings(&state, &g_options);
    if (had_warnings && g_options.strict_mode) {
        return 1;
    }

    return 0;
}
