;
; contador_intgerrupciones_PCINT.asm
;
; Created: 17/09/2025 11:34:49 a. m.
; Author : Miguel
;

.org 0x00
rjmp start          ; reset

.org 0x0012			; vector de interrupción para grupo 0
rjmp ISR_PCINT0

start:
	ldi r20, 0xff	; 1
	out ddrf, r20	; portF de salida

	ldi r23, 0xfb	; 1111 1011
	out ddrb, r23	; bit 3 portB ENTRADA

	sei				; Set Global Interrupt Flag (I=1) SREG.7

	; Se habilita el grupo 0 de interrupciones
	ldi r24, 0x01
	sts PCICR,r24

	; Se habilita la máscara para PCINT3
	ldi r25, 0x08	; 0000 1000
	sts PCMSK0,r25

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
ISR_PCINT0:
    inc r21		; aumentar contador
    reti

tabla:
    ;	0	  1		2	  3		4	  5	    6	  7     8     9		a	  b		c	  d		e	  f
	.db 0x01, 0x4f, 0x12, 0x06, 0x4c, 0x24, 0x20, 0x0f, 0x00, 0x04, 0x08, 0x60, 0x31, 0x42, 0x30, 0x38

