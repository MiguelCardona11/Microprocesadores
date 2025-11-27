

const uint8_t pines_int[6] = {21, 20, 19, 18, 2, 3}; // pines INT0, INT1, INT2, INT3, INT4, INT5
const uint8_t pines_leds[6] = {22, 23, 24, 25, 26, 27}; // pines para mostar LEDS

void setup() {
  // pines de INT5-4 y INT3-0 como entrada
  for (int i = 0; i < 6; i++) {
    pinMode(pines_int[i], INPUT); 
  }

  // pines para mostrar LEDS salida
  for (int i = 0; i < 6; i++) {
    pinMode(pines_leds[i], OUTPUT); 
    digitalWrite(pines_leds[i], LOW);
  }


  // **** CONFIGURACIÓN DE INTERRUPCIONES ****
  EICRA |= 0xFF; // EICRA decide si la interrupción será subida/bajada/ambos para INT3-0;  1111 1111: todos con cambio por bajada
  EICRB |= 0x0F; // EICRB decide si la interrupción será subida/bajada/ambos para INT7-4;  0000 1111: int5 e int4 con cambio por bajada
  EIMSK |= 0x3F; // EIMSK decide qué interrupciones habilitar (INT0-INT7) ; 0011 1111 habilitamos INT0-INT5
  sei();         // habilitamos interrupciones globales
}

void loop() {
  // put your main code here, to run repeatedly:
}

ISR(INT0_vect) {
  bool estadoActual = digitalRead(pines_leds[0]);
  digitalWrite(pines_leds[0], !estadoActual);
}

ISR(INT1_vect) {
  bool estadoActual = digitalRead(pines_leds[1]);
  digitalWrite(pines_leds[1], !estadoActual);
}

ISR(INT2_vect) {
  bool estadoActual = digitalRead(pines_leds[2]);
  digitalWrite(pines_leds[2], !estadoActual);
}

ISR(INT3_vect) {
  bool estadoActual = digitalRead(pines_leds[3]);
  digitalWrite(pines_leds[3], !estadoActual);
}

ISR(INT4_vect) {
  bool estadoActual = digitalRead(pines_leds[4]);
  digitalWrite(pines_leds[4], !estadoActual);
}

ISR(INT5_vect) {
  bool estadoActual = digitalRead(pines_leds[5]);
  digitalWrite(pines_leds[5], !estadoActual);
}
