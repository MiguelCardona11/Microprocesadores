#include <Wire.h>
#include <string.h>
#include <stdlib.h>

// Pines Keypad
const uint8_t filas[4] = {53, 52, 51, 50};
const uint8_t columnas[4] = {A8, A9, A10, A11};
const char tablateclado[16] = {
  '7', '8', '9', '/',
  '4', '5', '6', '*',
  '1', '2', '3', '-',
  'C', '0', '=', '+'
};

volatile int teclaPresionada = -1;
volatile bool teclaDetectada = false;

void setup() {
  Wire.begin(); // Maestro I2C

  for (int i = 0; i < 4; i++) {
    pinMode(filas[i], OUTPUT);
    digitalWrite(filas[i], HIGH);
  }
  PCICR |= (1 << PCIE2);
  PCMSK2 |= 0x0F;
  sei();
  for (int i = 0; i < 4; i++) {
    pinMode(columnas[i], INPUT_PULLUP);
  }
  delay(20);
}

void loop() {
  static int filaActiva = 0;
  for (int i = 0; i < 4; i++) digitalWrite(filas[i], HIGH);
  digitalWrite(filas[filaActiva], LOW);
  filaActiva = (filaActiva + 1) % 4;

  if (teclaDetectada) {
    int col = -1;
    for (int i = 0; i < 4; i++) {
      if (digitalRead(columnas[i]) == LOW) col = i;
    }
    int fila = -1;
    for (int i = 0; i < 4; i++) {
      if (digitalRead(filas[i]) == LOW) fila = i;
    }
    if (fila >= 0 && col >= 0) {
      teclaPresionada = fila * 4 + col;
      char tecla = tablateclado[teclaPresionada];
      enviarTeclaI2C(tecla);
      teclaDetectada = false;
      while (digitalRead(columnas[col]) == LOW);
      delay(150);
    }
  }
}

void detectarTecla() { teclaDetectada = true; }

void enviarTeclaI2C(char tecla) {
  Wire.beginTransmission(0x08); // Dirección I2C del esclavo
  Wire.write(tecla);
  Wire.endTransmission();
}

ISR(PCINT2_vect) { detectarTecla(); }
