const int trigPin = 22;  //초음파 센서 trig핀번호
const int echoPin = 23;  //초음파 센서 echo핀번호

void setup() 
{
  Serial.begin (9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() 
{
  long duration, distance;  // 시간, 거리      
  digitalWrite(trigPin, LOW);        
  delayMicroseconds(2);              
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);  // trigPin을 10마이크로초 동안 HIGH
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;
  // 2m가 넘어가거나 값이 0 이하
  if (distance >= 200 || distance <= 0)
  {
    Serial.println("거리를 측정할 수 없음");
  }
  else 
  {
    Serial.print(distance);
    Serial.println(" cm");
  }
  delay(500);
}
