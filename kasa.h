#ifndef MAIN_H_
#define MAIN_H_
#define _DEFAULT_SOURCE

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wait.h>

/* Definations */
#define internal	static
#define local_persist	static
#define global_variable static

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

#define array_length(array) ((Header *)(array) - 1)->count

#define kasa_push(A, ...) kasa_push_impl(A, __VA_ARGS__, (char *)NULL)

/* enums */
enum target {
	WIN32,
	LINUX,
};

/* struct definations */
typedef struct {
	size_t count;
	size_t capacity;
} Header;

typedef struct {
	char target;
    char *AppName;
	char **arr;
} kasa_t;


/* Global Variables */

global_variable kasa_t seibah_kasa_config;

/* function prototype */

/**
 * @brief Append arguments to a compiler command.
 *
 * Adds each argument to the command's argument array.
 * Arguments must be followed by a NULL sentinel.
 *
 * @param com Pointer to the command being constructed.
 * @param ... Argument strings, terminated by NULL.
 * @return true if successful, false otherwise.
 */
internal bool kasa_push_impl(kasa_t *, ...);

/**
 * @brief Execute the compiler command.
 *
 * Starts the requested program and waits for it to finish.
 *
 * @param com Command containing the program and its arguments.
 * @return true if the program exits successfully, false otherwise.
 */
internal bool kasa_exec_impl(kasa_t *com);

internal bool kasa_push_impl(kasa_t *com, ...)
{
	va_list args;
	va_start(args, com);
	char *val = va_arg(args, char *);
	while (val != NULL) {
		array_push(com->arr, val);
		val = va_arg(args, char *);
	}
	va_end(args);
	return true;
}

// WIFEXITED(status) returns true if the child terminated normally, that is, by
// calling exit(3) or _exit(2), or by returning from main().
// WEXITSTATUS(status) returns the exit status of the child.  This consists of
// the least significant 8 bits of the status argument that the child specified
// in a call to exit(3) or _exit(2) or as the argument for a return statement
// in main().  This macro should only be employed if WIFEXITED returned true.
// WIFSIGNALED(status) returns true if the child process was terminated by a
// signal. WTERMSIG(status) returns the number of the signal that caused the
// child process to terminate.  This macro should only be employed if
// WIFSIGNALED returned true. WCOREDUMP(status) returns true if the child
// produced a core dump.  This macro should only be employed if WIFSIGNALED
// returned true.  This macro is not specified in POSIX.1-2001 and is not
// available on some UNIX implementations (e.g., AIX, SunOS).  Only use this
// enclosed in #ifdef WCOREDUMP ... #endif. WIFSTOPPED(status) returns true if
// the child process was stopped by delivery of a signal; this is only possible
// if the call was done using WUNTRACED or when the child is being traced (see
// ptrace(2)). WSTOPSIG(status) returns the number of the signal which caused
// the child to stop.  This macro should only be employed if WIFSTOPPED
// returned true. WIFCONTINUED(status) (since Linux 2.6.10) returns true if the
// child process was resumed by delivery of SIGCONT.
internal void exitreason(pid_t cid, int status)
{
	if (WIFEXITED(status)) {
		printf("child %d terminated normally, that is, by calling "
		       "exit(3) or _exit(2), or by returning from main().\n",
		    cid);
		if (WEXITSTATUS(status)) {
			printf("child %d exit status %d.  This consists of "
			       "the least significant 8 bits of the status "
			       "argument that the child specified in a call "
			       "to exit(3) or _exit(2) or as the argument for "
			       "a return statement in main().\n",
			    cid, WEXITSTATUS(status));
		}
	}
	if (WIFSIGNALED(status)) {
		printf("child %d process was terminated by a signal.\n", cid);
		if (WTERMSIG(status)) {
			printf("child %d signal %d that caused the child "
			       "process to terminate.\n",
			    cid, WTERMSIG(status));
		}
		if (WCOREDUMP(status)) {
			printf("child %d produced a core dump.  WCOREDUMP() "
			       "is not specified in POSIX.1-2001 and is not "
			       "available on some UNIX implementations (e.g., "
			       "AIX, SunOS).  Only use this enclosed in "
			       "#ifdef WCOREDUMP ... #endif.\n",
			    cid);
		}
	}
	if (WIFSTOPPED(status)) {
		printf("child %d process was stopped by delivery of a signal; "
		       "this is only possible if the call was done using "
		       "WUNTRACED or when the child is being traced (see "
		       "ptrace(2)).\n",
		    cid);
		if (WSTOPSIG(status)) {
			printf("child %d number of the signal which caused "
			       "the child to stop.\n",
			    cid);
		}
	}
	if (WIFCONTINUED(status)) {
		printf(
		    "child %d process was resumed by delivery of SIGCONT.\n",
		    cid);
	}
}

internal bool kasa_exec_impl(kasa_t *com)
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
		if (!(cidX < 0) && status == 0) {
			return true;
		}
		return (false);
	} else {
		printf("child fork failed");
		return false;
	}
	return true;
}

/**
 * @brief Release the memory allocated for a compiler command.
 *
 * Frees the dynamically allocated argument array associated with the
 * compiler command.
 *
 * @param com Pointer to the compiler command whose argument array is
 *            to be freed.
 *
 * @note The function does not free the @p com structure itself.
 */
internal void kasa_destroy_impl(kasa_t *com)
{
	array_free(com->arr);
}


internal void kasa_project(char *app)
{
    seibah_kasa_config = (kasa_t){ 0 };
    seibah_kasa_config.AppName = app;
}

internal void kasa_args_print(kasa_t *com)
{
    for (int i = 0; i < array_length(com->arr); i++) {
        printf("%s ", com->arr[i]);
    }
    printf("\n");
}

#define kasa_sources_impl(A, ...) kasa_push(A, __VA_ARGS__)
#define kasa_sources(...) kasa_sources_impl(&seibah_kasa_config, __VA_ARGS__)

#define kasa_cflags_impl(A, ...) kasa_push(A, __VA_ARGS__)
#define kasa_cflags(...)	  kasa_cflags_impl(&seibah_kasa_config, __VA_ARGS__)


#define kasa_compiler_impl(A, ...) kasa_push(A, __VA_ARGS__)
#define kasa_compiler(...)	  kasa_compiler_impl(&seibah_kasa_config, __VA_ARGS__)

#define kasa_build() kasa_exec_impl(&seibah_kasa_config)

#define kasa_destroy() kasa_destroy_impl(&seibah_kasa_config)

#define kasa_print() kasa_args_print(&seibah_kasa_config)

#define kasa_output(A) kasa_push(&seibah_kasa_config, "-o", A);

#endif // MAIN_H_
