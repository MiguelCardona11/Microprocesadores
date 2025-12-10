#include <LiquidCrystal.h>
#include <SPI.h>
#include <MFRC522.h>

// ******** RFID *********
#define RST_PIN 22
#define SS_PIN 53

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
byte block = 4;

// ******** TECLADO *********
const uint8_t FILAS[4] = {A0, A1, A2, A3};
const uint8_t COLUMNAS[4] = {A8, A9, A10, A11};

volatile int teclaPresionada = -1;
volatile bool teclaDetectada = false;

// ******** LCD *********
bool actualizarPantalla = true;
#define LCD_ANCHO 16

int modo = -1;
int submodo = -1;
String mensajeScrollActual = "";
int opcionPedidoSeleccionada = -1;
int itemSeleccionado = -1;
String cantidadBuffer = "";
int mesaSeleccionada = -1;

int mesas[15][4][3];
int comandas[15][4][3];
int ultimoAgregado[15][4][3];

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
  '1', '2', '3', '/',
  '4', '5', '6', '*',
  '7', '8', '9', '-',
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

// BOTONES
#define PIN_BOTON_MODO            14
#define PIN_BOTON_SIGUIENTE_MESA  19
#define PIN_BOTON_LIMPIAR_COMANDA 18
#define PIN_LED_CONFIRMACION      40

LiquidCrystal lcd(RS, E, D0, D1, D2, D3, D4, D5, D6, D7);

// ******** DISPLAYS 7-SEGMENTOS *********
#define SEG_A 30
#define SEG_B 31
#define SEG_C 32
#define SEG_D 33
#define SEG_E 34
#define SEG_F 35
#define SEG_G 36

#define DIG1 37
#define DIG2 38

const uint8_t segment_map[10] = {
  0b00000001, 0b01001111, 0b00010010, 0b00000110, 0b01001100,
  0b00100100, 0b00100000, 0b00001111, 0b00000000, 0b00000100
};

// MODO COCINA GLOBAL
volatile bool modoCocina = true;   // arranca en cocina
volatile bool cambioModo = false;
volatile bool cambioMesaFlag = false;
volatile bool limpiarComandaFlag = false;

// VARIABLES DE COCINA
int modoCocinaSubmodo = -1;
int mesaCocinaActual = 0;
int ultimaMesaLista = -1;

// VARIABLES DE MESERO - NOTIFICACIONES
int mesaListaParaRecoger = -1;
unsigned long tiempoMostrarNotificacion = 0;
const unsigned long DURACION_NOTIFICACION = 3000;
bool mostrandoNotificacion = false;

// VARIABLES RFID
unsigned long totalAPagar = 0;
unsigned long montoRecarga = 0;
bool tarjetaLeyendo = false;

// ========== FUNCIONES DE COMUNICACION SERIAL ==========

// MESERO ENVÍA comandas a COCINA por Serial1
void enviarComanda(int mesa, int categoria, int producto, int cantidad) {
  Serial1.print("$");
  Serial1.print(mesa);
  Serial1.print(",");
  Serial1.print(categoria);
  Serial1.print(",");
  Serial1.print(producto);
  Serial1.print(",");
  Serial1.print(cantidad);
  Serial1.println("#");

  Serial.print(">> [MESERO ENVÍA] Comanda a Cocina: Mesa ");
  Serial.println(mesa);
}

// COCINA ENVÍA notificación a MESERO por Serial2
void enviarMesaListaAMesero(int mesa) {
  Serial2.print("$MESA_LISTA,");
  Serial2.print(mesa);
  Serial2.println("#");

  Serial.print(">> [COCINA ENVÍA] Notificación a Mesero: Mesa ");
  Serial.println(mesa);
}

// COCINA LEE mensajes de MESERO por Serial2
void procesarComandaSerialCocina() {
  static String bufferSerial = "";

  while (Serial2.available()) {
    char c = Serial2.read();

    if (c == '$') {
      bufferSerial = "";
    } else if (c == '#') {
      procesarMensajeEnCocina(bufferSerial);
      bufferSerial = "";
    } else {
      bufferSerial += c;
    }
  }
}

// MESERO LEE notificaciones de COCINA por Serial1
void procesarNotificacionSerialMesero() {
  static String bufferSerial = "";

  while (Serial1.available()) {
    char c = Serial1.read();

    if (c == '$') {
      bufferSerial = "";
    } else if (c == '#') {
      procesarMensajeEnMesero(bufferSerial);
      bufferSerial = "";
    } else {
      bufferSerial += c;
    }
  }
}

// PROCESAR mensaje en COCINA (recibe de Mesero)
void procesarMensajeEnCocina(String mensaje) {
  Serial.print("<< [COCINA RECIBE] ");
  Serial.println(mensaje);

  // Formato: MESA,CATEGORIA,PRODUCTO,CANTIDAD
  int valores[4];
  int indice = 0;
  int ultimaPos = 0;

  for (int i = 0; i <= mensaje.length(); i++) {
    if (mensaje[i] == ',' || i == mensaje.length()) {
      String valor = mensaje.substring(ultimaPos, i);
      valores[indice] = valor.toInt();
      indice++;
      ultimaPos = i + 1;
    }
  }

  if (indice == 4) {
    int mesa = valores[0];
    int categoria = valores[1];
    int producto = valores[2];
    int cantidad = valores[3];

    if (mesa >= 0 && mesa < 15 && categoria >= 0 && categoria < 4 && producto >= 0 && producto < 3 && cantidad > 0) {
      agregarAlComanda(mesa, categoria, producto, cantidad);
      Serial.print(">> [COCINA PROCESA] Comanda agregada: Mesa ");
      Serial.println(mesa);
    }
  }
}

// PROCESAR mensaje en MESERO (recibe de Cocina)
void procesarMensajeEnMesero(String mensaje) {
  Serial.print("<< [MESERO RECIBE] ");
  Serial.println(mensaje);

  if (mensaje.indexOf("MESA_LISTA") != -1) {
    int coma = mensaje.indexOf(",");
    int mesa = mensaje.substring(coma + 1).toInt();

    if (mesa >= 0 && mesa < 15) {
      mesaListaParaRecoger = mesa;
      tiempoMostrarNotificacion = millis();
      mostrandoNotificacion = true;
      Serial.print(">> [MESERO PROCESA] Notificación de mesa lista: ");
      Serial.println(mesa);
    }
  }
}

void setup() {
  Serial.begin(9600);
  delay(500);

  Serial1.begin(9600); // Mesero (TX1, RX1)
  Serial2.begin(9600); // Cocina (TX2, RX2)

  Serial.println("\n======================= SISTEMA INICIADO =======================");
  Serial.println("Configuración: Full-Duplex Serial1/Serial2");
  Serial.println("=============================================================\n");

  // Keypad
  for (int i = 0; i < 4; i++) {
    pinMode(FILAS[i], OUTPUT);
    digitalWrite(FILAS[i], HIGH);
  }
  for (int i = 0; i < 4; i++) {
    pinMode(COLUMNAS[i], INPUT_PULLUP);
  }
  PCICR |= 0x04;   // PCIE2 para A8-A11 (PCINT16..19)
  PCMSK2 |= 0x0F;  // habilita PCINT16..19 (A8..A11)

  // LCD
  lcd.begin(LCD_ANCHO, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("INICIANDO...");

  // Inicializar arrays
  for (int mesa = 0; mesa < 15; mesa++)
    for (int cat = 0; cat < 4; cat++)
      for (int prod = 0; prod < 3; prod++) {
        mesas[mesa][cat][prod] = 0;
        comandas[mesa][cat][prod] = 0;
        ultimoAgregado[mesa][cat][prod] = 0;
      }

  // BOTÓN MODO -> se lee con digitalRead en loop
  pinMode(PIN_BOTON_MODO, INPUT);

  // BOTÓN SIGUIENTE MESA -> INT4 (pin 19)
  pinMode(PIN_BOTON_SIGUIENTE_MESA, INPUT_PULLUP);
  EIFR  |= (1 << INTF4);     // limpia flag INT4
  EICRB |= (1 << ISC41);     // FALLING: ISC41=1, ISC40=0
  EICRB &= ~(1 << ISC40);
  EIMSK |= (1 << INT4);

  // BOTÓN LIMPIAR COMANDA -> INT5 (pin 18)
  pinMode(PIN_BOTON_LIMPIAR_COMANDA, INPUT_PULLUP);
  EIFR  |= (1 << INTF5);     // limpia flag INT5
  EICRB |= (1 << ISC51);     // FALLING: ISC51=1, ISC50=0
  EICRB &= ~(1 << ISC50);
  EIMSK |= (1 << INT5);

  // LED
  pinMode(PIN_LED_CONFIRMACION, OUTPUT);
  digitalWrite(PIN_LED_CONFIRMACION, LOW);

  // Displays
  pinMode(SEG_A, OUTPUT);
  pinMode(SEG_B, OUTPUT);
  pinMode(SEG_C, OUTPUT);
  pinMode(SEG_D, OUTPUT);
  pinMode(SEG_E, OUTPUT);
  pinMode(SEG_F, OUTPUT);
  pinMode(SEG_G, OUTPUT);
  pinMode(DIG1, OUTPUT);
  pinMode(DIG2, OUTPUT);
  apagarDisplays();

  // RFID
  SPI.begin();
  delay(100);
  mfrc522.PCD_Init();
  delay(100);

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SISTEMA LISTO");
  delay(1000);
  lcd.clear();

  sei(); // habilita interrupciones globales
}

void loop() {
  // Leer modo global desde el pin (1 = cocina, 0 = mesero)
  int estadoModo = digitalRead(PIN_BOTON_MODO);
  bool nuevoModoCocina = (estadoModo == HIGH); // HIGH = modo cocina

  if (nuevoModoCocina != modoCocina) {
    modoCocina = nuevoModoCocina;
    cambioModo = true;
  }

  if (modoCocina) {
    procesarComandaSerialCocina();
    mostrarCocina();
  } else {
    procesarNotificacionSerialMesero();
    mostrarMesero();
  }
}

// ========== ISR ==========

// Keypad A8-A11
ISR(PCINT2_vect) {
  teclaDetectada = true;
}

// BOTÓN SIGUIENTE MESA -> INT4 (pin 19)
ISR(INT4_vect) {
  parpadearLED(2, 100);
  cambioMesaFlag = true;
}

// BOTÓN LIMPIAR COMANDA -> INT5 (pin 18)
ISR(INT5_vect) {
  parpadearLED(2, 100);
  if (modoCocina && modoCocinaSubmodo == 10) {
    limpiarComandaFlag = true;
  }
}

// ========== DISPLAYS 7-SEGMENTOS ==========
void mostrarNumeroDisplay(int numero) {
  if (numero < 0 || numero > 14) numero = 0;

  int decenas = numero / 10;
  int unidades = numero % 10;

  escribirSegmentos(segment_map[decenas]);
  digitalWrite(DIG1, LOW);
  digitalWrite(DIG2, HIGH);
  delay(5);

  escribirSegmentos(segment_map[unidades]);
  digitalWrite(DIG1, HIGH);
  digitalWrite(DIG2, LOW);
  delay(5);
}

void escribirSegmentos(uint8_t patron) {
  digitalWrite(SEG_A, (patron >> 6) & 0x01);
  digitalWrite(SEG_B, (patron >> 5) & 0x01);
  digitalWrite(SEG_C, (patron >> 4) & 0x01);
  digitalWrite(SEG_D, (patron >> 3) & 0x01);
  digitalWrite(SEG_E, (patron >> 2) & 0x01);
  digitalWrite(SEG_F, (patron >> 1) & 0x01);
  digitalWrite(SEG_G, (patron >> 0) & 0x01);
}

void apagarDisplays() {
  // Primero apagar todos los segmentos
  escribirSegmentos(0xFF);  // Todos los segmentos OFF
  
  // Luego desactivar ambos dígitos
  digitalWrite(DIG1, HIGH);
  digitalWrite(DIG2, HIGH);
}

void parpadearLED(int veces, unsigned long intervalo) {
  for (int i = 0; i < veces; i++) {
    digitalWrite(PIN_LED_CONFIRMACION, HIGH);
    delay(intervalo);
    digitalWrite(PIN_LED_CONFIRMACION, LOW);
    if (i < veces - 1) {
      delay(intervalo);
    }
  }
}

// ========= FUNCION PARA MESERO ==========
void mostrarMesero() {
  apagarDisplays();

  // Notificación de mesa lista
  if (mostrandoNotificacion) {
    if (millis() - tiempoMostrarNotificacion < DURACION_NOTIFICACION) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("RECOGER PEDIDO");
      lcd.setCursor(0, 1);
      lcd.print("MESA: " + String(mesaListaParaRecoger));
      return;
    } else {
      mostrandoNotificacion = false;
      mesaListaParaRecoger = -1;
      actualizarPantalla = true;
    }
  }

  if (cambioModo) {
    lcd.clear();
    modo = -1;
    submodo = -1;
    mensajeScrollActual = "";
    itemSeleccionado = -1;
    cantidadBuffer = "";
    mesaSeleccionada = -1;
    actualizarPantalla = true;
    cambioModo = false;
  }

  // EJECUTAR RFID CONTINUAMENTE
  if (submodo == 16) {
    procesarRecargaRFID();
    return;  // No procesar teclado mientras lee RFID
  }
  
  if (submodo == 23) {
    procesarPagoRFID();
    return;  // No procesar teclado mientras lee RFID
  }
  // FIN

  if (actualizarPantalla) {
    lcd.clear();
    if (modo == -1) {
      lcd.setCursor(0, 0);
      lcd.print("ELIGE MODO:");
      mensajeScrollActual = "0.PEDIDO-1.RECARGAR-2.PAGAR-3.CONSULTAR";
    } else if ((modo == 0 || modo == 2 || modo == 3) && mesaSeleccionada == -1) {
      lcd.setCursor(0, 0);
      lcd.print("MESA (0-14):");
      mensajeScrollActual = "";
    } else {
      mensajeScrollActual = "";
    }
    actualizarPantalla = false;
  }

  if (mensajeScrollActual != "") {
    String temp = mensajeScrollActual;
    temp.toUpperCase();
    mensajeScroll(temp.c_str(), 1, 500);
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
      procesarTeclaMesero(tecla);
      teclaDetectada = false;
      while (digitalRead(COLUMNAS[col]) == LOW);
      delay(150);
    }
  }
}

// ========== FUNCIÓN AUXILIAR MOSTRAR COCINA ==========
bool mesaTieneComandas(int mesa) {
  for (int cat = 0; cat < 4; cat++) {
    for (int prod = 0; prod < 3; prod++) {
      if (comandas[mesa][cat][prod] > 0) {
        return true;
      }
    }
  }
  return false;
}

// ========= FUNCION PARA COCINA ==========
void mostrarCocina() {
  if (cambioModo) {
    lcd.clear();
    modoCocinaSubmodo = -1;
    mesaCocinaActual = 0;
    // NO resetear ultimaMesaLista
    mensajeScrollActual = "";
    cambioModo = false;
    cambioMesaFlag = false;
    limpiarComandaFlag = false;
  }

  if (modoCocinaSubmodo == -1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("COCINA");
    lcd.setCursor(0, 1);
    lcd.print("0.VER PEDIDOS");
    mensajeScrollActual = "";
    modoCocinaSubmodo = 0;
    if (ultimaMesaLista < 0) {
      apagarDisplays();
    }
    return;
  }

  if (ultimaMesaLista >= 0) {
    mostrarNumeroDisplay(ultimaMesaLista);
  } else {
    apagarDisplays();
  }

  // VALIDACION
  if (limpiarComandaFlag && modoCocinaSubmodo == 10) {
    limpiarComandaFlag = false;

    // VERIFICAR SI LA MESA TIENE COMANDAS
    if (!mesaTieneComandas(mesaCocinaActual)) {
      // La mesa NO tiene comandas, mostrar error
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("MESA " + String(mesaCocinaActual));
      lcd.setCursor(0, 1);
      lcd.print("SIN COMANDAS!");
      
      Serial.print(">> [COCINA] Mesa ");
      Serial.print(mesaCocinaActual);
      Serial.println(" no tiene comandas pendientes");
      
      delay(2000);
      
      // Volver a mostrar la comanda actual
      mostrarComandaCocina(mesaCocinaActual);
      return;
    }

    // SI TIENE COMANDAS, PROCESAR NORMALMENTE
    parpadearLED(3, 200);

    ultimaMesaLista = mesaCocinaActual;

    enviarMesaListaAMesero(mesaCocinaActual);
    limpiarComanda(mesaCocinaActual);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("MESA " + String(mesaCocinaActual));
    lcd.setCursor(0, 1);
    lcd.print("LISTO!");
    delay(1000);

    mesaCocinaActual = (mesaCocinaActual + 1) % 15;
    mostrarComandaCocina(mesaCocinaActual);
  }

  if (cambioMesaFlag && modoCocinaSubmodo == 10) {
    mesaCocinaActual = (mesaCocinaActual + 1) % 15;
    mostrarComandaCocina(mesaCocinaActual);
    cambioMesaFlag = false;
    delay(200);
  }

  if (mensajeScrollActual != "") {
    String temp = mensajeScrollActual;
    temp.toUpperCase();
    mensajeScroll(temp.c_str(), 1, 500);
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
      procesarTeclaCocina(tecla);
      teclaDetectada = false;
      while (digitalRead(COLUMNAS[col]) == LOW);
      delay(150);
    }
  }
}

// ========== PROCESAMIENTO DE TECLAS MESERO ==========
void procesarTeclaMesero(char tecla) {
  if (tecla == 'C') {
    lcd.clear();
    modo = -1;
    submodo = -1;
    mensajeScrollActual = "";
    actualizarPantalla = true;
    itemSeleccionado = -1;
    cantidadBuffer = "";
    mesaSeleccionada = -1;
    montoRecarga = 0;
    return;
  }

  if (modo == -1) {
    if (tecla >= '0' && tecla <= '3') {
      modo = tecla - '0';
      mesaSeleccionada = -1;
      cantidadBuffer = "";
      mensajeScrollActual = "";
      if (modo == 0) {
        submodo = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("MESA (0-14):");
        actualizarPantalla = false;
      }
      else if (modo == 1) {
        submodo = 1;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("RECARGAR");
        lcd.setCursor(0, 1);
        lcd.print("MONTO: $");
        cantidadBuffer = "";
        actualizarPantalla = false;
      }
      else if (modo == 2) {
        submodo = 2;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("MESA (0-14):");
        actualizarPantalla = false;
      }
      else if (modo == 3) {
        submodo = 3;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("MESA (0-14):");
        actualizarPantalla = false;
      }
    }
  } else if ((modo == 0 || modo == 2 || modo == 3) && mesaSeleccionada == -1) {
    if (tecla >= '0' && tecla <= '9') {
      if (cantidadBuffer.length() < 2) {
        cantidadBuffer += tecla;
        lcd.setCursor(12, 0);
        lcd.print(cantidadBuffer);
      }
    } else if (tecla == '=') {
      int val = cantidadBuffer.toInt();
      if (val >= 0 && val < 15) {
        mesaSeleccionada = val;
        cantidadBuffer = "";
        if (modo == 0) {
          submodo = 0;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("MESA ");
          lcd.print(mesaSeleccionada);
          lcd.setCursor(0, 1);
          lcd.print("OPCIONES");
          delay(100);
          mensajeScrollActual = "0.ENTRADAS-1.PLATOS-2.BEBIDAS-3.POSTRES";
        } else if (modo == 2) {
          submodo = 21;
          mostrarResumenMesa(mesaSeleccionada, false);
        } else if (modo == 3) {
          submodo = 31;
          mostrarResumenMesa(mesaSeleccionada, true);
        }
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("MESA (0-14):");
        lcd.setCursor(0, 1);
        lcd.print("NO VALIDA");
        delay(1000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("MESA (0-14):");
        cantidadBuffer = "";
      }
    }
  } else {
    procesarSubmodoMesero(tecla);
  }
}

void procesarSubmodoMesero(char tecla) {
  switch (submodo) {
    case 0: manejarOpcionesPedido(tecla); break;
    case 10: manejarSeleccionCategoria(tecla, 0, nombresEntradas); break;
    case 11: manejarSeleccionCategoria(tecla, 1, nombresPlatos); break;
    case 13: manejarSeleccionCategoria(tecla, 2, nombresBebidas); break;
    case 14: manejarSeleccionCategoria(tecla, 3, nombresPostres); break;

    case 1:
      if (tecla >= '0' && tecla <= '9') {
        if (cantidadBuffer.length() < 8) {
          cantidadBuffer += tecla;
          lcd.setCursor(9, 1);
          lcd.print(cantidadBuffer);
        }
      } else if (tecla == '=') {
        montoRecarga = cantidadBuffer.toInt();
        if (montoRecarga > 0) {
          submodo = 15;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("MONTO: $");
          lcd.print(montoRecarga);
          lcd.setCursor(0, 1);
          lcd.print("0.NO  1.SI");
          mensajeScrollActual = "";
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("MONTO INVALIDO");
          delay(1500);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("RECARGAR");
          lcd.setCursor(0, 1);
          lcd.print("MONTO: $");
          cantidadBuffer = "";
        }
      }
      break;

    case 15:
      if (tecla == '0') {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("RECARGA CANCEL.");
        delay(1500);
        modo = -1;
        submodo = -1;
        cantidadBuffer = "";
        montoRecarga = 0;
        mensajeScrollActual = "";
        actualizarPantalla = true;
      } else if (tecla == '1') {
        submodo = 16;
        tarjetaLeyendo = false;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ACERQUE TARJETA");
        lcd.setCursor(0, 1);
        lcd.print("RFID");
        mensajeScrollActual = "";
        Serial.println("\n>>> MODO RECARGA ACTIVADO <<<");
        Serial.println(">>> Submodo cambiado a 16");
      }
      break;

    case 21:
      if (tecla == '=') {
        submodo = 22;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("CONFIRMAR?");
        lcd.setCursor(0, 1);
        lcd.print("0.NO  1.RFID");
        mensajeScrollActual = "";
      }
      break;

    case 22:
      if (tecla == '0') {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("PAGO CANCELADO");
        delay(1500);
        modo = -1;
        submodo = -1;
        mesaSeleccionada = -1;
        mensajeScrollActual = "";
        actualizarPantalla = true;
      } else if (tecla == '1') {
        submodo = 23;
        tarjetaLeyendo = false;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ACERQUE TARJETA");
        lcd.setCursor(0, 1);
        lcd.print("RFID");
        mensajeScrollActual = "";
        Serial.println("\n>>> MODO PAGO ACTIVADO <<<");
        Serial.println(">>> Submodo cambiado a 23");
      }
      break;

    case 31:
      if (tecla == '=') {
        modo = -1;
        submodo = -1;
        mesaSeleccionada = -1;
        mensajeScrollActual = "";
        actualizarPantalla = true;
      }
      break;
  }
}

// ========== PROCESAMIENTO DE TECLAS COCINA ==========
void procesarTeclaCocina(char tecla) {
  if (tecla == 'C') {
    modoCocinaSubmodo = -1;
    mesaCocinaActual = 0;
    apagarDisplays();
    return;
  }

  switch (modoCocinaSubmodo) {
    case 0:
      if (tecla == '0') {
        modoCocinaSubmodo = 10;
        mesaCocinaActual = 0;
        mostrarComandaCocina(mesaCocinaActual);
      }
      break;
  }
}

// ========== FUNCIONES RFID ==========

void procesarRecargaRFID() {
  static unsigned long ultimoIntento = 0;
  
  if (millis() - ultimoIntento < 500) return;
  
  // PASO 1: Buscar tarjeta
  lcd.setCursor(0, 0);
  lcd.print("ACERQUE TARJETA ");
  lcd.setCursor(0, 1);
  lcd.print("Esperando...    ");
  
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  
  ultimoIntento = millis();
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TARJETA DETECT!");
  Serial.println(">>> Tarjeta detectada");
  delay(300);
  
  // PASO 2: Leer serial de la tarjeta
  if (!mfrc522.PICC_ReadCardSerial()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR LECTURA");
    lcd.setCursor(0, 1);
    lcd.print("Intente de nuevo");
    Serial.println("ERROR: No se pudo leer serial");
    delay(2000);
    mfrc522.PICC_HaltA();
    return;
  }
  
  tarjetaLeyendo = true;
  
  // PASO 3: Mostrar UID
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("UID OK!");
  Serial.print("UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();
  parpadearLED(1, 150);
  delay(800);
  
  // PASO 4: Leer saldo actual
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LEYENDO SALDO");
  lcd.setCursor(0, 1);
  lcd.print("Espere...       ");
  Serial.println("Leyendo saldo actual...");
  delay(300);
  
  long saldoActual = leerSaldoRFID();
  
  if (saldoActual == -1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR LECTURA");
    lcd.setCursor(0, 1);
    lcd.print("Verifique tag");
    Serial.println("ERROR: No se pudo leer saldo");
    parpadearLED(5, 100);
    delay(2500);
    submodo = 15;
    tarjetaLeyendo = false;
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return;
  }
  
  Serial.print("Saldo actual: $");
  Serial.println(saldoActual);
  
  // PASO 5: Mostrar saldo actual
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SALDO ACTUAL:");
  lcd.setCursor(0, 1);
  lcd.print("$");
  lcd.print(saldoActual);
  delay(2500);
  
  // PASO 6: Mostrar recarga
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RECARGA: $");
  lcd.print(montoRecarga);
  lcd.setCursor(0, 1);
  lcd.print("Procesando...");
  Serial.print("Monto a recargar: $");
  Serial.println(montoRecarga);
  delay(1000);
  
  // PASO 7: Calcular nuevo saldo
  long nuevoSaldo = saldoActual + (long)montoRecarga;
  
  if (nuevoSaldo > 999999) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR:");
    lcd.setCursor(0, 1);
    lcd.print("SALDO EXCEDIDO");
    Serial.println("ERROR: Saldo excede limite");
    parpadearLED(5, 100);
    delay(2500);
    submodo = 15;
    tarjetaLeyendo = false;
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return;
  }
  
  Serial.print("Nuevo saldo a escribir: $");
  Serial.println(nuevoSaldo);
  
  // PASO 8: Escribir nuevo saldo
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ESCRIBIENDO...");
  lcd.setCursor(0, 1);
  lcd.print("No retire tag!");
  delay(500);
  
  if (escribirSaldoRFID(nuevoSaldo)) {
    Serial.println("Escritura OK");
    
    // PASO 9: Verificar escritura
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("VERIFICANDO...");
    delay(300);
    
    long saldoVerif = leerSaldoRFID();
    
    if (saldoVerif == nuevoSaldo) {
      // ÉXITO COMPLETO
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("RECARGA EXITOSA");
      lcd.setCursor(0, 1);
      lcd.print("Nuevo: $");
      lcd.print(nuevoSaldo);
      Serial.println("=== RECARGA EXITOSA ===");
      Serial.print("Saldo verificado: $");
      Serial.println(saldoVerif);
      parpadearLED(3, 200);
      delay(3000);
      
      // Volver al menú
      modo = -1;
      submodo = -1;
      cantidadBuffer = "";
      montoRecarga = 0;
      mensajeScrollActual = "";
      actualizarPantalla = true;
      
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ERROR VERIF.");
      lcd.setCursor(0, 1);
      lcd.print("Leido: $");
      lcd.print(saldoVerif);
      Serial.print("ERROR: Verificacion fallo. Leido: $");
      Serial.println(saldoVerif);
      parpadearLED(5, 100);
      delay(2500);
      submodo = 15;
    }
    
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR ESCRITURA");
    lcd.setCursor(0, 1);
    lcd.print("Reintente");
    Serial.println("ERROR: No se pudo escribir");
    parpadearLED(5, 100);
    delay(2500);
    submodo = 15;
  }
  
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  tarjetaLeyendo = false;
  Serial.println("Tag liberado\n");
}

long leerSaldoRFID() {
  byte buffer[18];
  byte size = sizeof(buffer);
  byte trailerBlock = 7;
  
  Serial.println("  -> Autenticando para lectura...");
  
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A,
    trailerBlock,
    &key,
    &(mfrc522.uid)
  );
  
  if (status != MFRC522::STATUS_OK) {
    Serial.print("  -> Error auth lectura: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return -1;
  }
  
  Serial.println("  -> Auth OK, leyendo bloque 4...");
  delay(50);
  
  status = mfrc522.MIFARE_Read(block, buffer, &size);
  
  if (status != MFRC522::STATUS_OK) {
    Serial.print("  -> Error al leer: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return -1;
  }

  unsigned long saldo = ((unsigned long)buffer[0] << 24) |
                        ((unsigned long)buffer[1] << 16) |
                        ((unsigned long)buffer[2] << 8) |
                        ((unsigned long)buffer[3]);
  
  Serial.print("  -> Bytes leidos: ");
  Serial.print(buffer[0], HEX); Serial.print(" ");
  Serial.print(buffer[1], HEX); Serial.print(" ");
  Serial.print(buffer[2], HEX); Serial.print(" ");
  Serial.println(buffer[3], HEX);
  
  Serial.print("  -> Saldo leido: $");
  Serial.println(saldo);
  
  return (long)saldo;
}

bool escribirSaldoRFID(long saldo) {
  byte buffer[16];
  byte trailerBlock = 7;
  
  memset(buffer, 0, 16);
  
  unsigned long saldoUnsigned = (unsigned long)saldo;
  
  buffer[0] = (saldoUnsigned >> 24) & 0xFF;
  buffer[1] = (saldoUnsigned >> 16) & 0xFF;
  buffer[2] = (saldoUnsigned >> 8) & 0xFF;
  buffer[3] = saldoUnsigned & 0xFF;
  
  Serial.println("  -> Autenticando para escritura...");
  Serial.print("  -> Bytes a escribir: ");
  Serial.print(buffer[0], HEX); Serial.print(" ");
  Serial.print(buffer[1], HEX); Serial.print(" ");
  Serial.print(buffer[2], HEX); Serial.print(" ");
  Serial.println(buffer[3], HEX);
  
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A,
    trailerBlock,
    &key,
    &(mfrc522.uid)
  );
  
  if (status != MFRC522::STATUS_OK) {
    Serial.print("  -> Error auth escritura: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }
  
  Serial.println("  -> Auth OK, escribiendo bloque 4...");
  delay(50);
  
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  
  if (status != MFRC522::STATUS_OK) {
    Serial.print("  -> Error al escribir: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }
  
  Serial.println("  -> Escritura completada");
  return true;
}

void procesarPagoRFID() {
  static unsigned long ultimoIntentoLectura = 0;

  if (millis() - ultimoIntentoLectura < 500) return;
  ultimoIntentoLectura = millis();

  lcd.setCursor(0, 0);
  lcd.print("ACERQUE TARJETA ");
  lcd.setCursor(0, 1);
  lcd.print("Para pagar...   ");

  if (!mfrc522.PICC_IsNewCardPresent()) return;

  delay(100);

  if (!mfrc522.PICC_ReadCardSerial()) {
    mfrc522.PICC_HaltA();
    return;
  }

  tarjetaLeyendo = true;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TARJETA OK!");
  Serial.println(">>> Tarjeta detectada para pago");
  parpadearLED(1, 150);
  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PROCESANDO...");
  delay(500);

  long saldoActual = leerSaldoRFID();

  if (saldoActual == -1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR LECTURA");
    lcd.setCursor(0, 1);
    lcd.print("Reintente");
    Serial.println("ERROR: No se pudo leer saldo");
    parpadearLED(5, 100);
    delay(2000);
    submodo = 22;
    tarjetaLeyendo = false;
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return;
  }

  Serial.print("Saldo actual: $");
  Serial.println(saldoActual);
  Serial.print("Total a pagar: $");
  Serial.println(totalAPagar);

  if (saldoActual < (long)totalAPagar) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SALDO INSUF.");
    lcd.setCursor(0, 1);
    lcd.print("Saldo:$" + String(saldoActual));
    Serial.println("ERROR: Saldo insuficiente");
    parpadearLED(5, 100);
    delay(2500);
    submodo = 22;
    tarjetaLeyendo = false;
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return;
  }

  long nuevoSaldo = saldoActual - (long)totalAPagar;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PROCESANDO PAGO");
  lcd.setCursor(0, 1);
  lcd.print("No retire tag!");
  delay(500);

  if (escribirSaldoRFID(nuevoSaldo)) {
    Serial.println("Pago procesado OK");
    
    // Verificar
    long saldoVerif = leerSaldoRFID();
    
    if (saldoVerif == nuevoSaldo) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("PAGO EXITOSO!");
      lcd.setCursor(0, 1);
      lcd.print("Saldo:$" + String(nuevoSaldo));
      Serial.println("=== PAGO EXITOSO ===");
      Serial.print("Nuevo saldo: $");
      Serial.println(nuevoSaldo);
      parpadearLED(3, 200);
      delay(2500);

      limpiarMesa(mesaSeleccionada);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("MESA LIBERADA");
      delay(1500);

      modo = -1;
      submodo = -1;
      mesaSeleccionada = -1;
      mensajeScrollActual = "";
      actualizarPantalla = true;
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ERROR VERIF.");
      Serial.println("ERROR: Verificacion de pago fallo");
      parpadearLED(5, 100);
      delay(2000);
      submodo = 22;
    }
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR PAGO");
    lcd.setCursor(0, 1);
    lcd.print("Reintente");
    Serial.println("ERROR: No se pudo procesar pago");
    parpadearLED(5, 100);
    delay(2000);
    submodo = 22;
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  tarjetaLeyendo = false;
  Serial.println("Tag liberado\n");
}

// ========== FUNCIONES DE COCINA ==========
void agregarAlComanda(int mesa, int categoria, int producto, int cantidad) {
  comandas[mesa][categoria][producto] += cantidad;
}

void mostrarComandaCocina(int mesa) {
  lcd.clear();
  String resumen = "";
  for (int cat = 0; cat < 4; cat++) {
    for (int prod = 0; prod < 3; prod++) {
      if (comandas[mesa][cat][prod] > 0) {
        if (resumen != "") resumen += " - ";
        resumen += String(nombresCategorias[cat][prod]) + ":" + String(comandas[mesa][cat][prod]);
      }
    }
  }
  lcd.setCursor(0, 0);
  lcd.print("MESA " + String(mesa));
  if (resumen == "") {
    resumen = "<SIN COMANDAS>";
  }
  mensajeScrollActual = resumen;
}

void limpiarComanda(int mesa) {
  for (int cat = 0; cat < 4; cat++) {
    for (int prod = 0; prod < 3; prod++) {
      comandas[mesa][cat][prod] = 0;
    }
  }
}

// ========== FUNCIONES DEL MESERO ==========
void manejarOpcionesPedido(char tecla) {
  if (tecla >= '0' && tecla <= '3') {
    lcd.clear();
    lcd.setCursor(0, 0);
    mensajeScrollActual = "";
    switch (tecla) {
      case '0':
        lcd.print("ENTRADAS");
        mensajeScrollActual = "0.EMPANADAS-1.AREPAS-2.PATACONES";
        submodo = 10;
        break;
      case '1':
        lcd.print("PLATOS FUERTES");
        mensajeScrollActual = "0.BANDEJA PAISA-1.AJIACO-2.SANCOCHO";
        submodo = 11;
        break;
      case '2':
        lcd.print("BEBIDAS");
        mensajeScrollActual = "0.LIMONADA-1.AGUAPANELA-2.GASEOSA";
        submodo = 13;
        break;
      case '3':
        lcd.print("POSTRES");
        mensajeScrollActual = "0.AREQUIPE-1.BREVAS-2.OBLEAS";
        submodo = 14;
        break;
    }
    itemSeleccionado = -1;
    cantidadBuffer = "";
  }
}

void manejarSeleccionCategoria(char tecla, int categoria, const char * nombres[3]) {
  if (itemSeleccionado == -1) {
    if (tecla >= '0' && tecla <= '2') {
      itemSeleccionado = tecla - '0';
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(nombres[itemSeleccionado]);
      lcd.setCursor(0, 1);
      lcd.print("CANTIDAD:");
      cantidadBuffer = "";
      mensajeScrollActual = "";
    }
  } else {
    if (tecla >= '0' && tecla <= '9') {
      cantidadBuffer += tecla;
      lcd.setCursor(9, 1);
      lcd.print(cantidadBuffer);
    } else if (tecla == '=') {
      int cantidad = cantidadBuffer.toInt();
      if (cantidad > 0 && mesaSeleccionada >= 0 && mesaSeleccionada < 15 && itemSeleccionado >= 0 && itemSeleccionado < 3) {
        enviarComanda(mesaSeleccionada, categoria, itemSeleccionado, cantidad);
        mesas[mesaSeleccionada][categoria][itemSeleccionado] += cantidad;

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ENVIADO:");
        lcd.setCursor(0, 1);
        lcd.print(String(cantidad) + " " + nombres[itemSeleccionado]);
        delay(1500);
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ERROR CANTIDAD");
        delay(1000);
      }

      itemSeleccionado = -1;
      cantidadBuffer = "";
      submodo = 0;
      mensajeScrollActual = "0.ENTRADAS-1.PLATOS-2.BEBIDAS-3.POSTRES";
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("OPCIONES");
    }
  }
}

void mostrarResumenMesa(int mesa, bool soloConsulta) {
  lcd.clear();
  String resumen = "";
  unsigned long total = 0;
  for (int cat = 0; cat < 4; cat++) {
    for (int prod = 0; prod < 3; prod++) {
      int cant = mesas[mesa][cat][prod];
      if (cant > 0) {
        if (resumen != "") resumen += " - ";
        resumen += String(nombresCategorias[cat][prod]) + ":" + String(cant);
        total += (unsigned long)cant * preciosCategorias[cat][prod];
      }
    }
  }

  if (resumen == "") resumen = "<SIN PRODUCTOS>";
  mensajeScrollActual = resumen;

  lcd.setCursor(0, 0);
  lcd.print("TOTAL: $" + String(total));
  totalAPagar = total;
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

  if (mensajeAnterior != mensaje) {
    scrollIndex = 0;
    tiempoScroll = millis();
    mensajeAnterior = mensaje;
  }
  if (millis() - tiempoScroll >= intervalo) {
    tiempoScroll = millis();
    char buffer[LCD_ANCHO + 1];
    for (int i = 0; i < LCD_ANCHO; i++) {
      buffer[i] = mensaje[(scrollIndex + i) % lenScroll];
    }
    buffer[LCD_ANCHO] = '\0';
    lcd.setCursor(0, fila);
    lcd.print(buffer);
    scrollIndex = (scrollIndex + 1) % lenScroll;
  }
}