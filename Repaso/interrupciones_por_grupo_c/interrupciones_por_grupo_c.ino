

const uint8_t pines_pcint_0[8] = {53, 52, 51, 50, 10, 11, 12, 13}; // pines PCINT0 - PCINT7
const uint8_t pines_pcint_2[8] = {A8, A9, A10, A11, A12, A13, A14, A15}; // pines PCINT16 - PCINT23
const uint8_t pines_leds[2] = {22, 23}; // pines para mostar LEDS

void setup() {
  // pines pines PCINT0 - PCINT7 como entrada
  for (int i = 0; i < 8; i++) {
    pinMode(pines_pcint_0[i], INPUT); 
  }
  // pines pines PCINT16 - PCINT23 como entrada
  for (int i = 0; i < 8; i++) {
    pinMode(pines_pcint_2[i], INPUT); 
  }

  // pines para mostrar LEDS salida
  for (int i = 0; i < 2; i++) {
    pinMode(pines_leds[i], OUTPUT); 
    digitalWrite(pines_leds[i], LOW);
  }


  // **** CONFIGURACIÓN DE INTERRUPCIONES ****
  PCICR |= 0x05;  // PCICR indica cuales grupos de interrupciones se habilitarán; 0000 0101: se habilitan 2 grupos (23:16, 7:0)
  PCMSK0 |= 0xFF; // PCMSK0 decide qué pines del grupo 0 habilitar; 1111 1111: se habilita PCINT7:0
  PCMSK2 |= 0xFF; // PCMSK2 decide qué pines del grupo 2 habilitar; 1111 1111: se habilita PCINT23:16
  sei();         // habilitamos interrupciones globales
}

void loop() {
  // put your main code here, to run repeatedly:
}

ISR(PCINT0_vect) {
  bool estadoActual = digitalRead(pines_leds[0]);
  digitalWrite(pines_leds[0], !estadoActual);
}

ISR(PCINT2_vect) {
  bool estadoActual = digitalRead(pines_leds[1]);
  digitalWrite(pines_leds[1], !estadoActual);
}


