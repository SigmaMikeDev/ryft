/*
 * markdown.h - Markdown fence parsing
 */

#ifndef RYFT_MARKDOWN_H
#define RYFT_MARKDOWN_H

#include "types.h"

/* Count leading backticks in a line */
int count_backticks(const char *line);

/* Parse opening fence line: ```lang filename or ````lang etc */
FenceInfo parse_fence(const char *line);

/* Check if line is a closing fence matching the opening
 * Returns the number of backticks in the closing fence, or 0 if not a closing fence
 */
int get_closing_fence_backticks(const char *line, int open_backticks);

#endif /* RYFT_MARKDOWN_H */
