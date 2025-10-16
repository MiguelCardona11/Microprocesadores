;
; mostrar_nombre_desde_EEPROM.asm
;
; Created: 15/10/2025 11:19:28 a. m.
; Author : Miguel
;
; r16 : registro temporal para uso general
; r17 : nibble LOW de posicion de memoria EEPROM
; r18 : nibble HIGH de posicion de memoria EEPROM
; r19 : registro utilizado para leer la letra de la tabla en FLASH
; r20 : lectura de byte 1 de letra (PORTF)
; r21 : lectura de byte 2 de letra (PORTC)
; r22 : bit de lectura del puntero Z

.org 0x00
rjmp start

.org 0x0022				; Habilito el vector de interrupción del TIMER1 en modo comparacion
rjmp ISR_TIMER1_COMPA

start:
	; **************** CONFIGURACION DEL TEMPORIZADOR ****************
	; Cada vez que se termine el contador, se ejecuta la interrupción, queremos que se ejecute cada 1 segundo
	; borro la configuración previa de TIMER1 (TCCR1A y TCCR1B son quienes deciden el modo)
	ldi r16, 0x00
    sts TCCR1A, r16
	
	; Timer en Modo Comparación, se configura en TCCR1B
	ldi r16,(1<<WGM12)|(1<<CS12) ; 1<<WGM12 modo CTC (comparación) - (1<<CS12) Preescaler de 256
	sts TCCR1B,r16

	; TCNT1 indica el valor inicial del timer, por defecto es 0 pero se puede asegurar
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
    sei	; Habilitar interrupciones globales
	; *****************************************************************

    ; configurar puertos
    ldi r16,0xFF
    out DDRF,r16       ; PORTF salida
    out DDRC,r16       ; PORTC salida

	ldi r17, 0x00	   ; inicializacion
	ldi r18, 0x00	   ; inicializacion
	ldi r19, 0x00	   ; inicialización
	ldi r20, 0x00      ; inicialización 
	ldi r21, 0x01      ; inicializa en r20+1
	ldi r22, 0x01	   ; inicializa r22 con un valor diferente de 0

	escribir_eeprom:
		; Esperar a que no haya escritura en curso
		sbic EECR, EEPE        ; Salta si EEPE = 0 (no está escribiendo)
		rjmp escribir_eeprom   ; Si EEPE = 1, esperar

	ciclo_escritura_eeprom:
		cpi r19, 12			; 6 letras * 2 bytes = 12
		breq loop			; salta al loop principal cuando termina de escribir toda la palabra en EEPROM (cuando se encuentra un 0x00 en la tabla)
		ldi ZL,low(tabla*2)
		add ZL, r19			; r19 contiene el indice de la tabla
		lpm r22, Z

		; Cargar dirección de EEPROM
		out EEARL, r17
		out EEARH, r18

		; se carga en la EEPROM el patrón de la letra para el 14segmentos
		out EEDR, r22
		sbi EECR, EEMPE		; Habilita escritura
		sbi EECR, EEPE      ; Inicia escritura

		inc r17		; pasa a la siguiente posicion de memoria EEPROM
		inc r19		; pasa al siguiente índice de la tabla
		rjmp escribir_eeprom

loop:
	; primer byte (PORTF)
	call leer_eeprom_port_f
    out PORTF,r16
	;segundo byte (PORTC)
	call leer_eeprom_port_c
    out PORTC,r16

	; termina el loop cuando termina la palabra
    cpi  r21,12        ; 6 letras * 2 bytes = 12
    brlo loop
	; reinicio los lectores de la posicion de memoria EEPROM
    ldi r20, 0x00
	ldi r21, 0x01
    rjmp loop

leer_eeprom_port_f:
	; Esperar a que no haya escritura en curso
	sbic EECR, EEPE
	rjmp leer_eeprom_port_f

	; Cargar dirección de memoria EEPROM
	out EEARL, r20
	out EEARH, r18

	; Iniciar lectura
	sbi EECR, EERE

	; Leer dato desde EEDR
	in r16, EEDR               ; r16 ahora contiene el dato leído
	ret

leer_eeprom_port_c:
	; Esperar a que no haya escritura en curso
	sbic EECR, EEPE
	rjmp leer_eeprom_port_c

	; Cargar dirección de memoria EEPROM
	out EEARL, r21
	out EEARH, r18

	; Iniciar lectura
	sbi EECR, EERE

	; Leer dato desde EEDR
	in r16, EEDR               ; r16 ahora contiene el dato leído
	ret

; --- Rutina de interrupción de TIMER1 ---
ISR_TIMER1_COMPA:
	subi r20, -2	; incremento en 2 el índice del byte 1
	subi r21, -2	; incremento en 2 el índice del byte 2
    reti

tabla:
	;   M			I			G			U			E			L			
	.db 0x92, 0xf8, 0xfd, 0xdc, 0x43, 0x7c, 0x83, 0xfc, 0x63, 0x74, 0xe3, 0xfc

; abcdefgh jklmnp			portF  portC
; 10010010 11111000		0x92   0xf8
; 11111101 11011100		0xfd   0xdc
; 01000011 01111100		0x43   0x7c
; 10000011 11111100		0x83   0xfc
; 01100011 01110100		0x63   0x74
; 11100011 11111100		0xe3   0xfc