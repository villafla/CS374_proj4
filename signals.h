#ifndef SIGNALS_H
#define SIGNALS_H

#include "smallsh.h"

void signal_SIGINT(int signo);
void signal_SIGTSTP(int signo);

#endif
