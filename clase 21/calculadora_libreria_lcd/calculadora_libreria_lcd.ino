#include <LiquidCrystal.h>
#include <string.h>
#include <stdlib.h>

// ================== Configuración de pines TECLADO ==================
const uint8_t filas[4] = {53, 52, 51, 50};
const uint8_t columnas[4] = {A8, A9, A10, A11};

volatile int teclaPresionada = -1;
volatile bool teclaDetectada = false;

// Mapeo de teclas ("KEYPAD-SMALLCALC")
const char tablateclado[16] = {
  '1', '2', '3', '/',
  '4', '5', '6', '*',
  '7', '8', '9', '-',
  'C', '0', '=', '+'
};

// ================== Configuración de pines LCD ==================
#define RS 12
#define E 11
#define D4 5
#define D5 4
#define D6 3
#define D7 2

LiquidCrystal lcd(RS, E, D4, D5, D6, D7);

// ================== Variables de cálculo ==================
char entrada[17]; // Cadena de entrada
int largo = 0;
bool mostrarResultado = false;
float resultado = 0;
bool hayResultado = false; // Indica si hay un resultado previo

// ================== SETUP ==================
void setup() {
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
  lcd.begin(16, 2);
  lcd.clear();
  limpiarEntrada();
}

// ================== LOOP ==================
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
      procesarTecla(tecla);
      teclaDetectada = false;
      while (digitalRead(columnas[col]) == LOW);
      delay(150);
    }
  }
}

// ================== FUNCIONES TECLADO ==================
void detectarTecla() { teclaDetectada = true; }

// ================== FUNCIONES DE LA CALCULADORA ==================
void limpiarEntrada() {
  memset(entrada, 0, sizeof(entrada));
  largo = 0;
  lcd.clear();
}

void procesarTecla(char tecla) {
  if (tecla == 'C') {
    limpiarEntrada();
    hayResultado = false;
    return;
  }
  if (hayResultado && (tecla == '+' || tecla == '-' || tecla == '*' || tecla == '/')) {
    limpiarEntrada();
    char resultadoStr[18];
    dtostrf(resultado, 0, 4, resultadoStr);
    strcpy(entrada, resultadoStr);
    largo = strlen(entrada);
    int offset = 0;
    while (entrada[offset] == ' ') offset++;
    if (offset > 0) {
      for (int i = 0; i <= largo - offset; i++) entrada[i] = entrada[i + offset];
      largo -= offset;
    }
    if (largo < 16) {
      entrada[largo++] = tecla;
      entrada[largo] = '\0';
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(entrada);
    }
    hayResultado = false;
    return;
  }
  if (!mostrarResultado) {
    if (tecla == '=') {
      entrada[largo] = 0;
      mostrarResultado = true;
      resultado = calcular(entrada);
      lcd.clear();
      lcd.setCursor(0, 0);
      if (resultado != -999999) {
        char resultadoStr[18];
        dtostrf(resultado, 0, 4, resultadoStr);
        mostrarSinEspacios(resultadoStr);
        hayResultado = true;
      } else {
        lcd.print("Error!");
        hayResultado = false;
      }
      // Quita la llamada a limpiarEntrada();
      mostrarResultado = false;
    } else {
      if (largo < 16) {
        entrada[largo++] = tecla;
        lcd.setCursor(0, 0);
        lcd.print(entrada);
      }
    }
  }
}

// Elimina espacios iniciales antes de mostrar en el LCD
void mostrarSinEspacios(char* texto) {
  while (*texto == ' ') texto++;
  lcd.print(texto);
}

float calcular(char* expr) {
  char numA[17] = {0}, numB[17] = {0};
  char operador = 0;
  int i = 0, j = 0;

  // Permitir signo en el primer número
  if(expr[i] == '-') {
    numA[j++] = expr[i++];
  }
  while (expr[i] && isDigit(expr[i])) {
    numA[j++] = expr[i++];
  }
  numA[j] = 0;
  operador = expr[i++];
  if (operador != '+' && operador != '-' && operador != '*' && operador != '/')
    return -999999;
  j = 0;
  while (expr[i] && isDigit(expr[i])) {
    numB[j++] = expr[i++];
  }
  numB[j] = 0;
  if (numA[0] == 0 || numB[0] == 0) return -999999;
  long a = atol(numA);
  long b = atol(numB);

  switch (operador) {
    case '+': return (float)(a + b);
    case '-': return (float)(a - b);
    case '*': return (float)(a * b);
    case '/': if (b == 0) return -999999; return ((float)a) / ((float)b);
    default: return -999999;
  }
}

bool isDigit(char c) {
  return (c >= '0' && c <= '9') || c == '.';
}

ISR(PCINT2_vect) { detectarTecla(); }
