#include <LiquidCrystal.h>

// ******* TECLADO *******
// ***********************
const uint8_t filas[4] = {A0, A1, A2, A3};
const uint8_t columnas[4] = {A8, A9, A10, A11};

volatile int teclaPresionada = -1;
volatile bool teclaDetectada = false;

const char tablateclado[16] = {
  '7', '8', '9', '/',
  '4', '5', '6', '*',
  '1', '2', '3', '-',
  'C', '0', '=', '+'
};

// ********* LCD *********
// ***********************

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


// ================================ SETUP ================================ //

void setup() {
  // ******* TECLADO *******
  // ***********************
  for (int i = 0; i < 4; i++) {
    pinMode(filas[i], OUTPUT);
    digitalWrite(filas[i], HIGH);
  }
  for (int i = 0; i < 4; i++) {
    pinMode(columnas[i], INPUT_PULLUP);
  }

  PCICR |= 0x04;    // 0000 0100 Grupo 2: PCINT23:16
  PCMSK2 |= 0x0F;   // 0000 1111 Habilitar PCINT19:16
  sei();

  // ********* LCD *********
  // ***********************
  // Inicializa LCD 16 columnas, 2 filas
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);   // columna 0, fila 0
}

// ================================== LOOP ================================== //

void loop() {
  // ******* TECLADO *******
  // ***********************
  static int filaActiva = 0;  // Barrido de filas (rotar 0)
  for (int i = 0; i < 4; i++) {
    digitalWrite(filas[i], HIGH);
  }
  digitalWrite(filas[filaActiva], LOW);
  filaActiva = (filaActiva + 1) % 4;    // solo hay 4 filas

  // leer filas y columnas
  if (teclaDetectada) {
    int col = -1;
    for (int i = 0; i < 4; i++) {
      if (digitalRead(columnas[i]) == LOW) col = i;
    }
    int fila = -1;
    for (int i = 0; i < 4; i++) {
      if (digitalRead(filas[i]) == LOW) fila = i;
    }
    // mostrar tecla leida en el display
    if (fila >= 0 && col >= 0) {
      teclaPresionada = fila * 4 + col; // numero del 0 al 15

      //**** accion a tomar con la tecla ****
      char tecla = tablateclado[teclaPresionada];
      procesarTecla(tecla);
      //*************************************
      
      teclaDetectada = false;
      while (digitalRead(columnas[col]) == LOW);
      delay(150);
    }
  }
}

// =============================== FUNCIONES ================================ //

// ******* TECLADO *******
// ***********************
void procesarTecla(char tecla) {
  if (tecla == 'C') {
    lcd.clear();
    return;
  }
  lcd.print(tecla);
}

// ============================= INTERRUPCIONES ============================= //
// Interrupción del teclado para deteccion
ISR(PCINT2_vect) { 
  teclaDetectada = true;
}
