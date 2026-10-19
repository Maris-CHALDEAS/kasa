#include <seibah/kasa/kasa.h>

void add_compiler(comp_t *com)
{
    kasa_push(com, "cc");
}
void add_flags(comp_t *com)
{
    kasa_push(com, "-Werror");
}
void add_files(comp_t *com)
{
    kasa_push(com, "main.c");
}
void add_output(comp_t *com)
{
    kasa_push(com, "-o", "kasa_generated");
}

int main(void)
{
    comp_t com = { 0 };
    com.target = LINUX;
    add_compiler(&com);
    add_flags(&com);
    add_files(&com);
    add_output(&com);
    kasa_exec(&com);
    kasa_destroy(&com);
    return 0;
}

