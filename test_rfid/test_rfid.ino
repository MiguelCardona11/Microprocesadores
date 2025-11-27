#include <SPI.h>
#include <MFRC522.h>

// Definir pines para Arduino Mega
#define RST_PIN 5    // Pin reset
#define SS_PIN 53    // Pin SDA (SS)

// Crear instancia del lector RFID
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Clave de autenticación por defecto
MFRC522::MIFARE_Key key;

// Bloque donde escribiremos/leeremos (usaremos el bloque 4)
byte block = 4;

// Número que queremos escribir la primera vez
int numeroInicial = 12345;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  
  // Preparar la clave (por defecto es FF FF FF FF FF FF)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  
  Serial.println("========== Sistema RFID ==========");
  Serial.println("Primera lectura: Escribe el numero");
  Serial.println("Siguientes lecturas: Solo lee el numero");
  Serial.println("Numero a escribir: " + String(numeroInicial));
  Serial.println("Acerca una tarjeta...");
  Serial.println();
}

void loop() {
  // Buscar nuevas tarjetas
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  
  // Seleccionar una tarjeta
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  
  Serial.println("---------------------------------------");
  Serial.println("Tarjeta detectada!");
  Serial.print("UID: ");
  mostrarUID();
  Serial.println();
  
  // Primero intentamos leer si ya hay un número guardado
  int numeroLeido = leerNumero();
  
  if (numeroLeido == 0 || numeroLeido == -1) {
    // La tarjeta está vacía o es la primera vez
    Serial.println("→ Tarjeta vacia o primera lectura");
    Serial.println("→ Escribiendo numero: " + String(numeroInicial));
    
    if (escribirNumero(numeroInicial)) {
      Serial.println("✓ Numero escrito correctamente!");
      
      // Verificar que se escribió bien
      numeroLeido = leerNumero();
      if (numeroLeido == numeroInicial) {
        Serial.println("✓ Verificacion exitosa: " + String(numeroLeido));
      }
    } else {
      Serial.println("✗ Error al escribir en la tarjeta");
    }
    
  } else {
    // Ya hay un número guardado, solo lo mostramos
    Serial.println("→ Leyendo numero almacenado...");
    Serial.println("✓ Numero en la tarjeta: " + String(numeroLeido));
  }
  
  Serial.println("---------------------------------------");
  Serial.println();
  
  // Detener la lectura
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  
  delay(2000);
}

// Función para escribir un número en la tarjeta
bool escribirNumero(int numero) {
  byte buffer[16];
  
  // Limpiar el buffer
  for (byte i = 0; i < 16; i++) {
    buffer[i] = 0;
  }
  
  // Convertir el número a bytes y guardarlo en el buffer
  buffer[0] = (numero >> 24) & 0xFF;
  buffer[1] = (numero >> 16) & 0xFF;
  buffer[2] = (numero >> 8) & 0xFF;
  buffer[3] = numero & 0xFF;
  
  // Autenticar con el bloque
  byte trailerBlock = 7; // Bloque de control del sector 1
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid)
  );
  
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Error de autenticacion: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }
  
  // Escribir en el bloque
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Error al escribir: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }
  
  return true;
}

// Función para leer un número de la tarjeta
int leerNumero() {
  byte buffer[18];
  byte size = sizeof(buffer);
  
  // Autenticar
  byte trailerBlock = 7;
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid)
  );
  
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Error de autenticacion al leer: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return -1;
  }
  
  // Leer el bloque
  status = mfrc522.MIFARE_Read(block, buffer, &size);
  
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Error al leer: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return -1;
  }
  
  // Convertir los bytes a número
  int numero = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
  
  return numero;
}

// Función para mostrar el UID
void mostrarUID() {
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
}