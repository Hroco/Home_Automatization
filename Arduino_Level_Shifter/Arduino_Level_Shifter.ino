//arduino act as voltage level shifter from 3V on input to 5V on output

const int analogInPin = A0;
const int analogInPin2 = A1;
const int digitalOutPin = 7;
const int digitalOutPin2 = 8;
int sensorValue = 0;
int sensorValue2 = 0; 

void setup() {
   pinMode(digitalOutPin, OUTPUT);
   pinMode(digitalOutPin2, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  sensorValue = analogRead(analogInPin);
  sensorValue2 = analogRead(analogInPin2);

  if(sensorValue == 0)
  {
    digitalWrite(digitalOutPin, HIGH);
    delay(600);
    digitalWrite(digitalOutPin, LOW);  
  }
  if(sensorValue2 == 0)
  {
    digitalWrite(digitalOutPin2, HIGH);
    delay(600);
    digitalWrite(digitalOutPin2, LOW);  
  }
  delay(2);
}
