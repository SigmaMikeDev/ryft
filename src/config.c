/*
 * config.c - Config parsing and management
 */

#include "config.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Parse a single config line: key = value */
bool parse_config_line(const char *line, RyftConfig *config, RyftOptions *options)
{
    char buf[MAX_LINE];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    /* Skip comments */
    char *comment = strchr(buf, '#');
    if (comment) *comment = '\0';

    /* Find = separator */
    char *eq = strchr(buf, '=');
    if (!eq) return true;  /* No = sign, skip line */

    *eq = '\0';
    char *key = str_trim(buf);
    char *value = str_trim(eq + 1);
    value = str_unquote(value);

    if (!*key) return true;  /* Empty key, skip */

    /* Parse known keys */
    if (strcmp(key, "output") == 0) {
        strncpy(config->output, value, MAX_PATH - 1);
        config->output[MAX_PATH - 1] = '\0';
        if (options->verbose) {
            printf("  config: output = %s\n", config->output);
        }
    } else if (strcmp(key, "lang") == 0 || strcmp(key, "language") == 0) {
        strncpy(config->lang, value, MAX_LANG - 1);
        config->lang[MAX_LANG - 1] = '\0';
        if (options->verbose) {
            printf("  config: lang = %s\n", config->lang);
        }
    } else if (strcmp(key, "backup") == 0) {
        if (parse_bool(value, &config->backup)) {
            config->backup_set = true;
            if (options->verbose) {
                printf("  config: backup = %s\n", config->backup ? "on" : "off");
            }
        } else {
            fprintf(stderr, "warning: invalid boolean value for 'backup': %s\n", value);
        }
    } else if (strcmp(key, "verbose") == 0) {
        if (parse_bool(value, &config->verbose)) {
            config->verbose_set = true;
            if (options->verbose) {
                printf("  config: verbose = %s\n", config->verbose ? "on" : "off");
            }
        } else {
            fprintf(stderr, "warning: invalid boolean value for 'verbose': %s\n", value);
        }
    } else if (strcmp(key, "summary") == 0) {
        if (parse_bool(value, &config->summary)) {
            config->summary_set = true;
            if (options->verbose) {
                printf("  config: summary = %s\n", config->summary ? "on" : "off");
            }
        } else {
            fprintf(stderr, "warning: invalid boolean value for 'summary': %s\n", value);
        }
    } else if (strcmp(key, "strict_mode") == 0) {
        if (parse_bool(value, &config->strict_mode)) {
            config->strict_mode_set = true;
            if (options->verbose) {
                printf("  config: strict_mode = %s\n", config->strict_mode ? "on" : "off");
            }
        } else {
            fprintf(stderr, "warning: invalid boolean value for 'strict_mode': %s\n", value);
        }
    } else if (strcmp(key, "backup_timestamp") == 0) {
        if (parse_bool(value, &config->backup_timestamp)) {
            config->backup_timestamp_set = true;
            if (options->verbose) {
                printf("  config: backup_timestamp = %s\n", config->backup_timestamp ? "on" : "off");
            }
        } else {
            fprintf(stderr, "warning: invalid boolean value for 'backup_timestamp': %s\n", value);
        }
    } else if (strcmp(key, "backup_limit") == 0) {
        int limit = atoi(value);
        if (limit < 0) limit = 10;  /* Default for negative values */
        config->backup_limit = limit;
        config->backup_limit_set = true;
        if (options->verbose) {
            printf("  config: backup_limit = %d\n", config->backup_limit);
        }
    } else if (strcmp(key, "filename") == 0) {
        strncpy(config->filename, value, MAX_PATH - 1);
        config->filename[MAX_PATH - 1] = '\0';
        if (options->verbose) {
            printf("  config: filename = %s\n", config->filename);
        }
    } else if (strcmp(key, "version") == 0) {
        strncpy(config->version, value, sizeof(config->version) - 1);
        config->version[sizeof(config->version) - 1] = '\0';
        if (options->verbose) {
            printf("  config: version = %s\n", config->version);
        }
    } else {
        if (options->verbose) {
            printf("  config: unknown key '%s' (ignored)\n", key);
        }
    }

    return true;
}

/* Apply document config to global options (config overrides defaults, CLI overrides config) */
void apply_config(RyftConfig *config, RyftOptions *cli_options, RyftOptions *g_options)
{
    /* Only apply config values if they were set and CLI didn't override */
    if (config->backup_set && !cli_options->backup) {
        g_options->backup = config->backup;
    }
    if (config->backup_timestamp_set) {
        g_options->backup_timestamp = config->backup_timestamp;
    }
    if (config->backup_limit_set) {
        g_options->backup_limit = config->backup_limit;
    }
    if (config->verbose_set && !cli_options->verbose) {
        g_options->verbose = config->verbose;
    }
    if (config->summary_set && !cli_options->summary) {
        g_options->summary = config->summary;
    }
    if (config->strict_mode_set && !cli_options->strict_mode) {
        g_options->strict_mode = config->strict_mode;
    }
    /* If verbose is on, summary is also on */
    if (g_options->verbose) {
        g_options->summary = true;
    }
}

/* Load config from a file */
bool load_config_file(const char *path, RyftConfig *config, RyftOptions *options, bool verbose)
{
    char expanded[MAX_PATH];
    if (!expand_path(path, expanded, sizeof(expanded))) {
        return false;
    }

    FILE *f = fopen(expanded, "r");
    if (!f) {
        /* File doesn't exist is not an error for global config */
        return false;
    }

    if (verbose) {
        printf("loading config: %s\n", expanded);
    }

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), f)) {
        /* Temporarily enable/disable verbose based on parameter */
        bool saved_verbose = options->verbose;
        options->verbose = verbose;
        parse_config_line(line, config, options);
        options->verbose = saved_verbose;
    }

    fclose(f);
    return true;
}

/* Get path to global config file */
void get_global_config_path(char *out, size_t out_size)
{
    const char *home = getenv("HOME");
    if (home) {
        snprintf(out, out_size, "%s/.config/ryft/config", home);
    } else {
        out[0] = '\0';
    }
}
