;
; ejercicio_3.asm
;
; Created: 29/08/2025 3:28:03 p. m.
; Author : Miguel
;

; Se debe realizar el control de un depósito de agua de forma que el nivel del depósito está
; indicado mediante 4 sensores conectados al puerto F. El nivel se visualizará mediante una
; barra de 4 leds conectada al puerto K. Además al llegar al nivel mínimo se debe poner en
; marcha una bomba de agua, esto se observará en un led. Al llegar al nivel máximo la
; bomba debe parar y se debe indicar por medio de un led parpadeante.

start:
	; Estados Bomba
	; x = 1000 0000 -> Bomba estan encendida -> 0x80
	; x = 0100 0000	-> Bomba esta llena parpadeante -> 0x40
    
	; Niveles / Sensores
	ldi r18, 0x01	; valor 1 decimal -> Sensor 1 activado
	ldi r19, 0x03	; valor 3 decimal -> Sensor 1 y 2 activados
	ldi r20, 0x07	; valor 7 decimal -> Sensor 1, 2 y 3 activados
	ldi r21, 0x0f	; valor 15 decimal -> Sensor 1, 2, 3 y 4 activados

	; Puertos de entrada y salida
	ldi r16, 0x00	; r16 = 0000 0000 -> Poner puerto en entrada
	ldi r17, 0xff	; r17 = 1111 1111 -> Poner puerto en salida

	sts ddrf, r16	; port F de entrada (sensores)
	sts ddrk, r17	; port K de salida (LEDS)

	; r22 -> Este registro se utiliza solo para los valores de los sensores
	; r23 -> Este registro se utiliza solo para los valores para encender los leds (bomba encendida y bomba llena)

	
verificar_sensores:
	; leer pinf
	in r22, pinf
	call ignorar_nivel_alto
	call estado_bomba
	cp r22, r16
	breq en_nivel_1
	cp r22,r18
	breq en_nivel_1
	cp r22,r19
	breq en_nivel_2
	cp r22,r20
	breq en_nivel_3
	cp r22,r21
	breq en_nivel_4
	;volver a leer si el caso es incorrecto
	rjmp verificar_sensores

en_nivel_1:
	rjmp bomba_prendida
en_nivel_2:
	cp r24, r25
	; si el pin esta prendido se mantiene pero se actualizan los leds de los sensores
	breq bomba_prendida
	; sino, se apaga y se actualizan los leds de los sensores
	rjmp bomba_apagada
en_nivel_3:
	cp r24, r25
	; si el pin esta prendido se mantiene pero se actualizan los leds de los sensores
	breq bomba_prendida
	; sino, se apaga y se actualizan los leds de los sensores
	rjmp bomba_apagada
en_nivel_4:
	ldi r26,0x0f
	cp r24, r25
	; si el pin esta prendido se amntiene pero se actualizan los leds de los sensores
	breq parpadear
	; sino, se apaga y se actualizan los leds de los sensores
	rjmp bomba_apagada

bomba_prendida:
	; mantiene el estado actual de los sensores pero prende el led de la bomba
	ori r22, 0x80
	sts portk, r22
	rjmp verificar_sensores
bomba_apagada:
	andi r22, 0x7f
	sts portk, r22
	rjmp verificar_sensores

estado_bomba:
	;leer el pin de la bomba
	in r24,pinf
	andi r24, 0x80
	ldi r25, 0x80
	ret

ignorar_nivel_alto:
	; ignorar nible alto ( en nible bajo estan los sensores)
	andi r22, 0x0f
	ret

parpadear:
	; LED parpadeante
	ldi r25, 0x40
	or r25, r22
	sts portk, r25
	ldi r25, 0x00
	or r25, r22
	sts portk, r25
	dec r26
	cp r26,r16
	breq verificar_sensores
	rjmp parpadear