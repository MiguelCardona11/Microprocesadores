;
; ejemplo1.asm
;
; Created: 13/10/2025 4:30:21 p. m.
; Author : Miguel
;

.org 0x00
rjmp start

.org 0x0006     ; dirección del vector de INT2
rjmp ISR_INT2_GUARDAR_EEPROM

.org 0x0008     ; dirección del vector de INT3
rjmp ISR_INT3_CAMBIAR_MODO

.org 0x0022      ; Habilito el vector de interrupción del TIMER1 en modo comparacion
rjmp ISR_TIMER1_COMPA

; r16 : Registro temporal para asignaciones
; r17 : Nibble LOW de EEPROM
; r18 : Nibble HIGH de EEPROM
; r19 : Cantidad de datos en EEPROM
; r20 : Registro bandera para verificar el modo
; r21 : Contador del timer
; r22 : Registro para guardar el puntero de Z y mostar un número
; r23 : Nbble LOW del contador
; r24 : Nbble HIGH del contador
; r25 : Iterador de NIBBLE HIGH sobre EEPROM.
; r26 : Contador del ciclo dentro de mostrar_eeprom
; r27 : registro para mostrar los datos de la EEPROM

start:
	ldi r16, 0xfe	; 1111 1110
	out ddrf, r16	; portF de salida
	sts ddrk, r16	; portK de salida

	ldi r17, 0x00	; inicializa Nibble LOW de EEPROM
	ldi r18, 0x00	; inicializa Nibble HIGH de EEPROM
	ldi r20, 0x00	; inicializa registro de MODO
	ldi r21, 0x00	; inicializa contador en 0

	; ***** CONFIGURACION DEL TEMPORIZADOR *****
	;*******************************************
	; Cada vez que se termine el contador, se ejecuta la interrupción, queremos que se ejecute cada 1 segundo

	; borro la configuración previa de TIMER1 (TCCR1A y TCCR1B son quienes deciden el modo)
	ldi r16, 0x00
    sts TCCR1A, r16
	
	; Modo de comparación, se configura en 
	ldi r16,(1<<WGM12)|(1<<CS12) ; 1<<WGM12 modo CTC (comparación) - (1<<CS12) Preescaler de 256
	sts TCCR1B,r16

	; TCNTn indica el valor inicial del timer, por defecto es 0 pero se puede asegurar
	ldi r16,0
	sts TCNT1H, r16
	sts TCNT1L, r16

	; Establezco el valor límite del contador, el cual se guarda en OCRnA
	; 1 seg = 1000 ms = 1.000.000 microsegundos ;  tosc 16MhZ = 0.0625
	; tiempo = tosc * preescaler * conteos
	; 1000000 = 0.0625 * preescaler * conteos
	; 16.000.000 = preescaler * conteos		--> conteos = 16.000.000 / preescaler	--> conteos debe dar menor que 65536
	; si preescaler = 256 entonces	--> conteos = 62.500

	ldi r16,high(62500)
	sts OCR1AH,r16
	ldi r16,low(62500)
	sts OCR1AL,r16

    ; Habilitamos la interrupción del TIMER1, para que se habilite cuando se iguale a OCR1A, esto se en el registro TIMSKn
	ldi r16,(1<<OCIE1A)
	sts TIMSK1,r16

	; ***** CONFIGURACIÓN DE INTERRUPCIONES INT2 INT3 *****
	; *****************************************************
	; Se setea interrupción por flanco de subida
	ldi r16, 0xF0	; 1111 0000 bits 7:4 en 1 para INT3, INT2
	sts EICRA, r16	; 

	; Se habilitan INT2 e INT3
	ldi r16, 0x0c	; Activar máscara para INT2, INT3 ~ 0000 1100
	out EIMSK, r16

    sei	; Habilitar interrupciones globales


; *************** LOOP PRINCIPAL ***************
; **********************************************
contar:
	cpi r20, 0x01
	breq mostrar_eeprom

    ; Z equivale al puntero actual de la tabla
    ldi ZL, low(tabla*2)	; ZL = r30
    ldi ZH, high(tabla*2)	; ZH = r31 

	; Hago una copia del contador r21 y me quedo solo con su NIBBLE LOW
	; r23 = LOW
    mov r23, r21
    andi r23, 0x0F

	; Hago otra copia del contador r21 y me quedo solo con su NIBBLE HIGH
	; r24 = HIGH
    mov r24, r21
    swap r24
    andi r24, 0x0F

    ; =============== PORTK (LOW) ===============
    add ZL, r23
    adc ZH, r1

    lpm r22, Z
    sts portk, r22		; PORTK muestra el nibble LOW del contador

    ; =============== PORTF (HIGH) ===============
    ldi ZL, low(tabla*2)
    ldi ZH, high(tabla*2)
    add ZL, r24
    adc ZH, r1

    lpm r22, Z
    out portf, r22		; PORTF muestra el nibble HIGH del contador

    rjmp contar

mostrar_eeprom:
	ldi r20, 0x00		 ; vuelve al modo normal tras terminar de mostrar datos de EEPROM
	ldi r25, 0x00		 ; iniciar iterador de posiciones de memoria de EEPROM, inicia en la 0
	ldi r26, 0x00		 ; iniciar contador del ciclo para mostrar
	ciclo_mostrar_eeprom:
		cp r26, r19		; El ciclo termina cuando se han leido todos los datos de EEPROM disponibles
		breq fin_mostrar_eeprom

		; lectura de EEPROM para nibble LOW
		call leer_eeprom
		ldi ZL, low(tabla*2)	; ZL = r30
		ldi ZH, high(tabla*2)	; ZH = r31 

		mov r27, r16	; tomo valor leído en r27 (LOW)
		add ZL, r27
		adc ZH, r1
		lpm r22, Z
		sts portk, r22		; PORTK muestra el nibble LOW
		;															  L    H    L																	
		inc r25				; paso a la siguiente posición de EEPROM (00 - 10 - 20...)

		; lectura de EEPROM para nibble HIGH
		call leer_eeprom
		ldi ZL, low(tabla*2)	; ZL = r30
		ldi ZH, high(tabla*2)	; ZH = r31 

		mov r27, r16	; tomo valor leído en r27 (HIGH)
		add ZL, r27
		adc ZH, r1
		lpm r22, Z
		out portf, r22		; PORTF muestra el nibble HIGH

		inc r25				; paso a la siguiente posición de EEPROM (00 - 10 - 20...)

		inc r26		; aumenta el contador del ciclo para saber cuando terminar
		rjmp ciclo_mostrar_eeprom

; ej: presionó en 7f, 8a
;posicion	00  10		20	30	40
;guarda	     f	 7		 a	 8
; 	   		 L	 H		 L   H

leer_eeprom:
	sbic EECR, EEPE		 ; esperar al EEPROM por si esta escribiendo
	rjmp leer_eeprom
	
	out EEARL, r17	; posicion de memoria LOW de EEPROM
	out EEARH, r25	; posicion de memoria HIGH de EEPROM (El HIGH será el iterador r25 que empieza desde 0 y sube de 1 en 1)

	sbi EECR, EERE	; iniciar lectura

	in r16, EEDR	; leer dato en r16
	ret


; *************** INTERRUPCIONES ***************
; **********************************************

; --- Rutina de interrupción de TIMER1 Contador ---
ISR_TIMER1_COMPA:
	inc r21
    reti

; --- rutina de interrupción para INT2 guardar dato actual del contador en EEPROM---
ISR_INT2_GUARDAR_EEPROM:
	; esperar al EEPROM por si esta escribiendo
	sbic EECR, EEPE        ; Salta si EEPE = 0 (no está escribiendo)
	rjmp ISR_INT2_GUARDAR_EEPROM          ; Si EEPE = 1, esperar

	; Cargar dirección del EEPROM
	out EEARL, r17	; posicion de memoria LOW de EEPROM
	out EEARH, r18	; posicion de memoria HIGH de EEPROM
	out EEDR, r23	; Se guarda nibble LOW del contador

	sbi EECR, EEMPE	; Habilita escritura
	sbi EECR, EEPE	; ejecuta  escritura
	inc r18			; aumenta el nibble HIGH de EEPROM para pasar de posicion de memoria (00 - 10 - 20...)

	; Cargar dirección del EEPROM
	out EEARL, r17	; posicion de memoria LOW de EEPROM
	out EEARH, r18	; posicion de memoria HIGH de EEPROM
	out EEDR, r24	; Se guarda nibble HIGH del contador

	sbi EECR, EEMPE	; Habilita escritura
	sbi EECR, EEPE	; ejecuta  escritura
	inc r18			; aumenta el nibble HIGH de EEPROM para pasar de posicion de memoria (00 - 10 - 20...)
	inc r19			; Cantidad de datos en la EEPROM

    reti

; ej: presionó en 7f, 8a
;posicion	00  10		20	30	40
;guarda	     f	 7		 a	 8
; 	   		 L	 H		 L   H

; --- rutina de interrupción para INT3 cambiar modo ---
ISR_INT3_CAMBIAR_MODO:
    ldi r20, 0x01
	reti

tabla:
    ;	0	  1		2	  3		4	  5	    6	  7     8     9		a	  b		c	  d		e	  f
	.db 0x10, 0xf9, 0x43, 0x61, 0xa9, 0x25, 0x05, 0xf1, 0x01, 0xa1, 0x81, 0x0d, 0x17, 0x49, 0x07, 0x87
	
	; dfeg abc
	; 7654 3210
	; 1110 1111 		0: g
	; 1111 1001			1: b, c
	; 0100 0011			2: a, b, d, e, g
	; 0110 0001			3: a, b, c, d, g
	; 1010 1001			4: b, c, f, g
	; 0010 0101			5: a, c, d, f, g
	; 0000 0101			6: a, c, d, e, f, g   
	; 1111 0001			7: a, b, c
	; 0000 0001			8: a, b, c, d, e, f, g
	; 1010 0001			9: a, b, c, f, g
	; 1000 0001			a: a, b, c, e, f, g
	; 0000 1101			b: c, d, e, f, g
	; 0001 0111			c: a, d, e, f
	; 0100 1001			d: b, c, d, e, g
	; 0000 0111			e: a, d, e, f, g
	; 1000 0111			f: a, e, f, g


	; 0001 0000 		0x10
	; 1111 1001			0xf9
	; 0100 0011			0x43
	; 0110 0001			0x61
	; 1010 1001			0xa9
	; 0010 0101			0x25
	; 0000 0101			0x05
	; 1111 0001			0xf1
	; 0000 0001			0x01
	; 1010 0001			0xa1
	; 1000 0001			0x81
	; 0000 1101			0x0d
	; 0001 0111			0x17
	; 0100 1001			0x49
	; 0000 0111			0x07
	; 1000 0111			0x87