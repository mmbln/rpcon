/*
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "rspsrv.h"
#include "fifo.h"

#ifdef __arm__
  #include <linux/i2c-dev.h>
  #include <fcntl.h>
  #include <string.h>
  #include <sys/ioctl.h>
  #include <sys/types.h>
  #include <sys/stat.h>
#endif


extern pthread_mutex_t raspyMutex;
extern pthread_mutex_t timeMutex;
extern pthread_cond_t cond;

extern void fifoInit(void);
extern int putMessage(int, int);
extern int getMessage(int *, int *);

#define  MAX_SIZE  1024

extern unsigned char gDigit;
extern unsigned char gFrequence;
extern unsigned char gRelais;


int i2c_readInt(unsigned char add, unsigned int *val)
{
#ifdef __arm__
  int fd;			// File descrition
  char *fileName = "/dev/i2c-1"; // Name of the port we will be using
  int  address = 0x11;		// Address of LCD03 shifted right one bit
  unsigned char buf[10];	
  fd = open(fileName, O_RDWR); // Open port for reading and writing
  if (fd < 0) {
    return FALSE;
  }
  if (ioctl(fd, I2C_SLAVE, address) < 0) {
    // Set the port options and set the address of the device we wish to speak to
    return FALSE;
  }
  buf[0] = 0x20;			// prepare the address pointer	
  buf[1] = add;
  if ((write(fd, buf, 2)) != 2) {				
    return FALSE;
  }
  if ((read(fd, buf, 1)) != 1) {		// read address 6	
    return FALSE;
  }
  *val = buf[0];
  buf[0] = 0x20;			// prepare the address pointer	
  buf[1] = add+1;
  if ((write(fd, buf, 2)) != 2) {				
    return FALSE;
  }
  if ((read(fd, buf, 1)) != 1) {		// read address 7	
    return FALSE;
  }
  *val += buf[0] << 8; 
  close(fd);
  return TRUE;
#else
  fprintf (stderr, "Read integer. Address %d.\n", add);
  *val = 0;
  return TRUE;
#endif
}

int i2c_readChar(unsigned char add, unsigned char *val)
{
#ifdef __arm__
  int fd;			// File descrition
  char *fileName = "/dev/i2c-1"; // Name of the port we will be using
  int  address = 0x11;		// Address of LCD03 shifted right one bit
  char buf[10];	
  fd = open(fileName, O_RDWR); // Open port for reading and writing
  if (fd < 0) {
    return FALSE;
  }
  if (ioctl(fd, I2C_SLAVE, address) < 0) {
    // Set the port options and set the address of the device we wish to speak to
    return FALSE;
  }
  buf[0] = 0x20;			// prepare the address pointer	
  buf[1] = add;
  if ((write(fd, buf, 2)) != 2) {				
    return FALSE;
  }
  if ((read(fd, buf, 1)) != 1) {		// read address 6	
    return FALSE;
  }
  *val = buf[0];
  close(fd);
  return TRUE;
#else
  fprintf (stderr, "Read char. Address %d.\n", add);
  *val = 0;
  return TRUE;
#endif
}

int i2c_writeInt(unsigned char add, unsigned int val)
{
#ifdef __arm__
  int fd;			// File descrition
  char *fileName = "/dev/i2c-1"; // Name of the port we will be using
  int  address = 0x11;		// Address of LCD03 shifted right one bit
  char buf[10];	
  fd = open(fileName, O_RDWR); // Open port for reading and writing
  if (fd < 0) {
    return FALSE;
  }
  if (ioctl(fd, I2C_SLAVE, address) < 0) {
    // Set the port options and set the address of the device we wish to speak to
    return FALSE;
  }
  buf[0] = add;			// prepare the address pointer	
  buf[1] = (unsigned char)(val & 0xff);
  if ((write(fd, buf, 2)) != 2) {				
    return FALSE;
  }
  buf[0] = add+1;			// prepare the address pointer	
  buf[1] = (unsigned char)(val >> 8);
  if ((write(fd, buf, 2)) != 2) {				
    return FALSE;
  }
  close(fd);
  return TRUE;
#else
  fprintf (stderr, "Write integer. Address %d. Value %d.\n", add, val);
  return TRUE;
#endif
}

int i2c_writeChar(unsigned char add, unsigned char val)
{
#ifdef __arm__
  int fd;			// File descrition
  char *fileName = "/dev/i2c-1"; // Name of the port we will be using
  int  address = 0x11;		// Address of LCD03 shifted right one bit
  char buf[10];	
  fd = open(fileName, O_RDWR); // Open port for reading and writing
  if (fd < 0) {
    return FALSE;
  }
  if (ioctl(fd, I2C_SLAVE, address) < 0) {
    // Set the port options and set the address of the device we wish to speak to
    return FALSE;
  }
  buf[0] = add;			// prepare the address pointer	
  buf[1] = val;
  if ((write(fd, buf, 2)) != 2) {				
    return FALSE;
  }
  close(fd);
  return TRUE;
#else
  fprintf (stderr, "Write Byte. Address %d. Value %d.\n", add, val);
  return TRUE;
#endif
}

unsigned char parameterToNumber(char *p)
{
  return atoi(p) & 0xff;
}

void *controller(void *arg)
{
  int command;
  int parameter;
  unsigned char res;

  while(1) {
    pthread_mutex_lock(&raspyMutex);
    pthread_cond_wait(&cond, &raspyMutex);
    /* new message */
    while (getMessage(&command, &parameter) != 0) {
      if (command == FF_SWITCH_ON) {
	i2c_writeChar(0x01, 0x03);
	gRelais = 3;
      } else if (command == FF_SWITCH_OFF) {
	i2c_writeChar(0x01, 0x0);
	gRelais = 0;
      } else if (command == FF_GET_SWITCH) {
	i2c_readChar(0x01, &res);
	gRelais = res;
      } else if (command == FF_SET_DIGIT) {
	i2c_writeChar(0x02, parameter);
	gDigit = (unsigned char)parameter;
      } else if (command == FF_GET_DIGIT) {
	i2c_readChar(0x02, &res);
	gDigit = res;
      } else if (command == FF_SET_FREQUENCE) {
	i2c_writeChar(0x03, parameter);
	gFrequence = (unsigned char)parameter;
      } else if (command == FF_GET_FREQUENCE) {
	i2c_readChar(0x03, &res);
	gFrequence = res;
      }
    }
    pthread_mutex_unlock(&raspyMutex);

  }
}
