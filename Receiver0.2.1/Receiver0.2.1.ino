#include "RF24.h"
#define CE_PIN 10       //==================================================
#define CSN_PIN 9

#define ds 0
#define shcp 1
#define stcp 2
#define motorpin 3
#define buzzer 7
#define pushbutton 8

bool level0_status = false;
bool level6_status = false;
int val[6][8] = { { 0, 0, 0, 1, 0, 0, 0, 0 },
                  { 0, 0, 0, 1, 1, 0, 0, 0 },
                  { 0, 0, 0, 1, 1, 1, 0, 0 },
                  { 0, 0, 0, 1, 1, 1, 1, 0 },
                  { 0, 0, 0, 1, 1, 1, 1, 1 },
                  { 0, 0, 0, 0, 0, 0, 0, 0 } };
int level;

void led(int j) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(ds, val[j][i]);
    digitalWrite(shcp, HIGH);
    digitalWrite(shcp, LOW);
  }
  digitalWrite(stcp, HIGH);
  digitalWrite(stcp, LOW);
}

void buzzr(int time){
  digitalWrite(buzzer,HIGH);
  delay(time);
  digitalWrite(buzzer,LOW);
}


RF24 radio(CE_PIN, CSN_PIN);
uint8_t address[][6] = { "1Node", "2Node" };
bool radioNumber = false;  // 0 uses address[0] to transmit, 1 uses address[1] to transmit
bool role = false;         // true = TX node, false = RX node
short frame[3];
int str_time;
int end_time;

void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(i, OUTPUT);
  }
  pinMode(7, OUTPUT);
  pinMode(8, INPUT);

  digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);
  delay(500);

  if (!radio.begin()) {
    while (1) {
    }
  }
  radio.setPALevel(RF24_PA_MAX);
  radio.setPayloadSize(sizeof(frame));
  radio.openWritingPipe(address[radioNumber]);      // always uses pipe 0
  radio.openReadingPipe(1, address[!radioNumber]);  // using pipe 1
  radio.startListening();                           // put radio in RX mode

  str_time=millis();
}  //setup





void loop() {
  end_time=millis();
  if(end_time-str_time>100000000000){
    while(!radio.available()){
      nosignal();
    }
  }
  if (radio.available()) {
    radio.read(&frame, sizeof(frame));
    str_time=end_time;
    indicator(frame[0]);
  }
  bool Switch = digitalRead(8);
  if (Switch) {
    digitalWrite(motorpin, HIGH);
    digitalWrite(buzzer, HIGH);
    delay(1000);
    digitalWrite(buzzer, LOW);
  }
}
void indicator(int level) {
  if (level == 11 && !level0_status) {
    led(5);
    level0_status = true;
    digitalWrite(motorpin, HIGH);  //level 6
    digitalWrite(buzzer, HIGH);    //level 5
    delay(600);                    //
    digitalWrite(buzzer, LOW);     //
    delay(1000);                   //
  } else if (level == 22) {         //level 4
    led(5);                        //
    led(0);                        //level 3
    
  } else if (level == 33) {         //
    led(5);                        //level 33
    led(1);                        //
  } else if (level == 44) {         //level 22
    led(5);                        //level 11
    led(2);
    level0_status = false;
  } else if (level == 55) {
    led(5);
    led(3);
    level6_status = false;
  } else if (level ==66 ) {
    led(5);
    led(4);
    
  } else if (level == 77 && !level6_status) {
    led(4);
    level6_status = true;
    digitalWrite(motorpin, LOW);
    delay(100);
    digitalWrite(buzzer, HIGH);
    delay(300);
    digitalWrite(buzzer, LOW);
    delay(1000);
  }
}



void nosignal(){
  buzzr(100);
  buzzr(300);
  led(4);
  delay(100);
  led(5);


}



void airflow(){
  buzzr(100);
  led(4);
  delay(100);
  led(5); 
}
