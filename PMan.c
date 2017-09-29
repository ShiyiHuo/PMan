/* Reference
 * Linked list:
   http://www.learn-c.org/en/Linked_lists
   https://www.cs.bu.edu/teaching/c/linked-list/delete/
 * read pstat
   http://man7.org/linux/man-pages/man5/proc.5.html
*/


#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>


#define MAX_INPUT_SIZE 9999
#define MAX_PROCESS_SIZE 9999

// Linked list node for process
typedef struct node {
  pid_t pid;  //process id
  char* process;  // process: command and its argument
  struct node* next;
} process_node;

process_node* head = NULL;    // process list head


void update_process_status() {
  pid_t pid;
  int p_status;
  while (1) {
    pid = waitpid(-1, &p_status, WNOHANG);
    if (pid > 0) {  //if child process exits, remove it
      process_node *currP, *prevP;
      prevP = NULL;

      for (currP = head; currP != NULL;	prevP = currP, currP = currP->next) {
        if (currP->pid == pid) {  /* Found it. */
          if (prevP == NULL) {
             head = currP->next;
          } else {
            prevP->next = currP->next;
          }
          free(currP);
        }
      }
    } else {
      break;
    }

  }
}

/* to check if a process exists in process list
 * return 1 if the process exists
 * return 0 if the process does not exist
 */
int process_exists(pid_t pid) {
  process_node * curr = head;
    while (curr != NULL) {
      if (curr->pid == pid) {
        return 1;
      }
      curr = curr->next;
    }
    return 0;
}

void bg(char* command[]) {
  pid_t pid=fork();
  if (pid > 0) {
    /* in parent process */
    printf("Running process with pid %d\n", pid);

    // create a process node
    process_node* p = (process_node*)malloc(sizeof(process_node));
    p->pid = pid;
    p->process = command[1];
    p->next = NULL;

    // add process into list
    if (head == NULL) {
      head = p;
    } else {
      process_node* current = head;
      while (current->next != NULL) {
        current = current->next;
      }
      current->next = p;
    }
    return;
  }
  else if (pid == 0) {
    /* in child process */
    //char * args[] = {"./inf" ,"a", "1", NULL};
    // use command+1 to get cmd and its parameters
    if(execvp(command[1], command+1) == -1) {
      printf("Error: Cannot execute command %s\n", command[1]);
      return;
    }
  } else {
    /* failure */
    printf("Error: Failed to fork\n");
    return;
  }
}

// display a list of all the programs currently executing in the background
void bglist() {
  int count = 0;
  process_node* proc_count_ptr = head;

  while (proc_count_ptr != NULL) {
    count ++;
    printf("%d: %s\n", proc_count_ptr->pid, proc_count_ptr->process);
    proc_count_ptr = proc_count_ptr->next;
  }
  printf("Total background jobs: %d\n", count);
}

// terminate process <pid>
void bgkill(pid_t pid) {
  if (process_exists(pid)) {
    // kill() returns 0 if successes and returns -1 for fails
    if (kill(pid, SIGTERM) == 0) {
      printf("Process %d has been terminated\n", pid);
      usleep(1000);   // find the zombie and clean the zombie process
    } else {
      printf("Error: Failed to kill process %d\n", pid);
    }
  } else {
    printf("Error: Process %d does not exist\n", pid);
    return;
  }
}

// temporarily stop process <pid>
void bgstop(pid_t pid) {
  if (process_exists(pid)) {
    if (kill(pid, SIGSTOP) == 0) {  // if succeeds
      printf("Process %d has been stopped\n", pid);
    } else {
      printf("Error: Failed to kill process %d\n", pid);
    }
  } else {
    printf("Error: Process %d does not exist\n", pid);
    return;
  }
}

// restart process <pid> which has been previously stopped
void bgstart(pid_t pid) {
  if (process_exists(pid)) {
    if (kill(pid, SIGCONT) == 0) {  // if succeeds
      printf("Process %d has been resumed\n", pid);
    } else {
      printf("Error: Failed to kill process %d\n", pid);
    }
  } else {
    printf("Error: Process %d does not exist\n", pid);
    return;
  }
}

void pstat(pid_t pid) {
  if (process_exists(pid)) {
    /********************read stat*******************/
    /* field:       comm   state   utime   stime   rss
     * position:    2      3       14      15      24
     */
    FILE *file_stat;
    char filepath_stat[MAX_INPUT_SIZE];   // path of stat file
    sprintf(filepath_stat,"/proc/%d/stat",pid);
    file_stat = fopen(filepath_stat,"r");

    int count_stat = 0;
    char stat_input[1024];

    if (file_stat == NULL) {
      printf("Error: Cannot open input file pstat\n");
      continue;
    } else {

      while (fgets(stat_input,sizeof(stat_input)-1,file_stat) != NULL) {
        char* token_stat;
        token_stat = strtok(stat_input," ");
        while (token_stat != NULL) {
          count_stat++;
          //printf("pos: %d,\t%s\n", count_stat, token_stat);
          if (count_stat == 1)  printf("pid: %s\n", token_stat);
          else if (count_stat == 2) printf("comm: %s\n", token_stat);  // filename of the executable, in parenthese
          else if (count_stat == 3) printf("state: %s\n", token_stat);  //eg. R (Running)
          else if (count_stat == 14)  printf("utime: %lf\n", (atof(token_stat))/sysconf(_SC_CLK_TCK)); // time scheduled in user mode
          else if (count_stat == 15)  printf("stime: %lf\n", (atof(token_stat))/sysconf(_SC_CLK_TCK)); // time scheduled in kernel mode
          else if (count_stat == 24)  printf("rss: %s\n", token_stat); // VmRSS

          token_stat = strtok(NULL, " ");
        }
      }
      fclose(file_stat);
    }


    /*********************read status****************/
    FILE *file_status;
    char filepath_status[MAX_INPUT_SIZE];
    sprintf(filepath_status,"/proc/%d/status",pid);
    file_status = fopen(filepath_status,"r");

    char status_input[1024];

    if (file_status == NULL) {
      printf("Error: Cannot open input file pstatus\n");
      continue;
    } else {
      while (fgets(status_input, sizeof(status_input)-1, file_status) != NULL) {
        char* token_status;

        // TODO: modification needed!!!!!!!!!!!!
        token_status = strtok(status_input," :");
        if (token_status != NULL) {
          char* str1 = "voluntary_ctxt_switches";
          char* str2 = "nonvoluntary_ctxt_switches";
          if (strcmp(str1, token_status) == 0) {
            token_status = strtok(NULL, " :\n");
            printf("%s: %s\n", str1, token_status);
          }
          if (strcmp(str2, token_status) == 0) {
            token_status = strtok(NULL, " :\n");
            printf("%s: %s\n", str2, token_status);
          }
        }
      }

      fclose(file_status);
    }

  } else {
    printf("Error: Process %d does not exist\n", pid);
    continue;
  }
}


int main() {

  while(1) {
    /*****************************read in user input**************************/
    /* user input format:
     * bg cmd para1 para2 ... (there could be no parameters)
     * bglist
     * bgkill pid
     * bgstart pid
     * bgstop pid
     * pstat pid
     */
    char* input = readline("PMan: > ");
    char* token = strtok(input," ");
    char* command[MAX_INPUT_SIZE];  // tokenized user input
    int cmd_length = 0; // user input length

    while (token != NULL) {
      command[cmd_length] = token;
      token = strtok(NULL, " ");
      cmd_length++;
    }
    //fix for segmentation fault: ensure the last item in array is null
    command[cmd_length] = NULL;


    if (strcmp(command[0],"bg") == 0) {
      if (cmd_length < 2) {
        printf("Error: Input is not in the desired format 'bg cmd para1 para2 ...'\n");
        continue;
      } else {
        bg(command);
      }
    } else if (strcmp(command[0],"bglist") == 0) {
      if (cmd_length != 1) {
        printf("Error: Input is not in the desired format 'bglist'\n");
        continue;
      } else {
        bglist();
      }
    } else if (strcmp(command[0],"bgkill") == 0) {
      if (cmd_length != 2) {
        printf("Error: Input is not in the desired format 'bgkill pid'\n");
        continue;
      } else {
        pid_t pid = atoi(command[1]); // integer
        bgkill(pid);
      }
    } else if (strcmp(command[0],"bgstop") == 0) {
      if (cmd_length != 2) {
        printf("Error: Input is not in the desired format 'bgstop pid'\n");
        continue;
      } else {
        pid_t pid = atoi(command[1]); // integer
        bgstop(pid);
      }
    } else if (strcmp(command[0],"bgstart") == 0) {
      if (cmd_length != 2) {
        printf("Error: Input is not in the desired format 'bgstart pid'\n");
        continue;
      } else {
        pid_t pid = atoi(command[1]); // integer
        bgstart(pid);
      }
    }

    /************************************pstat******************************/
    else if (strcmp(command[0],"pstat") == 0) {
      if (cmd_length != 2) {
        printf("Error: Input is not in the desired format 'pstat pid'\n");
        continue;
      } else {
        pid_t pid = atoi(command[1]); // integer
        pstat(pid);

        // if (process_exists(pid)) {

        //   /********************read stat*******************/
        //   /* field:       comm   state   utime   stime   rss
        //    * position:    2      3       14      15      24
        //    */
        //   FILE *file_stat;
        //   char filepath_stat[MAX_INPUT_SIZE];   // path of stat file
        //   sprintf(filepath_stat,"/proc/%d/stat",pid);
        //   file_stat = fopen(filepath_stat,"r");

        //   int count_stat = 0;
        //   char stat_input[1024];

        //   if (file_stat == NULL) {
        //     printf("Error: Cannot open input file pstat\n");
        //     continue;
        //   } else {

        //     while (fgets(stat_input,sizeof(stat_input)-1,file_stat) != NULL) {
        //       char* token_stat;
        //       token_stat = strtok(stat_input," ");
        //       while (token_stat != NULL) {
        //         count_stat++;
        //         //printf("pos: %d,\t%s\n", count_stat, token_stat);
        //         if (count_stat == 1)  printf("pid: %s\n", token_stat);
        //         else if (count_stat == 2) printf("comm: %s\n", token_stat);  // filename of the executable, in parenthese
        //         else if (count_stat == 3) printf("state: %s\n", token_stat);  //eg. R (Running)
        //         else if (count_stat == 14)  printf("utime: %lf\n", (atof(token_stat))/sysconf(_SC_CLK_TCK)); // time scheduled in user mode
        //         else if (count_stat == 15)  printf("stime: %lf\n", (atof(token_stat))/sysconf(_SC_CLK_TCK)); // time scheduled in kernel mode
        //         else if (count_stat == 24)  printf("rss: %s\n", token_stat); // VmRSS

        //         token_stat = strtok(NULL, " ");
        //       }
        //     }
        //     fclose(file_stat);
        //   }


        //   /*********************read status****************/
        //   FILE *file_status;
        //   char filepath_status[MAX_INPUT_SIZE];
        //   sprintf(filepath_status,"/proc/%d/status",pid);
        //   file_status = fopen(filepath_status,"r");

        //   char status_input[1024];

        //   if (file_status == NULL) {
        //     printf("Error: Cannot open input file pstatus\n");
        //     continue;
        //   } else {
        //     while (fgets(status_input, sizeof(status_input)-1, file_status) != NULL) {
        //       char* token_status;

        //       // TODO: modification needed!!!!!!!!!!!!
        //       token_status = strtok(status_input," :");
        //       if (token_status != NULL) {
        //         char* str1 = "voluntary_ctxt_switches";
        //         char* str2 = "nonvoluntary_ctxt_switches";
        //         if (strcmp(str1, token_status) == 0) {
        //           token_status = strtok(NULL, " :\n");
        //           printf("%s: %s\n", str1, token_status);
        //         }
        //         if (strcmp(str2, token_status) == 0) {
        //           token_status = strtok(NULL, " :\n");
        //           printf("%s: %s\n", str2, token_status);
        //         }
        //       }
        //     }

        //     fclose(file_status);
        //   }


        // } else {
        //   printf("Error: Process %d does not exist\n", pid);
        //   continue;
        // }
      }
    }


    /*****************************command not found*********************/
    else {
      printf("Error: Command is not found\n");
      continue;
    }

    update_process_status();  // update process status after execution
  }

  return 0;
}
