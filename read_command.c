#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "read_command.h"
#include "utility.h"


int io_validate(char command[], char *operator)
{
  char * cmd_arr[100];
  int count = 0;
  remove_space(cmd_arr, command);
  int command_len = 0;
  for (int i=0; cmd_arr[i] != NULL;i++)
  {
    command_len += 1;
  }

  if (get_case(cmd_arr[0]) > 0) return -1;
  for(int i=0; cmd_arr[i] != NULL; i++)
  {
    if (strcmp(cmd_arr[i],operator) == 0)
    {
      count += 1;
      if (count > 1 || i == 0)
      {
      return -1;        
      }
      if (cmd_arr[i-1] && cmd_arr[i+1]) continue;
      else
      {
        return -1;
      }
    }
  }
  for (int i=0; i<command_len;i++)
  {
    if (strcmp(operator,">>")==0)
    {
      if(strcmp(cmd_arr[i],">>") == 0 && (i+2!=command_len)) return -1;
    }
    else if (strcmp(operator,">")==0)
    {
      if(strcmp(cmd_arr[i],">") == 0)
      {
        if (cmd_arr[i+2])
        {
          if (!((strcmp(cmd_arr[i+2],"<") == 0) && (i+4 == command_len))) return -1;
        }
      }
    }
    else if (strcmp(operator,"<")==0)
    {
      if(strcmp(cmd_arr[i],"<") == 0)
      {
        if(cmd_arr[i+2])
        {
          if((strcmp(cmd_arr[i+2],"|") == 0) || (i+2 == command_len) || (strcmp(cmd_arr[i+2],">") == 0) || (strcmp(cmd_arr[i+2],">>") == 0))
          {
            return 1;
          }
          else return -1;
        }
      }
    } 
  }
 return 1;
}

int pipe_validate(char command[])
{
  char * cmd_arr[100];
  int first_pipe = 0, last_pipe = 0;
  remove_space(cmd_arr, command);
  if (get_case(cmd_arr[0]) > 0) return -1;
  for(int i=0; cmd_arr[i] != NULL; i++)
  {
    if (strcmp(cmd_arr[i], "|") == 0)
    {
      if (i == 0) return -1;
      if (first_pipe == 0) first_pipe = i;
      if (first_pipe > 0) last_pipe = i;
      if (cmd_arr[i-1] && cmd_arr[i+1]) continue;
      else return -1; 
    }
  }
  if (last_pipe == 0)
  {
    for(int i=0; cmd_arr[i] != NULL; i++)
    {
      if (strcmp(cmd_arr[i], ">")==0 && i<first_pipe) return -1;
      if (strcmp(cmd_arr[i], ">>")==0 && i<first_pipe) return -1;
      if (strcmp(cmd_arr[i], "<")==0 && i>first_pipe) return -1;
    }
    return 0;
  }
  for(int i=0; cmd_arr[i] != NULL; i++)
  {
    if (strcmp(cmd_arr[i], ">")==0 && i<first_pipe) return -1;
    if (strcmp(cmd_arr[i], ">>")==0 && i<first_pipe) return -1;
    if (strcmp(cmd_arr[i], "<")==0 && last_pipe<i) return -1;
    if (strcmp(cmd_arr[i], ">") == 0 || strcmp(cmd_arr[i], ">>") == 0 )
    {
      if (i<first_pipe || (i>first_pipe && i<last_pipe)) return -1;
    }
    if (strcmp(cmd_arr[i], "<") == 0)
    {
      if (i>last_pipe || (i>first_pipe && i<last_pipe)) return -1;
    }
    }
    return 1;
}

int if_redirection(char command[])
{
  char cpy_command[1001];
  strcpy(cpy_command, command);

  char *append = strstr(cpy_command, ">>");
  char *input = strstr(cpy_command, "<");
  char *output = strstr(cpy_command, ">");
  char *append_reverse = strstr(cpy_command, "<<");
  char *slash = strstr(cpy_command, "/");
  
  // check if there is / and it is in first argument
  if (slash != NULL){
    char * cmd_arr[100];
    char cpy_command0[1001];
    strcpy(cpy_command0, command);
    remove_space(cmd_arr, cpy_command0);
    char *slash_1 = strstr(cmd_arr[0], "/");
    if (slash_1 != NULL) return 6;
  }

  if (append_reverse != NULL){
    fprintf(stderr, "Error: invalid command\n");
    return -2;
  }

  if (append != NULL)
  {
    char cpy_command1[1001];
    strcpy(cpy_command1, command);
    if (io_validate(cpy_command1, ">>") == -1) 
    {
      fprintf(stderr, "Error: invalid command\n");
      return -2;
    }
  }

  if (output != NULL)
  {
    char cpy_command2[1001];
    strcpy(cpy_command2, command);
    if (io_validate(cpy_command2, ">") == -1) 
    {
      fprintf(stderr, "Error: invalid command\n");
      return -2;
    }
  }

  if (input != NULL)
  {
    char cpy_command3[1001];
    strcpy(cpy_command3, command);
    if (io_validate(cpy_command3, "<") == -1) 
    {
      fprintf(stderr, "Error: invalid command\n");
      return -2;
    }
  } 

  if ((append != NULL ) && (input != NULL)) return 5;
  if ((output != NULL ) && (input != NULL)) return 4;
  if (append != NULL) return 3;
  if (output != NULL)  return 2;
  if (input != NULL) return 1;
  
  return 0;
}

int if_has_redirection(char command[])
{
  char cpy_command[1001];
  strcpy(cpy_command, command);

  char *append = strstr(cpy_command, ">>");
  char *input = strstr(cpy_command, "<");
  char *output = strstr(cpy_command, ">");

  if ((append != NULL ) && (input != NULL)) return 5;
  if ((output != NULL ) && (input != NULL)) return 4;
  if (append != NULL) return 3;
  if (output != NULL)  return 2;
  if (input != NULL) return 1;
  
  return 0;
}

int if_pipe(char command[], char *to_pipe[])
{
  int pipe_count = 0;
  char cpy_command1[1001], cpy_command2[1001];
  int j=0;
  char *ini_cmd[100], *pch;
  strcpy(cpy_command1, command);
  strcpy(cpy_command2, command);

  char *pipe = strstr(cpy_command1, "|");
  if (pipe != NULL)
  {
    if (pipe_validate(cpy_command1) < 0)
    {
      fprintf(stderr, "Error: invalid command\n");
      return -2;
    }
    pch = strtok(cpy_command2, "|");
    // pipe_count ++;
    while( pch != NULL)
    {
      ini_cmd[pipe_count++] = strdup(pch);
      pch = strtok(NULL, "|");
    }
    for(j=0; j<pipe_count;j++)
    {
      to_pipe[j] = ini_cmd[j];
    }
    to_pipe[pipe_count] = NULL;
  }
  return pipe_count-1;
}

int get_command(char command[]){
  char character;
  int count = 0;
  character = getchar();
  while(character != '\n' && character != EOF)                                                      
  {                                                                                 
      command[count]   = character;                              
      count++;
      character = getchar();
  };
  if (character == EOF)
  {
    return -1;
  }
  if (count == 1)
  {
    return 0;
  }                  
  command[count] = '\0';
  return 1;
}

int read_command(char command[], int result[],  char *to_pipe[])
{
  char cpy_command[1001];
  strcpy(cpy_command, command);

  int pipe = if_pipe(cpy_command, to_pipe);

  if(pipe == -2) return -1;
  int redirect = if_redirection(cpy_command);

  if (redirect == -2) return -1;


  result[0] = redirect;
  result[1] = pipe;
  return 1;
}
