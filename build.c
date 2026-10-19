#include <seibah/kasa/kasa.h>

bool add_compiler(comp_t *com)
{
    return kasa_push(com, "cc");
}
bool add_flags(comp_t *com)
{
    return kasa_push(com, "-Werror");
}
bool add_files(comp_t *com)
{
    return kasa_push(com, "main.c", "fun.c");
}
bool add_output(comp_t *com)
{
    return kasa_push(com, "-o", "kasa_generated");
}

int main(int argc, char* argv[])
{
    comp_t com = { 0 };
    com.target = LINUX;
    bool success = add_compiler(&com) && add_flags(&com) && add_files(&com) && add_output(&com) && kasa_exec(&com);
    kasa_destroy(&com);
    return success ? 0 : 1;
}

