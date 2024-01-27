#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>

// Define RX and TX pins for each PZEM module
SoftwareSerial pzemSerial1(10, 11); // RX, TX for phase 1
SoftwareSerial pzemSerial2(12, 13); // RX, TX for phase 2
SoftwareSerial pzemSerial3(9, 8); // RX, TX for phase 3

// Create PZEM instances for each phase
PZEM004Tv30 pzem1(pzemSerial1);
PZEM004Tv30 pzem2(pzemSerial2);
PZEM004Tv30 pzem3(pzemSerial3);

void setup() {
    Serial.begin(115200);
    pzemSerial1.begin(9600);
    pzemSerial2.begin(9600);
    pzemSerial3.begin(9600);
}

/*void loop() {
    // Read and display the information from each PZEM module
    readAndDisplay(pzem1);
    readAndDisplay(pzem2);
    readAndDisplay(pzem3);
    delay(2000);
}*/
void readAndDisplay(PZEM004Tv30 &pzem, const String &phaseName) {
    Serial.println(phaseName + " Readings:");
    
    
    float voltage = pzem.voltage();
    if (!isnan(voltage)) {
        Serial.print(phaseName + " Voltage: "); Serial.print(voltage); Serial.println("V");
        if(voltage > 180.0 && phaseName == "Phase 1"){
            Serial.println("Good voltage at Phase 1");
        }
    } else {
        Serial.println("Error reading voltage");
    }
    if (!isnan(voltage)) {
        Serial.print("Voltage: "); Serial.print(voltage); Serial.println("V");
    } else {
        Serial.println("Error reading voltage");
    }

    float current = pzem.current();
    if (!isnan(current)) {
        Serial.print("Current: "); Serial.print(current); Serial.println("A");
    } else {
        Serial.println("Error reading current");
    }

    float power = pzem.power();
    if (!isnan(power)) {
        Serial.print("Power: "); Serial.print(power); Serial.println("W");
    } else {
        Serial.println("Error reading power");
    }

    float energy = pzem.energy();
    if (!isnan(energy)) {
        Serial.print("Energy: "); Serial.print(energy); Serial.println("kWh");
    } else {
        Serial.println("Error reading energy");
    }

    float frequency = pzem.frequency();
    if (!isnan(frequency)) {
        Serial.print("Frequency: "); Serial.print(frequency); Serial.println("Hz");
    } else {
        Serial.println("Error reading frequency");
    }

    float pf = pzem.pf();
    if (!isnan(pf)) {
        Serial.print("PF: "); Serial.println(pf);
    } else {
        Serial.println("Error reading power factor");
    }
    
    Serial.println(); // Add an empty line for better readability
}

void loop() {
    // Read and display the information from each PZEM module
    readAndDisplay(pzem1, "Phase 1");
    readAndDisplay(pzem2, "Phase 2");
    readAndDisplay(pzem3, "Phase 3");
    delay(10000);
}


