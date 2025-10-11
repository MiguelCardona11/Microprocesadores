;
; timer_pulsador_count_arduino.asm
;
; Created: 3/10/2025 3:30:29 p. m.
; Author : Miguel
;

;
; contador_pulsos_timer1.asm
; Cuenta pulsos externos en T1 → muestra el nibble bajo (hex 0..F) en PORTF (7 segmentos).
;

; Configurar el Timer1 para que cuente pulsos externos en T1 (pin 11 del ATmega2560)
; El contador solo avanza cuando recibe un pulso en el pin T1

;
; contador_pulsos_timer1.asm
; Cuenta pulsos externos en T1 → muestra el nibble bajo (hex 0..F) en PORTF (7 segmentos).
;

; contador_pulsos_timer1_adaptado.asm
; Cuenta pulsos externos en T1 → muestra el nibble bajo (hex 0..F) en PORTF (7 segmentos).
;

; contador_pulsos_timer5.asm
; Cuenta pulsos externos en T5 → muestra el nibble bajo (hex 0..F) en PORTF (7 segmentos).
;

.include "m2560def.inc"

.org 0x0000
    rjmp start

; ------------------ INICIO ------------------
start:
    ; ====== DISPLAY: PORTF como salida ======
    ldi  r16, 0xFF
    out  DDRF, r16          ; PF7..PF0 = salida

    ; ====== TIMER5 como contador externo en T5 ======
    ; Modo normal: WGM53:0 = 0000 → TCCR5A=0, WGM52=0 en TCCR5B
    ldi  r16, 0x00
    sts  TCCR5A, r16

    ; Fuente de reloj = externa en T5 (flanco de subida): CS52:50 = 111
    ldi  r16, (1<<CS52) | (1<<CS51) | (1<<CS50)
    sts  TCCR5B, r16

    ; Reiniciar contador
    ldi  r16, 0x00
    sts  TCNT5H, r16
    sts  TCNT5L, r16

; ------------------ BUCLE PRINCIPAL ------------------
loop:
    ; Leer TCNT5 de forma atómica leyendo primero LOW luego HIGH
    lds  r18, TCNT5L        ; r18 = low byte
    ; Tomar nibble bajo de r18 (0..15)
    andi r18, 0x0F

    ; Apuntar Z a tabla + nibble
    ldi  ZL, low(tabla*2)
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


