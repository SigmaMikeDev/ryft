/*
 * ryft - Extract code blocks from markdown files
 *
 * Literate programming tool that tangles markdown code fences
 * into source/config files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE 4096
#define MAX_LANG 64
#define MAX_FILENAME 256
#define MAX_PATH 1024
#define MAX_OUTPUT_FILES 64

typedef struct {
    char lang[MAX_LANG];
    char filename[MAX_FILENAME];
    bool is_config;      /* ryft.config block */
    bool is_display;     /* 4+ backticks, skip extraction */
    int backtick_count;
} FenceInfo;

typedef struct {
    char path[MAX_PATH];
    FILE *fp;
    int block_count;           /* blocks written to this file */
    int unnamed_block_count;   /* blocks without explicit filename */
} OutputFile;

typedef struct {
    OutputFile files[MAX_OUTPUT_FILES];
    int count;
    int current;               /* index of current output target, -1 if none */
    char default_lang[MAX_LANG];
    bool has_named_blocks;
    bool has_unnamed_blocks;
    bool multiple_files;
} OutputState;

/* Language to extension mapping */
static const char *lang_to_ext(const char *lang)
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

/* Extract basename without extension from path */
static void get_basename_no_ext(const char *path, char *out, size_t out_size)
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

/* Count leading backticks in a line */
static int count_backticks(const char *line)
{
    int count = 0;
    while (*line == '`') {
        count++;
        line++;
    }
    return count;
}

/* Parse opening fence line: ```lang filename or ````lang etc */
static FenceInfo parse_fence(const char *line)
{
    FenceInfo info = {0};

    info.backtick_count = count_backticks(line);
    if (info.backtick_count < 3) {
        return info;
    }

    /* Skip past backticks */
    const char *p = line + info.backtick_count;

    /* Skip whitespace */
    while (*p == ' ' || *p == '\t') p++;

    /* Check for 4+ backticks = display only */
    if (info.backtick_count >= 4) {
        info.is_display = true;
    }

    /* Parse language */
    int i = 0;
    while (*p && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r' && i < MAX_LANG - 1) {
        info.lang[i++] = *p++;
    }
    info.lang[i] = '\0';

    /* Check for ryft.config */
    if (strcmp(info.lang, "ryft.config") == 0) {
        info.is_config = true;
        return info;
    }

    /* Skip whitespace before filename */
    while (*p == ' ' || *p == '\t') p++;

    /* Parse optional filename */
    i = 0;
    while (*p && *p != '\n' && *p != '\r' && i < MAX_FILENAME - 1) {
        info.filename[i++] = *p++;
    }
    info.filename[i] = '\0';

    /* Trim trailing whitespace from filename */
    while (i > 0 && (info.filename[i-1] == ' ' || info.filename[i-1] == '\t')) {
        info.filename[--i] = '\0';
    }

    return info;
}

/* Check if line is a closing fence matching the opening */
static bool is_closing_fence(const char *line, int open_backticks)
{
    int count = count_backticks(line);
    if (count < open_backticks) {
        return false;
    }

    /* Rest of line should be whitespace only */
    const char *p = line + count;
    while (*p) {
        if (*p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') {
            return false;
        }
        p++;
    }
    return true;
}

/* Find or create output file entry */
static int get_output_file(OutputState *state, const char *path)
{
    /* Check if already exists */
    for (int i = 0; i < state->count; i++) {
        if (strcmp(state->files[i].path, path) == 0) {
            return i;
        }
    }

    /* Create new entry */
    if (state->count >= MAX_OUTPUT_FILES) {
        fprintf(stderr, "error: too many output files (max %d)\n", MAX_OUTPUT_FILES);
        return -1;
    }

    int idx = state->count++;
    strncpy(state->files[idx].path, path, MAX_PATH - 1);
    state->files[idx].path[MAX_PATH - 1] = '\0';
    state->files[idx].fp = NULL;
    state->files[idx].block_count = 0;
    state->files[idx].unnamed_block_count = 0;

    if (state->count > 1) {
        state->multiple_files = true;
    }

    return idx;
}

/* Open output file for appending */
static FILE *open_output(OutputState *state, int idx)
{
    if (idx < 0 || idx >= state->count) {
        return NULL;
    }

    OutputFile *of = &state->files[idx];

    if (!of->fp) {
        of->fp = fopen(of->path, "w");
        if (!of->fp) {
            fprintf(stderr, "error: cannot create '%s'\n", of->path);
            return NULL;
        }
        printf("  creating: %s\n", of->path);
    }

    return of->fp;
}

/* Close all output files */
static void close_all_outputs(OutputState *state)
{
    for (int i = 0; i < state->count; i++) {
        if (state->files[i].fp) {
            fclose(state->files[i].fp);
            state->files[i].fp = NULL;
        }
    }
}

/* Print warnings about output state */
static void print_warnings(OutputState *state)
{
    /* Warn about mixed named/unnamed blocks with multiple files */
    if (state->multiple_files && state->has_unnamed_blocks) {
        fprintf(stderr, "\nwarning: multiple output files with some unnamed blocks\n");
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
}

/* Process a markdown file, extract code blocks to files */
static int process_file(const char *filepath)
{
    FILE *f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "error: cannot open '%s'\n", filepath);
        return 1;
    }

    OutputState state = {0};
    state.current = -1;

    /* Get default output basename from input file */
    char default_basename[MAX_FILENAME];
    get_basename_no_ext(filepath, default_basename, sizeof(default_basename));

    char line[MAX_LINE];
    bool in_block = false;
    FenceInfo current = {0};
    int block_num = 0;
    int extracted = 0;
    bool used_fallback = false;

    printf("processing: %s\n", filepath);

    while (fgets(line, sizeof(line), f)) {
        if (!in_block) {
            /* Check for opening fence */
            if (count_backticks(line) >= 3) {
                current = parse_fence(line);
                in_block = true;
                block_num++;

                /* Skip display blocks and config blocks */
                if (current.is_display || current.is_config) {
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
                    }
                } else if (state.current < 0) {
                    /* No current target, create fallback */
                    char fallback[MAX_PATH];
                    const char *ext = lang_to_ext(current.lang);

                    if (current.lang[0]) {
                        snprintf(fallback, sizeof(fallback), "%s%s", default_basename, ext);
                        fprintf(stderr, "warning: no output filename specified, assuming '%s' from ```%s\n",
                                fallback, current.lang);
                    } else {
                        snprintf(fallback, sizeof(fallback), "%s", default_basename);
                        fprintf(stderr, "warning: no language specified, outputting as plaintext '%s'\n",
                                fallback);
                    }

                    int idx = get_output_file(&state, fallback);
                    if (idx >= 0) {
                        state.current = idx;
                        used_fallback = true;
                    }
                } else {
                    /* Continuation block - append to current */
                    state.has_unnamed_blocks = true;
                    if (state.current >= 0) {
                        state.files[state.current].unnamed_block_count++;
                    }
                }
            }
        } else {
            /* Check for closing fence */
            if (is_closing_fence(line, current.backtick_count)) {
                in_block = false;

                /* Add newline between blocks in same file (except after first) */
                if (!current.is_display && !current.is_config && state.current >= 0) {
                    OutputFile *of = &state.files[state.current];
                    if (of->block_count > 0 && of->fp) {
                        fputs("\n", of->fp);
                    }
                    of->block_count++;
                    extracted++;
                }

                current = (FenceInfo){0};
            } else {
                /* Output block content */
                if (!current.is_display && !current.is_config && state.current >= 0) {
                    FILE *out = open_output(&state, state.current);
                    if (out) {
                        fputs(line, out);
                    }
                }
            }
        }
    }

    if (in_block) {
        fprintf(stderr, "warning: unclosed code block at end of file\n");
    }

    fclose(f);
    close_all_outputs(&state);

    /* Summary */
    printf("\nextracted %d block(s) to %d file(s):\n", extracted, state.count);
    for (int i = 0; i < state.count; i++) {
        printf("  %s (%d block%s)\n",
               state.files[i].path,
               state.files[i].block_count,
               state.files[i].block_count == 1 ? "" : "s");
    }

    print_warnings(&state);

    return 0;
}

static void usage(const char *prog)
{
    fprintf(stderr, "usage: %s <markdown-file>\n", prog);
    fprintf(stderr, "\nExtracts code blocks from markdown files.\n");
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    return process_file(argv[1]);
}
