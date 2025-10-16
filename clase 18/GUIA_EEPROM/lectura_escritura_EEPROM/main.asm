;1) Esperar a que no haya escritura en progreso (el bit EEPE debe estar en 0 antes de iniciar una nueva).
;2) Colocar la dirección de memoria destino en EEARH:EEARL.
;3)  Cargar el dato a escribir en EEDR.
;4)  Habilitar escritura: poner EEMPE = 1.
;5)  Iniciar escritura: inmediatamente después, poner EEPE = 1.

;==============================
; Escritura en EEPROM (ATmega2560)
;==============================


; Esperar a que finalice una escritura previa
WaitEE:
    sbic EECR, EEPE        ; Salta si EEPE = 0 (no está escribiendo)
    rjmp WaitEE            ; Si EEPE = 1, esperar

; Cargar dirección 0x000A
ldi r16, low(0x000A)
out EEARL, r16
ldi r16, high(0x000A)
out EEARH, r16

; Cargar dato a escribir
ldi r16, 0x55
out EEDR, r16

; Habilitar y ejecutar escritura
sbi EECR, EEMPE            ; Habilita escritura
sbi EECR, EEPE             ; Inicia escritura


;==============================
; Lectura en EEPROM (ATmega2560)
;==============================

; Esperar a que no haya escritura en curso
WaitEE2:
    sbic EECR, EEPE
    rjmp WaitEE2

; Cargar dirección 0x000A
ldi r16, low(0x000A)
out EEARL, r16
ldi r16, high(0x000A)
out EEARH, r16

; Iniciar lectura
sbi EECR, EERE

; Leer dato desde EEDR
in r17, EEDR               ; r17 ahora contiene el dato leído

