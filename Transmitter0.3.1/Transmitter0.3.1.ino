#include "RF24.h"
#include <EEPROM.h>

#define TRIG_PIN 7
#define ECHO_PIN 3
#define CE_PIN 10
#define CSN_PIN 9
#define data_len 20

RF24 radio(CE_PIN, CSN_PIN);
uint8_t address[][6] = { "1Node", "2Node" };
bool radioNumber = 1;  // 0 uses address[0] to transmit, 1 uses address[1] to transmit
short frame[3];
int test1[234];
int a=2;
int duration, distance;
int delay_time = 60000;
bool is_waterlevel_increasing = false;
short height, tem_distance;
short maxheight = 15;
short level[6] = { 6, 6, 6, 6, 6, maxheight };  // max water level remains 6cm below the sensor
short lvl, val;
short valid_data[data_len];  // get water level every 3 min

// Kalman filter variables
float Q = 0.4;  // Process noise covariance
float R = 6;    // Measurement noise covariance
float P = 1;    // Initial estimate error covariance
float K = 0;    // Kalman gain
float X = 0;    // Initial estimate

short MeasureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;
  return distance;
}
short filter() {
  float sensorValue = test1[a];   // Kalman filter prediction update
  P = P + Q;                      // Kalman gain
  K = P / (P + R);                // Measurement update
  X = X + K * (sensorValue - X);  // Update error covariance
  P = (1 - K) * P;
  val = P;
  return val;
}
void transmit_data(int data) {
}

bool StatusOfWaterLevel() {
  byte inc = 0;
  byte dec = 0;
  for (byte k = 0; k < data_len - 1; k++) {
    if (valid_data[k] < valid_data[k + 1]) {  // check weather water level is decreasing by using 'valid data' set
      inc++;
    } else if (valid_data[k] > valid_data[k + 1]) {
      dec++;
    }
  }
  for (byte i = 1; i < data_len; i++) {
    valid_data[i - 1] = valid_data[i];
  }
  if (inc > data_len * 0.8) { return false; }  // if water level is decreasing, false will return
  return true;
}


void setup() {
  pinMode(A1, INPUT);
  pinMode(A0, INPUT);
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
  X = 34;
  valid_data[0] = X;
  for (byte t = 1; t < data_len; t++) {
    valid_data[t] = filter();  // to intialize valid_data to measure slop
  }
  while (1) {
    tem_distance = filter();  //filter
    if (tem_distance > maxheight || maxheight > 1000) {
      maxheight = tem_distance;
    } else if (is_waterlevel_increasing) {
      maxheight = tem_distance - 15;  // reduce 15cm for filling water tank before it gets below critical level
      //EEPROM.put(reg_address, maxheight);
      delay(100);
      byte gap;
      gap = (maxheight - 6) / 5;
      for (byte d = 1; d < 5; d++) {
        level[d] = 6 + (gap * d);
      }
      break;
    }
  }
}  // setup()
void loop() {
  distance = MeasureDistance();
  delay(200);
  if (distance > level[4]) {
    lvl = 77;
  } else if (distance >= 0 && distance < level[0]) {
    lvl = 11;
  } else if (distance >= level[0] && distance < level[4]) {
    for (byte i; i < 5; i++) {
      if (distance >= level[i] && distance < level[i + 1]) {
        lvl = 22 + 11 * i;
      }
    }
  }


  frame[0] = lvl;
  frame[1] = distance;
  frame[2] = analogRead(A0);  // solar
  delay(50);
  // frame[2] = analogRead(A1);  // 3.7 Vcc
  // delay(50);
  bool report = radio.write(&frame, sizeof(frame));
  delay(300);
}  // loop
