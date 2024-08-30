#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

#ifndef NARGS
#define NARGS 4
#endif

int
execute_binary(char *bin, char *argv_for_exec[NARGS + 2])
{
	pid_t pid = fork();

	if (pid == 0) {
		execvp(bin, argv_for_exec);

		perror("Error executing binary\n");
		exit(EXIT_FAILURE);
	} else if (pid > 0) {
		wait(NULL);
		for (int i = 1; i < NARGS + 1; i++) {
			free(argv_for_exec[i]);
			argv_for_exec[i] = NULL;
		}
	} else {
		perror("Error forking\n");
		exit(EXIT_FAILURE);
	}

	return 0;
}

char *
remove_next_line_char(char *line)
{
	size_t newline_pos = strcspn(line, "\n");
	if (line[newline_pos] == '\n') {
		line[newline_pos] = '\0';
	}
	return line;
}

void
xargs(char *bin)
{
	FILE *stream = stdin;
	char *line = NULL;
	size_t len = 0;

	int argc_for_exec = 1;
	char *argv_for_exec[NARGS + 2];  // + 2 to account for the binary at [0] and NULL at [NARGS + 1]
	argv_for_exec[0] = bin;
	argv_for_exec[NARGS + 1] = NULL;

	while (getline(&line, &len, stream) != -1) {
		line = remove_next_line_char(line);

		argv_for_exec[argc_for_exec] = strdup(line);
		argc_for_exec++;

		if (argc_for_exec == NARGS + 1) {  // Reached the limit of arguments
			argc_for_exec = 1;
			execute_binary(bin, argv_for_exec);
		}
	}

	if (line) {
		execute_binary(bin, argv_for_exec);
		free(line);
	}
}

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		perror("Accepts only 1 argument\n");
		exit(EXIT_FAILURE);
	}
	char *bin = argv[1];
	xargs(bin);
	exit(EXIT_SUCCESS);
}
