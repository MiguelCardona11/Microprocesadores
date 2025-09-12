;
; ejercicio_1.asm
;
; Created: 29/08/2025 2:29:19 p. m.
; Author : Miguel
;


; Replace with your application code
start:
	ldi r17, 0x00	; pongo un 0 en r17
    out ddra, r17	; establezo port A como entrada (0)

	ldi r17, 0xff
	out ddrb, r17	; establezco port B como salida  (1)

	in r16, pina	; ingreso el dato que viene por el puerto A en r16
	inc r16

	out portb, r16	; saco el resultado por el puerto B
	
	rjmp start
