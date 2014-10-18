/**
 * This file is made for the rpcon board.
 *
 *
 */

//-------------------------------------------------------------- 

#include <stdlib.h>
#include <avr/io.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "USI_TWI_Slave.h"

//-------------------------------------------------------------- 

// defines
#define DIGIT_ON        (PORTA & 0x7f)
#define CURRENT_ON_TIME 200
#define CURRENT_MEASUREMENT_TIME 100
#define ONE_SECOND      1000
// My address for the I2C interface 
#define MY_I2C_ADDRESS (0x11)

// current measurement
#define CURRENT_ON      PORTB |= (1<<PORTB5)
#define CURRENT_OFF     PORTB &= ~(1<<PORTB5)
#define HIGH_CURRENT    (PORTB & (1 <<PORTB6))


//-------------------------------------------------------------- 

// Variables for the time handling and the seven segment display
volatile unsigned int displayTicks;
volatile unsigned int reloadValue, oldReloadValue;
volatile unsigned char frequence;
volatile unsigned char adcValuePtr;
volatile unsigned char digit, oldDigit;

// variables for the integer show mode
volatile enum ShowIntegerMode
{
  ShowInteger_None = 0,
  ShowInteger_Blank1,
  ShowInteger_Digit1,
  ShowInteger_Blank2,
  ShowInteger_Digit2,
  ShowInteger_Blank3,
  ShowInteger_Digit3,
  ShowInteger_Blank4,
  ShowInteger_Digit4,
  ShowInteger_Blank5
} showIntegerState;


unsigned int lightValue, lightValueSave;
unsigned int measurementTicks;

unsigned int showIntegerValue;

// vars for the I2C interface
unsigned char receiveBufferPtr;
unsigned char receiveBuffer[2];

unsigned int adVals[8];
/*--------------------------------------------------------------
 * Seven Segment display, show Pattern
 *       +---+  0x08
 *       |   |
 *  0x10 |   |  0x04
 *       |   |
 *       +---+  0x01
 *       |   |
 *  0x20 |   |  0x02
 *       |   |
 *       +---+  0x40
 *-------------------------------------------------------------- 
**/
void
showPattern (unsigned char p)
{
  PORTA = (PORTA & 0x80) | p;
}

/*
 *  Seven Segment display, show digit
**/
void
showDigit (unsigned char d)
{
  static unsigned char p[] = {
    0x7e,			/* 0 */
    0x06,			/* 1 */
    0x6d,			/* 2 */
    0x4f,			/* 3 */
    0x17,			/* 4 */
    0x5b,			/* 5 */
    0x7b,			/* 6 */
    0x0e,			/* 7 */
    0x7f,			/* 8 */
    0x5f			/* 9 */
  };

  if (d < 10)
    {
      showPattern (p[d]);
      digit = d;
    }
}

/*--------------------------------------------------------------
 * sets the blinking frequence of the display. 
 * The input frequence can be set between 0 (for constant on) 
 * and 10Hz. 
 *-------------------------------------------------------------- 
**/
void
setFrequence (unsigned char freq)
{
  /* sets blinking frequence of the seven-Segment digit
     0 = no blinking
     1-10 = frequence in Hz
     11 - 255 invalid values
   */
  static unsigned int phase[] = {
    0,				/* 0 */
    500,			/* 1 */
    250,			/* 2 */
    167,			/* 3 */
    125,			/* 4 */
    100,			/* 5 */
    83,				/* 6 */
    71,				/* 7 */
    63,				/* 8 */
    56,				/* 9 */
    50				/* 10 */
  };

  // when the showIntegerState is active this command is ignored
  if ((freq >= 0) && (freq <= 10) && !showIntegerState)
    {
      reloadValue = phase[freq];
      // set the global var
      frequence = freq;
    }
}

/*--------------------------------------------------------------
 * initiates the display of a four digit integer value.
 * The rest is done in the timer interrupt routine.
 *-------------------------------------------------------------- 
**/
void
showInteger (unsigned int uint)
{
  if (uint < 10000)
    {
      showIntegerValue = uint;
      oldReloadValue = reloadValue;

      // make sure the first blank is shown
      displayTicks = 0;
      // set frequence to 1 Hz
      setFrequence (1);
      // start with a blank
      showIntegerState = ShowInteger_Blank1;
    }
}


/*--------------------------------------------------------------
 *
 *
 *-------------------------------------------------------------- 
**/
ISR (ADC_vect)
{
  int i;
  /* ADC Conversion Complete */
  if (adcValuePtr == 0xff)
    {
      adcValuePtr = 0;
    }
  else if (adcValuePtr >= 0 && adcValuePtr < 8)
    {
      adVals[adcValuePtr++] = ADC;
      if (adcValuePtr == 8)
	{
	  // stop conversion, disable Interrupt 
	  ADCSR &= ~((1 << ADFR) | (1 << ADIE));
	  // switch off the current
	  CURRENT_OFF;
	  // write the var ADCVAL to a proper var.
	  lightValue = 0;
	  for (i = 0; i < 8; i++)
	    {
	      lightValue += adVals[i];
	    }
	}

    }
  else
    {
    }

}

/*--------------------------------------------------------------
 *
 *
 *-------------------------------------------------------------- 
**/
ISR (TIMER1_OVF1_vect)
{
  // Blinking of the display
  displayTicks++;
  if (reloadValue == 0)
    {
      displayTicks = 0;
      showDigit (digit);
    }
  else if (displayTicks >= reloadValue)
    {
      displayTicks = 0;
      switch (showIntegerState)
	{
	  // this is the normal case
	case ShowInteger_None:
	  if (DIGIT_ON)
	    showPattern (0x00);
	  else
	    showDigit (digit);
	  break;
	  // all cases from here belong to the state machine
	case ShowInteger_Blank1:
	  oldDigit = digit;
	  showIntegerState = ShowInteger_Digit1;
	  showPattern (0x00);
	  break;
	case ShowInteger_Digit1:
	  showIntegerState = ShowInteger_Blank2;
	  showDigit ((unsigned char) (showIntegerValue / 1000));
	  break;
	case ShowInteger_Blank2:
	  showIntegerState = ShowInteger_Digit2;
	  showPattern (0x00);
	  break;
	case ShowInteger_Digit2:
	  showIntegerState = ShowInteger_Blank3;
	  showDigit ((unsigned char) ((showIntegerValue % 1000) / 100));
	  break;
	case ShowInteger_Blank3:
	  showIntegerState = ShowInteger_Digit3;
	  showPattern (0x00);
	  break;
	case ShowInteger_Digit3:
	  showIntegerState = ShowInteger_Blank4;
	  showDigit ((unsigned char) ((showIntegerValue % 100) / 10));
	  break;
	case ShowInteger_Blank4:
	  showIntegerState = ShowInteger_Digit4;
	  showPattern (0x00);
	  break;
	case ShowInteger_Digit4:
	  showIntegerState = ShowInteger_Blank5;
	  showDigit ((unsigned char) (showIntegerValue % 10));
	  break;
	case ShowInteger_Blank5:
	  showIntegerState = ShowInteger_None;
	  showPattern (0x00);
	  /* show_integer mode is done restore old blinking frequence 
	   * and old digit value. 
	   */
	  reloadValue = oldReloadValue;
	  digit = oldDigit;
	  break;
	}
    }

  // Check for overload of the analoge input
  // current is on only CURRENT_ON_TIME ms per second
  measurementTicks++;
  if ((measurementTicks >= 0) && (measurementTicks < CURRENT_ON_TIME))
    {
      CURRENT_ON;
    }
  else if (measurementTicks == CURRENT_ON_TIME)
    {
      // stop measurement if shortcut. 
      if (!HIGH_CURRENT)
	{
	  // measure the current value
	  adcValuePtr = 0xff;
	  // start ADC and enable interrupts
	  ADCSR |= ((1 << ADSC) | (1 << ADFR) | (1 << ADIE));
	}
      else
	{
	  CURRENT_OFF;
	}
    }
  else if (measurementTicks >= ONE_SECOND)
    {
      measurementTicks = 0;
    }

}

/*--------------------------------------------------------------
 * This function fills the transmit buffer
 *-------------------------------------------------------------- 
**/
void
processSendCommand ()
{
  unsigned char address;

  address = receiveBuffer[1];
  switch (address)
    {
    case 0x01:
      USI_TWI_Transmit_Byte ((PORTB & 0x18) >> 3);
      break;
    case 0x02:
      USI_TWI_Transmit_Byte (digit);
      break;
    case 0x03:
      USI_TWI_Transmit_Byte (frequence);
      break;
    case 0x04:
      USI_TWI_Transmit_Byte ((unsigned char) (showIntegerValue & 0xff));
      break;
    case 0x05:
      USI_TWI_Transmit_Byte ((unsigned char) (showIntegerValue >> 8));
      break;
    case 0x06:
      // latch the value
      if (lightValueSave == 0xffff)
	{
	  ADCSR &= (~(1 << ADIE));
	  lightValueSave = lightValue;
	  ADCSR |= (1 << ADIE);
	}
      USI_TWI_Transmit_Byte ((unsigned char) (lightValueSave & 0xff));
      break;
    case 0x07:
      USI_TWI_Transmit_Byte ((unsigned char) (lightValueSave >> 8));
      lightValueSave = 0xffff;
      break;
    default:
      // transmit a -1 on the next request
      USI_TWI_Transmit_Byte (0xff);
      break;
    }
}

/*--------------------------------------------------------------
 * 
 *
 *-------------------------------------------------------------- 
**/
void
init (void)
{
  // Timer 0 configuration
  TCCR1A = 0x00;
  TCCR1B = 0x87;
  // Timer 0 interrupt every 1ms
  // ((8000000/64)/1000) = 125
  OCR1C = 125 - 1;

  // allow Compare Interrupt
  TIMSK = 0x04;

  // PORT A, 
  //   0-6: SevenSegment
  //   7  : Analogue input
  DDRA = 0x7f;
  PORTA = 0x00;

  // Port B
  //   0: SDA Input, open collector   0
  //   1: MISO: Input                 0
  //   2: SCL Output open collector   1
  //   3: Output Relais 1             1
  //   4: Output Relais 2             1
  //   5: Output CurrentOn            1
  //   6: Input High Current          0
  //   7: Input, Reset                0   
  PORTB = 0x05;			// SCL and SDA high
  DDRB = 0x3c;


  // Init the I2C Interface
  USI_TWI_Slave_Initialise (MY_I2C_ADDRESS);

  // display a non blinking 0 (zero)
  showDigit (0);
  setFrequence (0);

  // Light Value
  lightValue = 0;
  // set the semaphore value
  lightValueSave = 0xffff;
  // we are starting
  measurementTicks = 0;

  // AD Converter init
  ADMUX = (0 << 6) |		// use of AVCC as ref
    (0 << 5) |			// right adjusted default
    (6 << 0);			// ADC6 single ended no gain
  ADCSR = (1 << ADEN) |		// ADC Enable
    (0 << ADSC) |		// ADC Start not set
    (0 << ADFR) |		// ADC Free running mode not set
    (0 << ADIF) |		// Interrupt flag not reset
    (1 << ADIE) |		// Interrupt is enabled
    (6 << ADPS0);		// 8.000000 MHz / 64 = 125.000 KHz


  // activate Global Interrupts
  sei ();
}

/*
 *-------------------------------------------------------------
 * main starts here 
 *------------------------------------------------------------- 
**/
int
main (void)
{
  unsigned int tmpInteger = 0;

  init ();

  while (1)
    {
      // get all the byte from the i2c interface
      while (USI_TWI_Data_In_Receive_Buffer () && (receiveBufferPtr < 2))
	{
	  receiveBuffer[receiveBufferPtr++] = USI_TWI_Receive_Byte ();
	}
      // process a complete command from the i2c interface
      if (receiveBufferPtr == 2)
	{
	  switch (receiveBuffer[0])
	    {
	      /* 0x01: Relais */
	      /* 0x02: Digit */
	      /* 0x03: Blinking Frequence */
	      /* 0x04: Integer Low */
	      /* 0x05: Integer High */
	      /* 0x06: Analogue Integer Low (Read Only) */
	      /* 0x07: Analogue Integer High (Read Only) */
	      /* 0x20: Set address for reading */
	    case 0x01:
	      PORTB = (PORTB & 0xe7) | ((receiveBuffer[1] & 0x03) << 3);
	      break;
	    case 0x02:
	      showDigit (receiveBuffer[1]);
	      break;
	    case 0x03:
	      setFrequence (receiveBuffer[1]);
	      break;
	    case 0x04:
	      tmpInteger = receiveBuffer[1];
	      break;
	    case 0x05:
	      tmpInteger += (receiveBuffer[1] << 8);
	      showInteger (tmpInteger);
	      break;
	    case 0x20:
	      processSendCommand ();
	      break;
	    default:
	      break;
	    }			// switch receiveBuffer[0]
	  receiveBufferPtr = 0;
	}			// receiveBuffer == 2
    }				// while
}
