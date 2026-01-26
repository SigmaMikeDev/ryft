/*
 * markdown.c - Markdown fence parsing
 */

#include "markdown.h"

#include <string.h>

/* Count leading backticks in a line */
int count_backticks(const char *line)
{
    int count = 0;
    while (*line == '`') {
        count++;
        line++;
    }
    return count;
}

/* Parse opening fence line: ```lang filename or ````lang etc */
FenceInfo parse_fence(const char *line)
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

/* Check if line is a closing fence matching the opening
 * Returns the number of backticks in the closing fence, or 0 if not a closing fence
 */
int get_closing_fence_backticks(const char *line, int open_backticks)
{
    int count = count_backticks(line);
    if (count < open_backticks) {
        return 0;
    }

    /* Rest of line should be whitespace only */
    const char *p = line + count;
    while (*p) {
        if (*p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') {
            return 0;
        }
        p++;
    }
    return count;
}
