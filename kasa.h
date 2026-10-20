#ifndef MAIN_H_
#define MAIN_H_

#ifdef WIN32

#else

#define _DEFAULT_SOURCE

#endif // WIN32

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>


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
    (array)[header->count++] = element;                                                           \
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
    char *output;
    char *compiler;
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
		printf("[KASA] child %d terminated normally, that is, by calling "
		       "exit(3) or _exit(2), or by returning from main().\n",
		    cid);
		if (WEXITSTATUS(status)) {
			printf("[KASA] child %d exit status %d.  This consists of "
			       "the least significant 8 bits of the status "
			       "argument that the child specified in a call "
			       "to exit(3) or _exit(2) or as the argument for "
			       "a return statement in main().\n",
			    cid, WEXITSTATUS(status));
		}
	}
	if (WIFSIGNALED(status)) {
		printf("[KASA] child %d process was terminated by a signal.\n", cid);
		if (WTERMSIG(status)) {
			printf("[KASA] child %d signal %d that caused the child "
			       "process to terminate.\n",
			    cid, WTERMSIG(status));
		}
		if (WCOREDUMP(status)) {
			printf("[KASA] child %d produced a core dump.  WCOREDUMP() "
			       "is not specified in POSIX.1-2001 and is not "
			       "available on some UNIX implementations (e.g., "
			       "AIX, SunOS).  Only use this enclosed in "
			       "#ifdef WCOREDUMP ... #endif.\n",
			    cid);
		}
	}
	if (WIFSTOPPED(status)) {
		printf("[KASA] child %d process was stopped by delivery of a signal; "
		       "this is only possible if the call was done using "
		       "WUNTRACED or when the child is being traced (see "
		       "ptrace(2)).\n",
		    cid);
		if (WSTOPSIG(status)) {
			printf("[KASA] child %d number of the signal which caused "
			       "the child to stop.\n",
			    cid);
		}
	}
	if (WIFCONTINUED(status)) {
		printf(
		    "[KASA] child %d process was resumed by delivery of SIGCONT.\n",
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
		perror("[KASA] execvp exited abnormally\n");
		_exit(127);
		return false;
	} else if (pid > 0) {
		pid_t cidX = waitpid(pid, &status, waitoptions);
		printf("[KASA] child: %d\n", pid);
		exitreason(pid, status);
		if (!(cidX < 0) && status == 0) {
			return true;
		}
		return (false);
	} else {
		printf("[KASA] child fork failed\n");
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
    printf("[KASA] ");
    for (int i = 0; i < array_length(com->arr); i++) {
        printf("%s ", com->arr[i]);
    }
    printf("\n");
}

internal int isReadable(const char *source)
{
#ifdef WIN32
#else
	if (access(source, R_OK) == 0) {
        return 1;
	}
    return 0;

#endif // WIN32
}


void kasa_sources_impl(kasa_t *com, ...)
{
    bool success = true;
	va_list args;
	va_start(args, com);
	char *source = va_arg(args, char *);
	while (source != NULL) {
		if (isReadable(source)) {
            fprintf(stderr, "[KASA] File found: %s\n", source);
            array_push(com->arr, source);
		} else {
            success = false;
            fprintf(stderr, "[KASA] File not found: %s\n", source);
		}
		source = va_arg(args, char *);
	}
    if (!success) {
        fprintf(stderr,
            "All given source files not found.\nStopping the build process!\n");
        _exit(2);
        return;
    }
}
#define kasa_sources(...) kasa_sources_impl(&seibah_kasa_config, __VA_ARGS__, (char*)NULL)

#define kasa_cflags_impl(A, ...) kasa_push(A, __VA_ARGS__)
#define kasa_cflags(...)	  kasa_cflags_impl(&seibah_kasa_config, __VA_ARGS__)

internal int isExecutable(const char *source)
{
#ifdef WIN32
#else
	if (access(source, X_OK) == 0) {
        return 1;
	}
    return 0;
#endif // WIN32
}

internal int kasa_str_pos_find(const char *paths, char chr)
{
    int pos = -1;
    int i = 0;
    while (paths[i] != '\0') {
	    if (paths[i] == chr) {
		    pos = i;
		    break;
        }
	    i++;
    }
    return pos;
}
internal void kasa_substr(const char *paths, int start, int size, char **ans)
{
	char res[size + 1];
	for (int i = 0; i < size; i++) {
        res[i] = paths[i+start];
	}
	res[size] = '\0';
	*ans = res;
}

internal void kasa_split_string(const char *paths, char ***arr, char a)
{
	int pos = kasa_str_pos_find(paths, a);
	int initPos = 0;
	*arr = NULL;
    char *res;
	while (pos != -1) {
        pos += initPos;
        kasa_substr(paths ,initPos, pos - initPos, &res);
        array_push(*arr, res);
        initPos = pos + 1;
        pos = kasa_str_pos_find(paths + initPos, a);
	}
    kasa_substr(paths ,initPos, strlen(paths)+1, &res);
	array_push(*arr, res);
}

bool compilerExistenceOnPath(kasa_t *com, char *compiler)
{
	bool isPresent = false;
	const char *env = getenv("PATH");
	const char *paths = env ? env : "";
	if ((strcmp(paths, "")) == 0) {
        return isExecutable((compiler));
	}
	char **arr = NULL;
	kasa_split_string(paths, &arr, ':');
	if (arr == NULL) {
		fprintf(stdout,"PATH parsing failed");
        _exit(126);
    }
    char path[512] = "";
	for (int i = 0; i < array_length(arr); i++) {
		strcpy(path, arr[i]);
		strcat(path, "/");
		strcat(path, compiler);
		if (isExecutable(path)) {
			com->compiler = compiler;
			isPresent = true;
            break;
        }
	}
    array_free(arr);
	return isPresent;
}

void kasa_compiler_impl(kasa_t *com, ...)
{
	va_list args;
	va_start(args, com);
	char *compiler = va_arg(args, char *);
    bool success = false;
	while (compiler != NULL) {
		if (compilerExistenceOnPath(com, compiler)) {
            array_push(com->arr, compiler);
            success = true;
            fprintf(stdout, "[KASA] Compiler %s found\n", compiler);
            break;
		}
        fprintf(stdout, "[KASA] Compiler %s not found\n", compiler);
		compiler = va_arg(args, char *);
    }

    // TODO(souls): Try to not exit prematurely we will try to verify as many things as possible(like missing source files) so to give detailed view to the user;
	if (!success) {
		fprintf(stderr, "[KASA] None of the provided compiler on PATH\nStopping the execution\n");
        _exit(126);
	}
}

#define kasa_compiler(...)	  kasa_compiler_impl(&seibah_kasa_config, __VA_ARGS__, (char *)NULL)

#define kasa_build() kasa_exec_impl(&seibah_kasa_config)

#define kasa_destroy() kasa_destroy_impl(&seibah_kasa_config)

#define kasa_print() kasa_args_print(&seibah_kasa_config)

#define kasa_output(A) kasa_output_impl(&seibah_kasa_config, A);

void kasa_output_impl(kasa_t *com, char *output)
{
	com->output = output;
    kasa_push(com, "-o", output);
}

#define kasa_run() kasa_run_impl(&seibah_kasa_config)

bool kasa_run_impl(kasa_t *com)
{
    kasa_build();
    printf("[KASA] building...\n");
    char prefix[128] = "./";
    if (strlen(com->output) > 125) {
        fprintf(stderr, "[KASA] Execution File name length exceeds supported size: 125 char");
        return false;
    }
	strcat(prefix, com->output);
	char *args[] = {prefix, NULL};
	pid_t cid = fork();
	if (cid == 0) {
        fprintf(stderr, "[KASA] executing: %s\n", com->arr[0]);
		if (execv(args[0], args)) {
			perror("[KASA] Error encountered while trying to run the "
			       "program\n");
			printf("[KASA] Check weather the executable file is present "
			       "in the current directory\n");
			_exit(127);
            return false;
        }
	} else if (cid > 0) {
		int status = 0;
		int waitoptions = 0;
		pid_t cidx = waitpid(cid, &status, waitoptions);
		printf("[KASA] child: %d\n", cid);
		exitreason(cid, status);
		if ((cidx >= 0) && status == 0) {
            return true;
		}
		return false;
	} else {
		printf("[KASA] Child fork failed during file execution\n");
		return false;
    }
    return true;
}

#ifndef KASA_CUSTOM
#define KASA_CUSTOM 0
#endif // KASA_CUSTOM

#if !KASA_CUSTOM

void kasa_config(void);
int kasa_dispatch(int argc, char *argv[]);
int main(int argc, char *argv[])
{
	kasa_config();
	int val = kasa_dispatch(argc, argv);
	kasa_destroy();
	return val;
}

int kasa_dispatch(int argc, char **argv)
{
	if (argc == 1) {
        return 0;
	} else if (argc > 2) {
        printf("[KASA] Exceeds the number of arguments required(2)\n");
        return 1;
	} else {
		if ((strcmp(argv[1], "build")) == 0) {
            kasa_build();
            return 0;
		}
		if ((strcmp(argv[1], "run")) == 0) {
            kasa_run();
            return 0;
        }
	}
	return 0;
}

#endif // !KASA_CUSTOM
#endif // MAIN_H_
