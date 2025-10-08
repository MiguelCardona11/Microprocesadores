;
; ejercicio_2.asm
;
; Created: 22/08/2025 3:35:47 p. m.
; Author : Miguel
;

ejer2:
    lds r16, 0x0201       ; min
	lds r17, 0x0200       ; valor
	lds r18, 0x0202       ; max

	cp r16, r17
	brlo min_menor_valor

	rjmp valor_fuera_rango

min_menor_valor:

	cp r17, r18
	brlo valor_en_rango

	rjmp valor_fuera_rango

valor_en_rango:
	ldi r19, 0xff
	sts 0x0203, r19
	rjmp end

valor_fuera_rango:
	ldi r20, 0x00
	sts 0x0203, r20
	rjmp end
	
end:
    rjmp ejer2
