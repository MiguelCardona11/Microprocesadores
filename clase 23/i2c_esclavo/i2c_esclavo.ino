#include <Wire.h>
#include <string.h>

// LCD LM016L Pines
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
char entrada[17];
int largo = 0;

void setup() {
  // Inicia como esclavo en dirección 0x08
  Wire.begin(0x08);
  Wire.onReceive(onReceiveHandler);

  for (int i = 0; i < 10; i++) {
    pinMode(pines[i], OUTPUT);
  }
  delay(20);
  lcdComando(0x38); delayMicroseconds(50);
  lcdComando(0x06); delayMicroseconds(50);
  lcdComando(0x0F); delayMicroseconds(50);
  lcdComando(0x14); delayMicroseconds(50);
  lcdComando(0x01); delay(2);
  lcdComando(0x80);
  limpiarEntrada();
}

void loop() {
  // Todo lo importante ocurre en la interrupción I2C
}

void onReceiveHandler(int cantidad) {
  while (Wire.available()) {
    char tecla = Wire.read();

    // Mostrar la tecla en el LCD
    if (tecla == 'C') {
      lcdComando(0x01);
      limpiarEntrada();
    } else {
      if (largo < 16) {
        entrada[largo++] = tecla;
        lcdDato(tecla);
      }
    }
  }
}

void lcdComando(byte cmd) {
  digitalWrite(RS, LOW);
  enviarByte(cmd);
}
void lcdDato(byte data) {
  digitalWrite(RS, HIGH);
  enviarByte(data);
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
  digitalWrite(E, HIGH); delayMicroseconds(1);
  digitalWrite(E, LOW); delayMicroseconds(50);
}

void limpiarEntrada() {
  memset(entrada, 0, sizeof(entrada));
  largo = 0;
}
