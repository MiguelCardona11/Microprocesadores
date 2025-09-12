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
	ldi r21, 0x5b		; 5b , Nota: Z = 5a}

verificar_letra_1:
	lds r16, 0x0200		; VERIFICANDO LETRA 1

	cp r16, r19
	breq igual_cero

	cp r16, r20
	brsh puede_ser_min_may_1
	rjmp verificar_letra_2

puede_ser_min_may_1:
	cp r16, r21
	brsh puede_ser_min_simbolo_1
	; es mayuscula
	rjmp verificar_letra_2

puede_ser_min_simbolo_1:
	cp r16, r18
	brsh puede_ser_min_simbolo_sup_1
	; es simbolo --> no se hace nada
	rjmp verificar_letra_2

puede_ser_min_simbolo_sup_1:
	cp r16, r17
	brsh verificar_letra_2
	subi r16, 0x20
	sts 0x0200, r16
	rjmp verificar_letra_2


verificar_letra_2:
	lds r16, 0x0201		; VERIFICANDO LETRA 2
	cp r16, r19
	breq igual_cero

	cp r16, r20
	brsh puede_ser_min_may_2
	rjmp verificar_letra_3

puede_ser_min_may_2:
	cp r16, r21
	brsh puede_ser_min_simbolo_2
	; es mayuscula
	rjmp verificar_letra_3

puede_ser_min_simbolo_2:
	cp r16, r18
	brsh puede_ser_min_simbolo_sup_2
	; es simbolo --> no se hace nada
	rjmp verificar_letra_3

puede_ser_min_simbolo_sup_2:
	cp r16, r17
	brsh verificar_letra_3
	subi r16, 0x20
	sts 0x0201, r16
	rjmp verificar_letra_3

igual_cero:
	rjmp end

verificar_letra_3:
	lds r16, 0x0202		; VERIFICANDO LETRA 3
	cp r16, r19
	breq igual_cero

	cp r16, r20
	brsh puede_ser_min_may_3
	rjmp verificar_letra_4

puede_ser_min_may_3:
	cp r16, r21
	brsh puede_ser_min_simbolo_3
	; es mayuscula
	rjmp verificar_letra_4

puede_ser_min_simbolo_3:
	cp r16, r18
	brsh puede_ser_min_simbolo_sup_3
	; es simbolo --> no se hace nada
	rjmp verificar_letra_4

puede_ser_min_simbolo_sup_3:
	cp r16, r17
	brsh verificar_letra_4
	subi r16, 0x20
	sts 0x0202, r16
	rjmp verificar_letra_4

;

verificar_letra_4:
	lds r16, 0x0203		; VERIFICANDO LETRA 4
	cp r16, r19
	breq igual_cero

	cp r16, r20
	brsh puede_ser_min_may_4
	rjmp verificar_letra_5

puede_ser_min_may_4:
	cp r16, r21
	brsh puede_ser_min_simbolo_4
	; es mayuscula
	rjmp verificar_letra_5

puede_ser_min_simbolo_4:
	cp r16, r18
	brsh puede_ser_min_simbolo_sup_4
	; es simbolo --> no se hace nada
	rjmp verificar_letra_5

puede_ser_min_simbolo_sup_4:
	cp r16, r17
	brsh verificar_letra_5
	subi r16, 0x20
	sts 0x0203, r16
	rjmp verificar_letra_5

;

verificar_letra_5:
	lds r16, 0x0204		; VERIFICANDO LETRA 5
	cp r16, r19
	breq igual_cero

	cp r16, r20
	brsh puede_ser_min_may_5
	rjmp verificar_letra_6

puede_ser_min_may_5:
	cp r16, r21
	brsh puede_ser_min_simbolo_5
	; es mayuscula
	rjmp verificar_letra_6

puede_ser_min_simbolo_5:
	cp r16, r18
	brsh puede_ser_min_simbolo_sup_5
	; es simbolo --> no se hace nada
	rjmp verificar_letra_6

puede_ser_min_simbolo_sup_5:
	cp r16, r17
	brsh verificar_letra_6
	subi r16, 0x20
	sts 0x0204, r16
	rjmp verificar_letra_6

;

verificar_letra_6:
	lds r16, 0x0205		; VERIFICANDO LETRA 6
	cp r16, r19
	breq igual_cero

	cp r16, r20
	brsh puede_ser_min_may_6
	rjmp end

puede_ser_min_may_6:
	cp r16, r21
	brsh puede_ser_min_simbolo_6
	; es mayuscula
	rjmp end

puede_ser_min_simbolo_6:
	cp r16, r18
	brsh puede_ser_min_simbolo_sup_6
	; es simbolo --> no se hace nada
	rjmp end

puede_ser_min_simbolo_sup_6:
	cp r16, r17
	brsh end
	subi r16, 0x20
	sts 0x0205, r16
	rjmp end

end:
	rjmp start

