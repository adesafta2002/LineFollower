#include <QTRSensors.h>
const int m11Pin = 7;
const int m12Pin = 6;
const int m21Pin = 5;
const int m22Pin = 4;
const int m1Enable = 11;
const int m2Enable = 10;

int m1Speed = 0;
int m2Speed = 0;

float kp = 10;
float ki = 0;
float kd = 7;
int p = 1;
int i = 0;
int d = 0;

int error = 0;
int lastError = 0;

const int maxSpeed = 220;
const int minSpeed = -220;

const int baseSpeed = 220;

QTRSensors qtr;

const int sensorCount = 6;
int sensorValues[sensorCount];
int sensors[sensorCount] = { 0, 0, 0, 0, 0, 0 };

const byte neutral = 0;
const byte right = 1;
const byte left = 2;
byte currentDirection = neutral;

const byte calibrationMotorSpeeed = 200;
const byte calibrationMaxMoves = 10;
const int calibrationThreshold = 450;
const int calibrationDebounce = 225;
byte calibrationMoves = 0;

unsigned long lastDebounceTime = 0;

void setup() {
  initializePins();
}

void loop() {
  if (calibrationMoves < calibrationMaxMoves) {
    calibrate();
  } else {
    readErrorAndPid();

    int motorSpeed = kp * p + ki * i + kd * d;
    
    m1Speed = baseSpeed;
    m2Speed = baseSpeed;

    if (error < 0) {
      m1Speed += motorSpeed;
    }
    else if (error > 0) {
      m2Speed -= motorSpeed;
    }

    m1Speed = constrain(m1Speed, 0, maxSpeed);
    m2Speed = constrain(m2Speed, 0, maxSpeed);

    setMotorSpeed(m1Speed, m2Speed);
  }
}

void calibrate() {
  qtr.calibrate();

  if (millis() - lastDebounceTime > calibrationDebounce) {
    qtr.read(sensors);
  
    if (currentDirection == neutral) {
      turnLeft();
      calibrationMoves++;
      setMotorSpeed(m1Speed, m2Speed);
      lastDebounceTime = millis();
      return;
    }
    if (vehicleOnOneSide() && currentDirection == left) {
      turnRight();
      setMotorSpeed(m1Speed, m2Speed);
      calibrationMoves++;
      lastDebounceTime = millis();
    } else if (vehicleOnOneSide() && currentDirection == right) {
      turnLeft();
      setMotorSpeed(m1Speed, m2Speed);
      calibrationMoves++;
      lastDebounceTime = millis();
    }
  }

  if (calibrationMoves >= calibrationMaxMoves) {
    resetMotors();
  }
}

byte vehicleOnOneSide() {
  for(byte i = 0; i < sensorCount; i++) {
    if (sensors[i] >= calibrationThreshold) {
        return 0;
    }
  }
  return 1;
}

void resetMotors() {
  m1Speed = 0;
  m2Speed = 0;
  setMotorSpeed(m1Speed, m2Speed);
}

void turnLeft() {
  setFirstMotorSpeed(calibrationMotorSpeeed);
  setSecondMotorSpeed(-calibrationMotorSpeeed);
  currentDirection = left;
}

void turnRight() {
  setFirstMotorSpeed(-calibrationMotorSpeeed);
  setSecondMotorSpeed(calibrationMotorSpeeed);
  currentDirection = right;
}

void initializePins() {
  pinMode(m11Pin, OUTPUT);
  pinMode(m12Pin, OUTPUT);
  pinMode(m21Pin, OUTPUT);
  pinMode(m22Pin, OUTPUT);
  pinMode(m1Enable, OUTPUT);
  pinMode(m2Enable, OUTPUT);
  
  qtr.setTypeAnalog();
  qtr.setSensorPins((const uint8_t[]){A0, A1, A2, A3, A4, A5}, sensorCount);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // turn on Arduino's LED to indicate we are in calibration mode

  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(9600);
}

void readErrorAndPid() {
  error = map(qtr.readLineBlack(sensorValues), 0, 5000, -50, 50);
  p = error;
  i = i + error;
  d = error - lastError;
}

void setFirstMotorSpeed(int speed) {
  m1Speed = speed;
}

void setSecondMotorSpeed(int speed) {
  m2Speed = speed;
}

void setMotorSpeed(int motor1Speed, int motor2Speed) {

   motor2Speed = -motor2Speed;
  if (motor1Speed == 0) {
    digitalWrite(m11Pin, LOW);
    digitalWrite(m12Pin, LOW);
    analogWrite(m1Enable, motor1Speed);
  }
  else {
    if (motor1Speed > 0) {
      digitalWrite(m11Pin, HIGH);
      digitalWrite(m12Pin, LOW);
      analogWrite(m1Enable, motor1Speed);
    }
    if (motor1Speed < 0) {
      digitalWrite(m11Pin, LOW);
      digitalWrite(m12Pin, HIGH);
      analogWrite(m1Enable, -motor1Speed);
    }
  }
  if (motor2Speed == 0) {
    digitalWrite(m21Pin, LOW);
    digitalWrite(m22Pin, LOW);
    analogWrite(m2Enable, motor2Speed);
  }
  else {
    if (motor2Speed > 0) {
      digitalWrite(m21Pin, HIGH);
      digitalWrite(m22Pin, LOW);
      analogWrite(m2Enable, motor2Speed);
    }
    if (motor2Speed < 0) {
      digitalWrite(m21Pin, LOW);
      digitalWrite(m22Pin, HIGH);
      analogWrite(m2Enable, -motor2Speed);
    }
  }
}