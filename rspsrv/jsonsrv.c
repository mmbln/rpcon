
/* thserver.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <jansson.h>

#include "rspsrv.h"

#define  MAX_SIZE  1024
#define  PORT_NUMBER  6001

extern pthread_mutex_t timeMutex;
extern pthread_mutex_t raspyMutex;
extern pthread_cond_t cond;

extern char gTimeStr[];
extern enum globMode gMode;

static void * threading_socket( void *);

pthread_t th;

static void processCmd(const char *cmd, const char *param, int socket)
{
  char sendBuffer[MAX_SIZE];
  time_t t;
  struct tm *tm;

  if (strcmp(cmd, "getTime") == 0) {
    /* return the global time */
    pthread_mutex_lock(&timeMutex);
    sprintf(sendBuffer, "{\"result\": \"%s\"}\n", 
	    gTimeStr);
    pthread_mutex_unlock(&timeMutex);
  } else if (strcmp(cmd, "getSwitchState") == 0) {
    /* get the current state of the relais */
    sprintf(sendBuffer, "{\"result\": \"%s\"}\n", "true");
  } else if (strcmp(cmd, "getNextSwitchTimes") == 0) {
    /* get the next switch time */
    sprintf(sendBuffer, "{\"result\": \"%s\"}\n", "true");
  } else if (strcmp(cmd, "getMode") == 0) {
    /* get the current mode */
    sprintf(sendBuffer, "{\"result\": \"%s\"}\n", "true");
  } else if (strcmp(cmd, "setMode") == 0) {
    /* set the current mode */
    if (strcmp(param, "On") == 0) {
      /* switch on */
      pthread_mutex_lock(&raspyMutex);
      gMode = onMode;
      putMessage("switchOn", NULL);
      pthread_cond_signal(&cond);
      pthread_mutex_unlock(&raspyMutex);
      printf("On Mode\n");
    } else if (strcmp(param, "Off") == 0) {
      /* switch off */
      pthread_mutex_lock(&raspyMutex);
      gMode = offMode;
      putMessage("switchOff", NULL);
      pthread_cond_signal(&cond);
      pthread_mutex_unlock(&raspyMutex);
      printf("Off Mode\n");
    } else if (strcmp(param, "Automatic") == 0) {
      /* switch to automatic */
      pthread_mutex_lock(&raspyMutex);
      gMode = automaticMode;
      pthread_mutex_unlock(&raspyMutex);
      printf("Automatic Mode\n");
    } else {
      /* error */
      /* TODO */
    }
    sprintf(sendBuffer, "{\"result\": \"%s\"}\n", "true");
  } else {
    /* error unknown command */
    sprintf(sendBuffer, "{\"result\": \"unknown command\"}\n");
  }

  /* return the result */
  send(socket, sendBuffer, strlen(sendBuffer), 0); 
}

static void *threading_socket (void *arg) {
  int socket;
  char recvBuffer[MAX_SIZE];

  const char *cmd;
  const char *param;
  json_error_t jsonError;
  json_t *root;

  pthread_detach (pthread_self ());
  
  socket = (int)arg;
  recv(socket, recvBuffer, MAX_SIZE-1, 0); 

  root = json_loads(recvBuffer, 0, &jsonError);
  if (root) {
    if(json_unpack(root, "{s:s, s:s}", "cmd", &cmd, "param", &param) == 0) {
      processCmd(cmd, param, socket);
    } else {
      printf("JSON Strange Format: %s", recvBuffer);
    }
  } else {
    printf("No JSON sting: %s", recvBuffer);
  }    
  /* release memory */
  json_decref(root);
  close(socket);
  return NULL;
}

void *jsonServer(void *arg)
{
  int sockfd, connfd; 
  struct sockaddr_in address;
  socklen_t addLength = sizeof (struct sockaddr_in);
  const int y = 1;
  if ((sockfd = socket (PF_INET, SOCK_STREAM, 0)) < 0) { 
    printf ("Error at socket() ...(%s)\n",
	    strerror(errno));
    exit (EXIT_FAILURE);
  }
  address.sin_family = AF_INET;
  address.sin_port = htons (PORT_NUMBER);
  memset (&address.sin_addr, 0, sizeof (address.sin_addr));
  setsockopt( sockfd, SOL_SOCKET, 
	      SO_REUSEADDR, &y, sizeof(int));
  if (bind ( sockfd,
             (struct sockaddr *) &address,
             sizeof (address) ) ) {
    printf ("Error at bind() ...(%s)\n",
	    strerror(errno));
    exit (EXIT_FAILURE);
  }
  if (listen (sockfd, 5)) {
    printf ("Error at listen() ...(%s)\n",
	    strerror(errno));
    exit (EXIT_FAILURE);
  }
  while (1) {
    connfd = accept ( sockfd,
                      (struct sockaddr *) &address,
                      &addLength );
    if (connfd < 0) {
      if (errno == EINTR)
	continue;
      else {
	printf ("Error at accept() ...\n");
	exit (EXIT_FAILURE);
      }
    }
    pthread_create(&th, NULL, &threading_socket, connfd);
  }
  exit (EXIT_SUCCESS);
}

