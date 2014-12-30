/*
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <math.h>

#include "rspsrv.h"
#include "fifo.h"

extern pthread_mutex_t raspyMutex;
extern pthread_mutex_t timeMutex;
extern pthread_cond_t cond;
extern char gTimeStr[];

extern void fifoInit(void);
extern int putMessage(int, int);

extern enum globMode gMode;

#define TWO_PI (2.0 * 3.1415926535897931)

struct switchTimes {
  int switchOnTime;
  int switchOffTime;
};

#define BUFFER_SIZE 256

int getIntVal(char *b, int digits)
{
  int val=0;
  char c;
  int i;

  for (i = 0; i < digits; i++) {
    c = *(b+i);
    if (c < '0' || c > '9') {
      return -1;
    }
    val = val * 10 + (c - '0');
  }

  return val;
}

extern int getTimeByFile(time_t *t)
{
  int input_fd;
  char buffer[BUFFER_SIZE];
  struct tm tm;

  input_fd = open ("./faketime.txt", O_RDONLY);
  if (input_fd == -1) {
    return FALSE;
  }
  if (read(input_fd, &buffer, BUFFER_SIZE) <= 0) {
    return FALSE;
  }
  
  /* TODO error */
  tm.tm_year  = getIntVal(&buffer[0], 4) - 1900;  /* yyyy - 1900 */
  tm.tm_mon   = getIntVal(&buffer[4], 2) - 1;     /* 0-11 */
  tm.tm_mday  = getIntVal(&buffer[6], 2);         /* 1-31 */
  tm.tm_hour  = getIntVal(&buffer[8], 2);
  tm.tm_min   = getIntVal(&buffer[10], 2);
  tm.tm_sec   = getIntVal(&buffer[12], 2);
  tm.tm_isdst = 0;

  *t = mktime(&tm);

  printf("Time from faketime file: %04d-%02d-%02d %02d:%02d:%02d.\n",
	 tm.tm_year + 1900,
	 tm.tm_mon + 1,
	 tm.tm_mday,
	 tm.tm_hour,
	 tm.tm_min,
	 tm.tm_sec);

  return TRUE;
}

int calcSwitchTimes(int dayOfYear, struct switchTimes *swt)
{
  int diffSec;

  diffSec = (int)(9000.0 * sin((float)(dayOfYear - 80) * TWO_PI / 366.0));
  swt->switchOnTime = 17 * 3600 + 1800 + diffSec;
  swt->switchOffTime = 4 * 3600 + 1800 - diffSec;
}

void *timeControl(void *arg)
{
  time_t t;
  struct tm *tm;
  struct switchTimes swt;
  int timeNow;  
                     /* 123456789012345678 */
  char timeStr[18];  /* YYYY-MM-DD  HH:MM */
  enum globMode tmpMode;

  pthread_detach(pthread_self());

  while(TRUE) {
    /* write something in the thread every 10 second */
    sleep(1);

    pthread_mutex_lock(&raspyMutex);
    tmpMode = gMode;
    pthread_mutex_unlock(&raspyMutex);

    if (tmpMode != automaticMode)
      continue;

    if (!getTimeByFile(&t)) {
      /* no timefile get the time from the system */
      time(&t);
    }
    tm = gmtime(&t);
    /* print a printable string */
    pthread_mutex_lock(&timeMutex);
    sprintf(gTimeStr, "%04d-%02d-%02d  %02d:%02d", 
	    tm->tm_year + 1900,
	    tm->tm_mon + 1,
	    tm->tm_mday,
	    tm->tm_hour,
	    tm->tm_min);
    pthread_mutex_unlock(&timeMutex);
	    
    if (tmpMode != automaticMode)
      continue;

    /* calc the switch times of today */
    calcSwitchTimes(tm->tm_yday, &swt);
    /* */
    timeNow = (tm->tm_hour * 60 + tm->tm_min) * 60 + tm->tm_sec;
    
    if (timeNow > swt.switchOffTime && timeNow < swt.switchOnTime) {
      /* switch off */
      pthread_mutex_lock(&raspyMutex);
      putMessage(FF_SWITCH_OFF, 0);
    } else {
      /* switch on */
      pthread_mutex_lock(&raspyMutex);
      putMessage(FF_SWITCH_ON, 0);
    }
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&raspyMutex);
  } 
}
