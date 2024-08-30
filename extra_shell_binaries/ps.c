#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#define PROCESSES_DIR "/proc"
#define COMMAND_FILE "comm"
#define FILE_PATH_LEN 256

int
ps()
{
	DIR *dir;
	struct dirent *entry;

	dir = opendir(PROCESSES_DIR);
	if (!dir) {
		fprintf(stderr, "Error opening dir\n");
		return EXIT_FAILURE;
	}

	// Header
	printf("PID\tCMD\n");

	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type != DT_DIR ||
		    (entry->d_type == DT_DIR && atoi(entry->d_name) == 0)) {
			continue;
		}

		char comm_file_path[256];
		int err = snprintf(comm_file_path,
		                   FILE_PATH_LEN,
		                   "%s/%s/%s",
		                   PROCESSES_DIR,
		                   entry->d_name,
		                   COMMAND_FILE);
		if (err < 0) {
			fprintf(stderr, "Error creating file path\n");
			return EXIT_FAILURE;
		}

		FILE *comm_file = fopen(comm_file_path, "r");
		if (!comm_file) {
			fprintf(stderr, "Error opening file\n");
			return EXIT_FAILURE;
		}

		char cmd[FILE_PATH_LEN];
		if (!fgets(cmd, FILE_PATH_LEN, comm_file)) {
			fprintf(stderr, "Error reading file\n");
			return EXIT_FAILURE;
		}
		// Remove \n
		cmd[strcspn(cmd, "\n")] = '\0';

		// Print PID and CMD
		printf("%s\t%s\n", entry->d_name, cmd);

		fclose(comm_file);
	}

	closedir(dir);
	return EXIT_SUCCESS;
}

int
main()
{
	int err = ps();
	if (err) {
		fprintf(stderr, "Error in ps\n");
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}
