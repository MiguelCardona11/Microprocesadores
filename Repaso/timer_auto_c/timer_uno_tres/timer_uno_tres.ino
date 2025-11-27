// Display 7 segmentos → Salidas (ánodo común)
const uint8_t displayPins_timer1[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
const uint8_t displayPins_timer3[8] = {22, 23, 24, 25, 26, 27, 28};
volatile int contador_1 = 0;
volatile int contador_3 = 2;

// Tabla de segmentos (ánodo común)
const byte tabla7seg[16] = {
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
  0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71
};

// Mostrar número en displays
void mostrarEnDisplay_timer1(byte valor) {
  byte salida = tabla7seg[valor];
  for (int i = 0; i < 8; i++) {
    bool estado = !((salida >> i) & 0x01);   // Invertir para ánodo común (LOW en segmento encendido)
    digitalWrite(displayPins_timer1[i], estado ? HIGH : LOW);
  }
}

void mostrarEnDisplay_timer3(byte valor) {
  byte salida = tabla7seg[valor];
  for (int i = 0; i < 8; i++) {
    bool estado = !((salida >> i) & 0x01);   // Invertir para ánodo común (LOW en segmento encendido)
    digitalWrite(displayPins_timer3[i], estado ? HIGH : LOW);
  }
}

void setup() {
  // Configurar pines de display
  for (int i = 0; i < 8; i++) {
    pinMode(displayPins_timer1[i], OUTPUT);
    digitalWrite(displayPins_timer1[i], HIGH); // Apagar segmentos al comenzar

    pinMode(displayPins_timer3[i], OUTPUT);
    digitalWrite(displayPins_timer3[i], HIGH); // Apagar segmentos al comenzar
  }

  // ***** CONFIGURACION DEL TIMER1 *****
  TCCR1A = TCCR1B = 0;                  // Eliminar cualquier configuracion previa del TIMER1
  TCCR1B |= (1 << WGM12) | (1 << CS12); // 1<<WGM12 modo CTC (comparación) - (1<<CS12) Preescaler de 256
  TCNT1 = 0;                            // Valor inicial del timer
  OCR1A = 62500;                        // Valor limite del contador; 62500 para 1 segundo
  TIMSK1 |= (1 << OCIE1A);              // habilitar interrupción por comparación para TIMER1; cuando se iguale a OCR1A, se activa interrupcion

  // ***** CONFIGURACION DEL TIMER3 *****
  TCCR3A = TCCR3B = 0;                  
  TCCR3B |= (1 << WGM32) | (1 << CS32); 
  TCNT3 = 0;                            
  OCR3A = 62500;                        
  TIMSK3 |= (1 << OCIE3A);              

  sei();                                
}

void loop() {
  mostrarEnDisplay_timer1(contador_1);
  mostrarEnDisplay_timer3(contador_3);
}

ISR(TIMER1_COMPA_vect) {
  contador_1 = (contador_1 + 1) % 16;  // Contador de 0 a 15
}

ISR(TIMER3_COMPA_vect) {
  contador_3 = (contador_3 + 1) % 16;  // Contador de 0 a 15
}
