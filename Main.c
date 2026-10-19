#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <sys/wait.h>

#include "Main.h"

/* Definations */
#define ARRAY_INIT_CAPACITY 20

#define array_push(array, element)                                                                \
  do {                                                                                            \
    if (array == NULL) {                                                                          \
      Header *header = malloc((sizeof(*array)* ARRAY_INIT_CAPACITY) + sizeof(Header));            \
      header-> count = 0;                                                                         \
      header-> capacity = ARRAY_INIT_CAPACITY;                                                    \
      array = (void *)(header + 1);                                                               \
    };                                                                                            \
    Header *header = ((Header *)array - 1);                                                       \
    if(header->count >= header->capacity) {                                                       \
      header->capacity *=2;                                                                       \
      header = realloc(header, (sizeof(*array) * header->capacity) + sizeof(Header));             \
      array = (void *)(header + 1);                                                               \
    };                                                                                            \
    array[header->count++] = element;                                                             \
  } while(0)

#define array_free(array) free((Header *)(array) - 1)

#define array_length(array) ((Header*)(array)-1)->count

#define comp_push(A, ...) comp_push_impl(A, __VA_ARGS__, (char*)NULL)

/* enums */
enum target {
	WIN32,
	LINUX,
};

/* struct definations */
typedef struct  {
  size_t count;
  size_t capacity;
} Header ;

typedef struct {
	char target;
	char **arr;
} comp_t;

/* function prototype */

bool comp_push_impl(comp_t *, ...);

bool comp_exec(comp_t *com);

void add_compiler(comp_t *com)
{
    comp_push(com, "cc");
}
void add_flags(comp_t *com)
{
    comp_push(com, "-Werror");
}
void add_files(comp_t *com)
{
    comp_push(com, "main.c");
}
void add_output(comp_t *com)
{
    comp_push(com, "-o", "kasa_generated");
}


int main(int argc, char *argv[])
{
    char **arr = NULL;
    comp_t com = { LINUX, arr };
    /* comp_push(&com, "Hello mader"); */
    add_compiler(&com);
    add_flags(&com);
    add_files(&com);
    add_output(&com);
    for (int i = 0; i < array_length(com.arr); i++) {
        printf("%s ", com.arr[i]);
    }
    comp_exec(&com);
    printf("\n");
    return 0;
}

bool comp_push_impl(comp_t *com, ...)
{
	va_list args;
	va_start(args, com);
    char * val = va_arg(args, char*);
	while (val != NULL) {
        array_push(com->arr, val);
        val = va_arg(args, char*);
	}
	va_end(args);
    return true;
}

//WIFEXITED(status) returns true if the child terminated normally, that is, by calling exit(3) or _exit(2), or by returning from main().
//WEXITSTATUS(status) returns the exit status of the child.  This consists of the least significant 8 bits of the status argument that the child specified in a call to exit(3) or _exit(2) or as the argument for a return statement in main().  This macro should only be employed if WIFEXITED returned true.
//WIFSIGNALED(status) returns true if the child process was terminated by a signal.
//WTERMSIG(status) returns the number of the signal that caused the child process to terminate.  This macro should only be employed if WIFSIGNALED returned true.
//WCOREDUMP(status) returns true if the child produced a core dump.  This macro should only be employed if WIFSIGNALED returned true.  This macro is not specified in POSIX.1-2001 and is not available on some UNIX implementations (e.g., AIX, SunOS).  Only use this enclosed in #ifdef WCOREDUMP ... #endif.
//WIFSTOPPED(status) returns true if the child process was stopped by delivery of a signal; this is only possible if the call was done using WUNTRACED or when the child is being traced (see ptrace(2)).
//WSTOPSIG(status) returns the number of the signal which caused the child to stop.  This macro should only be employed if WIFSTOPPED returned true.
//WIFCONTINUED(status) (since Linux 2.6.10) returns true if the child process was resumed by delivery of SIGCONT.
void exitreason(pid_t cid, int status)
{
    if( WIFEXITED(status) )
    {
        printf("child %d terminated normally, that is, by calling exit(3) or _exit(2), or by returning from main().\n",cid);
        if( WEXITSTATUS(status) )
        {
        printf("child %d exit status %d.  This consists of the least significant 8 bits of the status argument that the child specified in a call to exit(3) or _exit(2) or as the argument for a return statement in main().\n",cid,WEXITSTATUS(status));
        }
    }
    if( WIFSIGNALED(status) )
    {
        printf("child %d process was terminated by a signal.\n",cid);
        if( WTERMSIG(status) )
        {
        printf("child %d signal %d that caused the child process to terminate.\n",cid,WTERMSIG(status));
        }
        if( WCOREDUMP(status) )
        {
        printf("child %d produced a core dump.  WCOREDUMP() is not specified in POSIX.1-2001 and is not available on some UNIX implementations (e.g., AIX, SunOS).  Only use this enclosed in #ifdef WCOREDUMP ... #endif.\n",cid);
        }
    }
    if( WIFSTOPPED(status) )
    {
        printf("child %d process was stopped by delivery of a signal; this is only possible if the call was done using WUNTRACED or when the child is being traced (see ptrace(2)).\n",cid);
        if( WSTOPSIG(status) )
        {
        printf("child %d number of the signal which caused the child to stop.\n",cid);
        }
    }
    if( WIFCONTINUED(status) )
    {
        printf("child %d process was resumed by delivery of SIGCONT.\n", cid);
    }
}

bool comp_exec(comp_t *com)
{
	array_push(com->arr, NULL);
	pid_t pid = fork();
	int waitoptions = 0;
    int status = 0;
    if (pid == 0) {
        execvp(com->arr[0], com->arr);
        perror("execvp exited abnormally");
        _exit(127);
        return false;
    } else if (pid > 0) {
        pid_t cidX = waitpid(pid, &status, waitoptions);
        printf("child1: %d\n", pid);
        exitreason(pid, status);
        if (!(cidX < 0) &&status == 0) {
            return true;
        }
        return (false);
    } else {
        printf("child fork failed");
        return false;
    }
    return true;
}
