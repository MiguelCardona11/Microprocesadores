/*
  Ejemplo: Mostrar "HOLA MUNDO" en LCD 2x16 (FDCC1602E - SPLC780)
  Modo: 8 bits
*/

#define RS 12 // Register Select (0: instrucción, 1: dato)
#define E 11  // Enable
#define D0 9
#define D1 8
#define D2 7
#define D3 6
#define D4 5
#define D5 4
#define D6 3
#define D7 2

int pines[] = {RS, E, D0, D1, D2, D3, D4, D5, D6, D7};
int bandera = 1;

void setup() {
  // Configurar pines como salida
  for (int i = 0; i < 10; i++) {
    pinMode(pines[i], OUTPUT);
  }

  delay(5); // Espera inicial (>15 ms)

  // --- RUTINA DE INICIALIZACIÓN ---
  // lcdComando solo tiene en cuenta los D0-D7
  lcdComando(0x38);       // 0011 1000  DL 1: 8 bits, N 1: 2 líneas
  delay(5);

  lcdComando(0x0C);       // 0000 1100 Display ON, cursor OFF, parpadeo OFF
  delay(5);  

  lcdComando(0x06);       // 0000 0110   I/D 1: incremento S 0: mensaje estático
  delay(5);

  lcdComando(0x01); // Clear display
  delay(5);         // Tiempo típico 1.64 ms
  // ---------------------------------
  // --- ESCRIBIR MENSAJE ---
  lcdTexto("HOLA");

}

void loop() {
 // vacio mensaje estatico

}

// ---------------------------------------
// Subrutina: enviar instrucción al LCD
void lcdComando(byte cmd) {
  digitalWrite(RS, LOW); // Modo instrucción
  enviarByte(cmd);
}

// Subrutina: enviar dato (carácter ASCII)
void lcdDato(byte data) {
  digitalWrite(RS, HIGH); // Modo dato
  enviarByte(data);
}

// Función para escribir una cadena completa
void lcdTexto(const char *texto) {
  // mientras el caracter apuntado no sea el fin de la cadena
  while (*texto) {
    lcdDato(*texto++);
    delay(5);
  }
}

// Envía un byte al bus D0–D7 y genera el pulso E
void enviarByte(byte valor) {
  // 0011 1000
  digitalWrite(D0, (valor >> 0) & 0x01);
  digitalWrite(D1, (valor >> 1) & 0x01);
  digitalWrite(D2, (valor >> 2) & 0x01);
  digitalWrite(D3, (valor >> 3) & 0x01);
  digitalWrite(D4, (valor >> 4) & 0x01);
  digitalWrite(D5, (valor >> 5) & 0x01);
  digitalWrite(D6, (valor >> 6) & 0x01);
  digitalWrite(D7, (valor >> 7) & 0x01);

  // Generar pulso de habilitación
  digitalWrite(E, HIGH);
  delay(1); // Duración mínima del pulso (~450 ns)
  digitalWrite(E, LOW);

  delay(5); // Tiempo para que LCD procese el comando
}