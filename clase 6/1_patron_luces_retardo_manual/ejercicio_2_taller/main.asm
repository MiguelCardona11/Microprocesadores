;
; ejercicio_2_taller.asm
;
; Created: 29/08/2025 2:40:42 p. m.
; Author : Miguel
;

; Realizar un juego de luces cuya secuencia dependerá de unos interruptores conectados al puerto F así:
; a. Si el valor leído es 01 la secuencia de luces del puerto K será 8 leds parpadeantes.
; b. Si el valor leído es 10 los leds realizarán un recorrido del centro hacia afuera y de regreso.
; c. Si el valor leído es 11 los leds realizará una secuencia de izquierda a derecha y de regreso.

start:
	ldi r16, 0x00	; 0
	ldi r21, 0xff	; 1 	
	
	; en el 17 se guardará el dato

	ldi r18, 0x01	; 01 caso a.
	ldi r19, 0x02	; 10 caso b.
	ldi r20, 0x03	; 11 caso c.

	out ddra, r16	; habilito lectura port A
	out ddrb, r21   ; habilito salida port B
	
escoger_caso:
	in r17, pina	; coloco el valor de A en r17
	cp r17, r18
	breq casoa

	cp r17, r19
	breq casob

	cp r17, r20
	breq casoc

	rjmp escoger_caso

casoa:
	out portb, r21
	call esperar_1s
	out portb, r16
	call esperar_1s
	
	rjmp escoger_caso

casob:
	ldi r22, 0x18
	out portb, r22
	call esperar_1s

	ldi r22, 0x24
	out portb, r22
	call esperar_1s

	ldi r22, 0x42
	out portb, r22
	call esperar_1s

	ldi r22, 0x81
	out portb, r22
	call esperar_1s

	ldi r22, 0x42
	out portb, r22
	call esperar_1s

	ldi r22, 0x24
	out portb, r22
	call esperar_1s

	rjmp escoger_caso

casoc:
	ldi r22, 0x80
	out portb, r22
	call esperar_1s

	ldi r22, 0x40
	out portb, r22
	call esperar_1s

	ldi r22, 0x20
	out portb, r22
	call esperar_1s

	ldi r22, 0x10
	out portb, r22
	call esperar_1s

	ldi r22, 0x08
	out portb, r22
	call esperar_1s

	ldi r22, 0x04
	out portb, r22
	call esperar_1s

	ldi r22, 0x02
	out portb, r22
	call esperar_1s

	ldi r22, 0x01
	out portb, r22
	call esperar_1s

	ldi r22, 0x02
	out portb, r22
	call esperar_1s

	ldi r22, 0x04
	out portb, r22
	call esperar_1s

	ldi r22, 0x08
	out portb, r22
	call esperar_1s

	ldi r22, 0x10
	out portb, r22
	call esperar_1s

	ldi r22, 0x20
	out portb, r22
	call esperar_1s

	ldi r22, 0x40
	out portb, r22
	call esperar_1s

	ldi r22, 0x80
	out portb, r22
	call esperar_1s

	rjmp escoger_caso


esperar_1s:	; ~ 15.904.767 ciclos
	ldi r27, 0x1b			; 1		--> 1c
	call ciclo_3			; 5		--> 5c + 15.904.755 = 15.904.760
	ret						; 5		--> 5c


ciclo_1:	; ~2.295 ciclos
	dec r25					; 1		--> 1c * 255 = 255
	cp r16, r25				; 1		--> 1c * 255 = 255
	brne ciclo_1			; 2		--> 2c * 255 = 510
	ret						; 5	    --> 5c * 255 = 1275

ciclo_2:	; ~589.050 ciclos
	ldi r25, 0xff			; 1		--> 1c * 255 = 255
	call ciclo_1			; 5		--> (5c + 2.295) * 255 = 586.500
	dec r26					; 1		--> 1c * 255 = 255
	cp r16, r26				; 1		--> 1c * 255 = 255
	brne ciclo_2			; 2		--> 2c * 255 = 510
	ret						; 5		--> 5c * 255 = 1275

ciclo_3:	; ~ 15.904.755 ciclos
	ldi r26, 0xff			; 1		--> 1c * 27 = 27
	call ciclo_2			; 5		--> (5c + 589.050) * 27 = 15.904.485
	dec r27					; 1		--> 1c * 27 = 27
	cp r16, r27				; 1		--> 1c * 27 = 27	
	brne ciclo_3			; 2		--> 2c * 27 = 54
	ret						; 5		--> 5c * 27 = 135


