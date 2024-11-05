;
; SevenSegDisplay4.asm
;
; Created: 2/19/2024 12:15:15 PM
; Author: Matt Stensby and Shaun Richter
; Team: Circuit Breaker Brigade

; Code Summary: The code below uses a microprocessor, two drivers,
; and two seven segment displays to display a tenths number and ones
; number. Every tenth of a second the "next" numeric(decimal) number
; is displayed. Goes from 0.0 to 9.9. Button pressed then on release
; timer should start. When pressed again timer should stop, when
; released, nothing should happen. When pressed again and then released
; timer should go to 0.0
;

; Sets pins in use to be approprate input or output
more:
	sbi DDRB, 0 ;Pin 0 is set to output(OE) - active low
	sbi DDRB, 1 ;Pin 1 is set to output(LE)
	sbi DDRB, 2 ;Pin 2 is set to output(clk)
	cbi DDRB, 3 ;Pin 3 is set to input(button)
	sbi DDRB, 4 ;Pin 4 is set to output(sdi)

	; Initial setup
	sbi PORTB, 0
	cbi PORTB, 1
	cbi PORTB, 2

	ldi r28, 0 ; count
	ldi r29, 0 ; tenths
	ldi r30, 0 ; ones
	ldi r31, 0 ; button helper count

	rjmp main

; This is the main loop of the program. It incrments registers for
; tenths and ones place, checks values, and defines button
; functionality and behavior
main: 
	rcall shiftZero; load to tenths place
	rcall shiftZero; load to ones place
	
	; latch data then send(enable) output
	sbi PORTB, 1
	cbi PORTB, 1
	cbi PORTB, 0
	
	;if button is not pressed continue loop(display 0.0) if pressed check if released
	sbic PINB, 3 
	rjmp main

	;if button is not released continue in loop, if released then go to loop pointer
	onRelease: 
		sbis PINB, 3 ;skip bit is button is set(not pressed)
		rjmp onRelease

	; Main loop this increments both seven segment displays. It checks the current
	; value of seven segment displays. Determines the values of the tenths place
	; and the ones place, and then send the data and diplay in the seven segment display.
	loop:
		a0: cpi r29, 0		;Checks if r29 is 0.
			brne a1			;If r29 is not 0 then branch to a1. If it is then call shiftZero and jump to aend as tenths place is done
			rcall shiftZero ;Shifts bits for zero to display "tenths" in seven segment display
			rjmp aend		;Finished tenths place so update ones place values now
							;IMPORTANT: The above in line comments are the same for a1 - a9, but compares different values and shifts the cooresponding value
		a1: cpi r29, 1
			brne a2
			rcall shiftOne
			rjmp aend
		a2: cpi r29, 2
			brne a3
			rcall shiftTwo
			rjmp aend
		a3: cpi r29, 3
			brne a4
			rcall shiftThree
			rjmp aend
		a4: cpi r29, 4
			brne a5
			rcall shiftFour
			rjmp aend
		a5: cpi r29, 5
			brne a6
			rcall shiftFive
			rjmp aend
		a6: cpi r29, 6
			brne a7
			rcall shiftSix
			rjmp aend
		a7: cpi r29, 7
			brne a8
			rcall shiftSeven
			rjmp aend
		a8: cpi r29, 8
			brne a9
			rcall shiftEight
			rjmp aend
		a9: cpi r29, 9
			rcall shiftNine
			rjmp aend
		;Do nothing
		aend:				;Finished with tenths place move to ones place now
		b0: cpi r30, 0		;Checks if r30 is 0
			brne b1			;If r30 is not 0 then branch to b1. If it is then call shiftZero and jukp to bend as ones place is done
			rcall shiftZero	;Shifts bits for zero to display in "ones" seven segment display
			rjmp bend		;Finished the ones place so update logic/register values now
							;IMPORTANT: The above in line comments are the same for b0 - b9, but compares different values and shifts the cooresponding value
		b1: cpi r30, 1
			brne b2
			rcall shiftone
			rjmp bend
		b2: cpi r30, 2
			brne b3
			rcall shiftTwo
			rjmp bend
		b3: cpi r30, 3
			brne b4
			rcall shiftThree
			rjmp bend
		b4: cpi r30, 4
			brne b5
			rcall shiftFour
			rjmp bend
		b5: cpi r30, 5
			brne b6
			rcall shiftFive
			rjmp bend
		b6: cpi r30, 6
			brne b7
			rcall shiftSix
			rjmp bend
		b7: cpi r30, 7
			brne b8
			rcall shiftSeven
			rjmp bend
		b8: cpi r30, 8
			brne b9
			rcall shiftEight
			rjmp bend
		b9: cpi r30, 9
			rcall shiftNine
			rjmp bend
		;Latches the data in the drivers and sends the data. Updates
		;the values that stores the value of the ones place, tenths place
		;and totla value
		bend:
			sbi PORTB, 1	;Latch the data
			cbi PORTB, 1	;Latch the data
			cbi PORTB, 0	;Enable output
			inc r28			;inc total value(if 100 then passed 9.9)
			cpi r28, 100	;Check if value is 100, if 100 then overflow
			brne c1			;if not equal to 100 then branch to c1
			rcall overflow	;overflow is hit
		c1:
			inc r29			;increment the tenths place
			cpi r29, 10		;If tenths place is at 10, then increment ones place
			brne c2			;If not ten go to c2
			inc r30			;increment the ones palce
			ldi r29, 0		;set tenths place back to 0
		c2:
			sbis PINB, 3 ;if button is not pressed skip line below
			ldi r31, 1	 ;button is pressed load r31 into 1
			sbis PINB, 3 ;if button is not pressed skip line below
			rjmp helper2 ;if button is pressed jmp to helper 2 function


		rcall delay ;Delay for a tenth of a second
		rjmp loop ;Continue loop

		;Button has been pressed check if released to move to next helper
		helper2:
			sbic PINB, 3 
			ldi r31, 2	 ;if button is not pressed ldi is 2(released)
			cpi r31, 2	 ;Compares value to 2
			brne helper3 ;If r31 is not equal to 2 branch to helper3 as button is still pressed for first time
			rjmp helper2 ;Continue to display current value

		;Checks to see if button is still released
		helper3:
			sbic PINB, 3
			rjmp helper4 ;Button has been released jump to helper 4
			rjmp helper3 ;Button is still pressed for first time continue loop to display current value

		;Checks if button is pressed for second time
		helper4:
			sbic PINB, 3
			rjmp helper4 ;If button is released for first time still show current display

		;Checks if button is released to reset back to startup state
		pointer:
			sbis PINB, 3
			rjmp pointer ;If button is pressed for second time continue loop, display current value
			rjmp helper5 ;If button is released jump to helper 5, to display reset
		
		;Reset to startup state
		helper5:
			ldi r28, 0 ; count
			ldi r29, 0 ; ones
			ldi r30, 0 ; tenths
			rjmp main  ;Restart

			

	;Displays overflow functionality
	overflow:
		rcall shiftV; load to tenths place
		rcall shiftO; load to ones place
		sbi PORTB, 1
		cbi PORTB, 1
		cbi PORTB, 0

		rcall delay
		rcall delay
		rcall delay
		rcall delay
		rcall delay

		rcall shiftNothing; load to tenths place
		rcall shiftNothing; load to ones place
		sbi PORTB, 1
		cbi PORTB, 1
		cbi PORTB, 0
		rcall delay
		rcall delay
		rcall delay
		rcall delay
		rcall delay


		rjmp overflow


	rjmp main

;Creates a clk cylce
clk:
	sbi PORTB, 2 ;Clock on
	cbi PORTB, 2 ;Clock off
	ret

;Create zero display
shiftZero:
	sbi PORTB, 4 ;(DP)
	rcall clk
	cbi PORTB, 4 ;(G)
	rcall clk
	sbi PORTB, 4 ;(F)
	rcall clk
	sbi PORTB, 4 ;(E)
	rcall clk
	sbi PORTB, 4 ;(D)
	rcall clk
	sbi PORTB, 4 ;(C)
	rcall clk
	sbi PORTB, 4 ;(B)
	rcall clk
	sbi PORTB, 4 ;(A)
	rcall clk
	ret

;Create one display
shiftOne:
	sbi PORTB, 4 ;(DP)
	rcall clk
	cbi PORTB, 4 ;(G)
	rcall clk
	cbi PORTB, 4 ;(F)
	rcall clk
	cbi PORTB, 4 ;(E)
	rcall clk
	cbi PORTB, 4 ;(D)
	rcall clk
	sbi PORTB, 4 ;(C)
	rcall clk
	sbi PORTB, 4 ;(B)
	rcall clk
	cbi PORTB, 4 ;(A)
	rcall clk
	ret

;Create two display
shiftTwo:
	sbi PORTB, 4 ;(DP)
	rcall clk
	sbi PORTB, 4 ;(G)
	rcall clk
	cbi PORTB, 4 ;(F)
	rcall clk
	sbi PORTB, 4 ;(E)
	rcall clk
	sbi PORTB, 4 ;(D)
	rcall clk
	cbi PORTB, 4 ;(C)
	rcall clk
	sbi PORTB, 4 ;(B)
	rcall clk
	sbi PORTB, 4 ;(A)
	rcall clk
	ret

;Create three display
shiftThree:
	sbi PORTB, 4 ;(DP)
	rcall clk
	sbi PORTB, 4 ;(G)
	rcall clk
	cbi PORTB, 4 ;(F)
	rcall clk
	cbi PORTB, 4 ;(E)
	rcall clk
	sbi PORTB, 4 ;(D)
	rcall clk
	sbi PORTB, 4 ;(C)
	rcall clk
	sbi PORTB, 4 ;(B)
	rcall clk
	sbi PORTB, 4 ;(A)
	rcall clk
	ret

;Create four display
shiftFour:
	sbi PORTB, 4 ;(DP)
	rcall clk
	sbi PORTB, 4 ;(G)
	rcall clk
	sbi PORTB, 4 ;(F)
	rcall clk
	cbi PORTB, 4 ;(E)
	rcall clk
	cbi PORTB, 4 ;(D)
	rcall clk
	sbi PORTB, 4 ;(C)
	rcall clk
	sbi PORTB, 4 ;(B)
	rcall clk
	cbi PORTB, 4 ;(A)
	rcall clk
	ret

;Create five display
shiftFive:
	sbi PORTB, 4 ;(DP)
	rcall clk
	sbi PORTB, 4 ;(G)
	rcall clk
	sbi PORTB, 4 ;(F)
	rcall clk
	cbi PORTB, 4 ;(E)
	rcall clk
	sbi PORTB, 4 ;(D)
	rcall clk
	sbi PORTB, 4 ;(C)
	rcall clk
	cbi PORTB, 4 ;(B)
	rcall clk
	sbi PORTB, 4 ;(A)
	rcall clk
	ret

;Create six display
shiftSix:
	sbi PORTB, 4 ;(DP)
	rcall clk
	sbi PORTB, 4 ;(G)
	rcall clk
	sbi PORTB, 4 ;(F)
	rcall clk
	sbi PORTB, 4 ;(E)
	rcall clk
	sbi PORTB, 4 ;(D)
	rcall clk
	sbi PORTB, 4 ;(C)
	rcall clk
	cbi PORTB, 4 ;(B)
	rcall clk
	sbi PORTB, 4 ;(A)
	rcall clk
	ret

;Create seven display
shiftSeven:
	sbi PORTB, 4 ;(DP)
	rcall clk
	cbi PORTB, 4 ;(G)
	rcall clk
	cbi PORTB, 4 ;(F)
	rcall clk
	cbi PORTB, 4 ;(E)
	rcall clk
	cbi PORTB, 4 ;(D)
	rcall clk
	sbi PORTB, 4 ;(C)
	rcall clk
	sbi PORTB, 4 ;(B)
	rcall clk
	sbi PORTB, 4 ;(A)
	rcall clk
	ret

;Create eight display
shiftEight:
	sbi PORTB, 4 ;(DP)
	rcall clk
	sbi PORTB, 4 ;(G)
	rcall clk
	sbi PORTB, 4 ;(F)
	rcall clk
	sbi PORTB, 4 ;(E)
	rcall clk
	sbi PORTB, 4 ;(D)
	rcall clk
	sbi PORTB, 4 ;(C)
	rcall clk
	sbi PORTB, 4 ;(B)
	rcall clk
	sbi PORTB, 4 ;(A)
	rcall clk
	ret

;Create nine display
shiftNine:
	sbi PORTB, 4 ;(DP)
	rcall clk
	sbi PORTB, 4 ;(G)
	rcall clk
	sbi PORTB, 4 ;(F)
	rcall clk
	cbi PORTB, 4 ;(E)
	rcall clk
	sbi PORTB, 4 ;(D)
	rcall clk
	sbi PORTB, 4 ;(C)
	rcall clk
	sbi PORTB, 4 ;(B)
	rcall clk
	sbi PORTB, 4 ;(A)
	rcall clk
	ret

;Create "O" display for overflow
ShiftO:
	cbi PORTB, 4 ;(DP)
	rcall clk
	cbi PORTB, 4 ;(G)
	rcall clk
	sbi PORTB, 4 ;(F)
	rcall clk
	sbi PORTB, 4 ;(E)
	rcall clk
	sbi PORTB, 4 ;(D)
	rcall clk
	sbi PORTB, 4 ;(C)
	rcall clk
	sbi PORTB, 4 ;(B)
	rcall clk
	sbi PORTB, 4 ;(A)
	rcall clk
	ret
;Create 'V' display for overflow
shiftV:
	cbi PORTB, 4 ;(DP)
	rcall clk
	cbi PORTB, 4 ;(G)
	rcall clk
	sbi PORTB, 4 ;(F)
	rcall clk
	sbi PORTB, 4 ;(E)
	rcall clk
	sbi PORTB, 4 ;(D)
	rcall clk
	sbi PORTB, 4 ;(C)
	rcall clk
	sbi PORTB, 4 ;(B)
	rcall clk
	cbi PORTB, 4 ;(A)
	rcall clk
	ret

;Light nothing
shiftNothing:
	cbi PORTB, 4 ;(DP)
	rcall clk
	cbi PORTB, 4 ;(G)
	rcall clk
	cbi PORTB, 4 ;(F)
	rcall clk
	cbi PORTB, 4 ;(E)
	rcall clk
	cbi PORTB, 4 ;(D)
	rcall clk
	cbi PORTB, 4 ;(C)
	rcall clk
	cbi PORTB, 4 ;(B)
	rcall clk
	cbi PORTB, 4 ;(A)
	rcall clk
	ret


;Delay for a tenth of a second
delay:
      ldi   r23,5      ; r23 <-- Counter for outer loop
  d1: ldi   r24,208     ; r24 <-- Counter for level 2 loop 
  d2: ldi   r25,255     ; r25 <-- Counter for inner loop
      nop               ; no operation
  d3: dec   r25
      brne  d3 
      dec   r24
      brne  d2
      dec   r23
      brne  d1

	  ldi r26, 54
  d4: nop
	  dec r26
	  brne d4
      ret

	


	
