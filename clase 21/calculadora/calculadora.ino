/*
  CALCULADORA
*/

#include <string.h>
#include <stdlib.h>

// ================== Configuración de pines TECLADO ==================
const uint8_t filas[4] = {53, 52, 51, 50};
const uint8_t columnas[4] = {A8, A9, A10, A11};

volatile int teclaPresionada = -1;
volatile bool teclaDetectada = false;

// Mapeo de teclas ("KEYPAD-SMALLCALC")
const char tablateclado[16] = {
  '7', '8', '9', '/',
  '4', '5', '6', '*',
  '1', '2', '3', '-',
  'C', '0', '=', '+'
};

// ================== Configuración de pines LCD ==================
#define RS 12
#define E 11
#define D0 9
#define D1 8
#define D2 7
#define D3 6
#define D4 5
#define D5 4
#define D6 3
#define D7 2

int pines[] = {RS, E, D0, D1, D2, D3, D4, D5, D6, D7};

// ================== Variables de cálculo ==================
char entrada[17]; // Cadena de entrada
int largo = 0;
bool mostrarResultado = false;
float resultado = 0;

// ================== SETUP ==================
void setup() {
  // Configurar filas como salida
  for (int i = 0; i < 4; i++) {
    pinMode(filas[i], OUTPUT);
    digitalWrite(filas[i], HIGH);
  }
  // Configurar interrupciones
  PCICR |= (1 << PCIE2);
  PCMSK2 |= 0x0F;
  sei();
  // Configurar columnas como entrada
  for (int i = 0; i < 4; i++) {
    pinMode(columnas[i], INPUT_PULLUP);
  }
  // Configurar pines LCD como salida
  for (int i = 0; i < 10; i++) {
    pinMode(pines[i], OUTPUT);
  }
  delay(20);
  lcdComando(0x38); delayMicroseconds(50);
  lcdComando(0x06); delayMicroseconds(50);
  lcdComando(0x0F); delayMicroseconds(50);
  lcdComando(0x14); delayMicroseconds(50);
  lcdComando(0x01); delay(2);

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

      // Debounce: esperar a que se suelte la tecla
      while (digitalRead(columnas[col]) == LOW);
      delay(150);
    }
  }
}

// ================== FUNCIONES TECLADO ==================
void detectarTecla() { teclaDetectada = true; }

// ================== FUNCIONES LCD ==================
void lcdComando(byte cmd) {
  digitalWrite(RS, LOW);
  enviarByte(cmd);
}
void lcdDato(byte data) {
  digitalWrite(RS, HIGH);
  enviarByte(data);
}
void lcdTexto(const char *texto) {
  while (*texto) {
    lcdDato(*texto++);
    delayMicroseconds(50);
  }
}
void enviarByte(byte valor) {
  digitalWrite(D0, (valor >> 0) & 0x01);
  digitalWrite(D1, (valor >> 1) & 0x01);
  digitalWrite(D2, (valor >> 2) & 0x01);
  digitalWrite(D3, (valor >> 3) & 0x01);
  digitalWrite(D4, (valor >> 4) & 0x01);
  digitalWrite(D5, (valor >> 5) & 0x01);
  digitalWrite(D6, (valor >> 6) & 0x01);
  digitalWrite(D7, (valor >> 7) & 0x01);
  digitalWrite(E, HIGH);
  delayMicroseconds(1);
  digitalWrite(E, LOW);
  delayMicroseconds(50);
}

// Vincular interrupción real con función
ISR(PCINT2_vect) { detectarTecla(); }

// ================== FUNCIONES DE LA CALCULADORA ==================
void limpiarEntrada() {
  memset(entrada, 0, sizeof(entrada));
  largo = 0;
}

// Procesa la tecla presionada
void procesarTecla(char tecla) {
  if (tecla == 'C') {
    lcdComando(0x01);
    limpiarEntrada();
    return;
  }
  if (!mostrarResultado) {
    if (tecla == '=') {
      entrada[largo] = 0;
      mostrarResultado = true;
      resultado = calcular(entrada);
      lcdComando(0x01);
      if (resultado != -999999) {
        char resultadoStr[18];
        floatToString(resultado, resultadoStr, 4);
        lcdComando(0x01);    // Limpiar pantalla
        delay(2);            // Espera para evitar perder el primer dígito
        lcdComando(0x80);    // Opcional, asegura el cursor al inicio
        mostrarSinEspacios(resultadoStr); // Imprime el resultado
      } else {
        lcdComando(0x01);
        delay(2);
        lcdComando(0x80);
        lcdTexto("Error!");
      }
      limpiarEntrada();
      delay(3000);
      lcdComando(0x01);
      mostrarResultado = false;
    } else {
      if (largo < 16) {
        entrada[largo++] = tecla;
        lcdDato(tecla);
      }
    }
  }
}

// Elimina espacios iniciales antes de mostrar en el LCD
void mostrarSinEspacios(char* texto) {
  while (*texto == ' ') texto++;
  lcdTexto(texto);
}

// Conversión flotante a string (sin librerías)
void floatToString(float num, char *str, int decimals) {
  char temp[18];
  long int_part = (long)num;
  float decimal = fabs(num - int_part);

  if (num < 0) {
    *str++ = '-';
    int_part = -int_part;
  }
  ltoa(int_part, temp, 10); // parte entera
  strcpy(str, temp);
  str += strlen(temp);

  if (decimals > 0) {
    *str++ = '.';
    for (int i = 0; i < decimals; i++) {
      decimal *= 10;
      int d = (int)decimal;
      *str++ = '0' + d;
      decimal -= d;
    }
  }
  *str = 0;
}

// Calcula el resultado de la cadena de entrada
float calcular(char* expr) {
  // Permite solo [0-9]+op[0-9]+
  char numA[9] = {0}, numB[9] = {0};
  char operador = 0;
  int i = 0, j = 0;

  // Primer número
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
  return (c >= '0' && c <= '9');
}
