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

typedef struct {
    char lang[MAX_LANG];
    char filename[MAX_FILENAME];
    bool is_config;      /* ryft.config block */
    bool is_display;     /* 4+ backticks, skip extraction */
    int backtick_count;
} FenceInfo;

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

/* Process a markdown file, extract code blocks to stdout */
static int process_file(const char *filepath)
{
    FILE *f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "error: cannot open '%s'\n", filepath);
        return 1;
    }

    char line[MAX_LINE];
    bool in_block = false;
    FenceInfo current = {0};
    int block_num = 0;

    while (fgets(line, sizeof(line), f)) {
        if (!in_block) {
            /* Check for opening fence */
            if (count_backticks(line) >= 3) {
                current = parse_fence(line);
                in_block = true;
                block_num++;

                /* Print block header info */
                printf("=== BLOCK %d ===\n", block_num);
                printf("backticks: %d\n", current.backtick_count);
                printf("lang: '%s'\n", current.lang);
                printf("filename: '%s'\n", current.filename);
                printf("is_config: %s\n", current.is_config ? "yes" : "no");
                printf("is_display: %s\n", current.is_display ? "yes" : "no");
                printf("--- content ---\n");
            }
        } else {
            /* Check for closing fence */
            if (is_closing_fence(line, current.backtick_count)) {
                in_block = false;
                printf("--- end ---\n\n");
                current = (FenceInfo){0};
            } else {
                /* Output block content */
                printf("%s", line);
            }
        }
    }

    if (in_block) {
        fprintf(stderr, "warning: unclosed code block at end of file\n");
    }

    fclose(f);
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
