/*
 * util.h - String and path utilities
 */

#ifndef RYFT_UTIL_H
#define RYFT_UTIL_H

#include <stdbool.h>
#include <stddef.h>

/* Language to extension mapping */
const char *lang_to_ext(const char *lang);

/* Strip leading/trailing whitespace in place */
char *str_trim(char *str);

/* Remove surrounding quotes from string */
char *str_unquote(char *str);

/* Parse boolean value: on/off, true/false, yes/no, 1/0 */
bool parse_bool(const char *value, bool *result);

/* Expand ~ to home directory */
bool expand_path(const char *path, char *out, size_t out_size);

/* Check if path looks like a directory */
bool is_directory_path(const char *path);

/* Extract basename without extension from path */
void get_basename_no_ext(const char *path, char *out, size_t out_size);

/* Build output path from config output setting and filename */
void build_output_path(const char *config_output, const char *filename,
                       char *out, size_t out_size);

/* Get directory portion of a path */
bool get_directory(const char *path, char *out, size_t out_size);

/* Check if file exists */
bool file_exists(const char *path);

/* Create directory and all parent directories (like mkdir -p) */
bool ensure_directory(const char *path);

#endif /* RYFT_UTIL_H */
