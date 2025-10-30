// Tabla segmento ANODO COMUN: enciende segmento con 0, apaga con 1
const uint8_t segment_map[10] = {
  0b11000000, // '0'
  0b11111001, // '1'
  0b10100100, // '2'
  0b10110000, // '3'
  0b10011001, // '4'
  0b10010010, // '5'
  0b10000010, // '6'
  0b11111000, // '7'
  0b10000000, // '8'
  0b10010000  // '9'
};

#define SEG_PORT  PORTA

#define DISP1_PIN  7       // pin display ENTERO
#define DISP2_PIN  6       // pin display DECIMAL

// display ENTERO
#define DISP1_ON()  (PORTB |= (1<<DISP1_PIN))
#define DISP1_OFF() (PORTB &= ~(1<<DISP1_PIN))

// display DECIMAL
#define DISP2_ON()  (PORTB |= (1<<DISP2_PIN))
#define DISP2_OFF() (PORTB &= ~(1<<DISP2_PIN))

void setup() {
  DDRA = 0xFF;                         // PA0..PA7 como salidas (segmentos)
  DDRB |= (1<<DISP1_PIN)|(1<<DISP2_PIN);     // Pines de selección de display como salida

  // ADMUX: ADC Multiplexer Selection Register
  // 0110 0000 - REFS0: referencia AVCC - ADLAR: lectura del resultado del ADC en ADCH (nibble high), es decir, ajuste a la izquierda
  ADMUX = (1 << REFS0) | (1 << ADLAR) | (0);

  // ADCSRA: configura el ADC para el registro A
  // ADC es el convertidor analógico–digital
  // 1000 0111  ADEN: habilitar ADC - ADPS2-0: Preescaler de 128 (16MHz / 128 = 125kHz)
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

  // Timer2: Fast PWM 8 bits en OC2B (D9)

  // TCCR2B y TCCR2B deciden el modo del temporizador
  // 1 << COM2B1: Cambio por comparación, OC2A at BOTTOM (no invertido)
  // (1 << WGM21) | (1 << WGM20): modo fast PWM
  TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
  TCCR2B = (1 << CS22); // prescaler 64 (~976Hz)
  // OCR2B es el comparador: valor 0–255 que controla el duty cycle.
  OCR2B = 0;
}

void loop() {
  // (1 << ADSC): iniciar conversión ADC
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));
  uint8_t adc_val = ADCH; // Toma solo los 8 bits altos

  // --- Actualiza PWM duty cycle ---
  OCR2B = adc_val;

  // --- Convierte a voltaje y dígitos ---
  float volt = (adc_val / 255.0) * 5.0; // Rango 0.0 a 5.0
  uint8_t entero = (uint8_t)volt;             // Parte entera (0 a 5)
  uint8_t decimal = (uint8_t)((volt - entero) * 10); // Primer decimal (0 a 9)

  // --- Multiplexación: alterna el encendido de los displays ---
  // Display 1 - parte entera
  SEG_PORT = segment_map[entero];
  DISP1_ON(); DISP2_OFF();
  delay(3); // La persistencia visual evita parpadeo

  // Display 2 - parte decimal, sin punto
  SEG_PORT = segment_map[decimal];
  DISP2_ON(); DISP1_OFF();
  delay(3);
}