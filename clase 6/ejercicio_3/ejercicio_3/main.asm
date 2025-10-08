;
; Ejercicio3.asm
;
; Created: 8/31/2025 1:56:13 PM
; Author : Santiago Bedoya
;


; Leer dos datos de 4 bits por el puerto F. Estos datos se operarán de acuerdo a lo indicado
; por los 2 bits más significativos del puerto K. El resultado de la operación se mostrará en
; los 4 bits menos significativos del puerto K. Las operaciones se indican así:
;	a. 00: suma
;	b. 01: resta
;	c. 10: and
;	d. 11: or
start:
	; Deshabilitar JTAG escribiendo dos veces el bit JTD (bit 7 de MCUCR) en menos de 4 ciclos

	ldi r16, 0x80    ; Cargar el valor con el bit JTD activado
	out MCUCR, r16          ; Primer write al registro MCUCR
	out MCUCR, r16          ; Segundo write al registro MCUCR dentro de 4 ciclos

	ldi r16, 0x00	; valor para poner un puerto como entrada (F)
	ldi r17, 0x3f	; primeros dos bits de entrada 0011 1111

	out DDRF, r16	; puerto f como entrada
	sts ddrk, r17	; poner puerto k primeros 2 bits de entrada y el resto salida 0011 1111

	; operaciones
	ldi r18, 0x00	; suma  -> 00xx xxxx -> 0x00
	ldi r19, 0x40	; resta	-> 01xx xxxx -> 0x40
	ldi r20, 0x80	; and	-> 10xx xxxx -> 0x80
	ldi r21, 0xc0	; or	-> 11xx xxxx -> 0xc0

	; en r22 se van a guardar los operandos
		; para separar los operados, se va utilizar r25 para operando 1 (high) y r24 para operando 2 (low)
	; en r23 se va a guardar el operador
	; en r24 se va a guardar el resultado de las operaciones

leer_datos:
	in r22, PINF
	call leer_operacion
	
	cp r23, r18
	breq sumar

	cp r23, r19
	breq restar

	cp r23, r20
	breq a_and_b
	
	cp r23, r21
	breq a_or_b

sumar:
	call mover_nibbles
	add r24, r25
	call mostrar_resultado
	rjmp leer_datos

restar:
	call mover_nibbles
	sub r24, r25
	call mostrar_resultado
	rjmp leer_datos

a_and_b:
	call mover_nibbles
	and r24, r25
	call mostrar_resultado
	rjmp leer_datos

a_or_b:
	call mover_nibbles
	or r24, r25
	call mostrar_resultado
	rjmp leer_datos

leer_operacion:
	lds r23, pink
	andi r23, 0xc0	; solo tengo en cuenta los 2 bits mas significativos, el resto no
	ret

mostrar_resultado:
	; r24 en este punto tiene el resultado en los nibles high, se hace swap
	sts portk, r24
	ret

mover_nibbles:
	mov r25, r22
	mov r24, r22
	; en r25 solo los bits mas significativos, el resto 0
	andi r25, 0x0f
	; para sumar los numeros deben estar en los mismos nibles
	swap r24	; pasamos nibles low a high
	andi r24, 0x0f
	ret