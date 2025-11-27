void clearScreen() {
  for (int i = 0; i < 30; i++) {
    Serial2.println();
  }
}

void setup() {
  Serial2.begin(9600);
  delay(100); // Pequeña pausa para estabilizar
}

void loop() {
  clearScreen();
  Serial2.println("Mensaje 1: Hola Mundo");
  delay(2000);
  
  clearScreen();
  Serial2.println("Mensaje 2: Texto actualizado");
  delay(2000);
  
  clearScreen();
  Serial2.println("Mensaje 3: Otro texto diferente");
  delay(2000);
}