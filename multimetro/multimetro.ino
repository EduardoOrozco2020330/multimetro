#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>  // Librería para crear un puerto serial en otros pines

LiquidCrystal_I2C lcd(0x27, 16, 2); // Dirección y tamaño del LCD
SoftwareSerial BTSerial(2, 3); // (RX, TX) Pines para el HC-05

const float R1 = 100000;  // Resistencia conocida (100k ohms)
const float R2 = 47000;   // Resistencia 2 para el divisor de voltaje
float vin, vout, input, vin2 = 0, iin, iin2 = 0, resistencia;
int entradaV = A0, entradaI = A1;  // Pines de entrada
long int ahora, antes = 0;

// Parámetros para el ACS712 (por ejemplo, el de 30A)
const float ACS712_offset = 2.5;  // Voltaje de salida cuando no hay corriente (2.5V)
const float ACS712_sensibilidad = 0.066;  // Sensibilidad del sensor para el modelo de 30A (66mV/A)

void setup() {
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  BTSerial.begin(9600);  // Inicia el módulo Bluetooth a 9600 baudios
  analogReference(DEFAULT); // Usar referencia de 5V

  pinMode(entradaV, INPUT);
  pinMode(entradaI, INPUT);
  
  lcd.setCursor(3, 0);
  lcd.print("VOLTIMETRO");
  lcd.setCursor(3, 1);
  lcd.print("AMPERIMETRO");
  delay(2000);
  lcd.clear();
}

void loop() {
  ahora = millis();
  if (ahora - antes > 333) {  // Actualiza cada 333 ms
    volt();
    amp();
    medirResistencia();
    antes = ahora;
  }
}

void volt() {
  for (int i = 1; i <= 150; i++) {
    input = analogRead(entradaV);
    vout = (input * 5.0) / 1024;  // Usar la referencia de 5V
    vin = vout / (R2 / (R1 + R2)); // Fórmula de divisor de voltaje
    vin2 += vin;
  }
  vin = vin2 / 150;  // Promedio de 150 muestras
  vin2 = 0;
  
  if (vin <= 0.001) { vin = 0; }
  
  lcd.setCursor(0, 0);
  if (vin >= 10) {
    lcd.print(vin);
    lcd.setCursor(5, 0);
    lcd.print("V");
  } else {
    lcd.print(vin);
    lcd.setCursor(4, 0);
    lcd.print("V ");
  }
  
  Serial.print("Voltaje: ");
  Serial.print(vin);
  Serial.println("V");

  // Enviar los datos a través del módulo Bluetooth
  BTSerial.print("Voltaje: ");
  BTSerial.print(vin);
  BTSerial.println(" V");
}

void amp() {
  for (int i = 1; i <= 150; i++) {
    input = analogRead(entradaI);
    vout = (input * 5.0) / 1024;  // Convertir la lectura del ADC a voltios (5V referencia)
    iin = (vout - ACS712_offset) / ACS712_sensibilidad;  // Restar el offset de 2.5V y dividir por la sensibilidad
    iin2 += iin;
  }
  
  iin = iin2 / 150;  // Promedio de 150 muestras
  iin2 = 0;

  lcd.setCursor(0, 1);
  if (iin <= 0.005) { iin = 0; }  // Si es cercano a 0, redondear
  if (iin <= 0.01) {
    lcd.print(iin * 1000);
    lcd.setCursor(4, 1);
    lcd.print("mA  ");
  } else if (iin <= 1) {
    lcd.print(iin * 1000);
    lcd.setCursor(5, 1);
    lcd.print("mA ");
  } else {
    lcd.print(iin);
    lcd.setCursor(4, 1);
    lcd.print("A   ");
  }

  Serial.print("Corriente: ");
  Serial.print(iin);
  Serial.println("A");

  // Enviar los datos de corriente a través del módulo Bluetooth
  BTSerial.print("Corriente: ");
  BTSerial.print(iin);
  BTSerial.println(" A");
}

void medirResistencia() {
  for (int i = 1; i <= 150; i++) {
    input = analogRead(entradaV); // Usar el pin A0 para leer el voltaje en el divisor
    vout = (input * 5.0) / 1024;  // Convertir la lectura del ADC a voltios (5V referencia)
    vin2 += vout;
  }
  
  vout = vin2 / 150;  // Promedio de 150 muestras
  vin2 = 0;

  if (vout <= 0.001) {  // Si el voltaje es muy bajo, evitar divisiones por cero
    resistencia = 0;
  } else {
    resistencia = R1 * ((5.0 / vout) - 1);  // Calcular la resistencia desconocida
  }

  lcd.setCursor(0, 1);
  if (resistencia >= 1000) {
    lcd.print(resistencia / 1000);
    lcd.setCursor(6, 1);
    lcd.print("kOhm ");
  } else {
    lcd.print(resistencia);
    lcd.setCursor(6, 1);
    lcd.print("Ohm ");
  }

  Serial.print("Resistencia: ");
  Serial.print(resistencia);
  Serial.println("Ohm");

  // Enviar los datos de resistencia a través del módulo Bluetooth
  BTSerial.print("Resistencia: ");
  BTSerial.print(resistencia);
  BTSerial.println(" Ohm");
}
