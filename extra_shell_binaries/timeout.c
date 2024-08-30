#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define MAX_ARGS 50

static timer_t timerid;
static pid_t child_pid;

// Kill child process when timeout
void
timeout_sig_handler(int sig __attribute__((unused)),
                    siginfo_t *si,
                    void *uc __attribute__((unused)))
{
	if (si->si_value.sival_ptr == &timerid) {
		printf("Command finished by timeout\n");
		kill(child_pid, SIGKILL);
	}
}

// Set up timer and signal action
void
set_timer(int duration)
{
	struct sigevent sev;
	struct sigaction sa;
	struct itimerspec its;

	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = timeout_sig_handler;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGRTMIN, &sa, NULL) == -1) {
		fprintf(stderr, "Error setting sigaction\n");
		kill(child_pid, SIGKILL);
		return;
	}

	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGRTMIN;
	sev.sigev_value.sival_ptr = &timerid;
	if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1) {
		fprintf(stderr, "Error creating timer\n");
		kill(child_pid, SIGKILL);
		return;
	}

	its.it_value.tv_sec = duration;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;

	if (timer_settime(timerid, 0, &its, NULL) == -1) {
		fprintf(stderr, "Error setting timer\n");
		kill(child_pid, SIGKILL);
		return;
	}
}

int
timeout(int duration, char *command_argv[])
{
	child_pid = fork();
	if (child_pid == 0) {
		execvp(command_argv[0], command_argv);
		fprintf(stderr, "Error executing command\n");
		return EXIT_FAILURE;
	} else if (child_pid > 0) {
		set_timer(duration);

		waitpid(child_pid, NULL, 0);

		timer_delete(timerid);
		return EXIT_SUCCESS;
	} else {
		fprintf(stderr, "Error forking\n");
		return EXIT_FAILURE;
	}
}

int
main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "Usage: %s <duration> <command> <args>\n", argv[0]);
		return EXIT_FAILURE;
	}

	int duration = atoi(argv[1]);
	if (duration <= 0) {
		fprintf(stderr, "Duration must be greater than 0\n");
		return EXIT_FAILURE;
	}

	// Make new args for the command ignoring the first two args (bin and duration)
	char *command_argv[MAX_ARGS];
	for (int i = 2; i < argc; i++) {
		command_argv[i - 2] = argv[i];
	}
	command_argv[argc - 2] = NULL;  // end of args

	int err = timeout(duration, command_argv);
	if (err) {
		fprintf(stderr, "Error in timeout\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
