# Kasa

Kasa is a small C helper for assembling and running compiler commands. It stores the command and its arguments in a `comp_t`, then launches it with `fork`, `execvp`, and `waitpid`.

## Requirements

- Linux or another POSIX-like system with `fork`, `execvp`, and `waitpid`
- A C compiler such as `cc`
- The Kasa header available at `seibah/kasa/kasa.h` for the include used in the example

## Using Kasa

Include `kasa.h` in your C program, create a zero-initialized `comp_t`, and add the compiler and its arguments with `kasa_push`. Call `kasa_exec` to run the command, then call `kasa_destroy` to release the argument array.

```c
#include <seibah/kasa/kasa.h>

int main(void)
{
    comp_t command = {0};
    command.target = LINUX;

    bool success = kasa_push(&command, "cc", "-Wall", "main.c", "-o", "app")
                && kasa_exec(&command);

    kasa_destroy(&command);
    return success ? 0 : 1;
}
```

`kasa_push` takes the program name first, followed by its arguments. It adds the terminating `NULL` argument internally. `kasa_exec` returns `true` when the child process succeeds and `false` if it cannot be started or exits unsuccessfully.

## Example in this repository

See [`build.c`](build.c) as an example of building a command in small helper functions. It configures `cc`, adds a warning flag, lists the source files, chooses an output name, runs the command, and cleans up the command storage.

The example's include path (`<seibah/kasa/kasa.h>`) assumes Kasa is installed or exposed in that directory layout. Adjust the include or compiler include path to match where you keep `kasa.h`.

The sample application consists of `main.c` and `fun.c`; the command configured in `build.c` compiles them into `kasa_generated`.
