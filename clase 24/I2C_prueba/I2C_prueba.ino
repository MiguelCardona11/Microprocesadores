#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <Wire.h>

// ----- CONFIG TECLADO -----
const byte ROW_PINS[4] = { 46, 47, 48, 49 };
const byte COL_PINS[4] = { 50, 51, 52, 53 };

const char KEYS[4][4] = {
  { '7', '8', '9', '/' },
  { '4', '5', '6', '*' },
  { '1', '2', '3', '-' },
  { 'C', '0', '=', '+' }
};

volatile bool keyDetected = false;
volatile byte currentRow = 0;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 200;

// Dirección del esclavo I2C
#define SLAVE_ADDR 8

// ----- MANEJO DE TECLAS -----
void handleKeyPress(char key) {

  // 1) Enviar siempre la tecla al esclavo
  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write(key);
  Wire.endTransmission();

  // 2) Si es '=', solicitamos el resultado para reenviarlo o verificarlo
  if (key == '=') {

    delay(50);

    // Se solicita hasta 10 caracteres del esclavo
    Wire.requestFrom(SLAVE_ADDR, 10);

    String result = "";
    while (Wire.available()) {
      result += (char)Wire.read();
    }

    // Mostrar por Serial para comprobar que funciona
    Serial.print("Resultado recibido: ");
    Serial.println(result);
  }
}

// ----- TECLADO -----
void keypadInit() {
  for (byte i = 0; i < 4; i++) {
    pinMode(ROW_PINS[i], OUTPUT);
    digitalWrite(ROW_PINS[i], HIGH);
  }
  for (byte i = 0; i < 4; i++) {
    pinMode(COL_PINS[i], INPUT_PULLUP);
  }

  PCICR |= (1 << PCIE0);
  PCMSK0 |= (1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2) | (1 << PCINT3);
}

ISR(PCINT0_vect) {
  keyDetected = true;
}

char keypadGetKey() {
  unsigned long currentTime = millis();
  if ((currentTime - lastDebounceTime) < DEBOUNCE_DELAY) return '\0';

  digitalWrite(ROW_PINS[currentRow], HIGH);
  currentRow = (currentRow + 1) % 4;
  digitalWrite(ROW_PINS[currentRow], LOW);

  if (keyDetected) {
    for (byte c = 0; c < 4; c++) {
      if (digitalRead(COL_PINS[c]) == LOW) {
        keyDetected = false;
        lastDebounceTime = currentTime;
        return KEYS[currentRow][c];
      }
    }
    keyDetected = false;
  }
  delayMicroseconds(500);
  return '\0';
}

// ---- MAIN ----
void setup() {
  Serial.begin(9600);
  keypadInit();
  sei();

  Wire.begin(); // Maestro I2C

  Serial.println("MAESTRO listo...");
}

void loop() {
  char key = keypadGetKey();
  if (key != '\0') {
    Serial.print("Tecla presionada: ");
    Serial.println(key);
    handleKeyPress(key);
  }
}
