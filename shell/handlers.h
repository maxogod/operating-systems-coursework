#ifndef HANDLERS_H
#define HANDLERS_H

#include <bits/types/siginfo_t.h>

// handles the SIGCHLD signal
// background processes special treatment
void handle_sigchld(int signo, siginfo_t *info, void *context);

#endif  // HANDLERS_H
