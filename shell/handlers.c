#include "handlers.h"
#include "defs.h"
#include <stdio.h>
#include <sys/wait.h>
#include "printstatus.h"

void
handle_sigchld(int signo, siginfo_t *info, void *context)
{
	pid_t pid = info->si_pid;

	// background processes have pid = pgid
	if (pid != getpgid(pid))
		return;

	int status;
	waitpid(pid, &status, 0);  // free system resources

	print_finished_info(pid);
}
