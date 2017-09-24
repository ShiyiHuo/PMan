// References: http://www.learn-c.org/en/Linked_lists


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


#define MAX_INPUT_SIZE 10

// Linked list node for process
typedef struct node {
  pid_t pid;  //process id
  char* process;  // process: command and its argument
  bool isRunning; // if a process is running
  struct node* next;
} proc_node;



// Linked list method if necessary


// commands

void bg() {
  return;
}

void bglist() {
  return;
}

void bgkill(pid_t pid) {
  return;
}

void bgstop(pid_t pid) {
  return;
}

void bgstart(pid_t pid) {
  return;
}

void pstat(pid_t pid) {
  return;
}



int main() {
  /*
  while(1) {
    waitpid();
    getline();
    if (command 1)
    if (command 2)

  }
  */
  char* input = readline("please give some input: \n");
  char* token = strtok(input," ");
  char* cmd_para_array[MAX_INPUT_SIZE];
  int count = 0;

  while (token != NULL) {
    cmd_para_array[count] = token;
    token = strtok(NULL, " ");
    count++;
  }

  int i;
  for (i=0; i<MAX_INPUT_SIZE; i++) {
    printf("token: %s\n", cmd_para_array[i]);
  }

  free(input);
  free(token);
  input = NULL;
  token = NULL;


  return 0;
}
