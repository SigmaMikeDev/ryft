# Test Document for Ryft

This is a sample markdown file to test ryft extraction.

## Configuration

```ryft.config
output = "~/.config/test/config"
backup = on
```

## Main Code

Here's our main file:

```c main.c
#include <stdio.h>

int main(void)
{
    printf("Hello from ryft!\n");
    return 0;
}
```

## Header File

```c main.h
#ifndef MAIN_H
#define MAIN_H

void greet(void);

#endif
```

## Continuation Block

This block has no filename, should append to previous target:

```c
void greet(void)
{
    printf("Greetings!\n");
}
```

## Display-Only Example

This block uses 4 backticks, so it should NOT be extracted:

````c
int this_is_just_for_display(void)
{
    /* This won't appear in output files */
    return 42;
}
````

## Plain Block

A block with no language specified:

```
just some plain text
config_option = value
```

End of test document.
