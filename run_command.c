#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

#include "prompt.h"
#include "read_command.h"
#include "utility.h"

// to handle jobs and fg
int CURRENT_PID;
char *suspended_commands[100];
int suspended_pids[100];
int sj_counter = 0;
char *CURRENT_COMMAND;

void signal_handler(int sig)
{
  if ((sig == SIGTSTP || sig == SIGSTOP) && CURRENT_PID >=0)
  {
    suspended_commands[sj_counter] = CURRENT_COMMAND;
    suspended_pids[sj_counter] = CURRENT_PID;
    sj_counter++;
  }
  signal(sig, signal_handler);
}

void print_suspended_jobs()
{
  for(int i=0; i<sj_counter ;i++)
  {
    printf("[%d]  %s\n",i+1, suspended_commands[i]);
    fflush(stdout);
  }
}

void run_fg(char s_index[])
{
  int index = atoi(s_index);
  if (index != 0 && index-1 < sj_counter)
  {
    int pid = suspended_pids[index-1];
    CURRENT_COMMAND = strdup(suspended_commands[index-1]);
    for (int i=index-1;i< sj_counter-1 ;i++)
    {
      suspended_commands[i] = strdup(suspended_commands[i+1]);
      suspended_pids[i] = suspended_pids[i+1];
    }
    sj_counter--;
    kill(pid, SIGCONT);
    CURRENT_PID = pid;
    waitpid(-1, NULL, WUNTRACED);
    CURRENT_PID = -1;
  }
  else
  {
    fprintf(stderr, "Error: invalid job\n");
    return;
  }
}

int built_in_commands(char command[])
{
  char *parameters[900], *cmd_split[100];
  int j=0, pid; 

  char * ptr_cmd = strtok(command, " ");
  int argc = split_command(ptr_cmd, cmd_split);
  for(j=0; j<argc;j++)
  {
    parameters[j] = cmd_split[j];
  }
  parameters[argc] = NULL;
  int cmd_type = get_case(parameters[0]);
  switch (cmd_type)
  {
  case 0: // exit
      if (argc > 1)
      {
        fprintf(stderr, "Error: invalid command\n");
        return -2;
      }
      else
      {
        if (sj_counter > 0)
        {
          fprintf(stderr, "Error: there are suspended jobs\n");
          return -2;
        }
        else 
        {
          exit(1);
        }
      }
      break;
      
  case 1: // cd
      if (argc == 1 || argc>2)
      {
        fprintf(stderr, "Error: invalid command\n");
        return -2;
      }
      else if(chdir(parameters[1]) == -1)
      {
        fprintf(stderr, "Error: invalid directory\n");
        return -2;
      }
      break;

  case 2: //fg
      if (argc == 1 || argc>2)
      {
        fprintf(stderr, "Error: invalid command\n");
        return -2;
      }
      else
      {
        run_fg(parameters[1]);
      }
      break;
      
  case 3: //jobs
      if (argc > 1)
      {
        fprintf(stderr, "Error: invalid command\n");
        return -2;
      }
      else
      {
        print_suspended_jobs();
      }
      break;
  default:
      pid = fork();
      if ( pid < 0 )
      {
        fprintf(stderr, "Error: fork()\n");
        return -1;
      }
      CURRENT_COMMAND = strdup(command);
      CURRENT_PID = pid;      
      if(pid == 0)
      {
        run_execve(parameters[0], parameters);
      }
      waitpid(-1, NULL, WUNTRACED); 
      CURRENT_PID = -1;
      break;
  }
  return 1;
}

int redirection_commands(char command[], int redirect)
{
  char *ptr_cmd;
  char filename[100], filename2[100];
  char *cmd_split[100], *cmd_array[100];
  int fd, fd2, pid, ori_fd, ori_fd_in, ori_fd_out;

  CURRENT_COMMAND = strdup(command);
  switch (redirect)
  {
  case 1: // < in
    ptr_cmd = strtok(command, "<");
    extract_command(ptr_cmd, cmd_split, cmd_array);
    strcpy(filename, cmd_split[1]);
    if((fd = open(filename, O_RDONLY, 0644)) < 0)
    {
      fprintf(stderr, "Error: invalid file\n");
      return -2;
    } 
    ori_fd = dup(STDIN_FILENO);
    pid = fork();
    if ( pid < 0 )
    {
      fprintf(stderr, "Error: fork()\n");
      return -1;
    } 
    CURRENT_PID = pid;  
    if(pid == 0)
    {
      dup2(fd, STDIN_FILENO);
      close(fd);
      fflush(stdout);
      run_execve(cmd_array[0], cmd_array);
    }
    waitpid(-1, NULL, WUNTRACED); 
    CURRENT_PID = -1;
    close(fd);
    dup2(ori_fd, fd);
    break;

  case 2: // > out
    ptr_cmd = strtok(command, ">");
    extract_command(ptr_cmd, cmd_split, cmd_array);
    strcpy(filename, cmd_split[1]);
    if((fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0)
    {
      fprintf(stderr, "Error: invalid file\n");
      return -2;
    }
    ori_fd = dup(STDOUT_FILENO);
    pid = fork();
    if ( pid < 0 )
    {
      fprintf(stderr, "Error: fork()\n");
      return -1;
    }         
    CURRENT_PID = pid;      
    if(pid == 0)
    {
      dup2(fd, STDOUT_FILENO); // send 'out' the content of terminal to a file
      close(fd);
      run_execve(cmd_array[0], cmd_array);
    }
    waitpid(-1, NULL, WUNTRACED); 
    CURRENT_PID = -1;
    close(fd);
    dup2(ori_fd, fd);
    break;

  case 3: // >> append
    ptr_cmd = strtok(command, ">");
    extract_command(ptr_cmd, cmd_split, cmd_array);
    strcpy(filename, cmd_split[2]);
    if((fd = open(filename, O_WRONLY | O_APPEND, 0644)) < 0)
    {
      fprintf(stderr, "Error: invalid file\n");
      return -2;
    }
    ori_fd = dup(STDOUT_FILENO);
    pid = fork();
    if ( pid < 0 )
    {
      fprintf(stderr, "Error: fork()\n");
      return -1;
    }         
    CURRENT_PID = pid;      
    if(pid == 0)
    {
      dup2(fd, STDOUT_FILENO);
      close(fd);
      run_execve(cmd_array[0], cmd_array);
    }
    waitpid(-1, NULL, WUNTRACED); 
    CURRENT_PID = -1;
    close(fd);
    dup2(ori_fd, fd);
    break;

  case 4: // < in > out
    ptr_cmd = strtok(command, "<");
    extract_command(ptr_cmd, cmd_split, cmd_array);

    strcpy(filename, cmd_split[1]);
    strcpy(filename2, cmd_split[3]);
    if((fd = open(filename, O_RDONLY, 0644)) < 0)
    {
      fprintf(stderr, "Error: invalid file\n");
      return -2;
    }

    if((fd2 = open(filename2, O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0)
    {
      fprintf(stderr, "Error: invalid file\n");
      return -2;
    }

    ori_fd_in = dup(STDIN_FILENO);
    ori_fd_out = dup(STDOUT_FILENO);
    pid = fork();
    if (pid < 0 )
    {
      fprintf(stderr, "Error: fork()\n");
      return -1;
    }        
    CURRENT_PID = pid;      
    if(pid == 0)
    {
      dup2(fd, STDIN_FILENO);
      close(fd);

      dup2(fd2, STDOUT_FILENO); // send 'out' the content of terminal to a file
      close(fd2);

      run_execve(cmd_array[0], cmd_array);
    }
    waitpid(-1, NULL, WUNTRACED); 
    CURRENT_PID = -1;
    close(fd);
    close(fd2);
    dup2(ori_fd_in, fd);
    dup2(ori_fd_out, fd2);
    break;

  case 5: // < in >> append
      ptr_cmd = strtok(command, "<");
      extract_command(ptr_cmd, cmd_split, cmd_array);
      strcpy(filename, cmd_split[1]);
      strcpy(filename2, cmd_split[3]);
      if((fd = open(filename, O_RDONLY, 0644)) < 0)
      {
        fprintf(stderr, "Error: invalid file\n");
        return -2;
      }

      if((fd2 = open(filename2, O_WRONLY | O_APPEND, 0644)) < 0)
      {
        fprintf(stderr, "Error: invalid file\n");
        return -2;
      }
      ori_fd_in = dup(STDIN_FILENO);
      ori_fd_out = dup(STDOUT_FILENO);
      pid = fork();
      if ( pid < 0 )
      {
        fprintf(stderr, "Error: fork()\n");
        return -1;
      }          
      CURRENT_PID = pid;      
      if(pid == 0)
      {      
        dup2(fd, STDIN_FILENO);
        close(fd);

        dup2(fd2, STDOUT_FILENO); // send 'out' the content of terminal to a file
        close(fd2);

        run_execve(cmd_array[0], cmd_array);
      }
      waitpid(-1, NULL, WUNTRACED);
      CURRENT_PID = -1;
      close(fd);
      close(fd2);
      dup2(ori_fd_in, fd);
      dup2(ori_fd_out, fd2);
      break;

  default:
    break;
  }
  return 1;
}

int simulate_pipe(char command[], int pipe_count, char *to_pipe[])
{
  CURRENT_COMMAND = strdup(command);
  int index = 0;
  int fd[pipe_count][2];
  int store_pid[pipe_count+1];
  char *cmd_array[100];
  int fd_in, fd_out, ori_stdin, ori_stdout;
  int in_redirect=0, out_redirect=0;

  for(int i = 0; i < pipe_count; i++) {
    if (pipe(fd[i]) == -1)
    {
      // close all the opened fds till now
      for(int m = 0; m <= i-1; m++) {
        close(fd[m][0]);
        close(fd[m][1]);
      }
      fprintf(stderr, "Error: pipe\n");
      return -1;
    }
  }

  // read 
  in_redirect = if_has_redirection(to_pipe[index]);
  if (in_redirect == 1)
  {
    char *ptr_cmd;
    char filename[100];
    char *cmd_split[100];

    ori_stdin = dup(STDIN_FILENO);
    ptr_cmd = strtok(to_pipe[index], "<");
    extract_command(ptr_cmd, cmd_split, cmd_array);
    strcpy(filename, cmd_split[1]);
    if((fd_in = open(filename, O_RDONLY, 0644)) < 0)
    {
      fprintf(stderr, "Error: invalid file\n");
      return -2;
      dup2(ori_stdin, fd_in);
      close(ori_stdin);
    }
    dup2(fd_in, STDIN_FILENO);
    close(fd_in);
  }
  else
  {
    remove_space(cmd_array, to_pipe[index]);
  }

  int pid_0 = fork();
  store_pid[index] = pid_0;
  if (pid_0 < 0)
  {
    fprintf(stderr, "Error: fork()\n");
    return -1;
  }
  CURRENT_PID = pid_0;
  if(pid_0 == 0)
  {
    dup2(fd[0][1], STDOUT_FILENO);
    for(int i = 0; i < pipe_count; i++) {
      close(fd[i][0]);
      close(fd[i][1]);
    }
    run_execve(cmd_array[0], cmd_array);
  }
  CURRENT_PID = -1;
  clear_command(cmd_array);
  
  // intermediate
  for(int idx=1; idx<pipe_count; idx++)
  {
    index+=1;
    remove_space(cmd_array, to_pipe[index]);
    int pid_in = fork();
    store_pid[index] = pid_in;
    if (pid_in < 0)
    {
      fprintf(stderr, "Error: fork()\n");
      return -1;
    }
    CURRENT_PID = pid_in;
    if (pid_in == 0)
    {
      dup2(fd[index -1][0], STDIN_FILENO);
      dup2(fd[index][1], STDOUT_FILENO);
      for(int i = 0; i < pipe_count; i++) {
        close(fd[i][0]);
        close(fd[i][1]);
      }
      run_execve(cmd_array[0], cmd_array);
    }
    CURRENT_PID = -1;
    clear_command(cmd_array);
  }

  // write
  index+=1;
  out_redirect = if_has_redirection(to_pipe[index]);
  if (out_redirect > 1)
  {
    char *ptr_cmd;
    char filename[100];
    char *cmd_split[100];

    ori_stdout = dup(STDOUT_FILENO);
    ptr_cmd = strtok(to_pipe[index], ">");
    extract_command(ptr_cmd, cmd_split, cmd_array);
    if (out_redirect == 2) 
    {
      strcpy(filename, cmd_split[1]);
      if((fd_out = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0)
      {
        fprintf(stderr, "Error: invalid file\n");
        return -2;
      }
    }
    else if (out_redirect == 3)
    {
      strcpy(filename, cmd_split[2]);
      if((fd_out = open(filename, O_WRONLY | O_APPEND, 0644)) < 0)
      {
        fprintf(stderr, "Error: invalid file\n");
        return -2;
        dup2(ori_stdout, fd_out);
        close(ori_stdout);
      }
    }
    dup2(fd_out, STDOUT_FILENO);
    close(fd_out);
  }
  else
  {
    remove_space(cmd_array, to_pipe[index]);
  }
  int pid_pc = fork();
  store_pid[index] = pid_pc;
  if (pid_pc < 0)
  {
    fprintf(stderr, "Error: fork()\n");
    return -1;
  }
  CURRENT_PID = pid_pc;
  if (pid_pc == 0)
  {
    dup2(fd[index-1][0], STDIN_FILENO);
    for(int i = 0; i < pipe_count; i++) {
      close(fd[i][0]);
      close(fd[i][1]);
    }
    run_execve(cmd_array[0], cmd_array);
  }
  CURRENT_PID = -1;
  clear_command(cmd_array);

  // restore original fds
  if (in_redirect == 1)
  {
    dup2(ori_stdin, STDIN_FILENO);
    close(ori_stdin);
  }
  if (out_redirect > 1)
  {
    dup2(ori_stdout, STDOUT_FILENO);
    close(ori_stdout);
  }

  // close all the fds in parent
  for(int i = 0; i < pipe_count; i++) {
    close(fd[i][0]);
    close(fd[i][1]);
  }
  run_waitpid(pipe_count, store_pid);
  return 1;
}

int locate_program(char command[])
{
  CURRENT_COMMAND = strdup(command);
  char *ptr_cmd;
  char *cmd_split[100], *cmd_array[100];

  ptr_cmd = strtok(command, " ");
  extract_command(ptr_cmd, cmd_split, cmd_array);
  if((cmd_array[0][0] != '.') && (cmd_array[0][1] != '/'))
  {
    char path[100] = "./";
    strcat(path, cmd_array[0]);
    cmd_array[0] = path;
  }

  int pid = fork();
  if ( pid < 0 )
  {
    fprintf(stderr, "Error: fork()\n");
    return -1;
  }       
  CURRENT_PID = pid;      
  if(pid == 0)
  {
    if (execvp(cmd_array[0], cmd_array) == -1)
    {
      fprintf(stderr, "Error: invalid program\n");
      exit(-1);
    }
  }  
  waitpid(-1, NULL, WUNTRACED); 
  CURRENT_PID = -1;
  return 1;
}
