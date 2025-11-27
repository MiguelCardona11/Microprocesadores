// ================== Configuración de pines ==================
// Filas → Salidas (rotan un 0)
const uint8_t filas[4] = {53, 52, 51, 50}; // PB0–PB3 en el Mega

// Columnas → Entradas con interrupción
const uint8_t columnas[4] = {A8, A9, A10, A11}; // PK0–PK3

// Display 7 segmentos → Salidas (ánodo común)
const uint8_t displayPins[8] = {A0, A1, A2, A3, A4, A5, A6};

// Tabla de segmentos (ánodo común)
const byte tabla7seg[16] = {
  0x4f, 0x12, 0x06, 0x08, 0x4c, 0x24, 0x20, 0x60, 0x0f, 0x00, 0x04, 0x31, 0x30, 0x01, 0x38, 0x42
};

volatile int teclaPresionada = -1; // Valor 0–15
volatile bool teclaDetectada = false;

// ================== FUNCIONES ==================

// Mostrar número en display
void mostrarEnDisplay(byte valor) {
  byte salida = tabla7seg[valor];
  for (int i = 0; i < 8; i++) {
    bool estado = (salida >> i) & 0x01;
    digitalWrite(displayPins[i], estado ? HIGH : LOW); // ánodo común
  }
}

// ================== SETUP ==================
void setup() {
  // Configurar pines de display
  for (int i = 0; i < 8; i++) {
    pinMode(displayPins[i], OUTPUT);
    digitalWrite(displayPins[i], HIGH); // apagar
  }

  // Configurar filas como salida
  for (int i = 0; i < 4; i++) {
    pinMode(filas[i], OUTPUT);
    digitalWrite(filas[i], HIGH);
  }

  // Configurar columnas como entrada con pull-up
  for (int i = 0; i < 4; i++) {
    pinMode(columnas[i], INPUT_PULLUP);
  }

  // Habilitar interrupciones de cambio (Pin Change Interrupt)
  PCICR |= (1 << PCIE2);   // Grupo PCINT2 (PK0–PK7)
  PCMSK2 |= 0x0F;          // Habilitar PK0–PK3
  sei();                   // Activar interrupciones globales
}

// ================== LOOP ==================
void loop() {
  // Barrido de filas (rotar 0)
  static int filaActiva = 0;
  for (int i = 0; i < 4; i++) {
    digitalWrite(filas[i], HIGH);
  }
  digitalWrite(filas[filaActiva], LOW);

  // Rotar fila cada 10 ms
  filaActiva = (filaActiva + 1) % 4;

  // Si se detectó tecla, mostrar en display
  if (teclaDetectada) {
    // Leer columnas
    int col = -1;
    for (int i = 0; i < 4; i++) {
      if (digitalRead(columnas[i]) == LOW) col = i;
    }

    // Leer fila activa
    int fila = -1;
    for (int i = 0; i < 4; i++) {
      if (digitalRead(filas[i]) == LOW) fila = i;
    }

    if (fila >= 0 && col >= 0) {
      teclaPresionada = fila * 4 + col; // Mapeo 0–15
      mostrarEnDisplay(teclaPresionada);
      teclaDetectada = false;
    }
  }
}

// Interrupción de cambio de pin (para las columnas)
ISR(PCINT2_vect) {
  teclaDetectada = true;
}
