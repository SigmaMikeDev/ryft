/*
 * types.h - Shared type definitions for ryft
 */

#ifndef RYFT_TYPES_H
#define RYFT_TYPES_H

#include <stdbool.h>
#include <stdio.h>

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
    char backup_path[MAX_PATH];  /* path to backup file if created */
    char lang[MAX_LANG];         /* primary language for this file */
    FILE *fp;
    int block_count;             /* blocks written to this file */
    int unnamed_block_count;     /* blocks without explicit filename */
    bool existed;                /* file existed before we wrote to it */
    bool backed_up;              /* backup was created */
    bool opened;                 /* file has been opened (or simulated in dry-run) */
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

/* Global options */
typedef struct {
    bool backup;               /* create backup before overwriting */
    bool backup_timestamp;     /* use timestamp in backup name (vs sequential) */
    int  backup_limit;         /* max backups to keep (0=unlimited) */
    bool dry_run;              /* don't write files, just show what would happen */
    bool verbose;              /* extra output during processing */
    bool summary;              /* print summary at end */
    bool strict_mode;          /* fail on warnings instead of continuing */
} RyftOptions;

/* Statistics for summary */
typedef struct {
    int total_blocks;          /* all code blocks found */
    int extracted_blocks;      /* blocks written to files */
    int display_blocks;        /* blocks skipped (4+ backticks) */
    int config_blocks;         /* ryft.config blocks */
    int files_created;         /* new files created */
    int files_overwritten;     /* existing files overwritten */
    int backups_created;       /* backup files created */
} RyftStats;

/* Document-level config from ryft.config blocks */
typedef struct {
    char output[MAX_PATH];     /* default output path (or filename) */
    char filename[MAX_PATH];   /* explicit output filename */
    char lang[MAX_LANG];       /* default language */
    char version[32];          /* config version */
    bool backup;               /* create backups */
    bool backup_set;           /* backup was explicitly set */
    bool backup_timestamp;     /* use timestamp in backup name */
    bool backup_timestamp_set;
    int  backup_limit;         /* max backups to keep */
    bool backup_limit_set;
    bool verbose;              /* verbose output */
    bool verbose_set;          /* verbose was explicitly set */
    bool summary;              /* print summary */
    bool summary_set;          /* summary was explicitly set */
    bool strict_mode;          /* fail on warnings */
    bool strict_mode_set;
} RyftConfig;

#endif /* RYFT_TYPES_H */
