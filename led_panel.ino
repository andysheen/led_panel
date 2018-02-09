#include <Wire.h>
#include <attiny_pwm.h>

#ifdef DEBUG
#include <SoftwareSerial.h>
SoftwareSerial mySerial(11, 5); // RX, TX
#endif

#define NUM_OUTS  6


//IR sensor setup
#define sensorThreshold 10
#define sampleInterval 30
#define sampleRate 10
unsigned long previousReadMillis;
static byte channels[NUM_OUTS] = { A0, A1, A2, A3, A7, A8 };
unsigned short sensorBaseAverages[NUM_OUTS];
unsigned short sensorMax[NUM_OUTS];
unsigned short sensor[NUM_OUTS];
short readIn;

//PWM driver setup
attiny_pwm pwm = attiny_pwm();
#define lightInterval 30
unsigned long previousLightMillis;
byte sensorState[NUM_OUTS];
unsigned short lightFade[NUM_OUTS];


void setup() {
#ifdef DEBUG
  mySerial.begin(9600);
#endif
  setupPWM();

  calibrateSensors();
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
    //map the read in from 0 - 1023 to 0 - 4095
    sensor[chan] = readIn * 4;
  }
}

void calibrateSensors() {
  for (byte i = 0; i < NUM_OUTS; i++) {
    for (byte j = 0; j < sampleRate; j++) {
      sensorBaseAverages[i] = sensorBaseAverages[i] + analogRead(channels[i]);
      delay(20);
    }
    sensorBaseAverages[i] = (sensorBaseAverages[i] / 10) ;
    sensorMax[i] = 900;
  }
}

void setupPWM() {
  pwm.begin();
  pwm.setPWMFreq(1600);  // This is the maximum PWM frequency
  Wire.setClock(400000);
}

void lightStates() {
  for (byte i = 0; i < NUM_OUTS; i++) {
    //sensor touching
    if (sensor[i] > sensorBaseAverages[i] + sensorThreshold && sensorState[i] == 0) {
      sensorState[i] = 1;
      //map the sensor value from base to
      lightFade[i] = map(sensor[i], sensorBaseAverages[i], sensorMax[i], 0, 4095);
    }
    //sensor touched
    if (sensor[i] < sensorBaseAverages[i] + sensorThreshold && sensorState[i] == 1) {
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
        pwm.setPin(i, 0, false);
        break;
      case 1:
        //light touching
        pwm.setPin(i, lightFade[i], false);
        break;
      case 2:
        //begin light animations
        if (lightFade[i] > 0) {
          lightFade[i]--;
        }
        if (lightFade[i] == 0) {
          sensorState[i] = 0;
        }
        pwm.setPin(i, lightFade[i], false);
        break;
    }
  }

}

