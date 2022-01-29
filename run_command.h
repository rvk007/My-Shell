#ifndef _RUNCOMMAND_H_
#define _RUNCOMMAND_H_

void signal_handler(int sig);
int built_in_commands(char command[]);
int redirection_commands(char command[], int redirect);
int simulate_pipe(char command[], int pipe_count, char *to_pipe[]);
int locate_program(char command[]);

#endif
