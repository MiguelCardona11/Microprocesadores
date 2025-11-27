// Display 7 segmentos → Salidas (ánodo común)
const uint8_t displayPins[8] = {A0, A1, A2, A3, A4, A5, A6, A7};

// Tabla de segmentos (ánodo común)
const byte tabla7seg[16] = {
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
  0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71
};

// Mostrar número en display
void mostrarEnDisplay_timer1(byte valor) {
  byte salida = tabla7seg[valor];
  for (int i = 0; i < 8; i++) {
    bool estado = !((salida >> i) & 0x01);   // Invertir para ánodo común (LOW en segmento encendido)
    digitalWrite(displayPins[i], estado ? HIGH : LOW);
  }
}

void setup() {
  // Configurar pines de display
  for (int i = 0; i < 8; i++) {
    pinMode(displayPins[i], OUTPUT);
    digitalWrite(displayPins[i], HIGH); // Apagar segmentos al comenzar
  }

  // ***** CONFIGURACION DEL TIMER5 *****
  // En este caso se deja el TIMER5 en modo normal (desborde)
	// En vez de preescalador, habrá un reloj externo en el pin T5, que hará que la cuenta en TCNT5 aumente
  TCCR5A = TCCR1B = 0;                          // Eliminar cualquier configuracion previa del TIMER1
  TCCR5B |= (1<<CS52) | (1<<CS51) | (1<<CS50);  // (1<<CS52) | (1<<CS51) | (1<<CS50) --> modo reloj externo con flanco de subida
  TCNT5 = 0;                                    // Valor inicial del contador, empieza en 0                                      // Activar interrupciones globales
}

void loop() {
  // El contador esta en TCNT5
  if (TCNT5 >= 16) {
    TCNT5 = 0;
  }
  mostrarEnDisplay_timer1(TCNT5);

}

