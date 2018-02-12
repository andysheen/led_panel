//#define DEBUG


#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#ifdef DEBUG
#include <SoftwareSerial.h>
SoftwareSerial mySerial(11, 5); // RX, TX
#endif

#define NUM_OUTS  6

//Analog MUX setup
static uint8_t control_pin[4] = {10, 11, 12, 13};
#define SIG_PIN A8

//IR sensor setup
#define sensorThreshold 20
#define sampleInterval 30
#define sampleRate 10
unsigned long previousReadMillis;
static uint8_t channels[NUM_OUTS] = { A0, A1, A2, A3, A7, A8 };
unsigned short sensorBaseAverages[NUM_OUTS];
unsigned short sensorMax[NUM_OUTS];
unsigned short sensor[NUM_OUTS];
short readIn;

//PWM driver setup
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
#define lightInterval 5
unsigned long previousLightMillis;
byte sensorState[NUM_OUTS];
unsigned short lightFade[NUM_OUTS];


void setup() {
#ifdef DEBUG
  mySerial.begin(9600);
#endif
  setupPWM();
  calibrateSensors();
  test();
}

void loop() {
  unsigned long timeMillis = millis();

  //reading sensors
  if (timeMillis - previousReadMillis > sampleInterval) {
    previousReadMillis = timeMillis;
    readSensors();
    lightStates();
  }

  //lightOutputs
  if (timeMillis - previousLightMillis > lightInterval) {
    previousLightMillis = timeMillis;
    lightAnimation();
  }

}


void readSensors() {
  for (uint8_t chan = 0; chan < NUM_OUTS; chan++) {
    readIn = analogRead(channels[chan]);
    //set max for channel
    if (readIn > sensorMax[chan]) {
      sensorMax[chan] = readIn;
    }
    sensor[chan] = readIn;
#ifdef DEBUG
    mySerial.print(chan);
    mySerial.print(": ");
    mySerial.println(sensor[chan]);
#endif
  }

}

void calibrateSensors() {
  for (byte i = 0; i < NUM_OUTS; i++) {
    for (byte j = 0; j < sampleRate; j++) {
      sensorBaseAverages[i] = sensorBaseAverages[i] + analogRead(channels[i]);
      delay(20);
    }
    sensorBaseAverages[i] = (sensorBaseAverages[i] / 10) ;
    sensorMax[i] = sensorBaseAverages[i] + 400;
  }
}

void setupPWM() {
  delay(100);
  pwm.begin();
  pwm.setPWMFreq(1200);  // This is the maximum PWM frequency
  Wire.setClock(100000);
}

void lightStates() {
  for (byte i = 0; i < NUM_OUTS; i++) {
    //sensor touching
    // if (sensor[i] > sensorBaseAverages[i] + sensorThreshold && sensorState[i] == 0) {
    if (sensor[i] > sensorBaseAverages[i] + (sensorBaseAverages[i] / 10) && sensorState[i] == 0) {
      sensorState[i] = 1;
      //map the sensor value from base to
      // lightFade[i] = map(sensor[i], sensorBaseAverages[i], sensorMax[i], 0, 4095);
      lightFade[i] = 2000;
    }
    //sensor touched
    // if (sensor[i] < sensorBaseAverages[i] + sensorThreshold && sensorState[i] == 1) {
    if (sensor[i] < sensorBaseAverages[i] + (sensorBaseAverages[i] / 10) && sensorState[i] == 1) {
      sensorState[i] = 2;
    }
  }
}

void lightAnimation() {
  /* MORE WORK NEEDED TO MAKE LOGARHYTHMIC ***/
  for (byte i = 0; i < NUM_OUTS; i++) {
    switch (sensorState[i]) {
      case 0:
        //light not touched
        pwm.setPin(i * 2, 0, false);
        pwm.setPin(i * 2 + 1 , 0, false);
        break;
      case 1:
        //light touching
        pwm.setPin(i * 2 , lightFade[i], false);
        pwm.setPin(i * 2 + 1, lightFade[i], false);
        break;
      case 2:
        //begin light animations
        if (lightFade[i] > 19) {
          lightFade[i] = lightFade[i] - 20;
          pwm.setPin(i * 2 + 1 , lightFade[i], false);
          pwm.setPin(i * 2, lightFade[i], false);
        }
        if (lightFade[i] < 20) {
          lightFade[i] = 0;
          sensorState[i] = 0;
          pwm.setPin(i * 2 + 1 , 0, false);
          pwm.setPin(i * 2, 0, false);
        }
        break;
    }
  }

}

void test() {
  delay(50);
  for (byte i = 0 ; i < 12; i++) {
    pwm.setPin(i, 500, false);
    delay(100);
  }
  for (byte i = 0 ; i < 12; i++) {
    pwm.setPin(i, 0, false);
    delay(100);
  }
  pwm.setPin(0, 0, false);
}

void setupMUX() {
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(control_pin[i], OUTPUT);
  }
  pinMode(SIG_PIN, INPUT);
}

void setChannel(int8_t pin) {
  for (uint8_t i = 0; i < 4; ++i)
  {
    digitalWrite(control_pin[i], pin & 0x01);
    pin >>= 1;
  }

}

