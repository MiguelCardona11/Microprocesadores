;
; a_to_f_14_seg.asm
;
; Created: 12/09/2025 4:03:49 p. m.
; Author : Miguel
;
;Abecedario de la A a la F. El usuario puede escribir una palabra de la longitud que quiera con este abecedario. Termina en 0.
;En memoria del programa (.db - ROM) se guardan en hex los equivalentes de las letras para 14-seg.
;El usuario escribe en ascii la palabra que quiera en la memoria de datos (RAM – data IRAM)
;Se muestra la palabra escrita en el 14-seg

start:
    ; configurar puertos
    ldi r20,0xFF
    out DDRF,r20       ; PORTF salida
    out DDRC,r20       ; PORTC salida

    ldi r21,0          ; índice de letra (0..5)

leer_palabra:
	lds r23, 0x200
	cp r23, r16
	breq termina_palabra

	cpi r23, 'A'
	breq mostrar_a
	

mostrar_a:
	

termina_palabra:
	




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
	;  A to F en portF y portC
	.db 0x13, 0xfc, 0x33, 0x74, 0x63, 0xfc, 0x0d, 0xdc, 0x63, 0x74, 0x73, 0x74

	; abcdefgh jklmnp
	;						portF	portC
	; 0001 0011 1111 1100	0x13	0xfc
	; 0011 0011 0111 0100	0x33	0x74
	; 0110 0011 1111 1100	0x63	0xfc
	; 0000 1101 1101 1100	0x0d	0xdc
	; 0110 0011 0111 0100	0x63	0x74
	; 0111 0011 0111 0100	0x73	0x74