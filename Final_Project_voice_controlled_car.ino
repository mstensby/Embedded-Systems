#include <SoftwareSerial.h>
#include "VoiceRecognitionV3.h"
#include <AFMotor.h>

#define MOTOR1RATE 1
#define MOTOR2RATE 1
#define MOTOR3RATE 2
#define MOTOR4RATE 2
#define DC_MOTOR_PWM_RATE 1000

AF_DCMotor motor1(1, MOTOR1RATE);
AF_DCMotor motor2(2, MOTOR1RATE);
AF_DCMotor motor3(3, MOTOR3RATE);
AF_DCMotor motor4(4, MOTOR3RATE);

/**        
  Connection
  Arduino    VoiceRecognitionModule
   2   ------->     TX
   13   ------->     RX
*/
VR myVR(2,13);    // 2:TX 13:RX, you can choose your favourite pins.

uint8_t records[7]; // save record
uint8_t buf[64];

uint8_t speedMeasurement = 0; // Variable to store motor speed
uint8_t mode = 0;

#define forward    (0)
#define stop       (1)
#define left       (2)
#define right      (3)
#define backward   (4)
#define forward1   (5)

void printVR(uint8_t *buf);
void printSignature(uint8_t *buf, int len);

void moveForward(){
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}

void moveRight() {
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}

void moveLeft() {
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
}

void moveBackward() {
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
}

void stopCar() {
  // Stop all motors
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
}

void setSpeed(){
  motor1.setSpeed(speedMeasurement);
  motor2.setSpeed(speedMeasurement);
  motor3.setSpeed(speedMeasurement);
  motor4.setSpeed(speedMeasurement);
}

void setMaxSpeed(){
  motor1.setSpeed(250);
  motor2.setSpeed(250);
  motor3.setSpeed(250);
  motor4.setSpeed(250);
}

void ADC_init() {
    // Set ADC reference voltage to AVCC with external capacitor at AREF pin
    ADMUX |= (1 << REFS0);

    // Set ADC prescaler to 128 (F_CPU / 128)
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // Enable ADC
    ADCSRA |= (1 << ADEN);
}

// Read ADC value from specified channel
uint16_t ADC_read(uint8_t channel) {
    // Set ADC channel (0-7 for Arduino Uno)
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);

    // Start ADC conversion
    ADCSRA |= (1 << ADSC);

    // Wait for conversion to complete
    while (ADCSRA & (1 << ADSC));

    // Return ADC value
    return ADC;
}

void readPotentiometer() {
  // Read ADC value from pin A0 or PC0
  uint16_t adc_value = ADC_read(0);

  // Convert ADC value to voltage (0-5V)
  float voltage = adc_value * (5.0 / 1023.0);

  // Convert voltage to motor speed (0-250)
  speedMeasurement = voltage * 50;

}

void checkMode() {
  if (!digitalRead(0)) { // Check if button is pressed (active low)
    while (!digitalRead(0));// Wait for button release
    mode = !mode; // Toggle mode
  }

  if(mode == 0){
    digitalWrite(10, LOW);
  }
  else if(mode == 1){
    digitalWrite(10, HIGH);
  }
}


void setup() {

  myVR.begin(9600);
  
  // Serial.begin(115200);
  // Serial.println("Elechouse Voice Recognition V3 Module\r\nControl LED sample");

  if(myVR.clear() == 0){
    // Serial.println("Recognizer cleared.");
  }else{
    // Serial.println("Not find VoiceRecognitionModule.");
    // Serial.println("Please check connection and restart Arduino.");
    while(1);
  }
  
  // if(myVR.load((uint8_t)forward) >= 0){
  //   Serial.println("forward loaded");
  // }

  // if(myVR.load((uint8_t)stop) >= 0){
  //   Serial.println("stop loaded");
  // }

  // if(myVR.load((uint8_t)left) >= 0){
  //   Serial.println("left loaded");
  // }

  // if(myVR.load((uint8_t)right) >= 0){
  //   Serial.println("right loaded");
  // }

  // if(myVR.load((uint8_t)backward) >= 0){
  //   Serial.println("backward loaded");
  // }

  // if(myVR.load((uint8_t)forward1) >= 0){
  //   Serial.println("forward1 loaded");
  // }

  myVR.load((uint8_t)forward);
  myVR.load((uint8_t)stop);
  myVR.load((uint8_t)left);
  myVR.load((uint8_t)right);
  myVR.load((uint8_t)backward);
  myVR.load((uint8_t)forward1);
  


  // put your setup code here, to run once:
  ADC_init();

  //Set Button to input
  pinMode(0,INPUT);

  //Set LED to output, initially low
  pinMode(10, OUTPUT);
  digitalWrite(10,LOW);

}

// void loop() {
//   //checkMode();
//   readPotentiometer();
//   setSpeed();
//   moveForward();
// }

void loop()
{
  checkMode();
  readPotentiometer();
  setSpeed();
  int ret;
  ret = myVR.recognize(buf, 50);
  if(ret > 0){
    switch(buf[1]) {
      if(mode == 0){
        case forward:
        case forward1:
          if(mode == 0){
            moveForward();
            delay(2000);
            stopCar();
            break;
          }
          else if(mode == 1){
            moveForward();
            break;
          }
        case stop:
          stopCar();
          break;
        case left:
          if(mode == 0){
            moveLeft();
            delay(2000);
            stopCar();
            break;
          }
          else if(mode == 1){
            moveLeft();
            break;
          }
        case right:
          if(mode == 0){
            moveRight();
            delay(2000);
            stopCar();
            break;
          }
          else if(mode == 1){
            moveRight();
            break;
          }
        case backward:
          if(mode == 0){
            moveBackward();
            delay(2000);
            stopCar();
            break;
          }
          else if(mode == 1){
            moveBackward();
            break;
          }
        default:
          Serial.println("Record function undefined");
          break;
      }
    }
    printVR(buf);
  }
}

/**
  @brief   Print signature, if the character is invisible, 
           print hexible value instead.
  @param   buf     --> command length
           len     --> number of parameters
*/
void printSignature(uint8_t *buf, int len)
{
  int i;
  for(i=0; i<len; i++){
    if(buf[i]>0x19 && buf[i]<0x7F){
      Serial.write(buf[i]);
    }
    else{
      Serial.print("[");
      Serial.print(buf[i], HEX);
      Serial.print("]");
    }
  }
}

/**
  @brief   Print signature, if the character is invisible, 
           print hexible value instead.
  @param   buf  -->  VR module return value when voice is recognized.
             buf[0]  -->  Group mode(FF: None Group, 0x8n: User, 0x0n:System
             buf[1]  -->  number of record which is recognized. 
             buf[2]  -->  Recognizer index(position) value of the recognized record.
             buf[3]  -->  Signature length
             buf[4]~buf[n] --> Signature
*/
void printVR(uint8_t *buf)
{
  Serial.println("VR Index\tGroup\tRecordNum\tSignature");

  Serial.print(buf[2], DEC);
  Serial.print("\t\t");

  if(buf[0] == 0xFF){
    Serial.print("NONE");
  }
  else if(buf[0]&0x80){
    Serial.print("UG ");
    Serial.print(buf[0]&(~0x80), DEC);
  }
  else{
    Serial.print("SG ");
    Serial.print(buf[0], DEC);
  }
  Serial.print("\t");

  Serial.print(buf[1], DEC);
  Serial.print("\t\t");
  if(buf[3]>0){
    printSignature(buf+4, buf[3]);
  }
  else{
    Serial.print("NONE");
  }
  Serial.println("\r\n");
}

