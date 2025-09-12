;
; ejercicio_2.asm
;
; Created: 27/08/2025 11:09:08 a. m.
; Author : Miguel
;


; Replace with your application code

start:
	ldi r17, 0x7b		; 7b , Nota: z = 7a
	ldi r18, 0x61		; 61 = a
	ldi r19, 0x00		; cero para comparar
	ldi r20, 0x41		; 41 para comparar (A)
	ldi r21, 0x5b		; 5b , Nota: Z = 5a

	ldi r27, 0x02	; higher byte X
	ldi r26, 0x00	; lower byte X

	ldi r29, 0x02	; higher byte Y
	ldi r28, 0x16	; lower byte Y



comparar:
	inc r16
	inc r17
	ld r16, X
	ld r17, Y

	cp r16, r17
	breq comparar



; funcion es_mayus(r25) -> r24
;	si r24 es 0 --> no es mayus
;	si r24 es 1 --> es mayus
es_mayus:
	cp r25, r20
	brsh puede_ser_mayus
	ret		; simbolo, no hace nada

puede_ser_mayus:
	cp r25, r21
	brsh no_es_mayus

	ldi r25, 0x01
	ret

no_es_mayus:
	ldi r25, 0x00
	ret
	

