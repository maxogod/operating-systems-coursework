#include "exec.h"
#include "defs.h"
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define FD_READ 0
#define FD_WRITE 1

#define STDIN_PERM O_RDONLY
#define STDOUT_PERM (O_TRUNC | O_WRONLY)
#define STDERR_PERM (O_TRUNC | O_WRONLY)
#define CREATING_FLAG O_CREAT
#define PERM_MASK (S_IWUSR | S_IRUSR)

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	char key[BUFLEN];
	char value[BUFLEN];
	for (int i = 0; i < eargc; i++) {
		int index_pos = block_contains(eargv[i], '=');
		if (index_pos > 0) {
			get_environ_key(eargv[i], key);
			get_environ_value(eargv[i], value, index_pos);
			setenv(key, value, 1);
		}
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	int fd = open(file, flags);
	// If the file doesn't exist, create it (only when writing)
	if (fd == -1 && flags != STDIN_PERM)
		fd = open(file, flags | CREATING_FLAG, PERM_MASK);
	return fd;
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC: {
		e = (struct execcmd *) cmd;

		set_environ_vars(e->eargv, e->eargc);

		if (execvp(e->argv[0], e->argv) == -1)
			exit(EXIT_FAILURE);
		break;
	}
	case BACK: {
		b = (struct backcmd *) cmd;
		exec_cmd(b->c);
		break;
	}
	case REDIR: {
		r = (struct execcmd *) cmd;
		cmd->type = EXEC;

		// redirect stdin to the given file
		if (strlen(r->in_file) > 0) {
			int fd_in = open_redir_fd(r->in_file, STDIN_PERM);
			if (fd_in == -1) {
				exit(EXIT_FAILURE);
			}
			dup2(fd_in, STDIN_FILENO);
			close(fd_in);
		}

		// redirect stdout to the given file
		if (strlen(r->out_file) > 0) {
			int fd_out = open_redir_fd(r->out_file, STDOUT_PERM);
			if (fd_out == -1) {
				exit(EXIT_FAILURE);
			}
			dup2(fd_out, STDOUT_FILENO);
			close(fd_out);
		}

		// redirect stderr to the given file
		if (strlen(r->err_file) > 0) {
			// if the file is "&1" then redirect stderr to stdout
			if (strcmp(r->err_file, "&1") == 0)
				dup2(STDOUT_FILENO, STDERR_FILENO);
			else {
				int fd_err =
				        open_redir_fd(r->err_file, STDERR_PERM);
				if (fd_err == -1) {
					exit(EXIT_FAILURE);
				}
				dup2(fd_err, STDERR_FILENO);
				close(fd_err);
			}
		}

		exec_cmd(cmd);
		break;
	}
	case PIPE: {
		p = (struct pipecmd *) cmd;

		int fds[2];
		if (pipe(fds) == -1) {
			perror("pipe");
			exit(EXIT_FAILURE);
		}

		// left side of the pipe
		int pid_left = fork();
		if (pid_left == 0) {
			close(fds[FD_READ]);
			dup2(fds[FD_WRITE], STDOUT_FILENO);
			close(fds[FD_WRITE]);
			exec_cmd(p->leftcmd);
		} else if (pid_left < 0) {
			perror("fork");
			exit(EXIT_FAILURE);
		}

		// right side of the pipe
		int pid_right = fork();
		if (pid_right == 0) {
			close(fds[FD_WRITE]);
			dup2(fds[FD_READ], STDIN_FILENO);
			close(fds[FD_READ]);
			exec_cmd(p->rightcmd);
		} else if (pid_right < 0) {
			perror("fork");
			exit(EXIT_FAILURE);
		}

		// end manager process
		close(fds[FD_READ]);
		close(fds[FD_WRITE]);
		waitpid(pid_left, NULL, 0);
		waitpid(pid_right, NULL, 0);
		exit(EXIT_SUCCESS);  // finished successfully
		break;
	}
	}
}
