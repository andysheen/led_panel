
int mean = 70;
int currVal;
int startVal;
boolean isOn = false;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  calcMean();
}

void loop() {
  // put your main code here, to run repeatedly:
  calcIntensity();
  delay(10);
}

void calcIntensity() {
  currVal = analogRead(0);
  if (currVal > mean + 5 && isOn == false) {
    isOn = true;
    startVal = currVal;
    Serial.println("HAND DETECTED");
  }
  if (isOn == true && currVal < mean + 2) {
    isOn = false;
    Serial.println("HAND REMOVED");
  }

if(isOn){
  Serial.print("intensity: ");
  Serial.println(map(currVal,startVal,startVal+200,0,100));
}

}
void calcMean() {
  mean = 0;
  for ( int i = 0 ; i < 40; i ++) {
    mean = analogRead(0) + mean;
  }
  mean = mean / 40;
}

