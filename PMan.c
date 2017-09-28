/* Reference:
  http://www.learn-c.org/en/Linked_lists
  https://www.cs.bu.edu/teaching/c/file-io/intro/
*/


#include <stdio.h>             // printf()
#include <string.h>            // strcmp()
#include <readline/readline.h> // readline
#include <ctype.h>             // isdigit()
#include <stdlib.h>            // malloc()
#include <sys/types.h>         // pid_t
#include <sys/wait.h>          // waitpid()
#include <unistd.h>            // fork(), execvp()
#include <signal.h>            // kill(), SIGTERM, SIGSTOP, SIGCONT
#include <stdbool.h>          //boolean


#define MAX_INPUT_SIZE 9999
#define MAX_PROCESS_SIZE 9999

// Linked list node for process
typedef struct node {
  pid_t pid;  //process id
  char* process;  // process: command and its argument
  //int status; // status of process: 0 - terminated, 1 - running, 2 - stopped
  struct node* next;
} process_node;

// process list head
process_node* head = NULL;


void update_process_status() {
  pid_t pid;
  int p_status;
  while (1) {
    pid = waitpid(-1, &p_status, WNOHANG);
    if (pid > 0) {  //if child process exits
      printf("Process %d has been terminated\n", pid);

      // check if pid is in process list: if yes, remove it
      process_node* curr = head;
      while (curr != NULL) {
          if (curr->pid ==pid) {
            /********* remove process by pid **********/
            // TODO: more modification!!!!!
            process_node* temp1 = head;
            process_node* temp2 = NULL;

            while (temp1 != NULL) {
              if (temp1->pid == pid) {
                if (temp1 == head) {
                  head = head->next;
                } else {
                  temp2->next = temp1->next;
                }
                free(temp1);
                break;
              }
              temp2 = temp1;
              temp1 = temp1->next;
            }
          }
          curr = curr->next;

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

int main() {

  while(1) {
    /******************update process status***************/
    update_process_status();


    /*******************read in user input*****************/
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



    /* user input format:
     * bg cmd para1 para2 ... (there could be no parameters)
     * bglist
     * bgkill pid
     * bgstart pid
     * bgstop pid
     * pstat pid
     */


    /********************bg*********************/
    if (strcmp(command[0],"bg") == 0) {
      if (cmd_length < 2) {
        printf("Error: Input is not in the desired format 'bg cmd para1 para2 ...'\n");
        continue;
      } else {  //cmd_length >= 2
        //execute bg
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
          continue;
        }
        else if (pid == 0) {
          /* in child process */
          //char * args[] = {"./inf" ,"a", "1", NULL};
          // use command+1 to get cmd and its parameters
          if(execvp(command[1], command+1) == -1) {
            printf("Error: Cannot execute command %s\n", command[1]);
            continue;
          }
        } else {
          /* failure */
          printf("Error: Failed to fork\n");
          continue;
        }
      }

    }

    /*******************bglist***************/
    else if (strcmp(command[0],"bglist") == 0) {
      if (cmd_length != 1) {
        printf("Error: Input is not in the desired format 'bglist'\n");
        continue;
      } else {
        int count = 0;
        process_node* proc_count_ptr = head;

        while (proc_count_ptr != NULL) {
          count ++;
          printf("%d: %s\n", proc_count_ptr->pid, proc_count_ptr->process);
          proc_count_ptr = proc_count_ptr->next;
        }
        printf("Total background jobs: %d\n", count);
      }
    }

    // kill() returns 0 if successes and returns -1 for fails

    /*******************bgkill***************/
    else if (strcmp(command[0],"bgkill") == 0) {
      if (cmd_length != 2) {
        printf("Error: Input is not in the desired format 'bgkill pid'\n");
        continue;
      } else {
        pid_t pid = atoi(command[1]); // integer
        if (process_exists(pid)) {
          if (kill(pid, SIGTERM) == 0) {  // if succeeds
            printf("Process %d has been terminated\n", pid);
          } else {
            printf("Error: Failed to kill process %d\n", pid);
          }
        } else {
          printf("Error: Process %d does not exist\n", pid);
          continue;
        }
      }
    }

    /*******************bgstop***************/
    else if (strcmp(command[0],"bgstop") == 0) {
      if (cmd_length != 2) {
        printf("Error: Input is not in the desired format 'bgstop pid'\n");
        continue;
      } else {
        pid_t pid = atoi(command[1]); // integer
        if (process_exists(pid)) {
          if (kill(pid, SIGSTOP) == 0) {  // if succeeds
            printf("Process %d has been stopped\n", pid);
          } else {
            printf("Error: Failed to kill process %d\n", pid);
          }
        } else {
          printf("Error: Process %d does not exist\n", pid);
          continue;
        }
      }
    }

    /*******************bgstart***************/
    else if (strcmp(command[0],"bgstart") == 0) {
      if (cmd_length != 2) {
        printf("Error: Input is not in the desired format 'bgstart pid'\n");
        continue;
      } else {
        pid_t pid = atoi(command[1]); // integer
        if (process_exists(pid)) {
          if (kill(pid, SIGCONT) == 0) {  // if succeeds
            printf("Process %d has been resumed\n", pid);
          } else {
            printf("Error: Failed to kill process %d\n", pid);
          }
        } else {
          printf("Error: Process %d does not exist\n", pid);
          continue;
        }
      }
    }

    /************************************pstat**************************/
    /* pstat includes:
     * comm
     * state
     * utime
     * stime
     * rss
     * voluntary_ctxt_switches
     * nonvoluntary_ctxt_switches
     */

    // TODO: segmentation fault!!!!!!!!!!!!!!!!!!!!!

    else if (strcmp(command[0],"pstat") == 0) {
      if (cmd_length != 2) {
        printf("Error: Input is not in the desired format 'pstat pid'\n");
        continue;
      } else {
        pid_t pid = atoi(command[1]); // integer
        printf("%d\n", pid);
        if (process_exists(pid)) {
          // open files and read data
          FILE *file_status;
          FILE *file_stat;
          char filepath_status[MAX_INPUT_SIZE];
          char filepath_stat[MAX_INPUT_SIZE];
          sprintf(filepath_status,"/proc/%d/status",pid);
          sprintf(filepath_stat,"/proc/%d/status",pid);

          /***************read stat into stat_content***************/
          // tokenize file
          file_stat = fopen(filepath_stat,"r");
          char* stat_content[MAX_INPUT_SIZE];
          char file_content[1024];
          if (file_stat == NULL) {
            printf("Error: Cannot open input file pstat\n");
            exit(1);
          } else {
            int count_stat=0;
            while (fgets(file_content,sizeof(file_content)-1,file_stat) != NULL) {
              char* token = strtok(file_content," ");
              stat_content[count_stat] = token;  // tokenized user input
              while (token != NULL) {
                stat_content[count_stat] = token;
                token = strtok(NULL, " ");
                count_stat++;
              }
            }
          }


          /***************read status into status_content************/
          file_status = fopen(filepath_status,"r");
          char* status_content[MAX_INPUT_SIZE];
          if (file_status == NULL) {
            printf("Error: Cannot open input file pstatus\n");
            exit(1);
          } else {
            int count_status=0;
            while (fgets(status_content[count_status], MAX_INPUT_SIZE, file_status)) {
              count_status++;
            }
          }

          /**************output stat and status content**************/
          // TODO: utime and stime
          printf("comm: %s\n", stat_content[1]);
          printf("state: %s\n", stat_content[2]);
          printf("utime: ");
          printf("stime: ");
          printf("rss: %s\n", stat_content[24]);
          printf("%s", status_content[39]);
          printf("%s", status_content[40]);


        } else {
          printf("Error: Process %d does not exist\n", pid);
          continue;
        }
      }
    }

    /*************command not found**********/
    else {
      printf("Error: Command is not found");
      continue;
    }

    update_process_status();  //ensure process status is updated
  }

  return 0;
}
