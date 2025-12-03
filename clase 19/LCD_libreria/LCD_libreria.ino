#include <LiquidCrystal.h>

#define RS 13
#define E 12
#define D0 11
#define D1 10
#define D2 9
#define D3 8
#define D4 7
#define D5 6
#define D6 5
#define D7 4

LiquidCrystal lcd(RS, E, D0, D1, D2, D3, D4, D5, D6, D7);

void setup() {
  // Inicializa LCD 16 columnas, 2 filas
  lcd.begin(16, 2);

  // Limpiar pantalla y escribir mensaje
  lcd.clear();
  lcd.setCursor(0, 0);   // columna 0, fila 0
  lcd.print("HOLA");
}

void loop() {
  //
}
