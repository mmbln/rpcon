#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "soapH.h"  
#include "rp2.nsmap"  


int i2c_readInt(unsigned char add, unsigned int *val)
{
#ifdef __arm__
  int fd;			// File descrition
  char *fileName = "/dev/i2c-1"; // Name of the port we will be using
  int  address = 0x11;		// Address of LCD03 shifted right one bit
  unsigned char buf[10];	
  fd = open(fileName, O_RDWR); // Open port for reading and writing
  if (fd < 0) {
    return SOAP_FAULT;
  }
  if (ioctl(fd, I2C_SLAVE, address) < 0) {
    // Set the port options and set the address of the device we wish to speak to
    return SOAP_FAULT;
  }
  buf[0] = 0x20;			// prepare the address pointer	
  buf[1] = add;
  if ((write(fd, buf, 2)) != 2) {				
    return SOAP_FAULT;
  }
  if ((read(fd, buf, 1)) != 1) {		// read address 6	
    return SOAP_FAULT;
  }
  *val = buf[0];
  buf[0] = 0x20;			// prepare the address pointer	
  buf[1] = add+1;
  if ((write(fd, buf, 2)) != 2) {				
    return SOAP_FAULT;
  }
  if ((read(fd, buf, 1)) != 1) {		// read address 7	
    return SOAP_FAULT;
  }
  *val += buf[0] << 8; 
  close(fd);
  return SOAP_OK;
#else
  fprintf (stderr, "Read integer. Address %d.\n", add);
  *val = 0;
  return SOAP_OK;
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
    return SOAP_FAULT;
  }
  if (ioctl(fd, I2C_SLAVE, address) < 0) {
    // Set the port options and set the address of the device we wish to speak to
    return SOAP_FAULT;
  }
  buf[0] = 0x20;			// prepare the address pointer	
  buf[1] = add;
  if ((write(fd, buf, 2)) != 2) {				
    return SOAP_FAULT;
  }
  if ((read(fd, buf, 1)) != 1) {		// read address 6	
    return SOAP_FAULT;
  }
  *val = buf[0];
  close(fd);
  return SOAP_OK;
#else
  fprintf (stderr, "Read char. Address %d.\n", add);
  *val = 0;
  return SOAP_OK;
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
    return SOAP_FAULT;
  }
  if (ioctl(fd, I2C_SLAVE, address) < 0) {
    // Set the port options and set the address of the device we wish to speak to
    return SOAP_FAULT;
  }
  buf[0] = add;			// prepare the address pointer	
  buf[1] = (unsigned char)(val & 0xff);
  if ((write(fd, buf, 2)) != 2) {				
    return SOAP_FAULT;
  }
  buf[0] = add+1;			// prepare the address pointer	
  buf[1] = (unsigned char)(val >> 8);
  if ((write(fd, buf, 2)) != 2) {				
    return SOAP_FAULT;
  }
  close(fd);
  return SOAP_OK;
#else
  fprintf (stderr, "Write integer. Address %d. Value %d.\n", add, val);
  return SOAP_OK;
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
    return SOAP_FAULT;
  }
  if (ioctl(fd, I2C_SLAVE, address) < 0) {
    // Set the port options and set the address of the device we wish to speak to
    return SOAP_FAULT;
  }
  buf[0] = add;			// prepare the address pointer	
  buf[1] = val;
  if ((write(fd, buf, 2)) != 2) {				
    return SOAP_FAULT;
  }
  close(fd);
  return SOAP_OK;
#else
  fprintf (stderr, "Write Byte. Address %d. Value %d.\n", add, val);
  return SOAP_OK;
#endif
}


int main(void)  
{  
    struct soap soap;
    int m,s;  // master ans slave sockets
    int i;

    soap_init(&soap);
    m = soap_bind(&soap, NULL, 8100, 100);
    if (m < 0) 
        soap_print_fault(&soap, stderr);
    else {
        fprintf(stderr, "Socket connection successfull: master socket = %d\n", m);
	for (i = 1; ; i++) {
	  s = soap_accept(&soap);
	  if (s < 0) {
	    soap_print_fault(&soap, stderr);
	    break;
	  }
	  fprintf(stderr, "%d accepted connection from IP=%d.%d.%d.%d socket=%d", i,
		  (soap.ip >> 24)&0xff,(soap.ip >> 16)&0xff,(soap.ip >> 8)&0xff,soap.ip &0xff,s);
	  if (soap_serve(&soap) != SOAP_OK)
	    soap_print_fault(&soap, stderr);
	  fprintf(stderr, " request served\n");
	  soap_destroy(&soap);
	  soap_end(&soap);
	}
    }
    soap_done(&soap);
}  


int rp2__setRelais(struct soap *soap, unsigned char relais, int *err) {
  return i2c_writeChar(0x01, relais);
}  

int rp2__setDigit(struct soap *soap, unsigned char digit, int *err) {
  return i2c_writeChar(0x02, digit);
}  

int rp2__setFrequence(struct soap *soap, unsigned char frequence, int *err) {
  return i2c_writeChar(0x03, frequence);
}  

int rp2__setIntegerValue(struct soap *soap, unsigned int value, int *err) {
  return i2c_writeInt(0x04, value);
}  


int rp2__getRelais(struct soap *soap, struct result_struct *r) {
  return i2c_readChar(0x01, &(r->charResult));
}  

int rp2__getDigit(struct soap *soap, struct result_struct *r) {
  return i2c_readChar(0x02, &(r->charResult));
}  

int rp2__getFrequence(struct soap *soap, struct result_struct *r) {
  return i2c_readChar(0x03, &(r->charResult));
}  

int rp2__getIntegerValue(struct soap *soap, struct result_struct *r) {
  return i2c_readInt(0x04, &(r->intResult));
}  

int rp2__getLightValue(struct soap *soap, struct result_struct *r) {
  return i2c_readInt(0x06, &(r->intResult));
}  


