#include <LiquidCrystal.h>
#include <SPI.h>
#include <MFRC522.h>

// ========== COMUNICACION SERIAL DUAL ==========
// Serial  (0,1)   = Debug al Monitor Serial
// Serial1 (18,19) = Mesero ENVÍA a Cocina (TX1)
// Serial2 (16,17) = Cocina RECIBE (RX2) - Conectar TX1 a RX2 físicamente
// NOTA: Pin 19 se usa para INT4, puede interferir con TX1

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

#define PIN_BOTON_COCINA 3
#define PIN_BOTON_SIGUIENTE_MESA 2
#define PIN_BOTON_LIMPIAR_COMANDA 18  // NUEVO: INT5
#define PIN_LED_CONFIRMACION 40       // NUEVO: LED de confirmación

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
volatile bool modoCocina = false;
volatile bool cambioModo = false;
volatile bool cambioMesaFlag = false;
volatile bool limpiarComandaFlag = false;  // NUEVO: Flag para limpiar comanda

// VARIABLES DE COCINA
int modoCocinaSubmodo = -1;
int mesaCocinaActual = 0;

// VARIABLES RFID
unsigned long totalAPagar = 0;
unsigned long montoRecarga = 0;
bool tarjetaLeyendo = false;

// ========== FUNCIONES DE COMUNICACION SERIAL ==========

// MESERO ENVÍA comandas a COCINA por Serial1
void enviarComanda(int mesa, int categoria, int producto, int cantidad) {
  // Formato: $MESA,CATEGORIA,PRODUCTO,CANTIDAD#
  Serial1.print("$");
  Serial1.print(mesa);
  Serial1.print(",");
  Serial1.print(categoria);
  Serial1.print(",");
  Serial1.print(producto);
  Serial1.print(",");
  Serial1.print(cantidad);
  Serial1.println("#");
  
  // Debug al monitor
  Serial.print(">> [MESERO ENVÍA] Mesa ");
  Serial.print(mesa);
  Serial.print(" | Cat ");
  Serial.print(categoria);
  Serial.print(" Prod ");
  Serial.print(producto);
  Serial.print(" x");
  Serial.println(cantidad);
}

// COCINA ENVÍA confirmación a MESERO por Serial2
void enviarLimpiarComanda(int mesa) {
  // Formato: $LIMPIAR,MESA#
  Serial2.print("$LIMPIAR,");
  Serial2.print(mesa);
  Serial2.println("#");
  
  Serial.print(">> [COCINA ENVÍA] LIMPIAR COMANDA: Mesa ");
  Serial.println(mesa);
}

// COCINA LEE mensajes de Serial2 (desde Mesero)
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

// PROCESAR mensaje en COCINA
void procesarMensajeEnCocina(String mensaje) {
  Serial.print("<< [COCINA RECIBE] ");
  Serial.println(mensaje);
  
  if (mensaje.indexOf("LIMPIAR") != -1) {
    // Formato: LIMPIAR,MESA
    int coma = mensaje.indexOf(",");
    int mesa = mensaje.substring(coma + 1).toInt();
    
    if (mesa >= 0 && mesa < 15) {
      limpiarComanda(mesa);
      Serial.print(">> [COCINA PROCESA] COMANDA LIMPIADA: Mesa ");
      Serial.println(mesa);
    }
  } else {
    // Formato: MESA,CATEGORIA,PRODUCTO,CANTIDAD
    int pos = 0;
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
      
      if (mesa >= 0 && mesa < 15 && 
          categoria >= 0 && categoria < 4 && 
          producto >= 0 && producto < 3 && 
          cantidad > 0) {
        agregarAlComanda(mesa, categoria, producto, cantidad);
        Serial.print(">> [COCINA PROCESA] COMANDA AGREGADA: Mesa ");
        Serial.println(mesa);
      }
    }
  }
}

void setup() {
  // Serial principal para DEBUG
  Serial.begin(9600);
  delay(500);
  
  // Serial1: Mesero ENVÍA (TX1=18)
  Serial1.begin(9600);
  
  // Serial2: Cocina RECIBE (RX2=17) - Debe estar conectado a TX1
  Serial2.begin(9600);
  
  Serial.println("\n======================= LOG =======================");
  Serial.println("====================================================\n");
  
  // Configurar teclado
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

  // Inicializar LCD
  lcd.begin(LCD_ANCHO, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("INICIANDO...");

  // Inicializar mesas
  for (int mesa = 0; mesa < 15; mesa++)
    for (int cat = 0; cat < 4; cat++)
      for (int prod = 0; prod < 3; prod++) {
        mesas[mesa][cat][prod] = 0;
        comandas[mesa][cat][prod] = 0;
        ultimoAgregado[mesa][cat][prod] = 0;
      }

  // Configurar botones con interrupciones
  pinMode(PIN_BOTON_COCINA, INPUT_PULLUP);
  EIMSK |= (1 << INT1);
  EICRA |= (1 << ISC11);
  EICRA &= ~(1 << ISC10);

  pinMode(PIN_BOTON_SIGUIENTE_MESA, INPUT_PULLUP);
  EIMSK |= (1 << INT0);
  EICRA |= (1 << ISC01);
  EICRA &= ~(1 << ISC00);

  pinMode(PIN_BOTON_LIMPIAR_COMANDA, INPUT_PULLUP);
  EIMSK |= (1 << INT5);          // Habilitar INT5
  EICRB |= (1 << ISC51);         // Flanco de bajada (usa EICRB, no EICRA)
  EICRB &= ~(1 << ISC50);

  // Configurar LED
  pinMode(PIN_LED_CONFIRMACION, OUTPUT);
  digitalWrite(PIN_LED_CONFIRMACION, LOW);

  // Configurar displays 7-segmentos
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

  // Inicializar SPI y RFID
  SPI.begin();
  delay(100);
  
  mfrc522.PCD_Init();
  delay(100);
}

void loop() {
  // SIEMPRE procesar mensajes de la cocina
  procesarComandaSerialCocina();
  
  if (modoCocina) {
    mostrarCocina();
  } else {
    mostrarMesero();
  }
}

// ========== ISR ==========
ISR(INT0_vect) {
  cambioMesaFlag = true;
}

ISR(PCINT2_vect) {
  teclaDetectada = true;
}

ISR(INT1_vect) {
  modoCocina = !modoCocina;
  cambioModo = true;
}

ISR(INT5_vect) {
  // Solo activar si estamos en modo cocina viendo pedidos
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
  digitalWrite(DIG1, HIGH);
  digitalWrite(DIG2, HIGH);
}

void parpadearLED(int veces, unsigned long intervalo) {
  for (int i = 0; i < veces; i++) {
    digitalWrite(PIN_LED_CONFIRMACION, HIGH);
    delay(intervalo);
    digitalWrite(PIN_LED_CONFIRMACION, LOW);
    if (i < veces - 1) {  // No esperar después del último parpadeo
      delay(intervalo);
    }
  }
}

// ========== FUNCION PARA MESERO ==========
void mostrarMesero() {
  apagarDisplays();
  
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

// ========== FUNCION PARA COCINA ==========
void mostrarCocina() {
  if (cambioModo) {
    lcd.clear();
    modoCocinaSubmodo = -1;
    mesaCocinaActual = 0;
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
    apagarDisplays();
    return;
  }

  mostrarNumeroDisplay(mesaCocinaActual);

  // AGREGAR ESTA VERIFICACIÓN PARA EL BOTÓN DE LIMPIEZA
  if (limpiarComandaFlag && modoCocinaSubmodo == 10) {
    limpiarComandaFlag = false;
    
    // Parpadear LED 3 veces
    parpadearLED(3, 200);
    
    // Enviar comando de limpieza y limpiar comanda
    enviarLimpiarComanda(mesaCocinaActual);
    limpiarComanda(mesaCocinaActual);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("MESA " + String(mesaCocinaActual));
    lcd.setCursor(0, 1);
    lcd.print("LISTO!");
    delay(1000);
    
    // Avanzar a la siguiente mesa
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
    temp += "   ";
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
    modo = -1; submodo = -1;
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
        lcd.setCursor(0,0); 
        lcd.print("MESA (0-14):"); 
        actualizarPantalla = false;
      }
      else if (modo == 1) { 
        submodo = 1; 
        lcd.clear(); 
        lcd.setCursor(0,0); 
        lcd.print("RECARGAR"); 
        lcd.setCursor(0,1); 
        lcd.print("MONTO: $"); 
        cantidadBuffer = "";
        actualizarPantalla = false;
      }
      else if (modo == 2) { 
        submodo = 2; 
        lcd.clear(); 
        lcd.setCursor(0,0); 
        lcd.print("MESA (0-14):"); 
        actualizarPantalla = false; 
      }
      else if (modo == 3) { 
        submodo = 3; 
        lcd.clear(); 
        lcd.setCursor(0,0); 
        lcd.print("MESA (0-14):"); 
        actualizarPantalla = false; 
      }
    }
  } else if ((modo == 0 || modo == 2 || modo == 3) && mesaSeleccionada == -1) {
    if (tecla >= '0' && tecla <= '9') {
      if (cantidadBuffer.length() < 2) { 
        cantidadBuffer += tecla; 
        lcd.setCursor(12,0); 
        lcd.print(cantidadBuffer); 
      }
    } else if (tecla == '=') {
      int val = cantidadBuffer.toInt();
      if (val >=0 && val < 15) {
        mesaSeleccionada = val;
        cantidadBuffer = "";
        if (modo == 0) {
          submodo = 0; 
          lcd.clear(); 
          lcd.setCursor(0,0); 
          lcd.print("MESA "); 
          lcd.print(mesaSeleccionada); 
          lcd.setCursor(0,1); 
          lcd.print("OPCIONES"); 
          delay(100);
          mensajeScrollActual = "0.ENTRADAS-1.PLATOS FUERTES-2.BEBIDAS-3.POSTRES";
        } else if (modo == 2) {
          submodo = 21;
          mostrarResumenMesa(mesaSeleccionada, false);
        } else if (modo == 3) {
          submodo = 31;
          mostrarResumenMesa(mesaSeleccionada, true);
        }
      } else {
        lcd.clear(); 
        lcd.setCursor(0,0); 
        lcd.print("MESA (0-14):"); 
        lcd.setCursor(0,1); 
        lcd.print("NO VALIDA");
        delay(1000); 
        lcd.clear(); 
        lcd.setCursor(0,0); 
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
    
    // ========== PROCESO DE RECARGA (Submodos 1, 15, 16) ==========
    case 1:  // Ingreso de monto
      if (tecla >= '0' && tecla <= '9') {
        if (cantidadBuffer.length() < 8) {  // Limitar a 8 dígitos
          cantidadBuffer += tecla;
          lcd.setCursor(9, 1);
          lcd.print(cantidadBuffer);
        }
      } else if (tecla == '=') {
        montoRecarga = cantidadBuffer.toInt();
        if (montoRecarga > 0) {
          submodo = 15;  // Pasar a confirmación
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
      
    case 15:  // Confirmación de recarga
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
        submodo = 16;  // Pasar a lectura de tarjeta
        tarjetaLeyendo = false;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ACERQUE TARJETA");
        lcd.setCursor(0, 1);
        lcd.print("RFID");
        mensajeScrollActual = "";
      }
      break;
      
    case 16:  // Lectura de tarjeta para recarga
      procesarRecargaRFID();
      break;
    
    // ========== PROCESO DE PAGO ==========
    case 21:
      if (tecla == '=') {
        submodo = 22;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("CONFIRMAR?");
        lcd.setCursor(0,1);
        lcd.print("0.NO  1.RFID");
        mensajeScrollActual = "";
      }
      break;
      
    case 22:
      if (tecla == '0') {
        lcd.clear();
        lcd.setCursor(0,0);
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
        lcd.setCursor(0,0);
        lcd.print("ACERQUE TARJETA");
        lcd.setCursor(0,1);
        lcd.print("RFID");
        mensajeScrollActual = "";
      }
      break;
      
    case 23:
      procesarPagoRFID();
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
      
    case 2: break;
    case 3: break;
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
    // ELIMINADO: Ya no se usa el caso con '=' para limpiar comanda
  }
}

// ========== FUNCIONES RFID ==========

// FUNCIÓN: Procesar recarga RFID
void procesarRecargaRFID() {
  static unsigned long ultimoIntentoLectura = 0;
  
  if (millis() - ultimoIntentoLectura < 300) {
    return;
  }
  ultimoIntentoLectura = millis();
  
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  
  delay(100);
  
  if (!mfrc522.PICC_ReadCardSerial()) {
    mfrc522.PICC_HaltA();
    return;
  }
  
  tarjetaLeyendo = true;
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("TARJETA OK!");
  lcd.setCursor(0,1);
  lcd.print("UID:");
  for (byte i = 0; i < 4 && i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) lcd.print("0");
    lcd.print(mfrc522.uid.uidByte[i], HEX);
  }
  delay(1500);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("PROCESANDO...");
  delay(500);
  
  int saldoActual = leerSaldoRFID();
  
  if (saldoActual == -1) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ERROR LECTURA");
    lcd.setCursor(0,1);
    lcd.print("SALDO");
    delay(2000);
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    submodo = 15;  // Volver a confirmación
    tarjetaLeyendo = false;
    return;
  }
  
  Serial.print("Saldo actual: ");
  Serial.println(saldoActual);
  Serial.print("Monto a recargar: ");
  Serial.println((int)montoRecarga);
  
  int nuevoSaldo = saldoActual + (int)montoRecarga;
  
  if (escribirSaldoRFID(nuevoSaldo)) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("RECARGA EXITOSA");
    lcd.setCursor(0,1);
    lcd.print("Saldo:$" + String(nuevoSaldo));
    delay(2500);
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("RECARGA +$");
    lcd.print(montoRecarga);
    delay(1500);
    
    modo = -1; 
    submodo = -1; 
    cantidadBuffer = "";
    montoRecarga = 0;
    mensajeScrollActual = ""; 
    actualizarPantalla = true;
  } else {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ERROR ESCRITURA");
    lcd.setCursor(0,1);
    lcd.print("REINTENTE");
    delay(2000);
    submodo = 15;  // Volver a confirmación
  }
  
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  tarjetaLeyendo = false;
}

void procesarPagoRFID() {
  static unsigned long ultimoIntentoLectura = 0;
  
  if (millis() - ultimoIntentoLectura < 300) {
    return;
  }
  ultimoIntentoLectura = millis();
  
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  
  delay(100);
  
  if (!mfrc522.PICC_ReadCardSerial()) {
    mfrc522.PICC_HaltA();
    return;
  }
  
  tarjetaLeyendo = true;
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("TARJETA OK!");
  lcd.setCursor(0,1);
  lcd.print("UID:");
  for (byte i = 0; i < 4 && i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) lcd.print("0");
    lcd.print(mfrc522.uid.uidByte[i], HEX);
  }
  delay(1500);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("PROCESANDO...");
  delay(500);
  
  int saldoActual = leerSaldoRFID();
  
  if (saldoActual == -1) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ERROR LECTURA");
    lcd.setCursor(0,1);
    lcd.print("SALDO");
    delay(2000);
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    submodo = 22;
    tarjetaLeyendo = false;
    return;
  }
  
  Serial.print("Saldo actual: ");
  Serial.println(saldoActual);
  Serial.print("Total a pagar: ");
  Serial.println((int)totalAPagar);
  
  if (saldoActual < (int)totalAPagar) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("SALDO INSUF.");
    lcd.setCursor(0,1);
    lcd.print("Saldo:$" + String(saldoActual));
    delay(2500);
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    submodo = 22;
    tarjetaLeyendo = false;
    return;
  }
  
  int nuevoSaldo = saldoActual - (int)totalAPagar;
  
  if (escribirSaldoRFID(nuevoSaldo)) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("PAGO EXITOSO!");
    lcd.setCursor(0,1);
    lcd.print("Saldo:$" + String(nuevoSaldo));
    delay(2500);
    
    limpiarMesa(mesaSeleccionada);
    enviarLimpiarComanda(mesaSeleccionada);
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("MESA LIBERADA");
    delay(1500);
    
    modo = -1; 
    submodo = -1; 
    mesaSeleccionada = -1;
    mensajeScrollActual = ""; 
    actualizarPantalla = true;
  } else {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ERROR ESCRITURA");
    lcd.setCursor(0,1);
    lcd.print("REINTENTE");
    delay(2000);
    submodo = 22;
  }
  
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  tarjetaLeyendo = false;
}

int leerSaldoRFID() {
  byte buffer[18];
  byte size = sizeof(buffer);
  byte trailerBlock = 7;
  
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A, 
    trailerBlock, 
    &key, 
    &(mfrc522.uid)
  );
  
  if (status != MFRC522::STATUS_OK) {
    Serial.println("Error autenticacion lectura");
    return -1;
  }
  
  delay(50);
  
  status = mfrc522.MIFARE_Read(block, buffer, &size);
  
  if (status != MFRC522::STATUS_OK) {
    Serial.println("Error lectura bloque");
    return -1;
  }
  
  long saldo = ((long)buffer[0] << 24) | 
               ((long)buffer[1] << 16) | 
               ((long)buffer[2] << 8) | 
               buffer[3];
  
  return (int)saldo;
}

bool escribirSaldoRFID(int saldo) {
  byte buffer[16];
  byte trailerBlock = 7;
  
  memset(buffer, 0, 16);
  
  buffer[0] = (saldo >> 24) & 0xFF;
  buffer[1] = (saldo >> 16) & 0xFF;
  buffer[2] = (saldo >> 8) & 0xFF;
  buffer[3] = saldo & 0xFF;
  
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A, 
    trailerBlock, 
    &key, 
    &(mfrc522.uid)
  );
  
  if (status != MFRC522::STATUS_OK) {
    Serial.println("Error autenticacion escritura");
    return false;
  }
  
  delay(50);
  
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  
  if (status != MFRC522::STATUS_OK) {
    Serial.println("Error escritura bloque");
    return false;
  }
  
  return true;
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
      int cant = comandas[mesa][cat][prod];
      if (cant > 0) {
        if (resumen != "") resumen += " - ";
        String nombreUpper = String(nombresCategorias[cat][prod]);
        nombreUpper.toUpperCase();
        resumen += nombreUpper + ":" + String(cant);
      }
    }
  }
  lcd.setCursor(0, 0);
  lcd.print("MESA " + String(mesa));
  lcd.setCursor(0, 1);
  if(resumen == ""){
    resumen+= "<SIN COMANDAS>  ";
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
    lcd.setCursor(0,0); 
    mensajeScrollActual = "";
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
      lcd.clear(); 
      lcd.setCursor(0,0);
      lcd.print(nombre);
      lcd.setCursor(0,1); 
      lcd.print("CANTIDAD:"); 
      cantidadBuffer = ""; 
      mensajeScrollActual = "";
    }
  } else {
    if (tecla >= '0' && tecla <= '9') {
      cantidadBuffer += tecla;
      lcd.setCursor(9,1); 
      lcd.print(cantidadBuffer);
    } else if (tecla == '=') {
      int cantidad = cantidadBuffer.toInt();
      if (cantidad > 0 &&
          mesaSeleccionada >= 0 && mesaSeleccionada < 15 &&
          itemSeleccionado >=0 && itemSeleccionado<3) {
        
        // Enviar por Serial1 a la Cocina
        enviarComanda(mesaSeleccionada, categoria, itemSeleccionado, cantidad);
        
        mesas[mesaSeleccionada][categoria][itemSeleccionado] += cantidad;
        
        String nombre = nombres[itemSeleccionado];
        nombre.toUpperCase();
        lcd.clear(); 
        lcd.setCursor(0,0); 
        lcd.print("ENVIADO:");
        lcd.setCursor(0,1);
        lcd.print(String(mesas[mesaSeleccionada][categoria][itemSeleccionado]) + " " + nombre);
        delay(1500);
      } else {
        lcd.clear(); 
        lcd.setCursor(0,0); 
        lcd.print("ERROR CANTIDAD");
        delay(1000);
      }
      itemSeleccionado = -1; 
      cantidadBuffer = ""; 
      submodo = 0; 
      opcionPedidoSeleccionada = -1;
      mensajeScrollActual = "0.ENTRADAS-1.PLATOS FUERTES-2.BEBIDAS-3.POSTRES"; 
      lcd.clear(); 
      lcd.setCursor(0,0); 
      lcd.print("OPCIONES");
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
  String totalStr = "TOTAL: $" + String(total);
  totalStr.toUpperCase();
  lcd.setCursor(0,0); 
  lcd.print(totalStr);
  
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
  String tempMsg = String(mensaje);
  tempMsg.toUpperCase();
  const char* upMsg = tempMsg.c_str();
  if (mensajeAnterior != tempMsg) {
    scrollIndex = 0; 
    tiempoScroll = millis(); 
    mensajeAnterior = tempMsg;
  }
  if (millis() - tiempoScroll >= intervalo) {
    tiempoScroll = millis();
    char buffer[LCD_ANCHO + 1];
    for (int i = 0; i < LCD_ANCHO; i++) {
      int idx = (scrollIndex + i) % lenScroll;
      buffer[i] = upMsg[idx];
    }
    buffer[LCD_ANCHO] = '\0';
    lcd.setCursor(0, fila); 
    lcd.print(buffer);
    scrollIndex = (scrollIndex + 1) % lenScroll;
  }
}