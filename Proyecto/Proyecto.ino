#include <LiquidCrystal.h>

// ******* TECLADO *******
const uint8_t FILAS[4] = {A0, A1, A2, A3};
const uint8_t COLUMNAS[4] = {A8, A9, A10, A11};

volatile int teclaPresionada = -1;
volatile bool teclaDetectada = false;

// ******* LCD *******
bool actualizarPantalla = true;
#define LCD_ANCHO 16

int modo = -1;
int submodo = -1;
String mensajeScrollActual = "";
int opcionPedidoSeleccionada = -1;
int itemSeleccionado = -1;
String cantidadBuffer = "";
int mesaSeleccionada = -1;
bool esperandoConfirmacion = false;

// MESAS[15][4][3] -> 15 MESAS, 4 CATEGORIAS, 3 PRODUCTOS POR CATEGORIA
int mesas[15][4][3];

// NOMBRES DE PRODUCTOS MAYUSCULAS
const char* nombresEntradas[3] = {"EMPANADAS", "AREPAS", "PATACONES"};
const char* nombresPlatos[3] = {"BANDEJA PAISA", "AJIACO", "SANCOCHO"};
const char* nombresBebidas[3] = {"LIMONADA", "AGUAPANELA", "GASEOSA"};
const char* nombresPostres[3] = {"AREQUIPE", "BREVAS", "OBLEAS"};

const unsigned long preciosEntradas[3] = {6200, 16000, 10600};
const unsigned long preciosPlatos[3] = {47500, 53400, 58600};
const unsigned long preciosBebidas[3] = {13800, 7000, 5000};
const unsigned long preciosPostres[3] = {19500, 15000, 13000};

const char** nombresCategorias[4] = {nombresEntradas, nombresPlatos, nombresBebidas, nombresPostres};
const unsigned long* preciosCategorias[4] = {preciosEntradas, preciosPlatos, preciosBebidas, preciosPostres};

const char tablateclado[16] = {
  '7', '8', '9', '/',
  '4', '5', '6', '*',
  '1', '2', '3', '-',
  'C', '0', '=', '+'
};

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
  for (int i = 0; i < 4; i++) {
    pinMode(FILAS[i], OUTPUT);
    digitalWrite(FILAS[i], HIGH);
  }
  for (int i = 0; i < 4; i++) {
    pinMode(COLUMNAS[i], INPUT_PULLUP);
  }
  PCICR |= 0x04;
  PCMSK2 |= 0x0F;
  sei();
  lcd.begin(LCD_ANCHO, 2);
  lcd.clear();

  for (int mesa = 0; mesa < 15; mesa++)
    for (int cat = 0; cat < 4; cat++)
      for (int prod = 0; prod < 3; prod++)
        mesas[mesa][cat][prod] = 0;
}

void loop() {
  if (actualizarPantalla) {
    lcd.clear();
    if (modo == -1) {
      String msj = "ELIGE MODO:";
      msj.toUpperCase();
      lcd.print(msj);
      mensajeScrollActual = "0.PEDIDO-1.RECARGAR-2.PAGAR-3.CONSULTAR";
    } else if ((modo == 0 || modo == 2 || modo == 3) && mesaSeleccionada == -1) {
      String msjMesa = "MESA (0-14):";
      msjMesa.toUpperCase();
      lcd.print(msjMesa);
      mensajeScrollActual = "";
    } else {
      mensajeScrollActual = "";
    }
    actualizarPantalla = false;
  }
  if (mensajeScrollActual != "") {
    String temp = mensajeScrollActual;
    temp += "   ";
    temp.toUpperCase();
    mensajeScroll(temp.c_str(), 1, 100);
  }
  static int filaActiva = 0;
  for (int i = 0; i < 4; i++) digitalWrite(FILAS[i], HIGH);
  digitalWrite(FILAS[filaActiva], LOW);
  filaActiva = (filaActiva + 1) % 4;

  if (teclaDetectada) {
    int col = -1;
    for (int i = 0; i < 4; i++) if (digitalRead(COLUMNAS[i]) == LOW) col = i;
    int fila = -1;
    for (int i = 0; i < 4; i++) if (digitalRead(FILAS[i]) == LOW) fila = i;
    if (fila >= 0 && col >= 0) {
      teclaPresionada = fila * 4 + col;
      char tecla = tablateclado[teclaPresionada];
      procesarTecla(tecla);
      teclaDetectada = false;
      while (digitalRead(COLUMNAS[col]) == LOW);
      delay(150);
    }
  }
}

void procesarTecla(char tecla) {
  if (tecla == 'C') {
    lcd.clear();
    modo = -1; submodo = -1;
    mensajeScrollActual = "";
    actualizarPantalla = true;
    itemSeleccionado = -1;
    cantidadBuffer = "";
    mesaSeleccionada = -1;
    return;
  }
  if (modo == -1) {
    if (tecla >= '0' && tecla <= '3') {
      modo = tecla - '0';
      mesaSeleccionada = -1;
      cantidadBuffer = "";
      mensajeScrollActual = "";
      if (modo == 0) { submodo = 0; lcd.clear(); lcd.setCursor(0,0); lcd.print("MESA (0-14):"); actualizarPantalla = false;}
      else if (modo == 1) { submodo = 1; lcd.clear(); lcd.setCursor(0,0); lcd.print("RECARGAR"); lcd.setCursor(0,1); lcd.print("INGRESE MONTO:"); actualizarPantalla = true;}
      else if (modo == 2) { submodo = 2; lcd.clear(); lcd.setCursor(0,0); lcd.print("MESA (0-14):"); actualizarPantalla = false; }
      else if (modo == 3) { submodo = 3; lcd.clear(); lcd.setCursor(0,0); lcd.print("MESA (0-14):"); actualizarPantalla = false; }
    }
  } else if ((modo == 0 || modo == 2 || modo == 3) && mesaSeleccionada == -1) {
    if (tecla >= '0' && tecla <= '9') {
      if (cantidadBuffer.length() < 2) { cantidadBuffer += tecla; lcd.setCursor(12,0); lcd.print(cantidadBuffer); }
    } else if (tecla == '=') {
      int val = cantidadBuffer.toInt();
      if (val >=0 && val < 15) {
        mesaSeleccionada = val;
        cantidadBuffer = "";
        if (modo == 0) {
          submodo = 0; lcd.clear(); lcd.setCursor(0,0); lcd.print("MESA "); lcd.print(mesaSeleccionada); lcd.setCursor(0,1); lcd.print("OPCIONES"); delay(100);
          mensajeScrollActual = "0.ENTRADAS-1.PLATOS FUERTES-2.BEBIDAS-3.POSTRES";
        } else if (modo == 2) {
          submodo = 21;
          mostrarResumenMesa(mesaSeleccionada, false);
        } else if (modo == 3) {
          submodo = 31;
          mostrarResumenMesa(mesaSeleccionada, true);
        }
      } else {
        lcd.clear(); lcd.setCursor(0,0); lcd.print("MESA (0-14):"); lcd.setCursor(0,1); lcd.print("NO VALIDA");
        delay(1000); lcd.clear(); lcd.setCursor(0,0); lcd.print("MESA (0-14):");
        cantidadBuffer = "";
      }
    }
  } else {
    procesarSubmodo(tecla);
  }
}

void procesarSubmodo(char tecla) {
  switch (submodo) {
    case 0: manejarOpcionesPedido(tecla); break;
    case 10: manejarSeleccionCategoria(tecla, 0, nombresEntradas); break;
    case 11: manejarSeleccionCategoria(tecla, 1, nombresPlatos); break;
    case 13: manejarSeleccionCategoria(tecla, 2, nombresBebidas); break;
    case 14: manejarSeleccionCategoria(tecla, 3, nombresPostres); break;
    case 21:
      if (!esperandoConfirmacion) {
        // Primera vez que presiona '=' - mostrar confirmación
        if (tecla == '=') {
          esperandoConfirmacion = true;
          mensajeScrollActual = ""; // Detener scroll
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("CONFIRMAR");
          lcd.setCursor(0, 1);
          lcd.print("0. NO - 1. SI");
        }
      } else {
        // Ya está esperando confirmación - procesar respuesta
        if (tecla == '1') {
          // Confirma el pago
          limpiarMesa(mesaSeleccionada);
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("PAGO EXITOSO!");
          lcd.setCursor(0,1);
          lcd.print("MESA LIBERADA");
          delay(1500);
          modo = -1; submodo = -1; mesaSeleccionada = -1; 
          mensajeScrollActual = ""; 
          esperandoConfirmacion = false;
          actualizarPantalla = true;
        } else if (tecla == '0') {
          // Cancela el pago
          esperandoConfirmacion = false;
          mostrarResumenMesa(mesaSeleccionada, false);
        }
      }
      break;
    case 31:
      if (tecla == '=') {
        modo = -1; submodo = -1; mesaSeleccionada = -1; mensajeScrollActual = ""; actualizarPantalla = true;
      }
      break;
    case 1:  break;
    case 2:  break;
    case 3:  break;
  }
}

void manejarOpcionesPedido(char tecla) {
  if (tecla >= '0' && tecla <= '3') {
    lcd.clear(); lcd.setCursor(0,0); mensajeScrollActual = "";
    switch (tecla) {
      case '0': lcd.print("ENTRADAS"); mensajeScrollActual = "0.EMPANADAS-1.AREPAS-2.PATACONES"; submodo = 10; break;
      case '1': lcd.print("PLATOS FUERTES"); mensajeScrollActual = "0.BANDEJA PAISA-1.AJIACO-2.SANCOCHO"; submodo = 11; break;
      case '2': lcd.print("BEBIDAS"); mensajeScrollActual = "0.LIMONADA-1.AGUAPANELA-2.GASEOSA"; submodo = 13; break;
      case '3': lcd.print("POSTRES"); mensajeScrollActual = "0.AREQUIPE-1.BREVAS-2.OBLEAS"; submodo = 14; break;
    }
    itemSeleccionado = -1;
    cantidadBuffer = "";
  }
}

void manejarSeleccionCategoria(char tecla, int categoria, const char * nombres[3]) {
  if (itemSeleccionado == -1) {
    if (tecla >= '0' && tecla <= '2') {
      itemSeleccionado = tecla - '0';
      String nombre = nombres[itemSeleccionado];
      nombre.toUpperCase();
      lcd.clear(); lcd.setCursor(0,0);
      lcd.print(nombre);
      lcd.setCursor(0,1); lcd.print("CANTIDAD:"); cantidadBuffer = ""; mensajeScrollActual = "";
    }
  } else {
    if (tecla >= '0' && tecla <= '9') {
      cantidadBuffer += tecla;
      lcd.setCursor(9,1); lcd.print(cantidadBuffer);
    } else if (tecla == '=') {
      int cantidad = cantidadBuffer.toInt();
      if (cantidad > 0 &&
          mesaSeleccionada >= 0 && mesaSeleccionada < 15 &&
          itemSeleccionado >=0 && itemSeleccionado<3) {
        mesas[mesaSeleccionada][categoria][itemSeleccionado] += cantidad;
        String nombre = nombres[itemSeleccionado];
        nombre.toUpperCase();
        lcd.clear(); lcd.setCursor(0,0); lcd.print("AGREGADO:");
        lcd.setCursor(0,1);
        lcd.print(String(mesas[mesaSeleccionada][categoria][itemSeleccionado]) + " " + nombre);
        delay(1500);
      } else {
        lcd.clear(); lcd.setCursor(0,0); lcd.print("ERROR CANTIDAD");
        delay(1000);
      }
      itemSeleccionado = -1; cantidadBuffer = ""; submodo = 0; opcionPedidoSeleccionada = -1;
      mensajeScrollActual = "0.ENTRADAS-1.PLATOS FUERTES-2.BEBIDAS-3.POSTRES"; lcd.clear(); lcd.setCursor(0,0); lcd.print("OPCIONES");
    }
  }
}

void mostrarResumenMesa(int mesa, bool soloConsulta) {
  lcd.clear();
  String resumen = "<";
  unsigned long total = 0;
  for (int cat = 0; cat < 4; cat++) {
    for (int prod = 0; prod < 3; prod++) {
      int cant = mesas[mesa][cat][prod];
      if (cant > 0) {
        if (resumen != "<") resumen += " - ";
        String nombreUpper = String(nombresCategorias[cat][prod]);
        nombreUpper.toUpperCase();
        resumen += nombreUpper + ":" + String(cant);
        total += ((unsigned long)cant) * preciosCategorias[cat][prod];
      }
    }
  }
  resumen +=">";
  if(resumen == "<>") resumen = "<--SIN PRODUCTOS-->";
  mensajeScrollActual = resumen;
  String totalStr = "TOTAL: " + String(total);
  totalStr.toUpperCase();
  lcd.setCursor(0,0); lcd.print(totalStr);
}

void limpiarMesa(int mesa) {
  for (int cat = 0; cat < 4; cat++)
    for (int prod = 0; prod < 3; prod++)
      mesas[mesa][cat][prod] = 0;
}

void mensajeScroll(const char* mensaje, uint8_t fila, unsigned long intervalo) {
  static int scrollIndex = 0;
  static unsigned long tiempoScroll = 0;
  static String mensajeAnterior = "";
  int lenScroll = strlen(mensaje);
  String tempMsg = String(mensaje);
  tempMsg.toUpperCase();
  const char* upMsg = tempMsg.c_str();
  if (mensajeAnterior != tempMsg) {
    scrollIndex = 0; tiempoScroll = millis(); mensajeAnterior = tempMsg;
  }
  if (millis() - tiempoScroll >= intervalo) {
    tiempoScroll = millis();
    char buffer[LCD_ANCHO + 1];
    for (int i = 0; i < LCD_ANCHO; i++) {
      int idx = (scrollIndex + i) % lenScroll;
      buffer[i] = upMsg[idx];
    }
    buffer[LCD_ANCHO] = '\0';
    lcd.setCursor(0, fila); lcd.print(buffer);
    scrollIndex = (scrollIndex + 1) % lenScroll;
  }
}

ISR(PCINT2_vect) {
  teclaDetectada = true;
}