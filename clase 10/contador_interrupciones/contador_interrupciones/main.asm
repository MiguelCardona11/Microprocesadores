;
; contador_interrupciones.asm
;
; Created: 17/09/2025 10:50:57 a. m.
; Author : Miguel
;


.org 0x00
rjmp start          ; reset

.org 0x0008     ; dirección del vector de INT3
rjmp ISR_INT3

start:
	ldi r20, 0xff	; 1
	out ddrf, r20	; portF de salida

	ldi r23, 0xfb	; 1111 1011
	out ddrd, r23	; bit 3 portD ENTRADA

	sei				; Set Global Interrupt Flag (I=1) SREG.7

	; EICRA decide si la interrupción sera con flanco de subida/bajada, ambos o por cambio (PARA INT 3:0) Nota: EICRB es para INT 7:4
	ldi r24, 0xC0	; 1100 0000 bit 6 y 7 en 1 (INT3 con flanco de subida)
	sts EICRA, r24	; 

	; EIMSK decide cual INT se habilitará, en este caso solo habilitamos INT3
	ldi r24, 0x08	; Activar máscara para INT3 
	out EIMSK, r24

	ldi r21, 0x00	; inicializa contador en 0

contar:
	; poner el valor de Z en el valor equivalente al contador para 7 segmentos
	ldi ZL, low(tabla*2)		; r30 nibble low de Z
	
	; tabla + valor de contador actual
	add ZL, r21
	
	lpm r22, Z		; pongo en r22 la direccion del valor de Z
	out portf, r22	; mando la señal para mostar Z en 7 segmentos

    cpi r21, 0x10
    brlo contar
    ldi r21, 0x00
    rjmp contar


; --- rutina de servicio para INT3 ---
ISR_INT3:
    inc r21		; aumentar contador
    reti

tabla:
    ;	0	  1		2	  3		4	  5	    6	  7     8     9		a	  b		c	  d		e	  f
	.db 0x01, 0x4f, 0x12, 0x06, 0x4c, 0x24, 0x20, 0x0f, 0x00, 0x04, 0x08, 0x60, 0x31, 0x42, 0x30, 0x38
