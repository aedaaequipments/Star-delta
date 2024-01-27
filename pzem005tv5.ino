#include <float.h>
#include <PZEM004Tv30.h>// current sensing up grades // POWER DISPLY IN HP
#include <SoftwareSerial.h>
bool phase1Flag = false;
bool phase2Flag = false;
bool phase3Flag = false;
float phase1InitialCurrent = 0.0;
float phase2InitialCurrent = 0.0;
float phase3InitialCurrent = 0.0;
bool deltaTransitionOccurred = false;
bool diagnosticCheckDone = false;
unsigned long diagnosticStartTime = 0;
const unsigned long diagnosticDuration = 60000; // 1 minute
unsigned long deltaTransitionTime = 0;
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
    if (voltage > 180.0 && voltage < 260.0) {
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
     readAndDisplay(pzem1, "Phase 1");
    readAndDisplay(pzem2, "Phase 2");
    readAndDisplay(pzem3, "Phase 3");
    
    checkVoltageAndSetFlag(pzem1.voltage(), "Phase 1", phase1Flag);
    checkVoltageAndSetFlag(pzem2.voltage(), "Phase 2", phase2Flag);
    checkVoltageAndSetFlag(pzem3.voltage(), "Phase 3", phase3Flag);
// Display sum of all powers in HP
    float totalPowerWatts = pzem1.power() + pzem2.power() + pzem3.power();
    float totalPowerHP = totalPowerWatts / 746.0;
    Serial.print("Total Motor Power: ");
    Serial.print(totalPowerHP);
    Serial.println(" HP");

// Delta transition and current check
    if (isMotorRunning && isStarActive && millis() - startMillis >= starDuration) {
        switchToDelta();
    }

    // Check for current rise 10 seconds after delta transition
    if (deltaTransitionOccurred && millis() - deltaTransitionTime >= 10000 && !diagnosticCheckDone) {
        checkCurrentRise();
        diagnosticCheckDone = true;
        diagnosticStartTime = millis();
    }

    // Stop motor after 1 minute if the diagnostic check fails
    if (diagnosticCheckDone && millis() - diagnosticStartTime >= diagnosticDuration) {
        stopMotor();
        Serial.println("Motor stopped due to diagnostic failure.");
        diagnosticCheckDone = false; // Reset diagnostic check flag
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
    Serial.println("Delta mode active");

    // Record the current immediately after switching to delta
    phase1InitialCurrent = pzem1.current();
    phase2InitialCurrent = pzem2.current();
    phase3InitialCurrent = pzem3.current();

    deltaTransitionOccurred = true;
    deltaTransitionTime = millis();

    // Call the evaluateCurrentState function
    evaluateCurrentState();
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
void checkCurrentRise() {
    float current1 = pzem1.current();
    float current2 = pzem2.current();
    float current3 = pzem3.current();

    if (current1 <= phase1InitialCurrent && current2 <= phase2InitialCurrent && current3 <= phase3InitialCurrent) {
        Serial.println("Alert: No rise in current observed. Issue with relay contactors.");
    } else {
        Serial.println("Current rise observed. Relay contactors functioning normally.");
    }
}
void evaluateCurrentState() {
    unsigned long evaluationStartTime = millis();
    const unsigned long evaluationDuration = 30000; // 30 seconds

    // Variables to store current readings
    float maxCurrent1 = 0.0, maxCurrent2 = 0.0, maxCurrent3 = 0.0;
    float minCurrent1 = FLT_MAX, minCurrent2 = FLT_MAX, minCurrent3 = FLT_MAX;
    float sumCurrent1 = 0.0, sumCurrent2 = 0.0, sumCurrent3 = 0.0;
    int numReadings = 0;

    while (millis() - evaluationStartTime < evaluationDuration) {
        // Read current for each phase
        float current1 = pzem1.current();
        float current2 = pzem2.current();
        float current3 = pzem3.current();
        numReadings++;

        // Update max, min, and sum currents
        maxCurrent1 = max(maxCurrent1, current1);
        minCurrent1 = min(minCurrent1, current1);
        sumCurrent1 += current1;

        maxCurrent2 = max(maxCurrent2, current2);
        minCurrent2 = min(minCurrent2, current2);
        sumCurrent2 += current2;

        maxCurrent3 = max(maxCurrent3, current3);
        minCurrent3 = min(minCurrent3, current3);
        sumCurrent3 += current3;

        delay(1000); // Delay for 1 second before next reading
    }

    // Calculate average currents
    float avgCurrent1 = sumCurrent1 / numReadings;
    float avgCurrent2 = sumCurrent2 / numReadings;
    float avgCurrent3 = sumCurrent3 / numReadings;

    // Check for various conditions
    if (maxCurrent1 <= phase1InitialCurrent && maxCurrent2 <= phase2InitialCurrent && maxCurrent3 <= phase3InitialCurrent) {
        Serial.println("Alert: No rise in current observed. Possible issue with relay contactors.");
    } else if (isUnstableCurrent(maxCurrent1, minCurrent1, avgCurrent1) ||
               isUnstableCurrent(maxCurrent2, minCurrent2, avgCurrent2) ||
               isUnstableCurrent(maxCurrent3, minCurrent3, avgCurrent3)) {
        Serial.println("Unstable currents observed. Possible cable fault.");
    } else {
        Serial.println("Currents are stable.");
    }
}

bool isUnstableCurrent(float maxCurrent, float minCurrent, float avgCurrent) {
    return (maxCurrent - minCurrent > 0.2 * avgCurrent);
}



