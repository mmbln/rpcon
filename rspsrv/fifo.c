#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct message {
  char *command;
  char *parameter;
  struct message *next;
} *queue_start;
int queue_length;


extern void fifoInit()
{
  queue_start = NULL;
  queue_length = 0;
}


extern int putMessage(char *cmd, char *params)
{
  struct message *msgPtr;

  struct message *newMsg = malloc(sizeof(struct message));
  if (newMsg == NULL) {
    printf("Exit, not enough memory\n");
    exit(1);
  }
  newMsg->command = malloc(strlen(cmd)+1);
  if (newMsg->command == NULL) {
    printf("Exit, not enough memory\n");
    exit(1);
  }
  strcpy(newMsg->command, cmd);

  if (params) {
    newMsg->parameter = malloc(strlen(params)+1);
    if (newMsg->parameter == NULL) {
      printf("Exit, not enough memory\n");
      exit(1);
    }
    strcpy(newMsg->parameter, params);
  }
  else {
    newMsg->parameter = NULL;
  }
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


extern int getMessage(char *cmd, char *params)
{
  struct message *msgPtr;

  /* you cannot get an entry from an empty list */
  if (queue_start == NULL) {
    printf("Warning; list is empty.\n");
    *cmd = *params = '\0';
    return(0);
  }    
  
  /* find the last entry */
  msgPtr = queue_start;
  while (msgPtr->next)
    msgPtr = msgPtr->next;

  /* copy the entry */
  strcpy(cmd, msgPtr->command);
  if (msgPtr->parameter)
    strcpy(params, msgPtr->parameter);
  else
    *params = '\0';
  
  /* free the memory */
  /*   find the entry before the last entry */
  if (msgPtr == queue_start) {
    queue_start = NULL;
    free(msgPtr->command);
    if (msgPtr->parameter)
      free(msgPtr->parameter);
    free(msgPtr);
  } else {
    msgPtr = queue_start;
    while (msgPtr->next->next) 
      msgPtr = msgPtr->next;
    free(msgPtr->next->command);
    if (msgPtr->next->parameter)
      free(msgPtr->next->parameter);
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
  char buf1[128], buf2[128];
  int cnt;

  if (argc != 2) {
    usage(argv[0]);
  }
  
  fifoInit();
  if (strcmp(argv[1], "1") == 0) {
    printf("putMessage\n");
    putMessage("command1", "parameter1");
    printf("getMessage\n");
    getMessage(buf1,buf2);
    printf("buf1: %s, buf2: %s\n", buf1, buf2);
  }
  else if (strcmp(argv[1], "2") == 0) {
    for (cnt=0; cnt < 10; cnt++) {
      sprintf(buf1, "cmd%d", cnt);
      sprintf(buf2, "param%d", cnt);
      putMessage(buf1, buf2);
    }
    for (cnt=0; cnt < 10; cnt++) {
      getMessage(buf1, buf2);
      printf("Cnt: %d, buf1: %s, buf2: %s.\n", cnt, buf1, buf2);
    }
  }
  else if (strcmp(argv[1], "3") == 0) {
    for (cnt=0; cnt < 10; cnt++) {
      sprintf(buf1, "cmd%d", cnt);
      sprintf(buf2, "param%d", cnt);
      putMessage(buf1, buf2);
    }
    for (cnt=0; cnt < 11; cnt++) {
      getMessage(buf1, buf2);
      printf("Cnt: %d, buf1: %s, buf2: %s.\n", cnt, buf1, buf2);
    }
  }
  else if (strcmp(argv[1], "4") == 0) {
    getMessage(buf1,buf2);
    printf("buf1: %s, buf2: %s\n", buf1, buf2);
  }
  else if (strcmp(argv[1], "5") == 0) {
    putMessage("cmd", NULL);
    getMessage(buf1,buf2);
    printf("buf1: %s, buf2: %s\n", buf1, buf2);
  }
  else {
    /*  */
    usage(argv[0]);
  }
}

#endif
