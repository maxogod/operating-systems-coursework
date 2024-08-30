#include "defs.h"
#include "readline.h"
#include "runcmd.h"
#include <bits/types/stack_t.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "handlers.h"

char prompt[PRMTLEN] = { 0 };

// initializes the shell
// with the "HOME" directory
static void
init_shell()
{
	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}
}

// runs a shell command
static void
run_shell()
{
	char *cmd;

	while ((cmd = read_line(prompt)) != NULL)
		if (run_cmd(cmd) == EXIT_SHELL) {
			return;
		}
}

// sets the signal handler for SIGCHLD
static void *
set_signal_handler()
{
	stack_t ss;
	ss.ss_sp = malloc(SIGSTKSZ);
	if (ss.ss_sp == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	ss.ss_size = SIGSTKSZ;
	ss.ss_flags = 0;
	if (sigaltstack(&ss, NULL) == -1) {
		perror("sigaltstack");
		exit(EXIT_FAILURE);
	}

	struct sigaction sa = { 0 };

	sa.sa_flags = SA_ONSTACK | SA_RESTART | SA_SIGINFO;
	sa.sa_sigaction = &handle_sigchld;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}

	return ss.ss_sp;
}

int
main(void)
{
	init_shell();

	void *sp = set_signal_handler();

	run_shell();

	free(sp);

	return EXIT_SUCCESS;
}
