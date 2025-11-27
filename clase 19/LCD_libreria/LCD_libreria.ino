#include <LiquidCrystal.h>

// Asocia los pines de acuerdo a tu conexión: LiquidCrystal(rs, en, d4, d5, d6, d7);
// Modo 4 bits (usa solo D4–D7)
#define RS 12
#define E  11
#define D4 5
#define D5 4
#define D6 3
#define D7 2

LiquidCrystal lcd(RS, E, D4, D5, D6, D7);

void setup() {
  // Inicializa LCD 16 columnas, 2 filas
  lcd.begin(16, 2);

  // Limpiar pantalla y escribir mensaje
  lcd.clear();
  lcd.setCursor(0, 0);   // columna 0, fila 0
  lcd.print("HOLA");
}

void loop() {
  // Pantalla estática, sin código adicional
}
