;
; ejercicio_3.asm
;
; Created: 22/08/2025 3:47:17 p. m.
; Author : Miguel
;


start:
	ldi r13, 0x00		; cero para comparar
	ldi r14, 0x41		; 41 para comparar (A)
	ldi r15, 0x5b		; 5b , Nota: Z = 5a

verificar_letra_1:

	lds r16, 0x0200		; VERIFICANDO LETRA 1
	cp r16, r13
	breq igual_cero

	cp r16, r14
	brsh puede_ser_min_may_1



	
igual_cero:
	rjmp end

puede_ser_min_may_1:
	cp r16, r15
	brsh puede_ser_min_1

	; es mayuscula
	rjmp verificar_letra_2
	; pasar a la siguente letra
	lds r17, 0x0201
	lds r18, 0x0202
	lds r19, 0x0203
	lds r20, 0x0204
	lds r21, 0x0205

puede_ser_min_1:
	



end:
	rjmp start

