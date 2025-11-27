#include <LiquidCrystal.h>

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

// *** COMUNICACIÓN SERIAL ***
// ***************************
String mensajeRecibido = "";
bool mensajeCompleto = false;

// ================================ SETUP ================================ //

void setup() {
  // *** SERIAL (USART2) ***
  Serial2.begin(9600);
  
  // ********* LCD *********
  // ***********************
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Esperando...");
  
  // Mensaje de bienvenida en Virtual Terminal
  Serial2.println("=== SISTEMA LCD ===");
  Serial2.println("Escribe tu mensaje y presiona ENTER");
  Serial2.println("Comandos especiales:");
  Serial2.println("  CLR  - Limpiar pantalla");
  Serial2.println("  L1:  - Escribir en linea 1");
  Serial2.println("  L2:  - Escribir en linea 2");
  Serial2.println("-------------------");
}

// ================================== LOOP ================================== //

void loop() {
  // Leer caracteres del Virtual Terminal
  while (Serial2.available() > 0) {
    char charRecibido = Serial2.read();
    
    // Si es Enter (salto de línea), procesar el mensaje
    if (charRecibido == '\n' || charRecibido == '\r') {
      if (mensajeRecibido.length() > 0) {
        mensajeCompleto = true;
      }
    } else {
      // Acumular caracteres
      mensajeRecibido += charRecibido;
    }
  }
  
  // Si hay un mensaje completo, procesarlo
  if (mensajeCompleto) {
    procesarMensaje(mensajeRecibido);
    mensajeRecibido = "";
    mensajeCompleto = false;
  }
}

// =============================== FUNCIONES ================================ //

void procesarMensaje(String mensaje) {
  // Eliminar espacios al inicio y final
  mensaje.trim();
  
  // Comando: CLR - Limpiar pantalla
  if (mensaje.equalsIgnoreCase("CLR")) {
    lcd.clear();
    lcd.setCursor(0, 0);
    Serial2.println("> Pantalla limpiada");
    return;
  }
  
  // Comando: L1: - Escribir en línea 1
  if (mensaje.startsWith("L1:")) {
    String texto = mensaje.substring(3);
    texto.trim();
    lcd.setCursor(0, 0);
    lcd.print("                "); // Limpiar línea 1
    lcd.setCursor(0, 0);
    lcd.print(texto.substring(0, 16)); // Máximo 16 caracteres
    Serial2.println("> Linea 1: " + texto);
    return;
  }
  
  // Comando: L2: - Escribir en línea 2
  if (mensaje.startsWith("L2:")) {
    String texto = mensaje.substring(3);
    texto.trim();
    lcd.setCursor(0, 1);
    lcd.print("                "); // Limpiar línea 2
    lcd.setCursor(0, 1);
    lcd.print(texto.substring(0, 16)); // Máximo 16 caracteres
    Serial2.println("> Linea 2: " + texto);
    return;
  }
  
  // Mensaje normal: mostrar en LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  
  // Si el mensaje es mayor a 16 caracteres, usar 2 líneas
  if (mensaje.length() > 16) {
    lcd.print(mensaje.substring(0, 16));
    lcd.setCursor(0, 1);
    lcd.print(mensaje.substring(16, 32));
    Serial2.println("> Mostrado en 2 lineas: " + mensaje);
  } else {
    lcd.print(mensaje);
    Serial2.println("> Mostrado: " + mensaje);
  }
}