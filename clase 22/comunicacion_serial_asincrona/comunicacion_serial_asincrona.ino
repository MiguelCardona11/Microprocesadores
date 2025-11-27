// Configuración de USART1: 9600 baudios, 8 bits, 1 bit de parada, sin paridad
// USART2: TX1: pin 16, RX1: pin 17

volatile char rxChar;
volatile bool newChar = false;

void setup() {
  UBRR2 = 103;                                // para 9600 Baud --> UBRR =  103
  UCSR2B = (1<<RXEN2)|(1<<TXEN2)|(1<<RXCIE2); // RXEN2: habilitar receptor, TXEN2: habilitar transmisor, RXCIE2: habilitar entrada de datos por interrupcion
  UCSR2C = (1<<UCSZ21)|(1<<UCSZ20);           // UCSZn1: 1, UCSZn0: 1 -> caracteres de 8 bits ; UMSELn1:0 y UMSELn0: 0 -> USART modo asincrono
  sei();                                      // Habilita interrupciones globales
}

void loop() {
  if (newChar) {
    // Si el bit UDRE2 de UCSR2A es 1, significa que UDR1 está listo para recibir un nuevo dato
    while (!(UCSR2A & (1<<UDRE2)));
    UDR2 = rxChar; // Transmite
    newChar = false; // Reinicia el flag
  }
}

// ISR para recepción de USART2
ISR(USART2_RX_vect) {
  // UDR2 lee el contenido del RX
  rxChar = UDR2;      // Guarda el valor recibido
  newChar = true;     // Marca que hay un dato nuevo
}
