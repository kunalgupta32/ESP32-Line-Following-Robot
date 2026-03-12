#include <BluetoothSerial.h>
#include <QTRSensors.h>

const int ENA = 13; const int IN1 = 26; const int IN2 = 27; 
const int ENB = 25; const int IN3 = 33; const int IN4 = 32; 

QTRSensors qtr;
BluetoothSerial SerialBT;

const uint8_t SensorCount = 8;
uint16_t sensorValues[SensorCount];

// --- CORE SETTINGS (Independent of BT) ---
bool isAutoMode = true;   // Starts in AUTO by default
int baseSpeed = 100;      // Set your preferred race speed here
float Kp = 0.34;          
float Kd = 8.00;          
int threshold = 850;      
int lastError = 0;
unsigned long lastDebug = 0;
unsigned long lostTime = 0;

void setMotors(int left, int right) {
  if (left == 0 && right == 0) {
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
    ledcWrite(ENA, 0); ledcWrite(ENB, 0);
    return;
  }
  digitalWrite(IN1, left >= 0 ? HIGH : LOW);
  digitalWrite(IN2, left >= 0 ? LOW : HIGH);
  digitalWrite(IN3, right >= 0 ? HIGH : LOW);
  digitalWrite(IN4, right >= 0 ? LOW : HIGH);
  
  int pL = abs(left);
  int pR = abs(right);

  // Deadzone compensation
  if (baseSpeed < 100) {
    if (pL > 0 && pL < 115) pL = 115;
    if (pR > 0 && pR < 115) pR = 115;
  }
  ledcWrite(ENA, constrain(pL, 0, 255)); 
  ledcWrite(ENB, constrain(pR, 0, 255));
}

void runCalibration() {
  for (uint16_t i = 0; i < 600; i++) { 
    if (i < 300) setMotors(150, -150); 
    else setMotors(-150, 150);
    qtr.calibrate();
    delay(5); 
  }
  setMotors(0, 0);
}

void setup() {
  Serial.begin(115200);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  ledcAttach(ENA, 1000, 8); ledcAttach(ENB, 1000, 8);

  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){4, 23, 2, 5, 18, 19, 21, 22}, SensorCount);

  // Bluetooth starts, but the robot doesn't WAIT for it
  SerialBT.begin("SantiBot_Independent"); 
  
  delay(1000); 
  runCalibration(); 
  delay(1000); // 1 second to place it on the line after calibration
}

void loop() {
  // --- SECTION 1: BLUETOOTH (Background Task) ---
  if (SerialBT.available()) {
    char cmd = SerialBT.read();
    if (cmd == 'W') isAutoMode = false;
    if (cmd == 'U') isAutoMode = true;
    if (cmd == 'S') { isAutoMode = false; setMotors(0, 0); } // Emergency Stop
    
    // Tuning still works via BT if connected
    if (cmd == 'a') baseSpeed = 60;
    if (cmd == 'b') baseSpeed = 100;
    if (cmd == 'c') baseSpeed = 160;
    if (cmd == 'V') baseSpeed = 250;
    if (cmd == 'K') Kp += 0.01;
    if (cmd == 'k') Kp -= 0.01;
    if (cmd == 'D') Kd += 0.2;
    if (cmd == 'd') Kd -= 0.2;
  }

  // --- SECTION 2: INDEPENDENT LOGIC (Always Running) ---
  uint16_t position = qtr.readLineBlack(sensorValues);

  if (isAutoMode) {
    int blackCounter = 0;
    for (int i = 0; i < SensorCount; i++) {
      if (sensorValues[i] > threshold) blackCounter++; 
    }

    if (blackCounter >= 1) { 
      lostTime = 0;
      int error = (int)position - 3500;
      if (abs(error) < 100) error = 0; 
      
      int adjustment = (Kp * error) + (Kd * (error - lastError));
      lastError = error;
      setMotors(baseSpeed - adjustment, baseSpeed + adjustment);
    } 
    else {
      // Line Loss recovery
      if (lostTime == 0) lostTime = millis();
      if (millis() - lostTime > 700) { 
        setMotors(0, 0); 
      } else {
        if (lastError > 0) setMotors(140, -140);
        else setMotors(-140, 140);
      }
    }
  }

  // Debugging (Only sends if BT is connected, doesn't stop the robot if not)
  if (millis() - lastDebug > 1000) {
    if (SerialBT.hasClient()) {
      SerialBT.print("POS: "); SerialBT.println(position);
    }
    lastDebug = millis();
  }
}