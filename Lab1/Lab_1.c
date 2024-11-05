;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; Assembly language file for Lab 1 in 55:036 (Embedded Systems)
; Spring ECE:3360. The University of Iowa.
;
; LEDs are connected via a 470 Ohm resistor from PB3, PB4 to Vcc
; or GND.
;
; This program enables two different LED's and lights them for different times.
; The white led should be lit for 325ms and the blue led should be lit for 320ms
;
; Shaun Richter and Matt Stensby
;
.include "tn45def.inc"
.cseg
.org 0
; Configure PB3 and PB4 as output pins.
more:
	sbi DDRB,3 ; PB3 is now output
	sbi DDRB,4 ; PB4 is now output
; Main loop follows. Toggle PB1 and PB2 out of phase.
; Assuming there are LEDs and current-limiting resistors
; on these pins, they will blink out of phase.
loop:
	sbi PORTB,3 ; LED at PB3 off
	cbi PORTB,4 ; LED at PB4 on
	rcall delay_white;
	
	cbi PORTB,3 ; LED at PB3 on
	sbi PORTB,4 ; LED at PB4 off
	rcall delay_blue ; Wait

	rjmp loop
; Generate a delay using three nested loops that does nothing.
; With a 8 MHz clock, the values below produce 325 ms delay.
	delay_white:
		ldi r23,14 ; r23 <-- Counter for outer loop
	d1: ldi r24,242 ; r24 <-- Counter for level 2 loop ldi r24, 242 instruction stored at the address pointed to by d1
	d2: ldi r25,254 ; r25 <-- Counter for inner loop
		nop ; no operation
		nop ;
	d3: dec r25
		brne d3
		dec r24
		brne d2
		dec r23
		brne d1
		rcall delay_alot ;function call
		ret
; Generate a delay using three nested loops that does nothing.
; With a 8 MHz clock, the values below produce 320 ms delay.
	delay_blue:
		ldi r20,13 ; r23 <-- Counter for outer loop
	b1: ldi r21,255 ; r24 <-- Counter for level 2 loop
	b2: ldi r22,255 ; r25 <-- Counter for inner loop
		nop ; no operation
	b3: dec r22
		brne b3
		dec r21
		brne b2
		dec r20
		brne b1
		rcall delay_alot ; function call
		rcall delay_alot ; function call

		ldi r18,5 ; loop 5 times
		b4: rcall delay_alot ;function call
		dec r18 ;decrement value
		brne b4

		ldi r16, 202 ; loop 202 times
		b5: nop
		nop
		nop
		dec r16 ; decrement value
		brne b5

		nop

		ret
; Generate a delay using one loop. Is a helper function to be called	
	delay_alot:
		ldi r19,224 ; r19 <-- Counter for loop to run 224 times
		c1: nop; ; nop called 3 seperate times
			nop
			nop
		dec r19 ; decrement value
		brne c1
		nop
		nop
		nop
		nop
		nop
		ret

.exit