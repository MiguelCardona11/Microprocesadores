#include <LiquidCrystal.h>

// ******* TECLADO *******
// ***********************
const uint8_t filas[4] = {A0, A1, A2, A3};
const uint8_t columnas[4] = {A8, A9, A10, A11};

volatile int teclaPresionada = -1;
volatile bool teclaDetectada = false;

bool actualizarPantalla = true;

volatile int cantidadPollos = 0;
volatile int cantidadVacas = 0;
bool modoVacas = false; // false: modo pollos, true: modo vacas


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

#define LED_MODE_PIN 14

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
  lcd.begin(16, 2); // Inicializa LCD 16 columnas, 2 filas
  lcd.clear(); // Limpiar pantalla

  // **** LED para ver modo de selección ****
  pinMode(LED_MODE_PIN, OUTPUT);
  digitalWrite(LED_MODE_PIN, LOW); // Empieza con pollos

}

// ================================== LOOP ================================== //

void loop() {

  static char lineaUno[21];
  static char lineaDos[21];
  static int vAntPollos = -1;
  static int vAntVacas = -1;

  static bool antModoVacas = false;

  // Actualiza LCD si cambió algún dato o cambiamos de selección
  if (actualizarPantalla || vAntPollos != cantidadPollos || vAntVacas != cantidadVacas || antModoVacas != modoVacas) {
    lcd.clear();
    delay(2);

    lcd.setCursor(0, 0);   // columna 0, fila 0
    sprintf(lineaUno, "Cant pollos:%2d", cantidadPollos);
    lcd.print(lineaUno);

    lcd.setCursor(0, 1);   // columna 0, fila 1
    sprintf(lineaDos, "Cant vacas :%2d", cantidadVacas);
    lcd.print(lineaDos);

    // LED: LOW si editando pollos, HIGH si editando vacas
    digitalWrite(LED_MODE_PIN, modoVacas ? HIGH : LOW);

    vAntPollos = cantidadPollos;
    vAntVacas = cantidadVacas;
    antModoVacas = modoVacas;
    actualizarPantalla = false;
  }

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
    if (!modoVacas) cantidadPollos = 0;
    else cantidadVacas = 0;
    actualizarPantalla = true;
    return;
  }
  // Cambiar selección por tecla "/"
  if (tecla == '/') {
    modoVacas = !modoVacas;
    actualizarPantalla = true;
    return;
  }
  // Solo acepta números del '0' al '9'
  // se resta '0' para pasar el valor de ASCII a INT
  if (tecla >= '0' && tecla <= '9') {
    if (!modoVacas) cantidadPollos = tecla - '0';
    else cantidadVacas = tecla - '0';
    actualizarPantalla = true;
  }
}

// ============================= INTERRUPCIONES ============================= //
// Interrupción del teclado para deteccion
ISR(PCINT2_vect) { 
  teclaDetectada = true;
}
