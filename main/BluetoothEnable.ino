#include <BluetoothSerial.h>
#include <QTRSensors.h>

// --- PIN DEFINITIONS ---
const int ENA = 13; const int IN1 = 26; const int IN2 = 27; 
const int ENB = 25; const int IN3 = 33; const int IN4 = 32; 

QTRSensors qtr;
BluetoothSerial SerialBT;

const uint8_t SensorCount = 8;
uint16_t sensorValues[SensorCount];

// --- PID & CONTROL VARIABLES ---
bool isAutoMode = true;
int baseSpeed = 60;       
float Kp = 0.34;          // Fixed Base Kp
float Kd = 8.00;          // Fixed Base Kd
int threshold = 850;      
int lastError = 0;
unsigned long lastDebug = 0;
unsigned long lostTime = 0;

void printStatus(String event) {
  SerialBT.println("\n--- " + event + " ---");
  SerialBT.print("SPD: "); SerialBT.print(baseSpeed);
  SerialBT.print(" | Kp: "); SerialBT.print(Kp, 3);
  SerialBT.print(" | Kd: "); SerialBT.print(Kd, 2);
  SerialBT.println("\n-------------------------");
}

// --- 3-SECOND 360 DEGREE CALIBRATION ---
void runCalibration() {
  isAutoMode = false;
  printStatus("CALIBRATING (3 SEC)");
  // 600 loops * 5ms = 3 seconds
  for (uint16_t i = 0; i < 600; i++) { 
    if (i < 300) setMotors(150, -150); // Spin Right
    else setMotors(-150, 150);        // Spin Left
    qtr.calibrate();
    delay(5); 
  }
  setMotors(0, 0);
  printStatus("READY TO RUN");
}

void setMotors(int left, int right) {
  if (left == 0 && right == 0) {
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
    ledcWrite(ENA, 0); ledcWrite(ENB, 0);
    return;
  }
  // Left Motor Direction
  digitalWrite(IN1, left >= 0 ? HIGH : LOW);
  digitalWrite(IN2, left >= 0 ? LOW : HIGH);
  // Right Motor Direction
  digitalWrite(IN3, right >= 0 ? HIGH : LOW);
  digitalWrite(IN4, right >= 0 ? LOW : HIGH);
  
  int pL = abs(left);
  int pR = abs(right);

  // Deadzone compensation for low battery/low speed
  if (baseSpeed < 100) {
    if (pL > 0 && pL < 115) pL = 115;
    if (pR > 0 && pR < 115) pR = 115;
  }

  ledcWrite(ENA, constrain(pL, 0, 255)); 
  ledcWrite(ENB, constrain(pR, 0, 255));
}

void setup() {
  Serial.begin(115200);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  
  // ESP32 PWM Setup
  ledcAttach(ENA, 1000, 8); 
  ledcAttach(ENB, 1000, 8);

  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){4, 23, 2, 5, 18, 19, 21, 22}, SensorCount);

  SerialBT.begin("SantiBot_Final_v1"); 
  delay(2000); 
  runCalibration(); 
}

void loop() {
  // Check Bluetooth Commands
  if (SerialBT.available()) {
    char cmd = SerialBT.read();
    
    if (cmd == 'W') { isAutoMode = false; setMotors(0, 0); printStatus("MANUAL"); }
    if (cmd == 'U') { isAutoMode = true; lastError = 0; printStatus("AUTO"); }
    if (cmd == 'C') { runCalibration(); } 

    // Speed Presets (Kp/Kd remain untouched)
    if (cmd == 'a') { baseSpeed = 60;  printStatus("S:60"); }
    if (cmd == 'b') { baseSpeed = 100; printStatus("S:100"); }
    if (cmd == 'c') { baseSpeed = 160; printStatus("S:160"); }
    if (cmd == 'V') { baseSpeed = 250; printStatus("S:MAX"); }

    // Manual PID Tuning
    if (cmd == 'K') { Kp += 0.01; printStatus("Kp+"); }
    if (cmd == 'k') { Kp -= 0.01; printStatus("Kp-"); }
    if (cmd == 'D') { Kd += 0.2;  printStatus("Kd+"); }
    if (cmd == 'd') { Kd -= 0.2;  printStatus("Kd-"); }

    // Manual Drive (Only in Manual Mode)
    if (!isAutoMode) {
      if (cmd == 'F') setMotors(baseSpeed, baseSpeed);   
      if (cmd == 'B') setMotors(-baseSpeed, -baseSpeed); 
      if (cmd == 'L') setMotors(-baseSpeed, baseSpeed);  
      if (cmd == 'R') setMotors(baseSpeed, -baseSpeed);  
      if (cmd == 'S') setMotors(0, 0);                   
    }
  }

  // --- CORE LINE FOLLOWING ---
  uint16_t position = qtr.readLineBlack(sensorValues);
  
  // Reduced Debug Frequency for BT Stability
  if (millis() - lastDebug > 1000) {
    SerialBT.print("POS: "); SerialBT.println(position);
    lastDebug = millis();
  }

  if (isAutoMode) {
    int blackCounter = 0;
    for (int i = 0; i < SensorCount; i++) {
      if (sensorValues[i] > threshold) blackCounter++; 
    }

    if (blackCounter >= 1) { 
      lostTime = 0;
      int error = (int)position - 3500;
      
      // Smoothing Deadzone
      if (abs(error) < 100) error = 0; 
      
      // PD Calculation
      int adjustment = (Kp * error) + (Kd * (error - lastError));
      lastError = error;
      
      setMotors(baseSpeed - adjustment, baseSpeed + adjustment);
    } 
    else {
      // Logic for line loss
      if (lostTime == 0) lostTime = millis();
      if (millis() - lostTime > 700) { 
        setMotors(0, 0); 
      } else {
        // Recovery spin
        if (lastError > 0) setMotors(130, -130);
        else setMotors(-130, 130);
      }
    }
  }
  
  delay(1); // Small delay helps Bluetooth chip background tasks
}