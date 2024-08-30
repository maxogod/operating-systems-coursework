#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define FD_READ 0
#define FD_WRITE 1
#define ARGUMENT 1


ssize_t
check_syscall(ssize_t syscall_ret, char *msg)
{
	if (syscall_ret < 0) {
		perror(msg);
		exit(EXIT_FAILURE);
	}
	return 1;
}

void
generate_next_primes(int fd_read, int fd_write, int n, int p)
{
	if (fd_read < 0) {  // First process (Generator)
		for (int i = 2; i <= n; i++) {
			check_syscall(write(fd_write, &i, sizeof(int)),
			              "Write error\n");
		}
		return;
	}

	// Processes in the middle
	int num_to_send;
	while (read(fd_read, &num_to_send, sizeof(int))) {
		if (num_to_send % p != 0) {
			check_syscall(write(fd_write, &num_to_send, sizeof(int)),
			              "Write error\n");
		}
	}
}

void
rec_sieve_of_eratosthenes(int fd_read_left, int n)
{
	int p;
	if (fd_read_left >= 0) {  // Not the first process
		if (read(fd_read_left, &p, sizeof(int)) == 0) {
			return;  // No more numbers to read
		}
		printf("primo %d\n", p);
	}

	int fd_right[2];
	check_syscall(pipe(fd_right), "Error creating pipe\n");

	pid_t pid = fork();

	if (pid == 0) {
		close(fd_read_left);  // Close the read end of the previous pipe
		close(fd_right[FD_WRITE]);
		rec_sieve_of_eratosthenes(fd_right[FD_READ], n);
		close(fd_right[FD_READ]);
	} else if (pid > 0) {
		close(fd_right[FD_READ]);
		generate_next_primes(fd_read_left, fd_right[FD_WRITE], n, p);
		close(fd_right[FD_WRITE]);
		wait(NULL);
	} else {
		perror("Fork error\n");
		exit(EXIT_FAILURE);
	}
}

void
sieve_of_eratosthenes(int n)
{
	rec_sieve_of_eratosthenes(-1, n);
}

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		perror("Accepts only one argument\n");
		exit(EXIT_FAILURE);
	}

	int n = atoi(argv[ARGUMENT]);
	sieve_of_eratosthenes(n);

	exit(EXIT_SUCCESS);
}
