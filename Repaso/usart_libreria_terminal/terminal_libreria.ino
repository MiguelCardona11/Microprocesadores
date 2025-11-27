void setup() {
  Serial2.begin(9600); // Configura USART2 a 9600 baudios, 8N1 por defecto
}

void loop() {
  if (Serial2.available()) {     // Verifica si hay datos disponibles
    char rxChar = Serial2.read(); // Lee el carácter recibido
    Serial2.write(rxChar);        // Retransmite el mismo carácter (echo)
  }
}