;
; temporizadores.asm
;
; Created: 24/09/2025 10:32:50 a. m.
; Author : Miguel
;

; TEMPORIZADOR
.org 0x00
rjmp start

.org 0x0022      ; vector de desborde de Timer1 pero en modo limite maximo de conteo establecido (ya no es de 0x000 a 0xffff) 
rjmp ISR_TIMER1_COMPA


start:
	ldi r20, 0xff	; 1
	out ddrf, r20	; portF de salida
	
	ldi r21, 0x00	; inicializa contador en 0


	; TEMPORIZADOR
	; poner Timer1 en modo CTC: WGM12=1
	; Preescaler
	ldi r16,(1<<WGM12)|(1<<CS12)|(1<<CS10) ; prescaler 1024 + CTC
	sts TCCR1B,r16
	
	; modo del timer (00 para modo normal)
	ldi r16,0
	sts TCCR1A,r16

	; valor para 1s
	ldi r16,high(15625)   ; OCR1A=15624 → 15625 ticks
	sts OCR1AH,r16
	ldi r16,low(15625)
	sts OCR1AL,r16

    ; habilitar interrupción de overflow del Timer1, para interrupción de tipo con limite maximo de conteo establecido (ya no es de 0x000 a 0xffff)
	ldi r16,(1<<OCIE1A)
	sts TIMSK1,r16

    sei                   ; habilitar globales



contar:
	; poner el valor de Z en el valor equivalente al contador para 7 segmentos
	ldi ZL, low(tabla*2)		; r30 nibble low de Z
	add ZL, r21
	
	lpm r22, Z		; pongo en r22 la direccion del valor de Z
	out portf, r22	; mando la señal para mostar Z en 7 segmentos
	

    cpi r21, 0x10
    brlo contar
    ldi r21, 0x00
    rjmp contar

; --- Rutina de interrupción de overflow de Timer1 ---
ISR_TIMER1_COMPA:
	inc r21
    reti

tabla:
    ;	0	  1		2	  3		4	  5	    6	  7     8     9		a	  b		c	  d		e	  f
	.db 0x01, 0x4f, 0x12, 0x06, 0x4c, 0x24, 0x20, 0x0f, 0x00, 0x04, 0x08, 0x60, 0x31, 0x42, 0x30, 0x38

