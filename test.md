# Test Document for Ryft

This is a sample markdown file to test ryft extraction.

## Configuration

```ryft.config
output = "~/.config/test/config"
backup = on
```

## Main Code

Here's our main file - first the includes:

```c main.c
#include <stdio.h>
#include "main.h"
```

Now the main function (continues into main.c):

```c
int main(void)
{
    greet();
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

## Back to Implementation

Now we implement greet() - note we explicitly say main.c:

```c main.c
void greet(void)
{
    printf("Hello from ryft!\n");
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

End of test document.
