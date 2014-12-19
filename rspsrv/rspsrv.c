#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "rspsrv.h"
#include "fifo.h"


extern void *controller(void *);
extern void *timeControl(void *);
extern void *jsonServer(void *);


pthread_mutex_t raspyMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t timeMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
char gTimeStr[18];

enum globMode gMode;
enum globCommand gCommand;

int main (void) {
  static pthread_t conThread, timerThread, jsonThread;

  gMode = automaticMode;
  
  fifoInit();

  /* Start the timer thread */
  if (pthread_create (&conThread, NULL, &controller, NULL) != 0) {
    printf("cannot create task\n");
    exit(EXIT_FAILURE);
  }

  /* start a detached thread */
  if (pthread_create (&timerThread, NULL, &timeControl, NULL) != 0) {
    printf("cannot create task\n");
    exit(EXIT_FAILURE);
  }

  /* Start the json server thread */
  if (pthread_create (&jsonThread, NULL, &jsonServer, NULL) != 0) {
    printf("cannot create task\n");
    exit(EXIT_FAILURE);
  }

  pthread_join(conThread, NULL);

  return EXIT_SUCCESS;
}

