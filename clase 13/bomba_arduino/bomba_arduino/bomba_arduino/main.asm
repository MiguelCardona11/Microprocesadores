;
; bomba_arduino.asm
;
; Created: 26/09/2025 3:55:18 p. m.
; Author : Miguel
;


;
; ejercicio_3.asm
;
; Created: 29/08/2025 3:28:03 p. m.
; Author : Miguel
;

; Se debe realizar el control de un depósito de agua de forma que el nivel del depósito está
; indicado mediante 4 sensores conectados al puerto F. El nivel se visualizará mediante una
; barra de 4 leds conectada al puerto K. Además al llegar al nivel mínimo se debe poner en
; marcha una bomba de agua, esto se observará en un led. Al llegar al nivel máximo la
; bomba debe parar y se debe indicar por medio de un led parpadeante.

start:
    ; Estados Bomba
    ; 0x80 -> LED bomba encendida (PK7)
    ; 0x40 -> LED bomba llena parpadeante (PK6)
    
    ; Niveles / Sensores
    ldi r18, 0x01    ; nivel 1
    ldi r19, 0x03    ; nivel 2
    ldi r20, 0x07    ; nivel 3
    ldi r21, 0x0f    ; nivel 4 (lleno)

    ; Puertos de entrada y salida
    ldi r16, 0x00
    ldi r17, 0xff
    sts ddrf, r16
    sts ddrk, r17

    clr r23          ; r23 = estado bomba: bit7 LED bomba

main_loop:
    in r22, pinf
    andi r22,0x0f        ; solo nibble bajo
    ; *** lógica de bomba ***
    cp r22, r21
    breq parpadear        ; si lleno → LED parpadeante

    ; si nivel <=1 → encender bomba
    cp r22,r18            ; ¿nivel 1?
    brlo bomba_on         ; menor → 0 sensores
    breq bomba_on         ; igual a 1 sensor
    ; si nivel 2 o 3 → mantener estado de bomba
    rjmp bomba_keep

bomba_on:
    ; activa bit 7 en estado bomba
    ori r23,0x80
    rjmp actualizar_salida

bomba_keep:
    ; no cambia r23 (mantiene bit7)
    rjmp actualizar_salida

actualizar_salida:
    ; combina leds de sensores + estado bomba
    mov r25,r22           ; leds sensores
    or  r25,r23           ; añadir LED bomba si está activo
    sts portk,r25
    rjmp main_loop

; --- LED parpadeante mientras lleno ---
parpadear:
    ; apagar LED bomba (bit7)
    andi r23,0x7f

parpadear_loop:
    ; encender LED lleno (bit6) + sensores
    mov r25,r22
    ori r25,0x40
    sts portk,r25
    call esperar_1s

    ; apagar LED lleno (bit6) + sensores
    mov r25,r22
    andi r25,0xbf
    sts portk,r25
    call esperar_1s

    ; leer sensores de nuevo
    in r22,pinf
    andi r22,0x0f
    cp r22,r21
    breq parpadear_loop   ; seguir parpadeando

    ; salir: volver a main_loop, bomba apagada
    sts portk,r22
    rjmp main_loop

; --- delay 1s ---
esperar_1s:
    ldi r27,0x3e
    call ciclo_3
    ret

ciclo_1:
    dec r25
    cp r16,r25
    brne ciclo_1
    ret

ciclo_2:
    ldi r25,0xff
    call ciclo_1
    dec r26
    cp r16,r26
    brne ciclo_2
    ret

ciclo_3:
    ldi r26,0xff
    call ciclo_2
    dec r27
    cp r16,r27
    brne ciclo_3
    ret
