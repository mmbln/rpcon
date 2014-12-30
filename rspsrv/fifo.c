#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fifo.h"




struct message {
  int command;
  int parameter;
  struct message *next;
} *queue_start;
int queue_length;


extern void fifoInit()
{
  queue_start = NULL;
  queue_length = 0;
}


extern int putMessage(int cmd, int params)
{
  struct message *msgPtr;

  struct message *newMsg = malloc(sizeof(struct message));
  if (newMsg == NULL) {
    printf("Exit, not enough memory\n");
    exit(1);
  }
  newMsg->command = cmd;
  newMsg->parameter = params;
  newMsg->next = NULL;

  if (queue_start == NULL) {
    /* a entry into an empty list */
    queue_start = newMsg;
  } else {
    /* find the end */
    msgPtr = queue_start;
    while (msgPtr->next)
      msgPtr = msgPtr->next;
    msgPtr->next = newMsg;
  }
  queue_length++;
  return (queue_length);
}


extern int getMessage(int *cmd, int *params)
{
  struct message *msgPtr;

  /* you cannot get an entry from an empty list */
  if (queue_start == NULL) {
    printf("Warning; list is empty.\n");
    *cmd = 0;
    return(0);
  }    
  
  /* find the last entry */
  msgPtr = queue_start;
  while (msgPtr->next)
    msgPtr = msgPtr->next;

  /* copy the entry */
  *cmd = msgPtr->command;
  *params = msgPtr->parameter;

  /* free the memory */
  /*   find the entry before the last entry */
  if (msgPtr == queue_start) {
    queue_start = NULL;
    free(msgPtr);
  } else {
    msgPtr = queue_start;
    while (msgPtr->next->next) 
      msgPtr = msgPtr->next;
    free(msgPtr->next);
    msgPtr->next = NULL;
  }
  queue_length--;
  return (queue_length+1);
}


#ifdef UNITTEST

void usage(char *prog) {
  printf("Call: %s PARAM\n\n", prog);
  printf("  PARAM: 1\n");
  printf("    Write one entry into the list and remove one entry.\n\n");
  printf("  PARAM: 2\n");
  printf("    Write 10 entries into the list and remove 10 entries.\n\n");
  printf("  PARAM: 3\n");
  printf("    Write 10 entries into the list and remove 11 entries.\n\n");
  printf("  PARAM: 4\n");
  printf("    Try to remove 1 entry from the list.\n\n");
  printf("  PARAM: 5\n");
  printf("    Write one command with an empty parameter into the list.\n\n");
  exit(1);
}

int main(int argc, char *argv[])
{
  char buf1[128];
  int a1,a2;
  int cnt;

  if (argc != 2) {
    usage(argv[0]);
  }
  
  fifoInit();
  if (strcmp(argv[1], "1") == 0) {
    printf("putMessage\n");
    putMessage(17, 1);
    printf("getMessage\n");
    getMessage(&a1,&a2);
    printf("Cmd: %d, parameter: %d\n", a1, a2);
  }
  else if (strcmp(argv[1], "2") == 0) {
    for (cnt=0; cnt < 10; cnt++) {
      putMessage(cnt+100, cnt);
    }
    for (cnt=0; cnt < 10; cnt++) {
      getMessage(&a1, &a2);
      printf("Cnt: %d, Cmd: %d, Parameter %d.\n", cnt, a1, a2);
    }
  }
  else if (strcmp(argv[1], "3") == 0) {
    for (cnt=0; cnt < 10; cnt++) {
      putMessage(cnt+100, cnt);
    }
    for (cnt=0; cnt < 11; cnt++) {
      getMessage(&a1, &a2);
      printf("Cnt: %d, cmd: %d, param: %d.\n", cnt, a1, a2);
    }
  }
  else if (strcmp(argv[1], "4") == 0) {
    getMessage(&a1, &a2);
    printf("cmd: %d, param: %d.\n", a1, a2);
  }
  else if (strcmp(argv[1], "5") == 0) {
    putMessage(110, 10);
    getMessage(&a1, &a2);
    printf("cmd: %d, param: %d\n", a1, a2);
  }
  else {
    /*  */
    usage(argv[0]);
  }
}

#endif
