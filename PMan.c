/* Reference:
  http://www.learn-c.org/en/Linked_lists
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
  int status; // status of process: 0 - terminated, 1 - running, 2 - stopped
  struct node* next;
} process_node;

//int process_count = 0;  //global variable counting # of process
//process_node* process_list[MAX_PROCESS_SIZE];

// process list head
process_node* head = NULL;

char* status_array[] = {
  "terminated",
  "running",
  "stopped"
};


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
            /********* remove process from process list **********/
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

int main() {

  while(1) {
    /******************update process status***************/
    update_process_status();

    /* user input format:
     * bg cmd para1 para2 ... (there could be no parameters)
     * bglist
     * bgkill pid
     * bgstart pid
     * bgstop pid
     * pstat pid
     */

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
    command[cmd_length] = NULL;
    // int i;
    // for (i=0; i<MAX_INPUT_SIZE; i++) {
    //   printf("token: %s\n", command[i]);
    // }

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

          // add process into list
          // process list is an array of process_node
          // process_list[process_count] = malloc(sizeof(process_node));
          // process_list[process_count] -> process = command[1];
          // process_list[process_count] -> pid = pid;
          // process_list[process_count] -> isRunning = true;
          // process_count++;

          // create a process node
          process_node* p = (process_node*)malloc(sizeof(process_node));
          p->pid = pid;
          p->process = command[1];
          p->status = 1;  // running
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
          printf("Error: failed to fork\n");
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
          // TODO: need to add process status into bglist
          printf("%d: %s\n", proc_count_ptr->pid, proc_count_ptr->process);
          proc_count_ptr = proc_count_ptr->next;
        }
        printf("Total background jobs: %d\n", count);
      }
    }

    /*******************bgkill***************/
    else if (strcmp(command[0],"bgkill") == 0) {
      if (cmd_length != 2) {
        printf("Error: Input is not in the desired format 'bgkill pid'\n");
        continue;
      } else {
        pid_t pid = atoi(command[1]); // integer
        kill(pid, SIGTERM);
        printf("Process %d has been terminated\n", pid);
      }
    }

    /*******************bgstop***************/
    else if (strcmp(command[0],"bgstop") == 0) {
      if (cmd_length != 2) {
        printf("Error: Input is not in the desired format 'bgstop pid'\n");
        continue;
      } else {
        pid_t pid = atoi(command[1]); // integer
        kill(pid, SIGSTOP);
        printf("Process %d has been stopped\n", pid);
      }
    }

    /*******************bgstart***************/
    else if (strcmp(command[0],"bgstart") == 0) {
      if (cmd_length != 2) {
        printf("Error: Input is not in the desired format 'bgstart pid'\n");
        continue;
      } else {
        pid_t pid = atoi(command[1]); // integer
        kill(pid, SIGCONT);
        printf("Process %d has been resumed\n", pid);
      }
    }

    /*******************pstat***************/
    else if (strcmp(command[0],"pstat") == 0) {
      
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
