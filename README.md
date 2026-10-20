# Kasa

Kasa is a small, single-header C build helper. It assembles a compiler command, can check for a compiler and source files, runs the command, and can optionally run the built program. You describe the build in C instead of writing a shell script.

Kasa currently targets POSIX-style process APIs (`fork`, `execvp`, `waitpid`, and `access`). The `WIN32` branches are placeholders; Windows support is not implemented yet.

## How it works

Kasa collects the compiler and command-line arguments in a dynamically growing array. When you build, it forks a child process and calls `execvp` with that argument array, then waits for the child and reports its status. Compiler arguments are passed as individual strings, so you should pass `"-Wall"` and `"-Werror"` separately rather than as one combined string.

There are two ways to use Kasa:

1. **Configuration mode** (the default, `KASA_CUSTOM` is `0`): Kasa supplies `main()` and a small command dispatcher. You implement `kasa_config()` to describe the project, then invoke the resulting program with `build` or `run`.
2. **Control mode** (`KASA_CUSTOM` is `1`): Kasa does not define `main()` or the dispatcher. Your program supplies `main()` and decides when to configure, build, run, and clean up. The same configuration helpers are available, or you can construct a `kasa_t` command directly.

## Configuration mode

Define `kasa_config()` after including the header. The configuration helpers fill Kasa's built-in project configuration:

```c
#include "kasa.h"

void kasa_config(void)
{
    kasa_project("hello");
    kasa_compiler("cc", "clang"); // first available compiler on PATH is selected
    kasa_sources("main.c", "greeting.c");
    kasa_cflags("-Wall", "-Werror");
    kasa_output("hello");
    kasa_print(); // optional: print the assembled command
}
```

Compile the configuration file as a program, then use its built-in commands:

```sh
cc build.c -o kasa
./kasa build  # compile the configured sources
./kasa run    # build, then run ./hello
```

The dispatcher accepts `build` or `run`. `kasa_compiler` checks the choices in order and stops at the first executable found on `PATH`. `kasa_sources` checks that each named source file is readable and stops if one is missing. `kasa_cflags` appends compiler options, and `kasa_output` appends `-o` and remembers the executable name for `run`. The generated `main()` calls `kasa_config()`, dispatches the command, and cleans up afterward.

## Control mode (`KASA_CUSTOM 1`)

Set the macro before including `kasa.h` to opt out of Kasa's generated `main()` and dispatcher. Use the configuration helpers and call the build/run functions yourself:

```c
#define KASA_CUSTOM 1
#include "kasa.h"

int main(void)
{
    kasa_project("hello");
    kasa_compiler("cc");
    kasa_sources("main.c", "greeting.c");
    kasa_cflags("-Wall");
    kasa_output("hello");

    bool ok = kasa_build();
    if (ok) {
        ok = kasa_run(); // kasa_run builds first, then executes ./hello
    }
    kasa_destroy();
    return ok ? 0 : 1;
}
```

`kasa_build()` returns whether the compiler process succeeded. `kasa_run()` builds and then executes the configured output in the current directory. `kasa_print()` is useful for inspecting the collected command. Call `kasa_destroy()` when finished to release Kasa's argument storage.

For direct command control, create a zero-initialized `kasa_t` and use the lower-level helpers:

```c
#define KASA_CUSTOM 1
#include "kasa.h"

int main(void)
{
    kasa_t command = {0};
    bool ok = kasa_push(&command, "cc", "-Wall", "main.c", "-o", "hello")
           && kasa_exec_impl(&command);
    kasa_destroy_impl(&command);
    return ok ? 0 : 1;
}
```

This direct form gives you explicit control over the argument list. `kasa_push` appends all strings passed to it; `kasa_exec_impl` runs the first string as the program and the remaining strings as its arguments. The implementation functions are declared `static` through `internal` in the header, so use them in the same translation unit that includes `kasa.h`.

## Example in this repository

See [`build.c`](build.c) as a configuration-mode example. It selects a compiler, lists sources, adds flags, sets an output name, and prints the resulting command. Update its source list to match the files in your own project before building it; source names passed to `kasa_sources` must exist and be readable.

## API quick reference

| Helper | Purpose |
| --- | --- |
| `kasa_project(name)` | Initialize/reset the built-in configuration and set its project name. |
| `kasa_compiler(a, b, ...)` | Choose the first named compiler found on `PATH`. |
| `kasa_sources(file, ...)` | Add readable source files; stops if any are missing. |
| `kasa_cflags(flag, ...)` | Append compiler flags or other compiler arguments. |
| `kasa_output(name)` | Set output name and append `-o name`. |
| `kasa_print()` | Print the assembled command. |
| `kasa_build()` | Run the configured compiler command. |
| `kasa_run()` | Build, then run the output from the current directory. |
| `kasa_destroy()` | Free the built-in configuration's argument storage. |

