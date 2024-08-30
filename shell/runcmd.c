#include "runcmd.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>

int status = 0;
struct cmd *parsed_pipe;

// runs the command in 'cmd'
int
run_cmd(char *cmd)
{
	pid_t p;
	struct cmd *parsed;

	// if the "enter" key is pressed
	// just print the prompt again
	if (cmd[0] == END_STRING)
		return 0;

	// "history" built-in call
	if (history(cmd))
		return 0;

	// "cd" built-in call
	if (cd(cmd))
		return 0;

	// "exit" built-in call
	if (exit_shell(cmd))
		return EXIT_SHELL;

	// "pwd" built-in call
	if (pwd(cmd))
		return 0;

	// parses the command line
	parsed = parse_line(cmd);

	// forks and run the command
	if ((p = fork()) == 0) {
		// keep a reference
		// to the parsed pipe cmd
		// so it can be freed later
		if (parsed->type == PIPE)
			parsed_pipe = parsed;

		exec_cmd(parsed);
	}

	// stores the pid of the process
	parsed->pid = p;

	// background process special treatment
	if (parsed->type == BACK) {
		print_back_info(parsed);
		// set pgid equal to pid --> that's how background processes are identified
		setpgid(p, 0);
		free_command(parsed);
		return 0;
	}

	// waits for the process to finish

	waitpid(p, &status, 0);

	print_status_info(parsed);

	free_command(parsed);

	return 0;
}
