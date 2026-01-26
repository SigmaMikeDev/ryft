/*
 * util.c - String and path utilities
 */

#include "util.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

/* Language to extension mapping */
const char *lang_to_ext(const char *lang)
{
    if (!lang || !*lang) return "";

    /* Common mappings */
    if (strcmp(lang, "c") == 0) return ".c";
    if (strcmp(lang, "h") == 0) return ".h";
    if (strcmp(lang, "cpp") == 0 || strcmp(lang, "c++") == 0) return ".cpp";
    if (strcmp(lang, "hpp") == 0) return ".hpp";
    if (strcmp(lang, "python") == 0 || strcmp(lang, "py") == 0) return ".py";
    if (strcmp(lang, "javascript") == 0 || strcmp(lang, "js") == 0) return ".js";
    if (strcmp(lang, "typescript") == 0 || strcmp(lang, "ts") == 0) return ".ts";
    if (strcmp(lang, "rust") == 0 || strcmp(lang, "rs") == 0) return ".rs";
    if (strcmp(lang, "go") == 0) return ".go";
    if (strcmp(lang, "java") == 0) return ".java";
    if (strcmp(lang, "ruby") == 0 || strcmp(lang, "rb") == 0) return ".rb";
    if (strcmp(lang, "shell") == 0 || strcmp(lang, "sh") == 0 || strcmp(lang, "bash") == 0) return ".sh";
    if (strcmp(lang, "zsh") == 0) return ".zsh";
    if (strcmp(lang, "fish") == 0) return ".fish";
    if (strcmp(lang, "lua") == 0) return ".lua";
    if (strcmp(lang, "perl") == 0 || strcmp(lang, "pl") == 0) return ".pl";
    if (strcmp(lang, "php") == 0) return ".php";
    if (strcmp(lang, "html") == 0) return ".html";
    if (strcmp(lang, "css") == 0) return ".css";
    if (strcmp(lang, "json") == 0) return ".json";
    if (strcmp(lang, "yaml") == 0 || strcmp(lang, "yml") == 0) return ".yaml";
    if (strcmp(lang, "toml") == 0) return ".toml";
    if (strcmp(lang, "xml") == 0) return ".xml";
    if (strcmp(lang, "sql") == 0) return ".sql";
    if (strcmp(lang, "markdown") == 0 || strcmp(lang, "md") == 0) return ".md";
    if (strcmp(lang, "make") == 0 || strcmp(lang, "makefile") == 0) return ".mk";
    if (strcmp(lang, "dockerfile") == 0) return ".dockerfile";
    if (strcmp(lang, "vim") == 0) return ".vim";
    if (strcmp(lang, "elisp") == 0 || strcmp(lang, "emacs-lisp") == 0) return ".el";
    if (strcmp(lang, "lisp") == 0) return ".lisp";
    if (strcmp(lang, "scheme") == 0) return ".scm";
    if (strcmp(lang, "haskell") == 0 || strcmp(lang, "hs") == 0) return ".hs";
    if (strcmp(lang, "ocaml") == 0 || strcmp(lang, "ml") == 0) return ".ml";
    if (strcmp(lang, "swift") == 0) return ".swift";
    if (strcmp(lang, "kotlin") == 0 || strcmp(lang, "kt") == 0) return ".kt";
    if (strcmp(lang, "scala") == 0) return ".scala";
    if (strcmp(lang, "r") == 0) return ".r";
    if (strcmp(lang, "julia") == 0 || strcmp(lang, "jl") == 0) return ".jl";
    if (strcmp(lang, "zig") == 0) return ".zig";
    if (strcmp(lang, "nim") == 0) return ".nim";
    if (strcmp(lang, "ini") == 0) return ".ini";
    if (strcmp(lang, "conf") == 0 || strcmp(lang, "config") == 0) return ".conf";

    /* Unknown language - return as extension */
    static char ext[MAX_LANG + 1];
    snprintf(ext, sizeof(ext), ".%s", lang);
    return ext;
}

/* Strip leading/trailing whitespace in place */
char *str_trim(char *str)
{
    if (!str) return NULL;

    /* Leading whitespace */
    while (*str == ' ' || *str == '\t') str++;

    /* Trailing whitespace */
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end-- = '\0';
    }

    return str;
}

/* Remove surrounding quotes from string */
char *str_unquote(char *str)
{
    if (!str || !*str) return str;

    size_t len = strlen(str);
    if (len >= 2) {
        /* Check for matching quotes */
        if ((str[0] == '"' && str[len-1] == '"') ||
            (str[0] == '\'' && str[len-1] == '\'')) {
            str[len-1] = '\0';
            return str + 1;
        }
    }
    return str;
}

/* Parse boolean value: on/off, true/false, yes/no, 1/0 */
bool parse_bool(const char *value, bool *result)
{
    if (!value || !result) return false;

    if (strcmp(value, "on") == 0 || strcmp(value, "true") == 0 ||
        strcmp(value, "yes") == 0 || strcmp(value, "1") == 0) {
        *result = true;
        return true;
    }
    if (strcmp(value, "off") == 0 || strcmp(value, "false") == 0 ||
        strcmp(value, "no") == 0 || strcmp(value, "0") == 0) {
        *result = false;
        return true;
    }
    return false;
}

/* Expand ~ to home directory */
bool expand_path(const char *path, char *out, size_t out_size)
{
    if (!path || !out || out_size == 0) {
        return false;
    }

    /* Handle ~ expansion */
    if (path[0] == '~') {
        const char *home = getenv("HOME");
        if (!home) {
            fprintf(stderr, "warning: HOME environment variable not set\n");
            strncpy(out, path, out_size - 1);
            out[out_size - 1] = '\0';
            return true;
        }

        if (path[1] == '\0') {
            /* Just ~ */
            strncpy(out, home, out_size - 1);
            out[out_size - 1] = '\0';
        } else if (path[1] == '/') {
            /* ~/something */
            snprintf(out, out_size, "%s%s", home, path + 1);
        } else {
            /* ~username - not supported, copy as-is */
            fprintf(stderr, "warning: ~username expansion not supported: %s\n", path);
            strncpy(out, path, out_size - 1);
            out[out_size - 1] = '\0';
        }
    } else {
        /* No expansion needed */
        strncpy(out, path, out_size - 1);
        out[out_size - 1] = '\0';
    }

    return true;
}

/* Check if path looks like a directory (ends with / or is an existing directory) */
bool is_directory_path(const char *path)
{
    if (!path || !*path) return false;

    size_t len = strlen(path);
    if (path[len - 1] == '/') return true;

    /* Check if it's an existing directory */
    struct stat st;
    char expanded[MAX_PATH];
    expand_path(path, expanded, sizeof(expanded));
    if (stat(expanded, &st) == 0 && S_ISDIR(st.st_mode)) {
        return true;
    }

    return false;
}

/* Extract basename without extension from path */
void get_basename_no_ext(const char *path, char *out, size_t out_size)
{
    const char *base = strrchr(path, '/');
    base = base ? base + 1 : path;

    strncpy(out, base, out_size - 1);
    out[out_size - 1] = '\0';

    /* Remove .md extension if present */
    size_t len = strlen(out);
    if (len > 3 && strcmp(out + len - 3, ".md") == 0) {
        out[len - 3] = '\0';
    }
}

/* Build output path from config output setting and filename
 * - If config_output is empty, use filename as-is (current directory)
 * - If config_output is a directory (ends with / or exists as dir), append filename
 * - Otherwise, use config_output as the full path (for single-file output)
 */
void build_output_path(const char *config_output, const char *filename,
                       char *out, size_t out_size)
{
    if (!config_output || !config_output[0]) {
        /* No config output - use filename in current directory */
        strncpy(out, filename, out_size - 1);
        out[out_size - 1] = '\0';
        return;
    }

    if (is_directory_path(config_output)) {
        /* Config output is a directory - append filename */
        size_t len = strlen(config_output);
        if (config_output[len - 1] == '/') {
            snprintf(out, out_size, "%s%s", config_output, filename);
        } else {
            snprintf(out, out_size, "%s/%s", config_output, filename);
        }
    } else {
        /* Config output is a file path - use as-is */
        strncpy(out, config_output, out_size - 1);
        out[out_size - 1] = '\0';
    }
}

/* Get directory portion of a path */
bool get_directory(const char *path, char *out, size_t out_size)
{
    const char *last_slash = strrchr(path, '/');

    if (!last_slash || last_slash == path) {
        /* No directory component or root */
        out[0] = '\0';
        return true;
    }

    size_t dir_len = last_slash - path;
    if (dir_len >= out_size) {
        dir_len = out_size - 1;
    }

    strncpy(out, path, dir_len);
    out[dir_len] = '\0';

    return true;
}

/* Check if file exists */
bool file_exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

/* Create directory and all parent directories (like mkdir -p) */
bool ensure_directory(const char *path)
{
    char tmp[MAX_PATH];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);

    /* Remove trailing slash */
    if (len > 0 && tmp[len - 1] == '/') {
        tmp[len - 1] = '\0';
    }

    /* Walk through path creating directories */
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';

            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                fprintf(stderr, "error: cannot create directory '%s': %s\n",
                        tmp, strerror(errno));
                return false;
            }

            *p = '/';
        }
    }

    /* Create final directory */
    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "error: cannot create directory '%s': %s\n",
                tmp, strerror(errno));
        return false;
    }

    return true;
}
