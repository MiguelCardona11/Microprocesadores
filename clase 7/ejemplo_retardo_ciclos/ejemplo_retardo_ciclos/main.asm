;
; ejemplo_retardo_ciclos.asm
;
; Created: 5/09/2025 2:13:46 p. m.
; Author : Miguel
;


esperar_1s:	; ~ 15.904.767 ciclos
	ldi r16, 0x00			; 1		--> 1c
	ldi r19, 0x1b			; 1		--> 1c

	call ciclo_3			; 5		--> 5c + 15.904.755 = 15.904.760
	ret						; 5		--> 5c


ciclo_1:	; ~2.295 ciclos
	dec r17					; 1		--> 1c * 255 = 255
	cp r16, r17				; 1		--> 1c * 255 = 255
	brne ciclo_1			; 2		--> 2c * 255 = 510
	ret						; 5	    --> 5c * 255 = 1275

ciclo_2:	; ~589.050 ciclos
	ldi r17, 0xff			; 1		--> 1c * 255 = 255
	call ciclo_1			; 5		--> (5c + 2.295) * 255 = 586.500
	dec r18					; 1		--> 1c * 255 = 255
	cp r16, r18				; 1		--> 1c * 255 = 255
	brne ciclo_2			; 2		--> 2c * 255 = 510
	ret						; 5		--> 5c * 255 = 1275

ciclo_3:	; ~ 15.904.755 ciclos
	ldi r18, 0xff			; 1		--> 1c * 27 = 27
	call ciclo_2			; 5		--> (5c + 589.050) * 27 = 15.904.485
	dec r19					; 1		--> 1c * 27 = 27
	cp r16, r19				; 1		--> 1c * 27 = 27	
	brne ciclo_3			; 2		--> 2c * 27 = 54
	ret						; 5		--> 5c * 27 = 135