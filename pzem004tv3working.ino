#include <PZEM004Tv30.h> // voltage sensing part
#include <SoftwareSerial.h>
bool phase1Flag = false;
bool phase2Flag = false;
bool phase3Flag = false;
const int ledPin = 2; // Change to the actual LED pin
// Define RX and TX pins for each PZEM module
SoftwareSerial pzemSerial1(10, 11); // RX, TX for phase 1
SoftwareSerial pzemSerial2(12, 13); // RX, TX for phase 2
SoftwareSerial pzemSerial3(9, 8); // RX, TX for phase 3
const int mainRelayPin = 3; // Main relay
const int starRelayPin = 4; // Star relay
const int deltaRelayPin = 5; // Delta relay
const int buttonPin = 6; // Push button pin
const int stopButtonPin = 7; // Stop button pin
// Variables to manage timing
unsigned long startMillis;
const unsigned long starDuration = 10000; // 10 seconds for star to delta transition

// Flags to manage motor state
bool isMotorRunning = false;
bool isStarActive = false;
// Create PZEM instances for each phase
PZEM004Tv30 pzem1(pzemSerial1);
PZEM004Tv30 pzem2(pzemSerial2);
PZEM004Tv30 pzem3(pzemSerial3);

void setup() {
    Serial.begin(115200);
    pzemSerial1.begin(9600);
    pzemSerial2.begin(9600);
    pzemSerial3.begin(9600);
    pinMode(ledPin, OUTPUT); // Set the LED pin as output
     pinMode(mainRelayPin, OUTPUT);
    pinMode(starRelayPin, OUTPUT);
    pinMode(deltaRelayPin, OUTPUT);
    pinMode(buttonPin, INPUT_PULLUP); // Assuming active LOW push button
    pinMode(stopButtonPin, INPUT_PULLUP); // Assuming active LOW push button
}

/*void loop() {
    // Read and display the information from each PZEM module
    readAndDisplay(pzem1);
    readAndDisplay(pzem2);
    readAndDisplay(pzem3);
    delay(2000);
}*/
void checkVoltageAndSetFlag(float voltage, const String &phaseName, bool &flag) {  // 
    if (voltage > 180.0 && voltage < 250.0) {
        Serial.println("Good voltage at " + phaseName);
        flag = true;
    } else {
        flag = false;
    }
}

void checkAllPhases() {
    if (phase1Flag && phase2Flag && phase3Flag) {
        digitalWrite(ledPin, HIGH); // Turn on the LED if all flags are set
        Serial.println("All phases have good voltage. LED turned ON.");
    } else {
        digitalWrite(ledPin, LOW); // Turn off the LED otherwise
    }
}
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
    readAndDisplay(pzem1, "Phase 1"); // checking perameters from each phase
    readAndDisplay(pzem2, "Phase 2");
    readAndDisplay(pzem3, "Phase 3");
     checkVoltageAndSetFlag(pzem1.voltage(), "Phase 1", phase1Flag); // seting flags for each phase
    checkVoltageAndSetFlag(pzem2.voltage(), "Phase 2", phase2Flag);
    checkVoltageAndSetFlag(pzem3.voltage(), "Phase 3", phase3Flag);

    // Check if all flags are set
    checkAllPhases();
    if (isMotorRunning && !(phase1Flag && phase2Flag && phase3Flag)) {
        stopMotor();
        Serial.println("Voltage issue detected. Motor stopped.");
    }
// Check for button press and start the motor in star configuration
   if (digitalRead(buttonPin) == HIGH && !isMotorRunning) { // Check if start button is pressed and motor is not running
        startMotorInStar();
         Serial.println("Starting ");
    }
    if (digitalRead(stopButtonPin) == HIGH && isMotorRunning) { // Check if stop button is pressed and motor is running
        stopMotor();
        Serial.println("Stop ");
    }
    // Check if it's time to switch from star to delta
    if (isMotorRunning && isStarActive && millis() - startMillis >= starDuration) {
        switchToDelta();
    }
    delay(1000);
}
void startMotorInStar() {
    if (phase1Flag && phase2Flag && phase3Flag) { // Check if all voltage flags are good
        digitalWrite(mainRelayPin, HIGH);
        digitalWrite(starRelayPin, HIGH);
        digitalWrite(deltaRelayPin, LOW); // Ensure delta relay is off
        startMillis = millis(); // Save the start time
        isMotorRunning = true;
        isStarActive = true;
        Serial.println("Starting motor in star mode");
    } else {
        Serial.println("Cannot start motor: Voltage not in good range");
    }
}


void switchToDelta() {
    digitalWrite(starRelayPin, LOW); // Turn off the star relay
    delay(1000); // Wait 1 second for the contactor to fully disengage
    digitalWrite(deltaRelayPin, HIGH); // Turn on the delta relay
    isStarActive = false; // Star is no longer active
    Serial.println("delta ");
}

void stopMotor() {
     if (isMotorRunning) {
        digitalWrite(mainRelayPin, LOW);
        digitalWrite(starRelayPin, LOW);
        digitalWrite(deltaRelayPin, LOW);
        isMotorRunning = false;
        isStarActive = false; // Make sure to reset the star flag as well
        Serial.println("Motor stopped");
    } else {
        Serial.println("Motor is not running");
    }
}
