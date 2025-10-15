# Microcontroladores Shortcuts

## Microprocesador vs Microcontrolador
- Un microprocesador tiene ALU, Unidad de control y registros internos, tiene una memoria unica para los datos e instrucciones. Es de proposito general 
- Un microcontrolador tiene ALU, Unidad de control, registros internos y tiene 2 memorias, una para los datos y otra para las instrucciones o el programa. Es de proposito especifico

## Saltos
```assembly
start:
    rjmp etiqueta
```

## Memoria IRAM (200H)
```assembly
start:
    ; cargar en un registro
    lds r16, 0x0200

    ; almacenar en posiciones de memoria
    sts 0x0202, r16
```

## Datos inmediatos en registros
```assembly
start:
    ldi r16, 12
```

## Comparadores
```assembly
start:
    ; greater or equal
    brge etiqueta_1

    ; same or higher
    brsh etiqueta_2
```

## Operadores logicos (entre bits)
```assembly
start:
    andi r16, 0xF0
```

## Funciones
```assembly
start:
    ; cargar en un registro
    lds r16, 0x0200

    ; almacenar en posiciones de memoria
    sts 0x0202, r16

    call funcion1;

funcion1:
    ; codigo aqui
    ret
```

## Puertos
Registro PORTX sirve sacar el dato

El registro DDRX debe estar en 1 para dejar salir el dato, 0 para entrada de datos

**Nota**: la X se intercambia según el puerto que estemos utilizando (A, B, C, D…)

Para obtener el valor de un puerto se utiliza la instruccion IN y para sacar un valor por un puerto se utiliza la instruccion OUT

```assembly
start:
    ldi r17, 0x00   ; 0s para entrada
    out ddra, r17

    ldi r17, 0xFF   ; 1s para salida
    out ddrb, r17

    in r16, pina    ; leer el PIN/Puerto A

    out portb, r16
```

## Apuntadores (Pointers)
El ATmega2560 dispone de tres registros específicos para apuntadores de memoria: X, Y y Z. Estos registros pueden contener direcciones de memoria y usarse para acceder a datos en SRAM u otras áreas de memoria. Por ejemplo:
- El registro X está compuesto por los registros R27:R26 (R27 es el byte más alto, R26 el más bajo).
- El registro Y está compuesto por R29:R28.
- El registro Z está compuesto por R31:R30.


```assembly
start:
    ldi ZL,low(tabla*2)
    ldi ZH,high(tabla*2)

    ; cargar en r22 el valor de Z
    lpm r22,Z

```

## Tabla
Para guardar los datos de los valores asociados a cada número, la guardamos en la memoria del programa (instrucciones), que es ROM, y no se pierden los datos al apagar la máquina:

```assembly
start:
    ldi r17, 0x00   ; 0s para entrada
    out ddra, r17

    ldi r17, 0xFF   ; 1s para salida
    out ddrb, r17

    in r16, pina    ; leer el PIN/Puerto A

    out portb, r16

    ; adiw, incrementa un apuntador en conjunto, es decir, incrementa los 2 registros que lo componen
    adiw ZL, 1  ;

tabla:
    .db 0x01
```



## Delays (Manual)
```assembly
esperar_1s:	; ~ 15.904.767 ciclos
	ldi r16, 0x00			; 1		--> 1c
	ldi r19, 0x1f			; 1		--> 1c

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
```

## Interrupciones (Externas)
INT0 a INT7
Para interrupciones externas (switches, sensores…) diferentes a los periféricos del microcontrolador.
**NOTA**: NO USAR pines INT0 e INT1. Dan problemas ya que se usan para otras cosas. INT6 e INT7 No tienen pines.

### Activar interrupciones globales
```assembly
start:
    sei ; Set Global Interrupt Flag (I=1) SREG.7
```

### Direcciones de vectores
- INT0 -> 0x0002
- INT1 -> 0x0004
- INT2 -> 0x0006
- INT3 -> 0x0008
- INT4 -> 0x000A
- INT5 -> 0x000C
- INT6 -> 0x000E
- INT7 -> 0x0010

### Pines de cada interrupcion
- INT0 -> Pin PE0
- INT1 -> Pin PE1
- INT2 -> Pin PE2
- INT3 -> Pin PE3
- INT4 -> Pin PE4
- INT5 -> Pin PE5
- INT6 -> No tiene **EVITAR USAR**
- INT7 -> No tiene **EVITAR USAR**

### Registro EICRA (modos de disparo) - INT0 a INT3
Cada interrupción usa dos bits para configurar el disparo: ISCnx1 e ISCnx0
Combinaciones
- 00: Disparo por nivel bajo
- 01: Disparo por cualquier cambio logico
- 10: Flanco de bajada
- 11: Flanco de subida

El registro EICRA tiene 8 bits donde cada par de bits configura el modo de disparo de una interrupcion en especifico

| BIT 7 | BIT 6 | BIT 5 | BIT 4 | BIT 3 | BIT 2 | BIT 1 | BIT 0 |
|-------|-------|-------|-------|-------|-------|-------|-------|
| INT3  | INT3  | INT2  | INT2  | INT1  | INT1  | INT0  | INT0  |

### Registro EICRB (modos de disparo) - INT4 a INT7
| BIT 7 | BIT 6 | BIT 5 | BIT 4 | BIT 3 | BIT 2 | BIT 1 | BIT 0 |
|-------|-------|-------|-------|-------|-------|-------|-------|
| INT7  | INT7  | INT6  | INT6  | INT5  | INT5  | INT4  | INT4  |

### Registro EIMSK para activar la interrupcion en especifico
El registro EIMSK es el que habilita o deshabilita las interrupciones externas específicas. Cada bit corresponde a una interrupción externa:

| BIT 7 | BIT 6 | BIT 5 | BIT 4 | BIT 3 | BIT 2 | BIT 1 | BIT 0 |
|-------|-------|-------|-------|-------|-------|-------|-------|
| INT7  | INT6  | INT5  | INT4  | INT3  | INT2  | INT1  | INT0  |


## Interrupciones PCINT
Las interrupciones PCINT (Pin Change Interrupt) son un tipo especial de interrupciones externas que se activan cuando cambia el estado lógico de uno o varios pines de un grupo, no solo en un pin específico como las interrupciones INT tradicionales.

### Grupos de pines y vectores PCINT en ATmega2560:
- PCINT0_vect: cubre PCINT[7:0] (pines del puerto B) -> 0x0012
- PCINT1_vect: cubre PCINT[14:8] (pines del puerto J) -> 0x0014
- PCINT2_vect: cubre PCINT[23:16] (pines del puerto K) -> 0x0016

### Registros PCICR (Habilitar grupo de pines)
| BIT 7 | BIT 6 | BIT 5 | BIT 4 | BIT 3 | BIT 2 | BIT 1 | BIT 0 |
|-------|-------|-------|-------|-------|-------|-------|-------|
|       |       |       |       |       | PCIE2 | PCIE1 | PCIE0 |

- PCIE0: Habilita el grupo PCINT0, relacionado con PCINT[7:0] (pines del puerto B).
- PCIE1: Habilita el grupo PCINT1, relacionado con PCINT[14:8] (pines del puerto J).
- PCIE2: Habilita el grupo PCINT2, relacionado con PCINT[23:16] (pines del puerto K).

Para habilitar interrupciones PCINT en el grupo 0 (pines del puerto B), se debe poner el bit PCIE0 en 1 escribiendo en PCICR, por ejemplo:
```assembly
start:
    ldi r24, (1 << PCIE0)
    sts PCICR, r24

    ; Habilitar 2 o mas grupos
    ldi r24, (1 << PCIE0) | (1 << PCIE1) ; habilita PCINT grupo 0 y grupo 1
    sts PCICR, r24
```

### Registro PCMSKx (Habilitar pines especifico de cada grupo)
PCMSKx (x = 0,1,2) para habilitar el INT especifico del grupo.

Ejemplo en PCMSK0 (8 bits):
| Bit 7   |  Bit 6   |  Bit 5   |  Bit 4   |  Bit 3   |  Bit 2   |  Bit 1   |  Bit 0 
| --------|----------|----------|----------|----------|----------|----------|--------
| PCINT7  |  PCINT6  |  PCINT5  |  PCINT4  |  PCINT3  |  PCINT2  |  PCINT1  |  PCINT0

PCMSK1 
| Bit 7  |  Bit 6    |  Bit 5    |  Bit 4    |  Bit 3    |  Bit 2    |  Bit 1   |  Bit 0 
| -------|-----------|-----------|-----------|-----------|-----------|----------|--------
|        |  PCINT14  |  PCINT13  |  PCINT12  |  PCINT11  |  PCINT10  |  PCINT9  |  PCINT8

PCMSK2
| Bit 7    |  Bit 6    |  Bit 5    |  Bit 4    |  Bit 3    |  Bit 2    |  Bit 1    |  Bit 0  
| ---------|-----------|-----------|-----------|-----------|-----------|-----------|---------
| PCINT23  |  PCINT22  |  PCINT21  |  PCINT20  |  PCINT19  |  PCINT18  |  PCINT17  |  PCINT16


### Configurar la rutina de interrupcion - ISR
```assembly

.org 0x00
rjmp start          ; reset

.org 0x0012			; vector de interrupción para grupo 0
rjmp ISR_PCINT0

start:
    ldi r24, (1 << PCIE0)
    sts PCICR, r24

    ; Habilitar 2 o mas grupos
    ldi r24, (1 << PCIE0) | (1 << PCIE1) ; habilita PCINT grupo 0 y grupo 1
    sts PCICR, r24
```

## Temporizadores
Un temporizador (timer) es un módulo del microcontrolador que cuenta pulsos del reloj internamente, permitiendo medir tiempo, generar delays precisos, o activar acciones periódicamente.

### Tipos
1. Timers de 8 bits: Timer 0 y Timer 2
2. Timers de 16 bits: Timer 1, Timer 3, Timer4 y Timer5

**IMPORTANTE**: NO UTILIZAR TIMER 0 y TIMER 1

### Modos
- Normal: Cuenta desde 0 hasta su valor maximo (255 para 8bits, 65535 para 16bits)
- CTC: Cuenta desde 0 hasta un valor definido en el registro OCRnA
- PWM: El timer genera una señal cuadrada cuyo ciclo de trabajo (duty cycle) puedes controlar, este modo se configura por medio de los bits de WGM
  - Fast PWM: 
  - Phase Correct PWM
- Modo Contador: Cuenta pulsos en lugar del reloj interno

### Registros
- TCCRnA / TCCRnB: Modo de operacion del timer
- TCNTn: Lleva la cuenta actual del temporizador
- OCRnA / OCRnB: Valor de comparacion para el modo CTC

### TCCRnA - Timer/Counter Control Register A

#### Timer 0 (TCCR0A)
Bit 7   |  Bit 6   |  Bit 5   |  Bit 4   |  Bit 3  |  Bit 2  |  Bit 1  |  Bit 0
--------|----------|----------|----------|---------|---------|---------|-------
COM0A1  |  COM0A0  |  COM0B1  |  COM0B0  |  –      |  –      |  WGM01  |  WGM00

#### Timer 1 (TCCR1A)
Bit 7   |  Bit 6   |  Bit 5   |  Bit 4   |  Bit 3   |  Bit 2   |  Bit 1  |  Bit 0
--------|----------|----------|----------|----------|----------|---------|-------
COM1A1  |  COM1A0  |  COM1B1  |  COM1B0  |  COM1C1  |  COM1C0  |  WGM11  |  WGM10

#### Timer 2 (TCCR2A)
Bit 7   |  Bit 6   |  Bit 5   |  Bit 4   |  Bit 3  |  Bit 2  |  Bit 1  |  Bit 0
--------|----------|----------|----------|---------|---------|---------|-------
COM2A1  |  COM2A0  |  COM2B1  |  COM2B0  |  –      |  –      |  WGM21  |  WGM20

### TCCRnA - Timer/Counter Control Register B
#### Timer 0 (TCCR0B)
Bit 7  |  Bit 6  |  Bit 5  |  Bit 4  |  Bit 3  |  Bit 2  |  Bit 1  |  Bit 0
-------|---------|---------|---------|---------|---------|---------|-------
FOC0A  |  FOC0B  |  –      |  –      |  WGM02  |  CS02   |  CS01   |  CS00 

#### Timer 1 (TCCR1B)
Bit 7  |  Bit 6  |  Bit 5  |  Bit 4  |  Bit 3  |  Bit 2  |  Bit 1  |  Bit 0
-------|---------|---------|---------|---------|---------|---------|-------
ICNC1  |  ICES1  |  –      |  WGM13  |  WGM12  |  CS12   |  CS11   |  CS10 

#### Timer 2 (TCCR2B)
Bit 7  |  Bit 6  |  Bit 5  |  Bit 4  |  Bit 3  |  Bit 2  |  Bit 1  |  Bit 0
-------|---------|---------|---------|---------|---------|---------|-------
FOC2A  |  FOC2B  |  –      |  –      |  WGM22  |  CS22   |  CS21   |  CS20 

### TCTnT - Timer/Counter Register
- TCNT0: 8 bits, cuenta actual de Timer0.
- TCNT1: 16 bits, cuenta actual de Timer1 (se accede como TCNT1H y TCNT1L).
- TCNT2: 8 bits, cuenta actual de Timer2.

### OCRnA / OCRnB - Output Compare Registers
- OCR0A / OCR0B: 8 bits, valores de comparación para Timer0.
- OCR1A / OCR1B / OCR1C: 16 bits, valores de comparación para Timer1.
- OCR2A / OCR2B: 8 bits, valores de comparación para Timer2.


### Vectores

#### Timer 0 (8bits)
| Modo             |  Vector ISR         |  Dirección
| -----------------|---------------------|-----------
| Overflow         |  TIMER0_OVF_vect    |  0x0020   
| Compare Match A  |  TIMER0_COMPA_vect  |  0x001A   
| Compare Match B  |  TIMER0_COMPB_vect  |  0x001C   

#### Timer 1 (16bits)
| Modo             |  Vector ISR         |  Dirección
| -----------------|---------------------|-----------
| Overflow         |  TIMER1_OVF_vect    |  0x0024   
| Compare Match A  |  TIMER1_COMPA_vect  |  0x0022   
| Compare Match B  |  TIMER1_COMPB_vect  |  0x0024   
| Input Capture    |  TIMER1_CAPT_vect   |  0x0020   

#### Timer 2 (8bits)
| Modo             |  Vector ISR         |  Dirección
| -----------------|---------------------|-----------
| Overflow         |  TIMER2_OVF_vect    |  0x0028   
| Compare Match A  |  TIMER2_COMPA_vect  |  0x0026   
| Compare Match B  |  TIMER2_COMPB_vect  |  0x0028   

#### Timer 3, 4, 5 (16bits)
| Modo             |  Vector ISR         |  Dirección
| -----------------|---------------------|-----------
| Overflow         |  TIMER3_OVF_vect    |  0x0036   
| Compare Match A  |  TIMER3_COMPA_vect  |  0x0033   
| Compare Match B  |  TIMER3_COMPB_vect  |  0x0034   
| Compare Match C  |  TIMER3_COMPC_vect  |  0x0035   
| Input Capture    |  TIMER3_CAPT_vect   |  0x0032   


### Prescaler
El prescaler es un divisor de frecuencia que se coloca a la entrada del temporizador. Su función es reducir la frecuencia con la que el timer incrementa su contador, permitiendo contar tiempos mucho más largos usando los mismos registros. Por ejemplo, si el reloj del microcontrolador va a 16MHz y eliges un prescaler de 1024, el timer avanzará una vez cada 
1024 ciclos del reloj principal.​
- Sin prescaler (divisor 1): cada ciclo del reloj suma +1 al contador del timer.
- Con prescaler: el contador suma +1 al pasar n ciclos (el valor del prescaler).


#### Timer0 (8 bits)
Registro: TCCR0B

Bits: CS02, CS01, CS00 (bits 2, 1, 0)

CS02  |  CS01  |  CS00  |  Fuente de reloj / Prescaler
------|--------|--------|-----------------------------
0     |  0     |  0     |  No clock (timer detenido)  
0     |  0     |  1     |  Sin prescaler (1)          
0     |  1     |  0     |  8                          
0     |  1     |  1     |  64                         
1     |  0     |  0     |  256                        
1     |  0     |  1     |  1024                       
1     |  1     |  0     |  Fuente externa, flanco bajo
1     |  1     |  1     |  Fuente externa, flanco alto

#### Timer1 (16 bits)
Registro: TCCR1B

Bits: CS12, CS11, CS10 (bits 2, 1, 0)

CS12  |  CS11  |  CS10  |  Fuente de reloj / Prescaler
------|--------|--------|-----------------------------
0     |  0     |  0     |  No clock (timer detenido)  
0     |  0     |  1     |  Sin prescaler (1)          
0     |  1     |  0     |  8                          
0     |  1     |  1     |  64                         
1     |  0     |  0     |  256                        
1     |  0     |  1     |  1024                       
1     |  1     |  0     |  Fuente externa, flanco bajo
1     |  1     |  1     |  Fuente externa, flanco alto

#### Timer2 (8 bits)
Registro: TCCR2B

Bits: CS22, CS21, CS20 (bits 2, 1, 0)

CS22  |  CS21  |  CS20  |  Fuente de reloj / Prescaler
------|--------|--------|-----------------------------
0     |  0     |  0     |  No clock (timer detenido)  
0     |  0     |  1     |  Sin prescaler (1)          
0     |  1     |  0     |  8                          
0     |  1     |  1     |  32                         
1     |  0     |  0     |  64                         
1     |  0     |  1     |  128                        
1     |  1     |  0     |  256                        
1     |  1     |  1     |  1024                       

Los timers 3, 4 y 5 del ATMega2560 son de 16 bits y su configuración de prescaler es idéntica a la de Timer1. Cada uno tiene su propio registro de control: TCCR3B, TCCR4B y TCCR5B.
CSn2  |  CSn1  |  CSn0  |  Fuente de reloj / Prescaler
------|--------|--------|-----------------------------
0     |  0     |  0     |  No clock (timer detenido)  
0     |  0     |  1     |  Sin prescaler (1)          
0     |  1     |  0     |  8                          
0     |  1     |  1     |  64                         
1     |  0     |  0     |  256                        
1     |  0     |  1     |  1024                       
1     |  1     |  0     |  Fuente externa, flanco bajo
1     |  1     |  1     |  Fuente externa, flanco alto

CSn2, CSn1, CSn0: Son los bits de prescaler en el registro TCCRnB, donde n = 3, 4, 5 según el timer que uses.

### Modo Normal
```assembly
start:
    ; Poner TCCR0A y TCCR0B en 0 (modo normal)
    ldi r16, 0
    out TCCR0A, r16
    out TCCR0B, r16

    ; Seleccionar pre-escalador 1024 (CS02=1, CS00=1)
    ldi r16, (1<<CS02)|(1<<CS00)
    out TCCR0B, r16

    ; Inicializar contador
    ldi r16, 0
    out TCNT0, r16
```

### Modo CTC
```assembly
start:
    ; Modo CTC: WGM01=1
    ldi r16, (1<<WGM01)
    out TCCR0A, r16
    ldi r16, 0
    out TCCR0B, r16
    
    ; Pre-escalador 1024
    ldi r16, (1<<CS02)|(1<<CS00)
    out TCCR0B, r16
    
    ; Valor de comparación
    ldi r16, 156 ; Por ejemplo
    out OCR0A, r16
    
    ; Habilitar interrupción de comparación
    ldi r16, (1<<OCIE0A)
    out TIMSK0, r16
    
    ; Inicializar contador
    ldi r16, 0
    out TCNT0, r16
```

### Modo PWM
- El temporizador cuenta desde 0 hasta un valor máximo (por ejemplo, 255 en 8 bits).
- Cuando el contador alcanza el valor del registro de comparación (OCRnA o OCRnB), la salida asociada cambia de estado.
- El ciclo de trabajo (duty cycle) se ajusta cambiando el valor de OCRnx:
  - Valor bajo: pulso corto (menos tiempo en alto)
  - Valor alto: pulso largo (más tiempo en alto)

#### Modos de PWM
- Fast PWM: El contador sube de 0 a TOP (por ejemplo, 255), la salida cambia al llegar a OCRnx, y se reinicia.
- Phase Correct PWM: El contador sube de 0 a TOP y luego baja a 0, generando una señal más simétrica.