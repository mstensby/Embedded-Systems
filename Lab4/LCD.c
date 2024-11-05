
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;
; Description: 
;
; Lab04.asm
;
; Created: 2/27/2024 3:19:25 PM
; Todays date: 2/22/2024 
; Authors: Matt Stensby, Shaun Richter
; Team: Circuit Breaker Brigade
;Description:
; In this lab, the goal is to program an LCD so that it will scroll through 7 total messages. The first one is just an introduction message
; That tells the user to press the button to scroll to the next line. As the button continues to be pressed, the LCD will display the new words line by line.
; After all 7 messages are displayed, it will circle back to the desired words in an infinite loop. The LED will always stay lit due to the nibble 
.include "m328pdef.inc"
.org 0

.cseg
	msg1: .db "Press to Scroll ", 0x00
	msg2: .db "Software        ", 0x00
	msg3: .db "Development     ", 0x00
	msg4: .db "Framework       ", 0x00
	msg5: .db "Version         ", 0x00
	msg6: .db "Debugging       ", 0x00
	msg7: .db "Integration     ", 0x00

more:
	cbi DDRD, 2 ;Button set to input
	sbi DDRB, 3 ;Enable
	sbi DDRB, 5 ;Instruction/Register Select

	sbi DDRC, 0 ;input to LCD
	sbi DDRC, 1 ;input to LCD
	sbi DDRC, 2 ;input to LCD
	sbi DDRC, 3 ;input to LCD
	sbi DDRC, 5 ;set to output
	cbi PORTC, 5

.MACRO writeNibble
	push @0
	push r26
	cbr @0, 0xF0
	in r26, @1
	cbr r26, 0x0F ;1111 0000 everthing there is a 1 gets cleared, leaves lower nibble
	or r26, @0
	out @1, r26
	pop r26
	pop @0

.ENDMACRO
 


initialization:
	rcall delay100m

	rcall setEightBitMode
	rcall delay5m

	rcall setEightBitMode
	rcall delay200u

	rcall setEightBitMode
	rcall delay200u

	rcall set4BitMode
	rcall delay200u

	rcall createChar1
	rcall createChar2

	rcall entrymode
	rcall delay40u

	rcall clearDisplay

	rcall delay1_5m
	
	cbi PORTB,5
	
	ldi r25, 0x0
	writeNibble r25, PORTC
	rcall LCDStrobe
	ldi r25, 0xc
	rcall write

	ldi r25, 0x0
	writeNibble r25, PORTC
	rcall LCDStrobe
	ldi r25, 0x6
	rcall write

	ldi r25, 0x0
	writeNibble r25, PORTC
	rcall LCDStrobe
	ldi r25, 0x0
	rcall write
	
	;rcall writeChar1
	;rcall writeChar2


	rjmp scrollSpecialChar


displayCString:
	lpm r0, Z+
	tst r0
	breq done
	swap r0
	out PORTC, r0
	rcall LCDStrobe
	swap r0
	out PORTC, r0
	rcall LCDStrobe
	rjmp displayCString
done:
	ret

write:

	writeNibble r25, PORTC 
	rcall LCDStrobe ; Strobe Enable line.
	rcall delay100u ; Wait.
	ret

line1:
	cbi PORTB, 5
	ldi r25, 0x8
	rcall write
	ldi r25, 0x0
	rcall write
	ret
line2:
	cbi PORTB, 5
	ldi r25, 0xc
	rcall write
	ldi r25, 0x0
	rcall write
	ret


setEightBitMode:
	cbi PORTB, 5
	ldi r27, 0x3
	out PORTC, r27
	ret

set4BitMode:
	cbi PORTB, 5
	ldi r25, 0x2
	out PORTC, r25
	ret

entryMode:
	cbi PORTB, 5
	ldi r27, 0x2
	writeNibble r27, PORTC 
	rcall LCDStrobe
	ldi r27, 0x8
	writeNibble r27, PORTC 
	rcall LCDStrobe
	ret

clearDisplay:
	cbi PORTB, 5
	ldi r25, 0x0
	writeNibble r25, PORTC
	rcall LCDStrobe
	ldi r25, 0x1
	out PORTC, r25
	rcall LCDStrobe
	ret

LCDStrobe:
	sbi PORTB, 3
	rcall delay100u
	cbi PORTB, 3
	ret


delay200u:
	ldi r23,26 ; r23 <-- Counter for outer loop
	d1: ldi r24,17 ; r24 <-- Counter for level 2 loop
	nop ; no operation
	nop ; no operation
	nop ; no operation
	d2: ldi r25,1 ; r25 <-- Counter for inner loop
	nop ; no operation
	d3: dec r25
	brne d3
	dec r24
	brne d2
	dec r23
	brne d1
	ret

delay100u:
	ldi r23,12 ; r23 <-- Counter for outer loop
	d4: ldi r24,17 ; r24 <-- Counter for level 2 loop
	nop ; no operation
	nop ; no operation
	nop ; no operation
	d5: ldi r25,1 ; r25 <-- Counter for inner loop
	nop ; no operation
	d6: dec r25
	brne d6
	dec r24
	brne d5
	dec r23
	brne d4
	ret

; function for creating a delay of approximately 0.1 seconds
delay100m:
	ldi r23, 10 ; r23 <-- Counter for outer loop
	nop
	d7: ldi r24, 214 ; r24 <-- Counter for level 2 loop
	d8: ldi r25, 248 ; r25 <-- Counter for inner loop
	nop ; no operation
	d9: dec r25
	brne d9
	dec r24
	brne d8
	dec r23
	brne d7
	ret ; return

delay5m:
	ldi r23, 2 ; r23 <-- Counter for outer loop
	nop
	d10: ldi r24, 65 ; r24 <-- Counter for level 2 loop
	d11: ldi r25, 200 ; r25 <-- Counter for inner loop
	nop ; no operation
	d12: dec r25
	brne d12
	dec r24
	brne d11
	dec r23
	brne d10
	ret ; return

delay40u:
	ldi r23,5 ; r23 <-- Counter for outer loop
	d13: ldi r24,17 ; r24 <-- Counter for level 2 loop
	nop ; no operation
	nop ; no operation
	nop ; no operation
	d14: ldi r25,1 ; r25 <-- Counter for inner loop
	nop ; no operation
	d15: dec r25
	brne d15
	dec r24
	brne d14
	dec r23
	brne d13
	ret

delay1_5m:
	rcall delay100u
	rcall delay100u
	rcall delay100u
	rcall delay100u
	rcall delay100u
	rcall delay100u
	rcall delay100u
	rcall delay100u
	rcall delay100u
	rcall delay100u
	rcall delay100u
	rcall delay100u
	rcall delay100u
	rcall delay100u
	rcall delay100u
	ret



scrollSpecialChar:
	rcall line1
	sbi PORTB, 5
	ldi r30,LOW(2*msg1)
	ldi r31,2*HIGH(msg1)
	rcall displayCString

	rcall line2
	rcall writeChar1
	rcall writeChar2


	sbis PIND, 2
	rcall specialCharSoftware

	rjmp scrollSpecialChar

specialCharSoftware:
	rcall line1
	rcall writeChar1
	rcall writeChar2

	rcall line2
	sbi PORTB, 5
	ldi r30,LOW(2*msg2)
	ldi r31,2*HIGH(msg2)
	rcall displayCString

	sbic PIND, 2
	rcall callSoftDev

	rjmp specialCharSoftware


softwareDevelopment:
	rcall line1
	sbi PORTB, 5
	ldi r30,LOW(2*msg2)
	ldi r31,2*HIGH(msg2)
	rcall displayCString

	rcall line2
	sbi PORTB, 5
	ldi r30,LOW(2*msg3)
	ldi r31,2*HIGH(msg3)
	rcall displayCString

	sbic PIND, 2
	rcall callDevFrame

	rjmp softwareDevelopment

developmentFramework:
	rcall line1
	sbi PORTB, 5
	ldi r30,LOW(2*msg3)
	ldi r31,2*HIGH(msg3)
	rcall displayCString

	rcall line2
	sbi PORTB, 5
	ldi r30,LOW(2*msg4)
	ldi r31,2*HIGH(msg4)
	rcall displayCString
	sbic PIND, 2
	rcall callFrameVers
	rjmp developmentFramework

frameworkVersion:
	rcall line1
	sbi PORTB, 5
	ldi r30,LOW(2*msg4)
	ldi r31,2*HIGH(msg4)
	rcall displayCString

	rcall line2
	sbi PORTB, 5
	ldi r30,LOW(2*msg5)
	ldi r31,2*HIGH(msg5)
	rcall displayCString
	sbic PIND, 2
	rcall callVersDebug
	rjmp frameworkVersion

versionDebugging:
	rcall line1
	sbi PORTB, 5
	ldi r30,LOW(2*msg5)
	ldi r31,2*HIGH(msg5)
	rcall displayCString

	rcall line2
	sbi PORTB, 5
	ldi r30,LOW(2*msg6)
	ldi r31,2*HIGH(msg6)
	rcall displayCString
	sbic PIND, 2
	rcall callDebugIntegration
	rjmp versionDebugging

debugIntegration:
	rcall line1
	sbi PORTB, 5
	ldi r30,LOW(2*msg6)
	ldi r31,2*HIGH(msg6)
	rcall displayCString

	rcall line2
	sbi PORTB, 5
	ldi r30,LOW(2*msg7)
	ldi r31,2*HIGH(msg7)
	rcall displayCString

	sbic PIND, 2
	rcall callIntSpecialChar
	rjmp debugIntegration

integrationSpecialChar:
	rcall line1
	sbi PORTB, 5
	ldi r30,LOW(2*msg7)
	ldi r31,2*HIGH(msg7)
	rcall displayCString

	rcall line2
	sbi PORTB, 5
	rcall writeChar1
	rcall writeChar2
	
	sbic PIND, 2
	rcall callSpecialCharSoftware
	rjmp integrationSpecialChar



integrationSoftware:
	rcall line1
	sbi PORTB, 5
	ldi r30,LOW(2*msg7)
	ldi r31,2*HIGH(msg7)
	rcall displayCString

	rcall line2
	sbi PORTB, 5
	ldi r30,LOW(2*msg2)
	ldi r31,2*HIGH(msg2)
	rcall displayCString
	sbic PIND,2
	rcall callSoftDev
	rjmp integrationSoftware


callDevFrame:
	sbis PIND, 2
	rjmp developmentFramework
	rjmp callDevFrame
callFrameVers:
	sbis PIND, 2
	rjmp frameworkVersion
	rjmp callFrameVers
callVersDebug:
	sbis PIND, 2
	rjmp versionDebugging
	rjmp callVersDebug
callDebugIntegration:
	sbis PIND, 2
	rjmp debugIntegration
	rjmp callDebugIntegration
callIntSpecialChar:
	sbis PIND, 2
	rjmp integrationSpecialChar
	rjmp callIntSpecialChar
callSoftDev:
	sbis PIND, 2
	rjmp softwareDevelopment
	rjmp callSoftDev

callSpecialCharSoftware:
	sbis PIND, 2
	rjmp specialCharSoftware
	rjmp callSpecialCharSoftware


writeChar:
	sbi PORTB, 5
	rcall write
	ret

createChar1:
	cbi PORTB, 5	;command mode
	ldi r25, 0x4	; programming first CGRAM position
	out PORTC, r25
	rcall LCDStrobe ; Strobe Enable line.
	ldi r25, 0x0
	out PORTC, r25
	rcall LCDStrobe

	cbi PORTB, 5	;command mode
	ldi r25, 0x4	; programming first CGRAM position
	out PORTC, r25
	rcall LCDStrobe ; Strobe Enable line.
	ldi r25, 0x1
	out PORTC, r25
	rcall LCDStrobe

	cbi PORTB, 5	;command mode
	ldi r25, 0x4	; programming first CGRAM position
	out PORTC, r25
	rcall LCDStrobe ; Strobe Enable line.
	ldi r25, 0x2
	out PORTC, r25
	rcall LCDStrobe

	cbi PORTB, 5	;command mode
	ldi r25, 0x4	; programming first CGRAM position
	out PORTC, r25
	rcall LCDStrobe ; Strobe Enable line.
	ldi r25, 0x3
	out PORTC, r25
	rcall LCDStrobe

	cbi PORTB, 5	;command mode
	ldi r25, 0x4	; programming first CGRAM position
	out PORTC, r25
	rcall LCDStrobe ; Strobe Enable line.
	ldi r25, 0x4
	out PORTC, r25
	rcall LCDStrobe


	cbi PORTB, 5	; command mode
	ldi r25, 0x4	; programming first CGRAM position
	out PORTC, r25
	rcall LCDStrobe ; Strobe Enable line.
	ldi r25, 0x5
	out PORTC, r25
	rcall LCDStrobe

	cbi PORTB, 5	;command mode
	ldi r25, 0x4	; programming first CGRAM position
	out PORTC, r25
	rcall LCDStrobe ; Strobe Enable line.
	ldi r25, 0x6
	out PORTC, r25
	rcall LCDStrobe

	cbi PORTB, 5	;command mode
	ldi r25, 0x4	; programming first CGRAM position
	out PORTC, r25
	rcall LCDStrobe ; Strobe Enable line.
	ldi r25, 0x7
	out PORTC, r25
	rcall LCDStrobe

	ret

; creates a special character accessable at 0x02
createChar2:
	cbi PORTB, 5	;command mode
	ldi r25, 0x5	; programming first CGRAM position
	out PORTC, r25
	rcall LCDStrobe ; Strobe Enable line.
	ldi r25, 0x0
	out PORTC, r25
	rcall LCDStrobe

	cbi PORTB, 5	;command mode
	ldi r25, 0x5	; programming first CGRAM position
	out PORTC, r25
	rcall LCDStrobe ; Strobe Enable line.
	ldi r25, 0x1
	out PORTC, r25
	rcall LCDStrobe

	cbi PORTB, 5	;command mode
	ldi r25, 0x5	; programming first CGRAM position
	out PORTC, r25
	rcall LCDStrobe ; Strobe Enable line.
	ldi r25, 0x2
	out PORTC, r25
	rcall LCDStrobe

	cbi PORTB, 5	;command mode
	ldi r25, 0x5	; programming first CGRAM position
	out PORTC, r25
	rcall LCDStrobe ; Strobe Enable line.
	ldi r25, 0x3
	out PORTC, r25
	rcall LCDStrobe

	cbi PORTB, 5	;command mode
	ldi r25, 0x5	; programming first CGRAM position
	out PORTC, r25
	rcall LCDStrobe ; Strobe Enable line.
	ldi r25, 0x4
	out PORTC, r25
	rcall LCDStrobe

	cbi PORTB, 5	;command mode
	ldi r25, 0x5	; programming first CGRAM position
	out PORTC, r25
	rcall LCDStrobe ; Strobe Enable line.
	ldi r25, 0x5
	out PORTC, r25
	rcall LCDStrobe

	cbi PORTB, 5	;command mode
	ldi r25, 0x5	; programming first CGRAM position
	out PORTC, r25
	rcall LCDStrobe ; Strobe Enable line.
	ldi r25, 0x6
	out PORTC, r25
	rcall LCDStrobe

	cbi PORTB, 5	;command mode
	ldi r25, 0x5	; programming first CGRAM position
	out PORTC, r25
	rcall LCDStrobe ; Strobe Enable line.
	ldi r25, 0x7
	out PORTC, r25
	rcall LCDStrobe

	ret

writeChar1:
	ldi r25, 0x0
	rcall writeChar
	ldi r25, 0xE
	rcall writeChar
	ret

writeChar2:
	ldi r25, 0x0
	rcall writeChar
	ldi r25, 0xA
	rcall writeChar

	;ldi r25, 0x0
	;rcall writeChar
	;ldi r25, 0xA
	;rcall writeChar

	;ldi r25, 0x0
	;rcall writeChar
	;ldi r25, 0xA
	;rcall writeChar

	;ldi r25, 0x0
	;rcall writeChar
	;ldi r25, 0x0
	;rcall writeChar

	;ldi r25, 0x1
	;rcall writeChar
	;ldi r25, 0x1
	;rcall writeChar

	;ldi r25, 0x1
	;rcall writeChar
	;ldi r25, 0x1
	;rcall writeChar

	;ldi r25, 0x1
	;rcall write
	;ldi r25, 0xF
	;rcall write
	ret

end:

	

	




.exit
	