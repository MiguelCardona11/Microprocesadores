;
; ejercicio_3.asm
;
; Created: 22/08/2025 3:47:17 p. m.
; Author : Miguel
;


start:
	ldi r17, 0x7b		; 7b , Nota: z = 7a
	ldi r18, 0x61		; 61 = a
	ldi r19, 0x00		; cero para comparar
	ldi r20, 0x41		; 41 para comparar (A)
	ldi r21, 0x5b		; 5b , Nota: Z = 5a

	ldi r27, 0x02		; higher byte X
	ldi r26, 0x00		; lower byte X

verificar_letra:
	inc r26		; incrementar en 1
	cpi r26, 0x06		; comparar con valor inmediato
	breq end

	ld r16, X			; VERIFICANDO LETRA 

	cp r16, r19
	breq end

	cp r16, r20
	brsh puede_ser_min_may

	rjmp verificar_letra

puede_ser_min_may:
	cp r16, r21
	brsh puede_ser_min_simbolo
	; es mayuscula

	rjmp verificar_letra

puede_ser_min_simbolo:
	cp r16, r18
	brsh puede_ser_min_simbolo_sup
	; es simbolo --> no se hace nada

	rjmp verificar_letra

puede_ser_min_simbolo_sup:
	cp r16, r17
	brsh verificar_letra
	subi r16, 0x20

	st X, r16	; escribir en X el nuevo valor en mayúscula
	rjmp verificar_letra

end:
	rjmp start

