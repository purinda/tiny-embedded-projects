/**
 * This sketch requires an ATTiny85 to run.
 * 
 * Purpose is for an LED (preferably white) to imitate candle flame effect.
 */
 

// Digital out pins
int ledPin1 = 0;
int ledPin2 = 1;

void setup() {
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
}

void loop() {
  analogWrite(ledPin1, random(120) + 135);
  analogWrite(ledPin2, random(120) + 135);
  delay(random(100));
}
