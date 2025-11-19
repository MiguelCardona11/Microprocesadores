#include <LiquidCrystal.h>

// ******* TECLADO *******
// ***********************
const uint8_t filas[4] = {A0, A1, A2, A3};
const uint8_t columnas[4] = {A8, A9, A10, A11};

volatile int teclaPresionada = -1;
volatile bool teclaDetectada = false;

volatile char* msgLinea1;
volatile char* msgLinea2;

// ******* LCD *******
// ***********************
bool actualizarPantalla = true;

#define LCD_ANCHO 16 // Cambia a 20 si tu LCD es de 20 columnas

int modo = -1; // 0: pedido, 1: recarga, 2: pagar

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
  lcd.begin(LCD_ANCHO, 2); // Inicializa LCD
  lcd.clear();
}

// ================================== LOOP ================================== //

void loop() {

  // Actualiza LCD si cambió algún dato o cambiamos de selección
  if (actualizarPantalla) {
    lcd.clear();  // Limpia pantalla
    lcd.setCursor(0, 0);
    lcd.print("Elegir modo: ");
    // Scroll reutilizable en la línea 2, intervalo de 300ms
    actualizarPantalla = false;
  }

  mensajeScroll("0.Pedido-1.Recargar-2.Pagar  ", 1, 300);

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
    actualizarPantalla = true;
    return;
  }

  if (modo == -1) { // si no han elegido el modo
    modo = tecla - '0';
    switch (modo) {
      case 0: // modo pedido
        lcd.setCursor(0, 1);
        lcd.clear();
        lcd.print("PEDIDO");
        modo = -1;
        actualizarPantalla = true;
        break;
      case 1: // modo recargar
        lcd.setCursor(0, 1);
        lcd.clear();
        lcd.print("RECARGAR");
        modo = -1;
        actualizarPantalla = true;
        break;
      case 2: // modo pagar
        lcd.setCursor(0, 1);
        lcd.clear();
        lcd.print("PAGAR");
        modo = -1;
        actualizarPantalla = true;
        break;
      default:
        modo = -1; 
    }
  }


  
  // Solo acepta números del '0' al '9'
  // se resta '0' para pasar el valor de ASCII a INT
}


// ********* SCROLL REUTILIZABLE *********
// ****************************************
void mensajeScroll(const char* mensaje, uint8_t fila, unsigned long intervalo) {
  static int scrollIndex = 0;              // ÍNDICE DEL SCROLL
  static unsigned long tiempoScroll = 0;   // Para el temporizador

  // Crea mensaje temporal con 4 espacios al inicio

  int lenScroll = strlen(mensaje);

  if (millis() - tiempoScroll >= intervalo) {
    tiempoScroll = millis();

    char buffer[LCD_ANCHO + 1];
    for (int i = 0; i < LCD_ANCHO; i++) {
      int idx = (scrollIndex + i) % lenScroll;
      buffer[i] = mensaje[idx];
    }
    buffer[LCD_ANCHO] = '\0';

    lcd.setCursor(0, fila); // Línea donde mostrar el scroll (0 o 1)
    lcd.print(buffer);

    scrollIndex = (scrollIndex + 1) % lenScroll;
  }
}

// ============================= INTERRUPCIONES ============================= //
// Interrupción del teclado para deteccion
ISR(PCINT2_vect) { 
  teclaDetectada = true;
}
