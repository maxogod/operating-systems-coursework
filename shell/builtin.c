#include "builtin.h"
#include <string.h>
#include <unistd.h>

#define EXIT_STR "exit"
#define CD_STR "cd"
#define PWD_STR "pwd"

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	if (strcmp(cmd, EXIT_STR) != 0) {
		return 0;
	}
	return 1;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	if (strncmp(cmd, CD_STR, 2) != 0) {
		return 0;
	}

	char *dir = &cmd[3];
	if (dir[0] == END_STRING) {
		dir = getenv("HOME");
	}

	if (chdir(dir) < 0) {
		perror("Error changing directory");
		return 0;
	}

	snprintf(prompt, sizeof prompt, "(%s)", getcwd(NULL, 0));

	return 1;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (strcmp(cmd, PWD_STR) != 0) {
		return 0;
	}

	snprintf(prompt, sizeof prompt, "(%s)", getcwd(NULL, 0));

	return 1;
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	// Your code here

	return 0;
}
