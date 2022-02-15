const int sensorPin = A0;
const int led = 2;

void setup()
{
  pinMode(led,OUTPUT);
  pinMode(sensorPin,INPUT);
  Serial.begin(9600);
}

void loop()
{
  int reading = analogRead(sensorPin);
  Serial.println("light : ");
  Serial.println(reading);
  delay(10);
  if(reading<400)
  {
    digitalWrite(led, HIGH);
  }
  if(reading>400)
  {
    digitalWrite(led, LOW);
  }
}
