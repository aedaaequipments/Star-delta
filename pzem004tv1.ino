#include <PZEM004Tv30.h>
HardwareSerial pzemSerial(2); // Using Serial2
// Define the RX and TX pins for ESP32
// Update these GPIO numbers based on your ESP32's wiring
const int RX_PIN = 16;  // Example: GPIO16 for RX
const int TX_PIN = 17;  // Example: GPIO17 for TX

// Create a PZEM instance using the specified RX and TX pins
//PZEM004Tv30 pzem(RX_PIN, TX_PIN);
PZEM004Tv30 pzem(&pzemSerial);
void setup() {
  Serial.begin(9600);
  pzemSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
}

void loop() {
  float voltage = pzem.voltage();
  if (!isnan(voltage)) {
    Serial.print("Voltage: ");
    Serial.print(voltage);
    Serial.println("V");
  } else {
    Serial.println("Error reading voltage");
  }

  float current = pzem.current();
  if (!isnan(current)) {
    Serial.print("Current: ");
    Serial.print(current);
    Serial.println("A");
  } else {
    Serial.println("Error reading current");
  }

  float power = pzem.power();
  if (!isnan(power)) {
    Serial.print("Power: ");
    Serial.print(power);
    Serial.println("W");
  } else {
    Serial.println("Error reading power");
  }

  float energy = pzem.energy();
  if (!isnan(energy)) {
    Serial.print("Energy: ");
    Serial.print(energy, 3);
    Serial.println("kWh");
  } else {
    Serial.println("Error reading energy");
  }

  float frequency = pzem.frequency();
  if (!isnan(frequency)) {
    Serial.print("Frequency: ");
    Serial.print(frequency, 1);
    Serial.println("Hz");
  } else {
    Serial.println("Error reading frequency");
  }

  float pf = pzem.pf();
  if (!isnan(pf)) {
    Serial.print("PF: ");
    Serial.println(pf);
  } else {
    Serial.println("Error reading power factor");
  }

  Serial.println();
  delay(2000);
}
