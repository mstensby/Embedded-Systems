// Embedded Lab 6
// By Matt Stensby and Shaun Richter
// Description: The code below creates an interface where the user has 4 options(M, R, T, S)
// The first of which is M and it if pressed will stop the data acquisition and display a menu.
// The second of which is R and it runs and collects data. It displays the ADC voltage and if
// the potentiometer is turned it changes the voltage within the range of 0-5V as that is the
// input voltage. It also displays the time.
// Next if T is pressed then it sets the RTC time by getting the actual time from the PC using
// a script.
// Lastly if S is pressed it displays the time from the RTC. 
// If any other button is pressed then a buzzer will sound.
// This lab uses and USART communication and a I2C bus to transfer data from micro controller to 
// secure CRT.
//USART used for printing to secure CRT. I2c is used for RTC and ADC


#ifndef F_CPU
#define F_CPU 16000000UL // 16 MHz clock speed.
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "avr/sfr_defs.h"
#include <stdio.h>
#include "i2cmaster.h"
#include <time.h>

//Configure ADVC
//REFS0 is voltage reference - Look at documentation
//Keep full ten bit resolution so shift 0.
#define ADC_VREF_TYPE ((1 << REFS0) | (0 << ADLAR)) // Set voltage reference to ADVC with right adjusted result

void _delay_5ms(void);
void showMenu(void);
void soundBuzzer();

unsigned char uart_buffer_empty(void);
void usart_prints(const char *ptr);
void usart_printf(const char *ptr);
void usart_init(void);
void usart_putc(const char c);
unsigned char usart_getc(void);
void usart_clear(void);
int char_to_int(char char_array[], int size);

void getPCTime(struct tm *rtc_date);
void getPCF8583Time(struct tm *rtc_date);
void setPCF8583Time(struct tm *rtc_date);

void adc_init();
uint16_t adc_read(uint8_t adc_channel);
double adc_to_voltage(uint16_t adc_value);

int main(void)
{
   struct tm rtc_date;
   char *buff;
   char c;

   usart_init(); // Initialize the USART
   i2c_init();   // Initialize I2C system.
   adc_init();
   sei(); // Enable interrupts.
   showMenu();

   while (1)
   {

      if (!uart_buffer_empty())
      { // If there are input characters in the buffer
		  
         c = usart_getc(); // Receive a single character from USART buffer

         switch (c)
         {

         case 'T':
            usart_prints("Getting time from PC. Run isotime.vbs\n\r");
            getPCTime(&rtc_date);      // Gets PC time in format yyyy-MM-dd hh:mm:ss
            setPCF8583Time(&rtc_date); // Set the RTC time
            break;

         case 'S': //ToDO
            getPCF8583Time(&rtc_date); //Gets RTC time
            buff = isotime(&rtc_date); //Format time to be readable
            usart_prints("\n\n\r");
            usart_prints(buff); // Prints it
            usart_prints("\n\n\r");
            showMenu();
            break;

         case 'R': //Update to include potentiometer stuff
            usart_prints("Collecting data. Press 'M' to stop data acquisition and show menu.\n\r");
            usart_prints("\n\n\r");

            while (1)
            {
               if (!uart_buffer_empty())
               {
                  char stop_char = usart_getc();
				  // Checks if M is hit to stop data collection and display menu
                  if (stop_char == 'M')
                  {
                     break;
                  }
                  else //Any other button besides M is hit then beeps
                  {
                     usart_prints("Beep\n\r");
                     soundBuzzer();
                     usart_clear(); // Clears buffer
                  }
               }

               getPCF8583Time(&rtc_date); // Get RTC time
               buff = isotime(&rtc_date); //Format time to be readable
               usart_prints(buff); // Print RTC time
               usart_prints(", ");

               uint16_t adc_result = adc_read(0);           // Read ADC value from channel 0 (PC0/ADC0) - using potentiometer
               double voltage = adc_to_voltage(adc_result); // Convert ADC result to voltage

               int intVolt = (int)voltage;
               int decVolt = (int)(voltage*100)%100; 

               char voltage_buffer[20];                  // Buffer to store voltage
               sprintf(voltage_buffer, "%d.%d", intVolt, decVolt); // Format voltage to be 2 decimal places
               usart_prints(voltage_buffer);             // Print voltage
               usart_prints("\n\r");
               _delay_ms(1000); // Delay for 1 second
            }

         case 'M': //TODO
            usart_prints("\n\n\r");
            usart_prints("Stopping data collection.\n\r");
            showMenu();
            break;

         default: //Hit button other than M,S,R,T
            usart_prints("Beep\n\r");
            soundBuzzer();
            usart_clear();
         }
      }
   }
}

// Store menu items in FLASH. TODO
const char fdata1[] PROGMEM = "Press Any of These Keys\n\n\r";
const char fdata2[] PROGMEM = "     M - Stop Data Acquisition and Show Menu\n\r";
const char fdata3[] PROGMEM = "     R - Run and Collect Data\n\r";
const char fdata4[] PROGMEM = "     T - Get Time from PC and set RTC\n\r";
const char fdata5[] PROGMEM = "     S - Show RTC Time\n\r";

// Function to convert a character array representing a number to an integer.
int char_to_int(char char_array[], int size)
{
	int integer = 0;

	// Iterate through each character in the character array.
	// The loop condition is based on the size of the array divided by the size of a character.
	// This assumes that the 'size' parameter represents the total number of bytes in the array.
	// 'sizeof(char)' is 1 by definition in C, so 'size / sizeof(char)' effectively gives the number of characters.
	for (int i = 0; i < size / sizeof(char); i++)
	{
		// Convert the character to its integer representation and add it to the 'integer' variable.
		// Each character is converted to its corresponding integer value by subtracting the ASCII value of '0'.
		// This relies on the fact that in the ASCII encoding, the digits '0' to '9' are consecutive.
		// Multiplying the existing integer value by 10 shifts its digits to the left to make space for the new digit.
		integer = integer * 10 + (char_array[i] - '0');
	}

	return integer;
}


void adc_init()
{
   // Configure ADMUX register based on ADC_VREF_TYPE
   ADMUX = ADC_VREF_TYPE;
   // ADMUX = 0x40;

   // Enable ADC and set prescaler to 128 for 125 kHz ADC clock
   ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
   // ADCSRA = 0x97;
   // ADCSRB = 0x00;
}

uint16_t adc_read(uint8_t adc_channel)
{
   // Could use PC0 so MUX0-3 would remain set as 0000
   // Also may not need this if we just use ADC0, was trying to go off slides but prob wrong
   ADMUX = adc_channel | ADC_VREF_TYPE; // Select ADC channel set right most bits to zero
   _delay_us(10);

   ADCSRA |= (1 << ADSC); // Start ADC conversion(convert analog to digital signal)

   while (ADCSRA & (1 << ADSC)) // Wait for conversion to complete, wait for ADSC to be clear
      ;

   return ADCW; // Return ADC result
}

double adc_to_voltage(uint16_t adc_value)
{
   // Calculate voltage using ADC reading and reference voltage
   return (double)((5.0 / 1023.0) * adc_value); // 10 bit ADC resolution with 1024(2^10 - 1 = 1023) steps. 5 volts voltage reference
}

// Display menu items
void showMenu(void)
{
   usart_prints("-------------------------------------------------\n\r");
   usart_printf(fdata1);
   usart_printf(fdata2);
   usart_printf(fdata3);
   usart_printf(fdata4);
   usart_printf(fdata5);
   usart_prints("\n\r");
}
////////////////////////////////////////////////////////////////
//
// Get time from PC via serial port. The format the PC send is a
//
//      yyyy-MM-dd hh:mm:ss  2024-03-28 22:51:41vBCrLf
//
// where vbCrLf is \r\n or 0x0D 0x0A. Months are 1..12.
//
// The function blocks.
// TODO
void getPCTime(struct tm *rtc_date) // 2024-03-28 22:51:41
{
   // Parse and store date/time from PC
   usart_clear();

   char year[4] = "";
   for (int i = 0; i < 4; i++)
   {
      year[i] = usart_getc();
   }

   usart_getc(); // Remove '-'
   char month[2] = "";
   for (int i = 0; i < 2; i++)
   {
      month[i] = usart_getc();
   }

   usart_getc(); // Remove '-'
   char day[2] = "";
   for (int i = 0; i < 2; i++)
   {
      day[i] = usart_getc();
   }

   usart_getc(); // Remove ' '
   char hour[2] = "";
   for (int i = 0; i < 2; i++)
   {
      hour[i] = usart_getc();
   }

   usart_getc(); // Remove ':'
   char minute[2] = "";
   for (int i = 0; i < 2; i++)
   {
      minute[i] = usart_getc();
   }

   usart_getc(); // Remove ':'
   char second[2] = "";
   for (int i = 0; i < 2; i++)
   {
      second[i] = usart_getc();
   }

   usart_clear();

   // Convert char dates/time into integers
   rtc_date->tm_year = char_to_int(year, 4) - 1900; //tm_year number of years from 1900 - year is 2024 + 1900 so minues 1900
   rtc_date->tm_mon = char_to_int(month, 2) - 1; //January starts at 0 index
   rtc_date->tm_mday = char_to_int(day, 2);
   rtc_date->tm_hour = char_to_int(hour, 2);
   rtc_date->tm_min = char_to_int(minute, 2);
   rtc_date->tm_sec = char_to_int(second, 2);
}

////////////////////////////////////////////////////////////////
//
//  Sets the PCF8583 RTC using contents of rtc_date.
// TODO
void setPCF8583Time(struct tm *rtc_date)
{
   // Convert decimal values to BCD
   uint8_t sec_bcd = ((rtc_date->tm_sec / 10) << 4) | (rtc_date->tm_sec % 10); //Left side shifts to 4 so it gets left nibble(tens place). Right side gets the right nibble(ones place) 
   uint8_t min_bcd = ((rtc_date->tm_min / 10) << 4) | (rtc_date->tm_min % 10);
   uint8_t hour_bcd = ((rtc_date->tm_hour / 10) << 4) | (rtc_date->tm_hour % 10);
   uint8_t mday_bcd = ((rtc_date->tm_mday / 10) << 4) | (rtc_date->tm_mday % 10);
   uint8_t mon_bcd = (((rtc_date->tm_mon + 1) / 10) << 4) | ((rtc_date->tm_mon + 1) % 10);
   uint8_t year_bcd = ((rtc_date->tm_year / 10) << 4) | (rtc_date->tm_year % 10);

   i2c_start((unsigned char)0xA0 + I2C_WRITE); // Start communication with RTC - A0 + 0 = A0 for writing
   i2c_write(0x02);		// Start writing at 'seconds' memory address
   i2c_write(sec_bcd);  // Write seconds
   i2c_write(min_bcd);  // Write minute
   i2c_write(hour_bcd); // Write hour
   i2c_write(mday_bcd); // Write day
   i2c_write(mon_bcd);  // Write month
   i2c_write(year_bcd); // Write year

   i2c_stop(); // Stop data transfer
}

//  Get PCF8583 RTC time and fill rtc_date. TODO
void getPCF8583Time(struct tm *rtc_date)
{
   int8_t bcd1, bcd2, bcd3, bcd4, bcd5;

	// Move pointer to second slot in memory because thats where we started storing it
   i2c_start((unsigned char)0xA0 + I2C_WRITE);
   i2c_write(0x02);
   i2c_start((unsigned char)0xA0 + I2C_READ); //Read mode

   bcd1 = i2c_readAck(); // 10 and 1 second
   bcd2 = i2c_readAck(); // 10 and 1 minute
   bcd3 = i2c_readAck(); // 10 and 1 hour
   bcd4 = i2c_readAck(); // Year and date counter
   bcd5 = i2c_readAck(); // Weekday and month

   // Transformations converting from BCD to decimal
   rtc_date->tm_sec = ((bcd1 & 0xF0) >> 4) * 10 + (bcd1 & 0x0F);
   rtc_date->tm_min = (((bcd2 & 0xF0) >> 4) * 10 + (bcd2 & 0x0F));
   rtc_date->tm_hour = (((bcd3 & 0xF0) >> 4) * 10 + (bcd3 & 0x0F));
   rtc_date->tm_mday = (((bcd4 & 0x30) >> 4) * 10 + (bcd4 & 0x0F));
   rtc_date->tm_year = ((bcd4 & 0x03) >> 7) + 124; // Year since 1900
   rtc_date->tm_mon = (((bcd5 & 0x10) >> 4) * 10 + (bcd5 & 0x0F)) - 1;

   // Begin clock
   i2c_rep_start((unsigned char)0xA0 + I2C_WRITE);
   i2c_write(0);
   i2c_write(0b00001000); //Enable timer interrupt bit

   i2c_stop(); // Stop data transfer
}


// Delay 5ms
void _delay_5ms(void) // This assumes an 8-MHz clock.
{
   _delay_loop_2((uint16_t)9000);
   _delay_loop_2((uint16_t)1000);
}

////////////////////////////// USART Related Are Below ////////////////////////////

#define BAUD_RATE 9600 // Baud rate. The usart_init routine uses this.

// Variables and #define for ring RX ring buffer.
#define RX_BUFFER_SIZE 64
unsigned char rx_buffer[RX_BUFFER_SIZE];
volatile unsigned char rx_buffer_head;
volatile unsigned char rx_buffer_tail;

// USART Receive Interrupt Service Routine (ISR)
// Handles the receive interrupt for the USART module.
ISR(USART_RX_vect)
{
	// Disable interrupts globally to prevent interference during ISR execution.
	cli();

	// Read the received character from the USART data register (UDR0) into variable 'c'.
	char c = UDR0;

	// Store the received character in the receive buffer at the current buffer head position.
	rx_buffer[rx_buffer_head] = c;

	// Update the buffer head index to point to the next location in the receive buffer.
	if (rx_buffer_head == RX_BUFFER_SIZE - 1)
	rx_buffer_head = 0;
	else
	rx_buffer_head++;

	// Re-enable interrupts globally to allow other interrupts to occur.
	sei();
}

// Function to clear the USART receive buffer.
void usart_clear(void)
{
   rx_buffer_tail = rx_buffer_head;
}

// Function to initialize USART (Universal Synchronous/Asynchronous Receiver/Transmitter) communication.
void usart_init(void)
{
	unsigned short s;  // Variable to store the baud rate calculation result.

	// Set Baud rate based on the #define BAUD_RATE.
	s = (double)F_CPU / (BAUD_RATE * 16.0) - 1.0;  // Calculate the Baud rate register value.

	// Set the high and low bytes of the Baud rate register (UBRR0) for asynchronous mode.
	UBRR0H = (s & 0xFF00);  // Set high byte.
	UBRR0L = (s & 0x00FF);  // Set low byte.

	// Enable USART Receive Complete Interrupt (RXCIE0), Receiver (RXEN0), and Transmitter (TXEN0).
	UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);

	// Set frame format: 8 data bits, 1 stop bit, no parity.
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

	// Configure the data direction of USART pins (PD0: TX, PD1: RX).
	// Set PD0 (TX pin) as output and PD1 (RX pin) as input.
	DDRD |= (1 << 1);   // PD0 is output (TX).
	DDRD &= ~(1 << 0);  // PD1 is input (Rx).

	// Initialize receive buffer indices to zero to indicate empty buffer.
	rx_buffer_head = 0;
	rx_buffer_tail = 0;
}


// Function to send NULL-terminated string from FLASH via USART.
// Uses polling and blocks until all characters are sent.
void usart_printf(const char *ptr)
{
	char c;  // Variable to store each character read from FLASH.

	// Loop through the string until NULL terminator is encountered.
	while (pgm_read_byte_near(ptr))
	{
		// Read the character from FLASH memory and store it in variable 'c'.
		c = pgm_read_byte_near(ptr++);

		// Send the character via USART.
		usart_putc(c);
	}
}


// Function to send a single character via USART.
// Uses polling and blocks until the character is sent.
void usart_putc(const char c)
{
	// Wait until the USART Data Register Empty (UDRE0) flag becomes set.
	// This indicates that the USART buffer is empty and ready to accept the next character.
	//UDRE0 is a flag in the UCSR0A register indicating that the USART data register (UDR0) is empty and ready to accept new data.
	while (!(UCSR0A & (1 << UDRE0))); //Loops until UDRE0 flag becomes set indicating that the USART buffer is empty and ready to accept the next character.

	// Once the buffer is empty, write the character to the USART Data Register (UDR0).
	UDR0 = c;
}


// Function to send a NULL-terminated string from SRAM via USART.
// Uses polling and blocks until all characters are sent.
void usart_prints(const char *ptr)
{
	// Loop through the string until NULL terminator is encountered.
	while (*ptr)
	{
		// Wait until the USART Data Register Empty (UDRE0) flag becomes set.
		// This indicates that the USART buffer is empty and ready to accept the next character.
		while (!(UCSR0A & (1 << UDRE0)));
		
		// Once the buffer is empty, write the character pointed to by 'ptr' to the USART Data Register (UDR0).
		UDR0 = *(ptr++);
	}
}


// Function to retrieve a character from the USART receive buffer.
// This function blocks until a character arrives in the buffer.
unsigned char usart_getc(void)
{
	unsigned char c;  // Variable to store the received character.

	// Wait until a character is available in the receive buffer.
	while (rx_buffer_tail == rx_buffer_head)
	;

	// Read the character from the receive buffer at the current tail index.
	c = rx_buffer[rx_buffer_tail];

	// Update the tail index to point to the next location in the receive buffer.
	if (rx_buffer_tail == RX_BUFFER_SIZE - 1)
	rx_buffer_tail = 0;
	else
	rx_buffer_tail++;

	// Return the retrieved character.
	return c;
}


// Function to check if the USART receive buffer is empty.
// Returns TRUE if the buffer is empty, FALSE otherwise.
unsigned char uart_buffer_empty(void)
{
	// Returns TRUE if the tail index is equal to the head index,
	// indicating that the receive buffer is empty.
	return (rx_buffer_tail == rx_buffer_head);
}


////////////////////////////// IC2 Related Are Below ////////////////////////////

#include <inttypes.h>
#include <compat/twi.h>
// #include <i2cmaster.h>

// define CPU frequency in Here here if not defined in Makefile  or above

#ifndef F_CPU
#define F_CPU 4000000UL
#endif

/* I2C clock in Hz */
#define SCL_CLOCK 100000L

/*************************************************************************
 Initialization of the I2C bus interface. Need to be called only once
*************************************************************************/
void i2c_init(void)
{
   /* initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */

   TWSR = 0;                              /* no prescaler */
   TWBR = ((F_CPU / SCL_CLOCK) - 16) / 2; /* must be > 10 for stable operation */

} /* i2c_init */

/*************************************************************************
  Issues a start condition and sends address and transfer direction.
  return 0 = device accessible, 1= failed to access device
*************************************************************************/
unsigned char i2c_start(unsigned char address)
{
   uint8_t twst;

   // send START condition
   TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

   // wait until transmission completed
   while (!(TWCR & (1 << TWINT)))
      ;

   // check value of TWI Status Register. Mask prescaler bits.
   twst = TW_STATUS & 0xF8;
   if ((twst != TW_START) && (twst != TW_REP_START))
      return 1;

   // send device address
   TWDR = address;
   TWCR = (1 << TWINT) | (1 << TWEN);

   // wail until transmission completed and ACK/NACK has been received
   while (!(TWCR & (1 << TWINT)))
      ;

   // check value of TWI Status Register. Mask prescaler bits.
   twst = TW_STATUS & 0xF8;
   if ((twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK))
      return 1;

   return 0;

} /* i2c_start */

/*************************************************************************
 Issues a start condition and sends address and transfer direction.
 If device is busy, use ack polling to wait until device is ready

 Input:   address and transfer direction of I2C device
*************************************************************************/
void i2c_start_wait(unsigned char address)
{
   uint8_t twst;

   while (1)
   {
      // send START condition
      TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

      // wait until transmission completed
      while (!(TWCR & (1 << TWINT)))
         ;

      // check value of TWI Status Register. Mask prescaler bits.
      twst = TW_STATUS & 0xF8;
      if ((twst != TW_START) && (twst != TW_REP_START))
         continue;

      // send device address
      TWDR = address;
      TWCR = (1 << TWINT) | (1 << TWEN);

      // wail until transmission completed
      while (!(TWCR & (1 << TWINT)))
         ;

      // check value of TWI Status Register. Mask prescaler bits.
      twst = TW_STATUS & 0xF8;
      if ((twst == TW_MT_SLA_NACK) || (twst == TW_MR_DATA_NACK))
      {
         /* device busy, send stop condition to terminate write operation */
         TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);

         // wait until stop condition is executed and bus released
         while (TWCR & (1 << TWSTO))
            ;

         continue;
      }
      // if( twst != TW_MT_SLA_ACK) return 1;
      break;
   }

} /* i2c_start_wait */

/*************************************************************************
 Issues a repeated start condition and sends address and transfer direction

 Input:   address and transfer direction of I2C device

 Return:  0 device accessible
          1 failed to access device
*************************************************************************/
unsigned char i2c_rep_start(unsigned char address)
{
   return i2c_start(address);

} /* i2c_rep_start */

/*************************************************************************
 Terminates the data transfer and releases the I2C bus
*************************************************************************/
void i2c_stop(void)
{
   /* send stop condition */
   TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);

   // wait until stop condition is executed and bus released
   while (TWCR & (1 << TWSTO))
      ;

} /* i2c_stop */

/*************************************************************************
  Send one byte to I2C device

  Input:    byte to be transfered
  Return:   0 write successful
            1 write failed
*************************************************************************/
unsigned char i2c_write(unsigned char data)
{
   uint8_t twst;

   // send data to the previously addressed device
   TWDR = data;
   TWCR = (1 << TWINT) | (1 << TWEN);

   // wait until transmission completed
   while (!(TWCR & (1 << TWINT)))
      ;

   // check value of TWI Status Register. Mask prescaler bits
   twst = TW_STATUS & 0xF8;
   if (twst != TW_MT_DATA_ACK)
      return 1;
   return 0;

} /* i2c_write */

/*************************************************************************
 Read one byte from the I2C device, request more data from device

 Return:  byte read from I2C device
*************************************************************************/
unsigned char i2c_readAck(void)
{
   TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
   while (!(TWCR & (1 << TWINT)))
      ;

   return TWDR;

} /* i2c_readAck */

/*************************************************************************
 Read one byte from the I2C device, read is followed by a stop condition

 Return:  byte read from I2C device
*************************************************************************/
unsigned char i2c_readNak(void)
{
   TWCR = (1 << TWINT) | (1 << TWEN);
   while (!(TWCR & (1 << TWINT)))
      ;

   return TWDR;

} /* i2c_readNak */

// This function sounds a buzzer that is connected to arduino at PB2
void soundBuzzer(){
	// Set PB2 as an output
	DDRB |= (1 << PORTB2);  // Set the buzzer port as output
	// Set the buzzer on
	PORTB |= (1 << PORTB2);

   for(int i =0; i < 100; i++){
      _delay_5ms();
   }

	//Set the buzzer off
	PORTB &= ~(1 << PORTB2);
}