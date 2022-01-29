#ifndef _READCOMMAND_H_
#define _READCOMMAND_H_

int get_command(char command[]);
// void read_command(char command[], char cmd[], char *par[], int result[]);
// void read_command(char command[], int result[]);
int read_command(char command[], int result[],  char *to_pipe[]);
int if_redirection(char command[]);
int if_has_redirection(char command[]); 

#endif
