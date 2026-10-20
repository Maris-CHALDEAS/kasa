#include "kasa.h"

int main(int argc, char* argv[])
{
    kasa_project("App");
    kasa_compiler("cc");
    kasa_sources("main.c", "fun.c");
    kasa_cflags("-Wall", "-Werror");
    kasa_output("App");
    kasa_build();
    kasa_destroy();
}
