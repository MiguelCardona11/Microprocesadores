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
	ldi r16, 0x00
    ldi r20,0xFF
    out DDRF,r20       ; PORTF salida
    out DDRC,r20       ; PORTC salida

	; Letras a mostrar de la memoria de datos (ASCII: ABC)
	ldi r20, 0x41
	sts 0x200, r20
	ldi r20, 0x42
	sts 0x201, r20
	ldi r20, 0x43
	sts 0x202, r20
	
    ldi r21, 0          ; índice de letra (0..5)
	; puntero X empieza a leer en 0x200 (Ahi empieza la palabra)
	ldi XL, low(0x200)
	ldi XH, high(0x200)

leer_palabra:
	ld r23, X
	cp r23, r16
	breq termina_palabra

	; si r23 es 0x41 ('A') y se le resta 0x41, el índice será el 0, si el índice es 0x42 el indice sera 1, y así...
	subi r23, 'A'
	lsl r23	; se multiplica el índice por 2, para que avance en la tabla de 2 en 2. Si la primera letra es A (0 * 2 = 0) si es B (1 * 2 = 2)
	call mostrar_letra
	call esperar_1s
	adiw XL, 1	; avanzar el puntero XH:XL en 1
	rjmp leer_palabra

termina_palabra:
	ldi XL, low(0x200)
	rjmp leer_palabra	

mostrar_letra:
	; cargar base de tabla en Z
    ldi ZL,low(tabla*2)
    ldi ZH,high(tabla*2)
	; apuntar Z a la dirección de la letra, la cual está en r23
    add ZL,r23
    adc ZH,r16		; arrastra el acarreo si se desborda
	
	; leer byte para PORTF (letra en la posicion de la tabla "r23" (primer byte) y lo muestra en el portF)
    lpm r22,Z
    out PORTF,r22
	
	; avanza al siguiente byte de la tabla para mostrarlo en el portK
    adiw ZL,1          ; Z = Z + 1 avanzar el puntero ZH:ZL en 1
    lpm r22,Z
    out PORTC,r22
	ret	





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
	; abcd efgh jklm np
	; 0001 0011 0111 0111 --> A --> 0x13 0x77
	; 0000 1101 0101 1111 --> B --> 0x0D 0x5F
	; 0110 0011 1111 1111 --> C --> 0x63 0xFF
	; 0000 1101 1101 1111 --> D --> 0x0D 0xDF
	; 0110 0011 1111 0111 --> E --> 0x63 0xF7
	; 0111 0011 1111 0111 --> F --> 0x73 0xF7

	.db 0x13, 0x77, 0x0D, 0x5F, 0x63, 0xFF, 0x0D, 0xDF, 0x63, 0xF7, 0x73, 0xF7
	;   A(0)

	; User = A --> 0
	;		 B --> 2
	;		 C --> 4