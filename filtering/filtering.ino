//#include <SPI.h>
#include <RF24.h>
#include <EEPROM.h>


#define TRIG_PIN     7  // Pin connected to the trigger pin of the ultrasonic sensor
#define ECHO_PIN     3  // Pin connected to the echo pin of the ultrasonic sensor

#define CE_PIN       10
#define CSN_PIN      9

int reg_address = 0;  // EEPROM address to store the variable

RF24 radio(CE_PIN, CSN_PIN);
uint8_t address[][6] = { "1Node", "2Node" };
bool radioNumber = 1;  // 0 uses address[0] to transmit, 1 uses address[1] to transmit

short frame[3];

float startTime = millis();         //Mark starting time
float maxDepth = 0;

// Kalman filter variables
float Q = 0.4; // Process noise covariance
float R = 6;  // Measurement noise covariance
float P = 1;   // Initial estimate error covariance
float K = 0;   // Kalman gain
float X = 0;   // Initial estimate


void setup() {
  pinMode(A1, INPUT);         //3.7 Vcc
  pinMode(A0, INPUT);         //solar
  pinMode(A2, INPUT);
  pinMode(9, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  if (!radio.begin()) {
    while (1) {
    }
  }
  radio.setPALevel(RF24_PA_MAX);
  radio.setPayloadSize(sizeof(frame));
  radio.openWritingPipe(address[radioNumber]);
  radio.openReadingPipe(1, address[!radioNumber]);
  radio.stopListening();

  //first time implementation
  




}

void loop() {
  // put your main code here, to run repeatedly:
  const int numReadings = 10;
  float distances[numReadings];
  float filteredDistances[numReadings];
  int filteredCount = 0;

  for (int i = 0; i < numReadings; i++) {
    distances[i] = MeasureDistance();
    delay(1000);  // Short delay between readings
  }

  X =distances[0];
  for (int j = 0; j < numReadings; j++) {
    float sensorValue = distances[j];
    // Kalman filter prediction update
    P = P + Q;

    // Kalman gain
    K = P / (P + R);

    // Measurement update
    X = X + K * (sensorValue - X);

    // Update error covariance
    P = (1 - K) * P;

    filteredDistances[j] = X;
  }


  int distance = filteredDistances[numReadings];

  //mapping the distance to water level
  int waterlevel = calculateWaterLevel(distance);

  frame[0] = waterlevel;
  frame[1] = analogRead(A0);  // solar
  frame[2] = analogRead(A1);  // 3.7 Vcc

  //transmit the frame
  transmit_frame(frame);

  bool waterlevel_increasing;

  if ((waterlevel >= 40) || (waterlevel <= 20)) {
    float slope = linearRegression(filteredDistances, numReadings);
    if (slope > 0.1) {
      waterlevel_increasing = true;  
    }else if (slope < -0.1){
      waterlevel_increasing = false;
    } else {
      String errorcode = "E";
      radio.openWritingPipe(0xF0F0F0F0E1LL); // Set the address to transmit to
      bool success = radio.write(&errorcode, sizeof(errorcode));
    } 
  }

  if ((waterlevel <= 20) && (waterlevel_increasing)) {
    delay(2000);  // Transmitting more frequently
  } else if ((waterlevel >=30) && (!waterlevel_increasing)) {
    delay(2000);  // Transmitting more frequently   
  } else {
    delay(10000);  // Transmitting less frequently
  }
}




float linearRegression(float arr[], int length) {
  if (length == 0) return 0;

  float xSum = 0, ySum = 0, xySum = 0, xxSum = 0;
  for (int i = 0; i < length; i++) {
    xSum += i;
    ySum += arr[i];
    xySum += i * arr[i];
    xxSum += i * i;
  }

  float slope = (length * xySum - xSum * ySum) / (length * xxSum - xSum * xSum);
  return slope;
}

int calculateWaterLevel(int distance) {
  // Assuming h is the maxDepth calibrated during setup
  int h = maxDepth; // Adjust this value based on your actual calibration result
  int x = 4  ; // deistance from sensor to level 0 (L0)
  int waterLevel = map(distance, 0, h-x, 0, 50);
  return constrain(waterLevel, 0, 50);
}

void transmit_frame(short frame){ 
  // Transmit the water level data
  //radio.openWritingPipe(0xF0F0F0F0E1LL); // Set the address to transmit to
  bool success = radio.write(&frame, sizeof(frame));
 
}

short MeasureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  float duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration * 0.034 / 2;
  return distance;
}

