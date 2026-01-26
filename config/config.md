# Ryft Main Config

[toc]

## Config Header

```
# Ryft - Ripping Your Fenced Text
# Default Config File 

```

## Version 

Version of the current `config` file in use. 

```
version="0.1.0"

```

## Backup

### `backup`

`ryft` can backup the output filename if it already exists by default. This behavior can be turned on or off in the config file. The default behavior is `on`. To turn `off` backup behavior, set `backup=off`.


### `backup_limit`

By default `ryft` will keep 10 backup files if `backup=on`. Adjust the number of backups kept with `backup_limit=#` where `#` is the number of backups kept. A value of `0` will keep an unlimited number of backups. Negative values will result in the default of `10`. Note: if `backup=off`, no backups will be kept, regardless of the number set in `backup_limit`.

### `backup_timestamp`

`ryft` will by default generate a timestamp and append it to the original file before overwriting with the new file, resulting in `filename.ext` becoming `filename.ext.timestamp.bak`. If you wish to use the next number instead, such as `filename.ext.1.bak`, `filename.ext.2.bak`; set `backup_timestamp=off`.

| Backup Key         | Accepted Values                                          |
| ------------------ | -------------------------------------------------------- |
| `backup`           | `on` - *Default* turns backups `on                       |
|                    | `off` - turns all backups `off`                          |
| `backup_limit`     | `10` - *Default* `10` backups                            |
|                    | `0` - unlimited backups                                  |
|                    | `#` - Sets max number of backups to `#`                  |
| `backup_timestamp` | `on` - *Default* Timestamp will be added to backup file. |
|                    | `off` - Backups will be sequentially numbered.           |

```
backup=on
backup_limit=10
backup_timestamp=on 

```

## `verbose` mode 

`ryft` can print out a vast amount of information about the files it processes. By default, `verbose` mode is off. To set `verbose` mode to run every time `ryft` is run, set `verbose=on`. Note: If verbose mode is set to `on`, `summary` will be turned `on` as well, regardless of the `summary` value stored.

```
verbose=on 

```

## `summary` mode 

`ryft` can show a `summary` of the details after parsing a file. `summary` can be shown even if `verbose` is `off`, however, if `verbose` is turned on, `summary` will automatically display, regardless of the value set here. By default, `summary` is set to `off`. 

```
summary=off 

```

## `strict_mode` 

`ryft` can be run in `strict_mode` to disable parsing of poorly formatted or non-specific code blocks that would otherwise just generate a warning. By default, this is turned `off`. To enable a strict parsing of files and fail if the input file does not meet all requirements, set `strict_mode=on`. 

```
strict_mode=off 

```

## Ryft Config Block

The following is a format block used by `ryft`. It will not be output to the final file. 

```ryft.confg 
filename="$HOME/.config/ryft/config"
backup=on
backup_timestamp=on
verbose=on
strict_mode=off
```
