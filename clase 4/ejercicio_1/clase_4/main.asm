;
; clase_4.asm
;
; Created: 22/08/2025 2:18:05 p. m.
; Author : Miguel
;

ejer1:

    lds r16, 0x0200       ; cargar valor de RAM[200H] en r16
    lds r17, 0x0201       ; cargar valor de RAM[201H] en r17

    cp  r16, r17          ; comparar r16 y r17, si r16<r17 --> C = 1
    brlo r16_menor   ; brlo se habilita cuando C = 1

    ; si no saltó, puede ser igual o mayor

	breq r16_igual		  ; salta si Z = 1

    ; si no saltó, es porque es mayor
	ldi r19, 'M'
	ldi r20, 'A'
	ldi r21, 'Y'
	ldi r22, 'O'
	ldi r23, 'R'

	sts 0x0202, r19
	sts 0x0203, r20
	sts 0x0204, r21
	sts 0x0205, r22
	sts 0x0206, r23

    rjmp end              ; ir al final

r16_menor:
    ldi r19, 'M'
	ldi r20, 'E'
	ldi r21, 'N'
	ldi r22, 'O'
	ldi r23, 'R'

	sts 0x0202, r19
	sts 0x0203, r20
	sts 0x0204, r21
	sts 0x0205, r22
	sts 0x0206, r23

	rjmp end

r16_igual:
	ldi r19, 'I'
	ldi r20, 'G'
	ldi r21, 'U'
	ldi r22, 'A'
	ldi r23, 'L'

	sts 0x0202, r19
	sts 0x0203, r20
	sts 0x0204, r21
	sts 0x0205, r22
	sts 0x0206, r23

end:
    rjmp ejer1