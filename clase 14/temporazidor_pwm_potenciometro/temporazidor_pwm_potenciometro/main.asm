;
; temporazidor_pwm_potenciometro.asm
;
; Created: 1/10/2025 10:09:00 a. m.
; Author : Miguel
;
; pwm_con_potenciometro.asm
; PWM en OC2B (PH6 / Arduino D9) controlado por potenciometro en A0 (ADC0)
;

.include "m2560def.inc"

.org 0x0000
    rjmp start

; ================== INICIO ==================
start:
    ; -------- Entrada analogica (ADC0 / A0) --------
    ; ADMUX: REFS0=1 (AVcc como referencia), ADLAR=1 (alineado a la izquierda), MUX=0000 (ADC0)
    ldi  r16, (1<<REFS0) | (1<<ADLAR)
    sts  ADMUX, r16

    ; ADCSRB: MUX5=0 (canales 0..7), sin trigger especial
    ldi  r16, 0x00
    sts  ADCSRB, r16

    ; ADCSRA: ADEN=1 (habilita ADC), prescaler=128 (ADPS2:0=111 → 16MHz/128 ≈ 125 kHz)
    ldi  r16, (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0)
    sts  ADCSRA, r16

    ; -------- Salida PWM en OC2B (PH6 / D9) --------
    ; PH6 como salida
    lds  r16, DDRH
    ori  r16, (1<<PH6)
    sts  DDRH, r16

    ; Timer2: Fast PWM 8-bit (WGM21:20=11), no inversor en OC2B (COM2B1=1)
    ldi  r16, (1<<COM2B1) | (1<<WGM21) | (1<<WGM20)
    sts  TCCR2A, r16

    ; Prescaler = 64 (CS22=1) → ~976 Hz
    ldi  r16, (1<<CS22)
    sts  TCCR2B, r16

    ; Duty inicial
    ldi  r16, 0x00
    sts  OCR2B, r16

; ================== BUCLE PRINCIPAL ==================
loop:
    ; Iniciar conversion ADC (ADSC=1)
    lds  r16, ADCSRA
    ori  r16, (1<<ADSC)
    sts  ADCSRA, r16

adc_espera:
    ; Esperar a que termine la conversion (ADSC → 0)
    lds  r16, ADCSRA
    sbrc r16, ADSC       ; si ADSC sigue en 1, aún convierte
    rjmp adc_espera

    ; Leer 8 bits altos del resultado (por ADLAR=1, ADCH = bits 9..2)
    lds  r18, ADCH       ; 0..255 aprox.
    sts  OCR2B, r18      ; duty = lectura del potenciometro

    rjmp loop