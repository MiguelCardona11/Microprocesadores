;
; timer_pulsador_count_arduino.asm
;
; Created: 3/10/2025 3:30:29 p. m.
; Author : Miguel
;

.include "m2560def.inc"

.org 0x00
    rjmp start

; ----------------- START -----------------
start:
    ; --- configurar display (igual que antes) ---
    ldi r20, 0xff
    out ddrf, r20        ; PORTF como salida (7-seg)
    ldi r21, 0x00        ; contador = 0

    ; --- configurar boton en PD2 como entrada con pull-up ---
    cbi DDRD, PD2        ; DDRD.PD2 = 0 => entrada
    sbi PORTD, PD2       ; habilita pull-up interno en PD2

    ; guardar estado previo del pin (1 = no pulsado)
    ldi r23, (1<<PD2)    ; r23 mantiene la lectura previa de PD2 (mascara)

    ; --- (opcional) dejar configuración de Timer1 como en tu código original ---
    ldi r16,(1<<WGM12)|(1<<CS12)|(1<<CS10) ; CTC + prescaler 1024 (si lo quieres usar)
    sts TCCR1B,r16
    ldi r16,0
    sts TCCR1A,r16
    ldi r16,high(15625)
    sts OCR1AH,r16
    ldi r16,low(15625)
    sts OCR1AL,r16
    ; NOTA: NO habilitamos OCIE1A aquí (no queremos que la ISR incremente)

; ----------------- BUCLE PRINCIPAL -----------------
contar:
    ; --- mostrar el dígito actual (igual que antes) ---
    ldi ZL, low(tabla*2)
    add ZL, r21
    lpm r22, Z
    out portf, r22

    ; --- leer botón y detectar flanco 1->0 (no pulsado -> pulsado) ---
    in  r24, PIND            ; leer todo PIND
    andi r24, (1<<PD2)      ; dejar solo el bit PD2 (0 o (1<<PD2))
    cp  r24, r23            ; comparar con lectura previa
    breq no_cambio_btn      ; si igual -> nada que hacer

    ; hubo cambio en el pin -> ver si ahora está PRESIONADO (0)
    cpi r24, 0
    brne actualizar_prev    ; si r24 != 0 => ahora está 1 (liberado) -> actualizar y seguir

    ; r24 == 0 => transición 1 -> 0 detectada -> contamos 1
    inc r21
    cpi r21, 0x10
    brlo despues_contar
    ldi r21, 0x00           ; wrap a 0 tras 0x0F
despues_contar:
    ; esperar hasta que el botón sea liberado (simple debounce / evitar repetición por mantener)
espera_liberacion:
    in r24, PIND
    andi r24, (1<<PD2)
    cpi r24, 0
    breq espera_liberacion  ; permanecer aquí mientras siga PRESIONADO (PD2 == 0)

    ; opcional: pequeña espera adicional para rebotado (si quieres)
    ; (puedes implementar un pequeño bucle de retardo aquí)

actualizar_prev:
    mov r23, r24           ; guardar lectura actual como previa (0 o (1<<PD2))

no_cambio_btn:
    rjmp contar

; ----------------- TABLA 7-seg (igual que la tuya) -----------------
tabla:
    ; 0   1    2    3    4    5    6    7    8    9    a    b    c    d    e    f
    .db 0x01,0x4f,0x12,0x06,0x4c,0x24,0x20,0x0f,0x00,0x04,0x08,0x60,0x31,0x42,0x30,0x38



