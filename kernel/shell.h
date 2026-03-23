/* kernel/shell.h
 * Minimal interactive kernel shell.
 * Reads commands from the PS/2 keyboard and dispatches
 * to built-in handlers. No dynamic allocation required. */

#ifndef SHELL_H
#define SHELL_H

#include "../kernel/kernel.h"

/* Maximum characters in one input line (including NUL) */
#define SHELL_LINE_MAX 128

/* Start the shell — never returns (runs the read-eval loop) */
void shell_run(void);

#endif /* SHELL_H */