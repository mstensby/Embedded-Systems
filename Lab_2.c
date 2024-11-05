;
; tilt.asm
;
; Created: 1/31/2024 6:17:15 PM
; Author : Matt Stensby & shaun richter
;
; SUMMARY: The code below utilizes a tilt switch which in turn, when "tilted"
; will sounds a buzzer. A green led is light when not tilted, and the buzzer is off
; When the device is tilted, the green led shall be turned off, and the buzzer 
; should sound. 
;
;
.include "tn45def.inc"
.cseg
.org 0

; Sets pins in use to be approprate input or output
more:
	sbi DDRB,3 ;PB3 is now output
	sbi DDRB,4 ;PB4 is now output

	cbi DDRB,0 ;PB0 is now input

; This is the main function of the program it checks if tilt switch is set
; If set the board is not tilted and alarm should not be called. Turns the led
; at PB3 on
loop:
	sbis PINB, 0; If PB0 is set then board is not tilted
	rcall alarm ;Calls alarm function to turn LED off and sound buzzer

	sbi PORTB, 3; LED at PB3 turned on

	rcall isPressed ;Debounce
	rjmp loop

; This function enables the sounds of the buzzer. It implements the delay
; function in order to create a frequency and this will sound the buzzer. 
buzz:
	cbi PORTB, 4 ;Pull PB4 low
	rcall  delay
	sbi PORTB,4 ;Pull PB4 high
	rcall delay

	; If pin at PB0 is set board is not titled and leave function, 
	; else continue buzzing
	sbis PINB, 0
	rjmp buzz

	ret

; This function creates a delay of approzimatley 232.5 micro seconds
; using loops. 232.5 microseconds equates a total frequency of 4.3 kHz
; and an on frequency of 2.15 kHz. Multiplied given frequency by 2 to get total period. T = 1\f
delay:
	ldi r24, 5
	d2: ldi r25, 122
		nop
	d3: dec r25
		brne d3
		dec r24
		brne d2
	nop
	nop
	nop
	nop
	ret

; This function turns the LED off, and calls the buzzer
; function in order to enable sound from the hardware buzzer.
alarm:
	cbi PORTB, 3 ;Turn off LED
	rcall buzz
	rjmp loop

;Debounce delay run a loop in a loop for a delay of about 5ms
isPressed:
	ldi r24, 255
	d9: ldi r25, 150
		nop
	d8: dec r25
		brne d8
		dec r24
		brne d9

	ret
	


.exit
	