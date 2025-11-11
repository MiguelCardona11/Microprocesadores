// Configuración de USART1: 9600 baudios, 8 bits, 1 bit de parada, sin paridad
// Arduino Mega 2560 – TX1 = pin 18, RX1 = pin 19

volatile char rxChar;
volatile bool newChar = false;

void setup() {
  // Configura el baud rate
  // Asynchronous Normal mode (U2Xn = 0)
  // para 9600 Baud --> UBRR =  103
  unsigned int baud = 103;
  UBRR1H = (unsigned char)(baud>>8);
  UBRR1L = (unsigned char)baud;

  // Habilita recepción, transmisión y la interrupción de recepción
  // RXEN1: habilitar receptor, TXEN1: habilitar transmisor, RXCIE1: habilitar entrada de datos por interrupcion
  UCSR1B = (1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1);

  // UCSZn1: 1, UCSZn0: 1 -> 8 bits
  UCSR1C = (1<<UCSZ11)|(1<<UCSZ10);

  // Habilita interrupciones globales
  sei();
}

void loop() {
  if (newChar) {
    // Si el bit UDRE1 de UCSR1A es 1, significa que UDR1 está listo para recibir un nuevo dato
    while (!(UCSR1A & (1<<UDRE1)));
    UDR1 = rxChar; // Transmite
    newChar = false; // Reinicia el flag
  }
}

// ISR para recepción de USART1
ISR(USART1_RX_vect) {
  // UDR1 lee el contenido del RX
  rxChar = UDR1;      // Guarda el valor recibido
  newChar = true;     // Marca que hay un dato nuevo
}
