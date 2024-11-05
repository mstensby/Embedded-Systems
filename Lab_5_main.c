/*
 * Complete Embedded System5.cpp
 *
; Todays date: 3/28/2024
; Authors: Matt Stensby, Shaun Richter
; Team: Circuit Breaker Brigade
; Description:
; This code implements the entire embedded system. It prescales proper clock times, as well as
; sets inputs and outputs for specific pins. It calculates the RPMs and outputs to the first line
; of the LCD. Based on the button press determines what mode and what gets displayed on the second line
; (fan status or duty cycle). Implemtns Fan, LCd, RGB, buzzer, and push button will calculating appropriate
; values. Utilizes interrupts and timers.
 */

// Define F_CPU which is arudino clock
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

//Include statements
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include "defines.h"
#include "lcd.h"
#include "avr/sfr_defs.h"

//Setup stream to output to LCD
FILE lcd_str = FDEV_SETUP_STREAM(lcd_putchar, NULL, _FDEV_SETUP_WRITE);

//Global variables
float compare = 100;
float top = 200;
uint8_t mode = 0; //determines mode1 or 2
uint16_t tach_ticks = 3800; //RPM's
uint8_t isOverflow = 0; //Flag bit set to 1 if timer overflow meaning rpm stalled
uint8_t clearTime = 0; //Variable used to clear LCD periodically

//Interupt used to update the timer compare value, everytime timer gets to "0" detects overflow and update compare value
ISR(TIMER0_OVF_vect){
	OCR0B = compare;
}

//This interrupt gets hit everytime timer1 hits a rising edge and then calculates
ISR(INT1_vect)
{
	uint8_t sreg;
	stop16BitTimer(); // Stop the timer.
	sreg = SREG; // Save global interrupt flag.
	cli(); // Turn of all interrupts.
	tach_ticks = TCNT1; // Grab the count (in us how long its been running)
	TCNT1 = 0x0000; // Start counting at 0.
	SREG = sreg; // Restore global interrupt flag.
	start16BitTimer(); // Start counter.
	isOverflow = 0;
}

//When Timer1 overflows and thus are at a stall
ISR(TIMER1_OVF_vect){
	isOverflow = 1;
}

//Sounds buzzer method declaration
void soundBuzzer();

// checking the status of the RPG method declaration
void  checkRPG();

// showing the duty cycle method declaration
void showDutyCycle();

// putting the device into check mode method declaration
void checkMode();

//Starts the 16 bit timer method declaration
void start16BitTimer();

//Stops the 16 bit timer method declaration
void stop16BitTimer();

//Calcuatles the RPM method declaration
float calculateRPM();




// Main program that prints out a string to the LCD. It also sounds the buzzer and it outputs a frequency 
// to PD5 by setting clock to mode5. This function sets all inputs outputs, prescales, enables interrupts,
// and has a while loop that updates the LCD, recalculates the RPM, checks if a stall, and updates the buzzer
int main(void)
{
	//LCD initialziation
	stdout = &lcd_str;
	lcd_init();
	sei(); //enables interrupts
	
	TIMSK0 = TIMSK0 | (1 << TOIE1); // overflow interrupt enable for timer0
	TIMSK1 = TIMSK1 | (1 << TOIE1); // overflow interrupt enabled for timer1
	
	clock_prescale_set(clock_div_2); //Setting Clock pre-scale value to 2
	
	
	//Set Outputs
	DDRD |= (1 << DDD5);
	
	//Set Input
	DDRB &= ~(1 << PINB0);
	DDRB &= ~(1 << PINB1);
	DDRD &= ~(1 << PIND2);
	DDRD &= ~(1 << PIND3);

	
	//Counter used for timer
	TCNT0 = 0x00;
	
	
	//Divide the clock scalar by 1 so clock runs on 8MHZ 
	TCCR0B = TCCR0B | (1 << CS00) | (1 << WGM02);
	
	//Sets the timer to Mode 5 through WGM0 bit. Com bits set register if clock counts up, clears register if counts down.
	// Toggles PD5 pin on compare match. WGM bits puts it into phase correct mode(generate a waveform)
	TCCR0A = TCCR0A | (1 << COM0B1) | (0 << COM0B0) | (0 << WGM01) | (1 << WGM00);
	
	//TOP
	OCR0A = 200;
	top = OCR0A;
	
	//COMPARE
	OCR0B = 100;
	compare = OCR0B;

	//Configure timer1 to trigger on rising edge
	EICRA = EICRA | (1 << 3) | (1 << 2); //Rising edge
	EIMSK = EIMSK | (1 << INT1); //Enables timer1 external interrput
	start16BitTimer();
	
	//Loop that updates the RPG, Duty Cycle, and checker to see what mode we are in, and sounds buzzer
	while (1) 
	{
		checkRPG();
		checkMode();
		showDutyCycle();
		if(isOverflow == 1){
			soundBuzzer();
		}
	}
}

// This function creates a frequency to sound the buzzer at PB2
void soundBuzzer(){
	// Set PB2 as an output
	DDRB |= (1 << PORTB2);  // Set the buzzer port as output
	clear();
	
	while(isOverflow == 1){
		checkRPG();
		home();
		
		// Ouputs to LCD that there is a stall
		printf("Fan RPM: %d", 0);
		row2();
		printf("Fan Stalled");

		// Set the buzzer on
		PORTB |= (1 << PORTB2);
	}
	
	//Set the buzzer off
	PORTB &= ~(1 << PORTB2);
}

// Checks if RPG was moved clockwise or counter clockwise. Makes sure compare value doesn't exceed 200 or go below 0
void checkRPG(){
	//If PB0 and PB1 are low
	if(bit_is_clear(PINB, 0) && bit_is_clear(PINB, 1)){
		while(bit_is_clear(PINB, 0) && bit_is_clear(PINB, 1))
		{
		}
		
		if(bit_is_set(PINB, 1)){ //clockwise
			if(compare < 200){
				compare += 1;
			}
		}
		else if(bit_is_set(PINB, 0)){ //counterclockwise
			if(compare > 0)
			{
				compare -= 1;
			}
		}
	}
	
	return;
}


// Will show the current duty cycle that was set by the RPG. Outputs RPM's and duty cycle if in mode1
// and outputs the calciulated RPM's and if the fan is ok if RPM's greater than or equal to 2400, and then
// low RPM if not
void showDutyCycle(){
	//Calculates the duty cycle
	float dutyCycle = (float)(2 *compare + 1)/(2*top); // given equation to find the duty cycle
	char dutyCycleStr[9];
	int dutyCycleInt = (int)(dutyCycle * 10000);
	
	//Gets left side of decimal and right side of duty cycle
	int leftOfDecimal = dutyCycleInt / 100;
	int rightOfDecimal = dutyCycleInt % 100 - 25;
	
	//Ensures proper notation
	if(leftOfDecimal >= 100){
		leftOfDecimal = 100;
		rightOfDecimal = 0;
	}
	
	//If dutyCycleInt greater than 1000 then we have 4 or 5 digits
	if(dutyCycleInt >= 1000){
		sprintf(dutyCycleStr, "%d.%02d%%", leftOfDecimal, rightOfDecimal);
	}
	//only 3 digits
	else{
		sprintf(dutyCycleStr, "%0d.%02d%%", leftOfDecimal, rightOfDecimal); //Format when integer falls into ones place
	}	
	
	
	//Calculates the RPM's
	float RPM = calculateRPM();
	int rpmINT = (int)(RPM);
	char rpmStr[5];
	sprintf(rpmStr, "%d", rpmINT);
	
	// Checks mode 1 outputs FAN RPM on line1 and duty cycle on line2
	if(mode == 0){
		home();
		printf("Fan RPM:%s", rpmStr);
		row2();
		printf("D: %s", dutyCycleStr);
	} //If in mode 2 output FAN RPM on line1 and fan status on line2
	else if(mode == 1){
		home();
		printf("Fan RPM:%s", rpmStr);
		row2();
		if(RPM < 2400){
			printf("Low RPM");			
		}
		else{
			printf("Fan OK");
		}
	}
	
	//Periodically clears the LCD
	clearTime++;
	if(clearTime > 100){
		clearTime = 0;
		clear();
	}
}

//Checks if the button was pressed if it was change the value of mode(0,1)
void checkMode(){
	if(bit_is_clear(PIND, 2)){
		while(bit_is_clear(PIND, 2)){} // waits for the button to be released and then sets the mode to 0 or 1
		mode = !mode;
		clear();
	}
}

//Stops 16 bit timer to be used for RPM Calculations
void stop16BitTimer(){
	TCCR1B = TCCR1B & ~(1 << CS12);
	TCCR1B = TCCR1B & ~(1 << CS11);
	TCCR1B = TCCR1B & ~(1 << CS10); 
}

//Starts 16 bit timer to be used for RPM calculations
void start16BitTimer(){
	TCCR1B = (1 << CS11); //Starts timer y setting bit
}

//Calculates the RPM based off of the calculated period
float calculateRPM(){
	float rpm; //RPM's
	float T; //Period
	T = tach_ticks * (1e-6);
	T = T*2.0; 
	rpm = 60.0/T; // Puts it into seconds
	return rpm;
}



