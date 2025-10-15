;
; nombre_14_seg.asm
;
; Created: 12/09/2025 2:04:30 p. m.
; Author : Miguel
;


; Replace with your application code

	; abcdefgh jklmnp			portF  portC

	; 10010010 11111000		0x92   0xf8
	; 11111101 11011100		0xfd   0xdc
	; 01000011 01111100		0x43   0x7c
	; 10000011 11111100		0x83   0xfc
	; 01100011 01110100		0x63   0x74
	; 11100011 11111100		0xe3   0xfc

start:
    ; configurar puertos
    ldi r20,0xFF
    out DDRF,r20       ; PORTF salida
    out DDRC,r20       ; PORTC salida

    ldi r21,0          ; índice de letra (0..5)

contar:
    ; cargar base de tabla en Z
    ldi ZL,low(tabla*2)
    ldi ZH,high(tabla*2)

    ; sumar offset en bytes a puntero Z
    add ZL,r21
    adc ZH,r1

    ; leer byte para PORTF
    lpm r22,Z
    out PORTF,r22

    ; siguiente byte = PORTC
    adiw ZL,1          ; Z = Z + 1
    lpm r22,Z
    out PORTC,r22

    call esperar_1s

    ; avanzar offset en bytes a la siguiente letra (sumar 2)
    subi r21,-2        ; r21 = r21 + 2
    cpi  r21,12        ; 6 letras * 2 bytes = 12
    brlo contar
    ldi r21,0
    rjmp contar


esperar_1s:	; ~ 15.904.767 ciclos
	ldi r16, 0x00			; 1		--> 1c
	ldi r19, 0x1f			; 1		--> 1c

	call ciclo_3			; 5		--> 5c + 15.904.755 = 15.904.760
	ret						; 5		--> 5c


ciclo_1:	; ~2.295 ciclos
	dec r17					; 1		--> 1c * 255 = 255
	cp r16, r17				; 1		--> 1c * 255 = 255
	brne ciclo_1			; 2		--> 2c * 255 = 510
	ret						; 5	    --> 5c * 255 = 1275

ciclo_2:	; ~589.050 ciclos
	ldi r17, 0xff			; 1		--> 1c * 255 = 255
	call ciclo_1			; 5		--> (5c + 2.295) * 255 = 586.500
	dec r18					; 1		--> 1c * 255 = 255
	cp r16, r18				; 1		--> 1c * 255 = 255
	brne ciclo_2			; 2		--> 2c * 255 = 510
	ret						; 5		--> 5c * 255 = 1275

ciclo_3:	; ~ 15.904.755 ciclos
	ldi r18, 0xff			; 1		--> 1c * 27 = 27
	call ciclo_2			; 5		--> (5c + 589.050) * 27 = 15.904.485
	dec r19					; 1		--> 1c * 27 = 27
	cp r16, r19				; 1		--> 1c * 27 = 27	
	brne ciclo_3			; 2		--> 2c * 27 = 54
	ret						; 5		--> 5c * 27 = 135

tabla:
	;   M			I			G			U			E			L
	.db 0x92, 0xf8, 0xfd, 0xdc, 0x43, 0x7c, 0x83, 0xfc, 0x63, 0x74, 0xe3, 0xfc