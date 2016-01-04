// EBIKE STUFF

// Thun to CA connector. Clip facing up, leads toward me.
// grey(t), blue(b), brown(a), black(-), white(+).

// The last time we calculated our output values.
unsigned long lastMillis = 0;
// How often should we calculate output values.
int calcInterval = 100;

// PINS

// The number of the onboard LED pin is 13.
const int ledPin =  13;
// Thun sensor inputs.
// Cadence pins.
int pinCadenceA = 2; // Brown
int pinCadenceB = 4; // Blue
// Torque pin.
int torqPin = A3; // Grey
// Uno board pins 5 and 6 are PWM 980 Hz.
int throttlePin = 6;
// One of the motor hall sensor wires.
int pinMotorSpeed = 7;
// Wheel magnet.
int wheelSpeed = 8;

void setup() {
  Serial.begin(9600);
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // Setup cadence sensors.
  pinMode(pinCadenceA, INPUT);
  pinMode(pinCadenceB, INPUT);
  // Setup torque.
  pinMode(torqPin, INPUT);
  // Throttle.
  pinMode(throttlePin, OUTPUT);
  pinMode(pinMotorSpeed, INPUT);
} 

void loop() {
  if (millis() - lastMillis >= calcInterval) {
    cadenceCalcValue();
    torqueCalcValue();
    debugLog();
    lastMillis = millis();
  }
  cadenceLoop();
  torqLoop();
}

// TORQUE

void torqLoop() {
  // Read sensor and save as an average.
}

int torqNmPerVolt = -200;
int torqNm = 0;
float torqRestingVolts = 2.75;

// Convert 10 bit number into Newton Meters.
void torqueCalcValue() {
  // 5 volts / 1024 or .0049 per unit.
  float torqVolts = analogRead(torqPin) * 0.0049;
  torqNm = (torqVolts - torqRestingVolts) * torqNmPerVolt;
}

// CADENCE

// The quantity of clicks per full revolution.
int cadencePolls = 16;
// Establish what turns our cadenceClick into RPM.
float cadenceMultiplier = (60 * (calcInterval / 1000)) / cadencePolls;
// true = A first. false = B first.
boolean cadenceDirection = true;
// Reset when both sensors are LOW.
boolean cadenceValidDirection = false;
// Increment with each low->high transition for every poll.
volatile int cadenceClicks = 0;
int cadenceRPM = 0;

// Keep track of what cadence pin is high/low.
boolean cadenceAwasHigh = false;
boolean cadenceAisHigh = false;
boolean cadenceBwasHigh = false;
boolean cadenceBisHigh = false;

// Check for changes on cadence hall sensors.
void cadenceLoop() {
  // Read A.
  cadenceAisHigh = digitalRead(pinCadenceA);
  if (cadenceAisHigh != cadenceAwasHigh) {
    if (cadenceAisHigh) {
      cadenceAwasHigh = true;
      cadenceAup();
    }
    else {
      cadenceAwasHigh = false;
      cadenceAdown();
    }
  }
  // Read B.
  cadenceBisHigh = digitalRead(pinCadenceB);
  if (cadenceBisHigh != cadenceBwasHigh) {
    if (cadenceBisHigh) {
      cadenceBwasHigh = true;
      cadenceBup();
    }
    else {
      cadenceBwasHigh = false;
      cadenceBdown();
    }
  }
}

// When cadence hall sensor becomes activated.
void cadenceAup() {
  digitalWrite(ledPin, HIGH);
  cadenceSetDirection(true);
  cadenceClicks++;
}
void cadenceBup() {
  cadenceSetDirection(false);
  cadenceClicks++;
}
// When cadence hall sensor deactivates.
void cadenceAdown() {
  digitalWrite(ledPin, LOW);
  if (!cadenceBisHigh) {
    cadenceClearDirection();
  }
}
void cadenceBdown() {
  if (!cadenceAisHigh) {
    cadenceClearDirection();
  }
}

// Establish a direction.
void cadenceSetDirection(boolean dir) {
  if(!cadenceValidDirection) {
    cadenceValidDirection = true;
    if (dir != cadenceDirection) {
      // Set correct direction.
      cadenceDirection = dir;
      // Reset cadence click counters.
      cadenceClicks = 0;
      // cadenceB = 0;
    }
  }
}

// When both cadence sensors go LOW invalidate direction.
void cadenceClearDirection() {
  cadenceValidDirection = false;
}

void cadenceCalcValue() {
  // Disable interrupt when calculating?
  cadenceRPM = cadenceClicks * cadenceMultiplier;
//  cadenceClicks = 0;
}

// THROTTLE

// Variable to keep the throttle value.
int throttleOutput = 0;
int throttleRamp = 100; // ms
// analogWrite values from 0 to 255
byte throttleMin = 1 * 50; // 1 volt.
byte throttleMax = 190; // 200 is 4 volts.

// CONSOLE

void debugLog() {
  if (cadenceDirection) {
    Serial.print("a:");
  }
  else {
    Serial.print("b:");
  }
  Serial.print(cadenceRPM);
  Serial.print(", ");
  Serial.println(torqNm);
}

