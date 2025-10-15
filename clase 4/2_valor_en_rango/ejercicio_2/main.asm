;
; ejercicio_2.asm
;
; Created: 22/08/2025 3:35:47 p. m.
; Author : Miguel
;

; Verifique si el contenido de la posicion de memoria 200H se encuentra en el rango min < 200H < max
; el valor minimo esta almacenado en la posicion 201H y el maximo en la posicion 202H.
; Si la posicion 200H está en el rango, poner FF en 203H, si no es así poner 0.

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
