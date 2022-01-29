#ifndef _UTILITY_H_
#define _UTILITY_H_

int get_case(char argv[]);
int run_execve(char cmd[], char *parameters[]);
int split_command(char *pcmd, char *cmd_split[]);
void remove_space(char *cmd_array[], char input[]);
char* extract_command(char *pcmd, char *cmd_split[], char *cmd_array[]);
void run_waitpid(int count, int store_pid[]);
void clear_command(char *command[]);

#endif
