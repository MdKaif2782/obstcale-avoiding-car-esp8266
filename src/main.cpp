#include<Arduino.h>
#define TRIG_PIN 16
#define ECHO_PIN 10

// Motor control pins
#define LEFT_FORWARD 13
#define LEFT_BACKWARD 4
#define RIGHT_FORWARD 14
#define RIGHT_BACKWARD 12
#define LEFT_MOTOR 5
#define RIGHT_MOTOR 2


// Calibration constants
int MOTOR_SPEED = 150;           // Base speed (0-255)
const int TURN_DURATION = 50;         // ms for 90° turn
const int BRAKE_DURATION = 15;         // ms for left motor braking
const float FWD_LEFT_MULT = 0.95;      // Forward left motor multiplier
const float BCK_RIGHT_MULT = 0.77;     // Backward right motor multiplier
const float SAFE_DISTANCE = 120;
const float WARNING_DISTANCE = 80;
const float CRITICAL_DISTANCE = 40;

// Function prototypes
void moveForward(int duration);
void moveBackward(int duration);
void turnRight();
void stopMotors();
void applyBrake(bool wasMovingForward);
float readDistance();
void moveForward();

void setup() {
  // Setup motor pins
  pinMode(LEFT_FORWARD, OUTPUT);
  pinMode(LEFT_BACKWARD, OUTPUT);
  pinMode(RIGHT_FORWARD, OUTPUT);
  pinMode(RIGHT_BACKWARD, OUTPUT);
  pinMode(LEFT_MOTOR, OUTPUT);
  pinMode(RIGHT_MOTOR, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT); // Sets the trigPin as an Output
  pinMode(ECHO_PIN, INPUT); // Sets the echoPin as an Input
  Serial.begin(9600); // Starts the serial communication
  stopMotors();
}

void loop() {
  float distance = readDistance();

  if(distance<=CRITICAL_DISTANCE) {
    MOTOR_SPEED = 150;
    moveBackward(70);
    MOTOR_SPEED = 100;
    turnRight();
  } else if(distance>CRITICAL_DISTANCE && distance<WARNING_DISTANCE) {
    MOTOR_SPEED = 100;
    moveForward();
  } else {
    if(distance>SAFE_DISTANCE) MOTOR_SPEED = 255;
    else MOTOR_SPEED = 255*(distance)/SAFE_DISTANCE;
    moveForward();
  }
}

// Motor control functions
void moveForward(int duration) {
  // Set motor directions
  digitalWrite(LEFT_FORWARD, HIGH);
  digitalWrite(LEFT_BACKWARD, LOW);
  digitalWrite(RIGHT_FORWARD, HIGH);
  digitalWrite(RIGHT_BACKWARD, LOW);

  // Apply calibrated speeds
  analogWrite(LEFT_MOTOR, MOTOR_SPEED * FWD_LEFT_MULT);
  analogWrite(RIGHT_MOTOR, MOTOR_SPEED);

  // Run for specified duration
  if(duration > 0) {
    delay(duration);
    applyBrake(true);  // Apply brake for forward motion
  }
}

void moveForward() {
  // Set motor directions
  digitalWrite(LEFT_FORWARD, HIGH);
  digitalWrite(LEFT_BACKWARD, LOW);
  digitalWrite(RIGHT_FORWARD, HIGH);
  digitalWrite(RIGHT_BACKWARD, LOW);

  // Apply calibrated speeds
  analogWrite(LEFT_MOTOR, MOTOR_SPEED * FWD_LEFT_MULT);
  analogWrite(RIGHT_MOTOR, MOTOR_SPEED);
}

void moveBackward(int duration) {
  // Set motor directions
  digitalWrite(LEFT_FORWARD, LOW);
  digitalWrite(LEFT_BACKWARD, HIGH);
  digitalWrite(RIGHT_FORWARD, LOW);
  digitalWrite(RIGHT_BACKWARD, HIGH);

  // Apply calibrated speeds
  analogWrite(LEFT_MOTOR, MOTOR_SPEED);
  analogWrite(RIGHT_MOTOR, MOTOR_SPEED * BCK_RIGHT_MULT);

  // Run for specified duration
  if(duration > 0) {
    delay(duration);
    applyBrake(false);  // Apply brake for backward motion
  }
}

void turnRight() {
  // Set motor directions for right turn
  digitalWrite(LEFT_FORWARD, HIGH);
  digitalWrite(LEFT_BACKWARD, LOW);
  digitalWrite(RIGHT_FORWARD, LOW);
  digitalWrite(RIGHT_BACKWARD, HIGH);

  // Apply full speed for turning
  analogWrite(LEFT_MOTOR, MOTOR_SPEED);
  analogWrite(RIGHT_MOTOR, MOTOR_SPEED);

  // Turn for calibrated duration
  delay(TURN_DURATION);
  stopMotors();
}

void applyBrake(bool wasMovingForward) {
  // Only brake the left motor in the opposite direction
  if(wasMovingForward) {
    // Apply reverse to left motor
    digitalWrite(LEFT_FORWARD, LOW);
    digitalWrite(LEFT_BACKWARD, HIGH);
    analogWrite(LEFT_MOTOR, MOTOR_SPEED);
  } else {
    // Apply forward to left motor
    digitalWrite(LEFT_FORWARD, HIGH);
    digitalWrite(LEFT_BACKWARD, LOW);
    analogWrite(LEFT_MOTOR, MOTOR_SPEED);
  }

  // Brake duration
  delay(BRAKE_DURATION);

  // Complete stop
  stopMotors();
}

void stopMotors() {
  // Stop all motors
  digitalWrite(LEFT_FORWARD, LOW);
  digitalWrite(LEFT_BACKWARD, LOW);
  digitalWrite(RIGHT_FORWARD, LOW);
  digitalWrite(RIGHT_BACKWARD, LOW);
  analogWrite(LEFT_MOTOR, 0);
  analogWrite(RIGHT_MOTOR, 0);
}

float readDistance() {
  const int sampleInterval = 10; // Time between samples in ms
  const int totalDuration = 50;  // Total sampling window in ms
  const int numSamples = totalDuration / sampleInterval;

  float totalDistance = 0;

  for (int i = 0; i < numSamples; i++) {
    // Trigger ultrasonic pulse
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Measure echo time
    int duration = pulseIn(ECHO_PIN, HIGH);
    float distance = duration * 0.034 / 2;

    totalDistance += distance;

    delay(sampleInterval); // Wait before next sample
  }

  float averageDistance = totalDistance / numSamples;

  Serial.print("Filtered Distance: ");
  Serial.println(averageDistance);

  return averageDistance;
}
