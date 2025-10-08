;
; 7_seg_anodo_arduino.asm
;
; Created: 3/10/2025 2:40:51 p. m.
; Author : Miguel
;

start:
	ldi r20, 0xff	; 1
	out ddrf, r20	; portF de salida
	
	ldi r21, 0x00	; inicializa contador en 0

contar:
	; poner el valor de Z en el valor equivalente al contador para 7 segmentos
	ldi ZL, low(tabla*2)		; r30 nibble low de Z
	add ZL, r21
	
	lpm r22, Z		; pongo en r22 la direccion del valor de Z
	out portf, r22	; mando la señal para mostar Z en 7 segmentos
	call esperar_1s

	inc r21
    cpi r21, 0x10
    brlo contar
    ldi r21, 0x00
    rjmp contar


; --- delay 1s ---
esperar_1s:
    ldi r27,0x4c
    call ciclo_3
    ret

ciclo_1:
    dec r25
    cp r16,r25
    brne ciclo_1
    ret

ciclo_2:
    ldi r25,0xff
    call ciclo_1
    dec r26
    cp r16,r26
    brne ciclo_2
    ret

ciclo_3:
    ldi r26,0xff
    call ciclo_2
    dec r27
    cp r16,r27
    brne ciclo_3
    ret

tabla:
    ;	0	  1		2	  3		4	  5	    6	  7     8     9		a	  b		c	  d		e	  f
	.db 0x01, 0x4f, 0x12, 0x06, 0x4c, 0x24, 0x20, 0x0f, 0x00, 0x04, 0x08, 0x60, 0x31, 0x42, 0x30, 0x38