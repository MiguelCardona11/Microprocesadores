;
; timer_pulsador_count_arduino.asm
;
; Created: 3/10/2025 3:30:29 p. m.
; Author : Miguel
;

; Contador de 0 a f que avanza cuando se presiona un botón externo que aumenta el TIMER. NO HAY NECESIDAD DE INTERRUPCIONES
; 
.include "m2560def.inc"

.org 0x0000
    rjmp start

; ------------------ INICIO ------------------
start:
    ; ====== DISPLAY: PORTF como salida ======
    ldi  r16, 0xFF
    out  DDRF, r16          ; PF7..PF0 = salida

	; borro la configuración previa de TIMER5 (TCCR5A y TCCR5B son quienes deciden el modo)
	ldi  r16, 0x00
    sts  TCCR5A, r16

    ; ====== TIMER5 como contador externo en T5 ======
    ; En este caso se deja el TIMER5 en modo normal (desborde) por lo que WGMn0 a WGMn3 quedan en 0 por defecto
	; En vez de preescalador, habrá un reloj externo en el pin T5, que hará que la cuenta en TCNT5 aumente
	; (1<<CS52) | (1<<CS51) | (1<<CS50) --> modo reloj externo con flanco de subida
    ldi  r16, (1<<CS52) | (1<<CS51) | (1<<CS50)
    sts  TCCR5B, r16

    ; El contador interno de TIMER5 está en TCNT5, lo pongo a que inicie desde 0
    ldi  r16, 0x00
    sts  TCNT5H, r16
    sts  TCNT5L, r16

; ------------------ BUCLE PRINCIPAL ------------------
loop:
    ; Leo el contador interno de TIMER5 que está en TCNT5
    lds  r18, TCNT5L        ; r18 = low byte
    ; Me aseguro de que solo llegue hasta máximo 15 --> 0000 XXXX
    andi r18, 0x0F

    ; Apuntar Z a tabla + nibble
    ldi  ZL, low(tabla*2)
	; Busco el indice del numero en la tabla correspondiente al valor actual del contador
    add  ZL, r18

    ; Cargar patrón 7 segmentos y enviar a PORTF
    lpm  r20, Z
    out  PORTF, r20

    rjmp loop

; ------------------ TABLA 7-SEG (0..F) ------------------
; Ajusta los valores según tu tipo de display (ánodo común / cátodo común).
tabla:
    ;    0     1     2     3     4     5     6     7     8     9     A     b     c     d     e     f
    .db  0x01, 0x4F, 0x12, 0x06, 0x4C, 0x24, 0x20, 0x0F, 0x00, 0x04, 0x08, 0x60, 0x31, 0x42, 0x30, 0x38


