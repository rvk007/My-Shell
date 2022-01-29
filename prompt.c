#include "prompt.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void prompt_user(){
  char cwd[256];  
  char *dir[100], *pcwd;
  int i = 0;
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    pcwd = strtok(cwd, "/");
    while( pcwd != NULL)
    {
      dir[i++] = strdup(pcwd);
      pcwd = strtok(NULL, "/");
    } 
  }
  printf("[nyush %s]$ ",dir[i-1]);
  fflush(stdout);
}

