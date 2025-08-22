;
; Clase_3.asm
;
; Created: 20/08/2025 10:09:58 a. m.
; Author : Miguel
;
jmp ejer5

; Replace with your application code

;Tomar los datos de las posiciones de memoria 200H y 201H, los van a sumar y el resultado se pondrá en la posición 202H.
ejer1:
	lds r0, 0x0200
	lds r1, 0x0201
	add r0, r1
	sts 0x0202, r0

; Tomar el dato que se encuentra en la posición de memoria 200H y dejar el nible más significativo intacto y cambiar el nible más significativo por cero

ejer2:
    lds r16, 0x0200     ; cargar dato desde la dirección 200H
    andi r16, 0xF0      ; enmascarar → dejar nibble alto y poner nibble bajo en 0
    sts 0x0200, r16     ; guardar de nuevo en la misma dirección (200H)
	rjmp ejer2

; Dejar Nibble menos significativo intacto y el más significativo dejarlo en F
ejer3:
	lds r16, 0x0200     ; cargar dato desde dirección 200H
    ori r16, 0xF0       ; forzar nibble alto en 1s, dejar nibble bajo igual
    sts 0x0200, r16     ; guardar de nuevo en la misma dirección
	rjmp ejer3


ejer4:
	lds  r16, 0x0200      ; cargar en r16 el dato en 0x0200 (ejemplo: 0101 1011)

    mov  r17, r16         ; copiar en r17 para trabajar --> r17: 0101 1011
	andi r17, 0x0F        ; aislar nibble bajo (0000 XXXX) --> 0101 1011 AND 0000 1111 --> 0000 1011
	com  r17			  ; complemento 0000 1011 --> 1111 0100 --> nibble bajo listo
    andi r17, 0x0F        ; 1111 0100 AND 0000 1111 --> 0000 0100 --> se deja en 0 el high para juntarlo con el low despues

	; se vuelve al original
	; 0101 1011 AND 1111 0000 --> 0101 0000 --> se deja en 0 el low para juntarlo con el high despues
    andi r16, 0xF0
	
	; se junta high y low para el resultado
	; 0000 0100 OR 0101 0000 --> 0101 0100
    or   r16, r17

    sts  0x0200, r16      ; guardar resultado en dirección 200H

	rjmp ejer4

ejer5:

    lds r16, 0x0200       ; cargar valor de RAM[200H] en r16
    lds r17, 0x0201       ; cargar valor de RAM[201H] en r17

    cp  r16, r17          ; comparar r16 y r17, si r16<r17 --> C = 1
    brsh r16_mayor   ; brsh se habilita cuando C = 0

    ; no saltó
    sts 0x0202, r17       ; guardar r17 en 202H
    rjmp end              ; ir al final

r16_mayor:
	; si saltó
    sts 0x0202, r16       ; guardar r16 en 202H

end:
    rjmp ejer5

