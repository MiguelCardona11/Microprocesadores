;
; temporizadores.asm
;
; Created: 24/09/2025 10:32:50 a. m.
; Author : Miguel
;

; TEMPORIZADOR
.org 0x00
rjmp start

.org 0x0022      ; Habilito el vector de interrupción del TIMER1 en modo comparacion
rjmp ISR_TIMER1_COMPA


start:
	ldi r20, 0xff	; 1
	out ddrf, r20	; portF de salida
	
	ldi r21, 0x00	; inicializa contador en 0

	; *** CONFIGURACION DEL TEMPORIZADOR ***
	; Cada vez que se termine el contador, se ejecuta la interrupción, queremos que se ejecute cada 1 segundo

	; borro la configuración previa de TIMER1 (TCCR1A y TCCR1B son quienes deciden el modo)
	ldi r16, 0x00
    sts TCCR1A, r16
	
	; Modo de comparación, se configura en 
	ldi r16,(1<<WGM12)|(1<<CS12)|(1<<CS10) ; 1<<WGM12 modo CTC (comparación) - (1<<CS12)|(1<<CS10) Preescaler de 1024
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
	; si preescaler = 1024 entonces	--> conteos = 15.625

	ldi r16,high(15625)   ; OCR1A=15624 → 15625 ticks
	sts OCR1AH,r16
	ldi r16,low(15625)
	sts OCR1AL,r16

    ; Habilitamos la interrupción del TIMER1, para que se habilite cuando se iguale a OCR1A, esto se en el registro TIMSKn
	ldi r16,(1<<OCIE1A)
	sts TIMSK1,r16

	; Habilitar interrupciones globales
    sei

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

; --- Rutina de interrupción de TIMER1 ---
ISR_TIMER1_COMPA:
	inc r21
    reti

tabla:
    ;	0	  1		2	  3		4	  5	    6	  7     8     9		a	  b		c	  d		e	  f
	.db 0x01, 0x4f, 0x12, 0x06, 0x4c, 0x24, 0x20, 0x0f, 0x00, 0x04, 0x08, 0x60, 0x31, 0x42, 0x30, 0x38


