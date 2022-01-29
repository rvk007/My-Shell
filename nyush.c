/***

References:
https://www.ibm.com/docs/en/zos/2.3.0?topic=functions-getcwd-get-path-name-working-directory
https://stackoverflow.com/questions/5367068/clear-a-terminal-screen-for-real/5367075#5367075
https://iq.opengenus.org/implementing-cd-command-in-c/
https://www.youtube.com/watch?v=PIb2aShU_H4
https://stackoverflow.com/questions/20437988/using-a-word-as-a-delimiter-for-strtok
https://stackoverflow.com/questions/7136416/opening-file-in-append-mode-using-open-api
https://www.cs.utexas.edu/~theksong/2020/243/Using-dup2-to-redirect-output/
https://man7.org/linux/man-pages/man2/open.2.html
https://www.youtube.com/watch?v=6xbLgZpOBi8
https://www.youtube.com/watch?v=NkfIUo_Qq4c
https://stackoverflow.com/questions/1716296/why-does-printf-not-flush-after-the-call-unless-a-newline-is-in-the-format-strin
https://www.tutorialspoint.com/c_standard_library/c_function_strcat.htm
https://linux.die.net/man/3/execvp
https://github.com/pranav93y/myshell/blob/master/lib/myshell.c
https://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c
https://www.csl.mtu.edu/cs4411.ck/www/NOTES/signal/handler.html
https://stackoverflow.com/questions/64165433/different-ways-to-ignore-a-signal
https://condor.depaul.edu/glancast/374class/hw/shlab-readme.html
https://man7.org/linux/man-pages/man2/kill.2.html
https://www.geeksforgeeks.org/write-a-c-program-that-doesnt-terminate-when-ctrlc-is-pressed/
https://superuser.com/questions/667380/sigint-and-sigtstp-ignored-by-most-common-applications
https://www.gnu.org/software/libc/manual/html_node/Basic-Signal-Handling.html
https://stackoverflow.com/questions/9296949/how-to-suspend-restart-processes-in-c-linux
https://www.youtube.com/watch?v=3MZjaZxZYrE
https://www.youtube.com/watch?v=4xN6pdUXj14
Student: Shantanu Acharya (N10710714)

***/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "prompt.h"
#include "read_command.h"
#include "run_command.h"

int main() {
  char command[1001];
  int result[2];
  char *to_pipe[1000];
  int c_id=0;

  signal(SIGINT, signal_handler); 
  signal(SIGQUIT, signal_handler); 
  signal(SIGTERM, signal_handler); 
  signal(SIGTSTP, signal_handler); 

  while(1){
    prompt_user(); 
    c_id = get_command(command);
    if(c_id>0)
    {
      if(read_command(command, result, to_pipe) == -1)
      {
        continue;
      }
      int redirect = result[0];
      int pipe = result[1];
      if (command[0])
      {
        if (redirect == 6)
        {
          locate_program(command);
          continue;
        }
        if (pipe>0){
          simulate_pipe(command,pipe, to_pipe);
          continue;
        }
        if(redirect>0)
        {
          redirection_commands(command, redirect);
          continue;
        }
        else
        {
          built_in_commands(command);
          continue;
        }
      }
    } 
    if(c_id == -1)
    {
      puts("");
      return 0;
    }
  }
}
