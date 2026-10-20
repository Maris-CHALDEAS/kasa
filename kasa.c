#include <seibah/kasa/kasa.h>


void kasa_config()
{
	kasa_project("kasa");
	kasa_compiler("cc");
	kasa_sources("build.c");
	kasa_cflags("-Wall", "-fsanitize=address,undefined");
    kasa_output("kasa");
    kasa_build();
    kasa_destroy();
}
