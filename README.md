# Ryft

**R**ip **Y**our **F**enced **T**ext - a literate programming tool that extracts code blocks from markdown files.

Ryft "tangles" fenced code blocks from markdown documents into source files, enabling you to write documentation and code together in a single markdown file.

## Installation

```sh
make
sudo make install
```

This installs `ryft` to `/usr/local/bin`. To install elsewhere:

```sh
make PREFIX=~/.local install
```

## Usage

```sh
ryft [options] <markdown-file>
```

### Options

| Option | Description |
|--------|-------------|
| `-b, --backup` | Create timestamped backup before overwriting |
| `-n, --dry-run` | Show what would be done without writing files |
| `-s, --summary` | Print detailed summary after processing |
| `-S, --strict` | Strict mode: fail on warnings |
| `-v, --verbose` | Verbose output (includes summary) |
| `-V, --version` | Show version information |
| `-h, --help` | Show help message |

## How It Works

Ryft extracts fenced code blocks from markdown and writes them to files.

### Basic Example

Given a file `hello.md`:

````markdown
# Hello World

```c hello.c
#include <stdio.h>

int main(void) {
    printf("Hello, world!\n");
    return 0;
}
```
````

Running `ryft hello.md` creates `hello.c` with the code block contents.

### Specifying Output Files

Code blocks can specify their output file after the language:

````markdown
```python script.py
print("This goes to script.py")
```

```python utils.py
def helper():
    return "This goes to utils.py"
```
````

### Continuation Blocks

Blocks without a filename append to the current output file:

````markdown
```c main.c
#include <stdio.h>
```

Some explanation here...

```c
int main(void) {
    return 0;
}
```
````

Both blocks are written to `main.c`.

### Display-Only Blocks

Use 4+ backticks for blocks that should not be extracted (for showing examples):

`````markdown
````c
// This block is for display only and won't be extracted
````
`````

### Controlling Blank Lines

By default, code blocks are concatenated directly. To add a blank line after a block, use 4+ backticks on the closing fence:

`````markdown
```c main.c
#include <stdio.h>
````

```c
int main(void) {
    return 0;
}
```
`````

This produces:

```c
#include <stdio.h>

int main(void) {
    return 0;
}
```

The extra backtick on the first closing fence signals ryft to insert a blank line, useful for separating includes, function definitions, or logical sections.

### Document Configuration

Use `ryft.config` blocks to set document-level options:

````markdown
```ryft.config
filename = output.c
lang = c
backup = on
```
````

#### Config Options

| Key | Description |
|-----|-------------|
| `output` | Output directory or file path |
| `filename` | Explicit output filename |
| `lang` | Default language |
| `backup` | Create backups (`on`/`off`) |
| `backup_timestamp` | Use timestamps in backup names |
| `backup_limit` | Maximum backups to keep |
| `verbose` | Enable verbose output |
| `summary` | Print summary after processing |
| `strict_mode` | Fail on warnings |

### Global Configuration

Create `~/.config/ryft/config` for persistent settings:

```
backup = on
summary = on
```

## Language Extensions

Ryft automatically maps language identifiers to file extensions:

| Language | Extension |
|----------|-----------|
| `c` | `.c` |
| `python`, `py` | `.py` |
| `javascript`, `js` | `.js` |
| `rust`, `rs` | `.rs` |
| `go` | `.go` |
| ... | ... |

Unknown languages use the identifier as the extension (e.g., `foo` becomes `.foo`).

## Examples

```sh
# Preview what would be extracted
ryft -n document.md

# Extract with verbose output
ryft -v document.md

# Extract with backups enabled
ryft -b document.md

# Strict mode (fail on warnings)
ryft -S document.md
```

## License

MIT
