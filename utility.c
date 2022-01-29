#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int get_case(char argv[])
{
  if(strcmp(argv, "exit") == 0) return 0;
  if(strcmp(argv, "cd") == 0) return 1;
  if(strcmp(argv, "fg") == 0) return 2;
  if(strcmp(argv, "jobs") == 0) return 3;
  return -1;
}

int run_execve(char cmd[], char *parameters[])
{
  char path[100];
  char *envp[] = { (char *) "PATH=/bin", 0};
  char *envpusr[] = { (char *) "PATH=/usr/bin", 0 };

  strcpy(path, "/bin/" );
  strcat(path, cmd);
  if (execve(path, parameters, envp) == -1){
    strcpy(path, "/usr/bin/" );
    strcat(path, cmd);
    if (execve(path, parameters, envpusr) == -1){
      fprintf(stderr, "Error: invalid program\n");
      exit(-1);
    }
  }
  return 1;
}

void remove_space(char *cmd_array[], char input[])
{
  char *ptr_cmd;
  int i=0;

  ptr_cmd = strtok(input, " ");
  while( ptr_cmd != NULL)
  {
    cmd_array[i++] = strdup(ptr_cmd);
    ptr_cmd = strtok(NULL, " ");
  }
  cmd_array[i] = NULL;  
}

int split_command(char *pcmd, char *cmd_split[])
{
  int argc =0;
  while( pcmd != NULL)
  {
    cmd_split[argc++] = strdup(pcmd);
    pcmd = strtok(NULL, " ");
  }
  return argc;
}

char* extract_command(char *pcmd, char *cmd_split[], char *cmd_array[])
{
  split_command(pcmd, cmd_split);
  remove_space(cmd_array, cmd_split[0]);
  return pcmd;
}

void run_waitpid(int count, int store_pid[])
{
  for( int i=0; i<count+1; i++)
  {
    waitpid(store_pid[i], NULL, WUNTRACED);
  }
}

void clear_command(char *command[])
{
  for(int i=0; command[i] != NULL; i++)
  {
    command[i] = NULL;
  }
}
