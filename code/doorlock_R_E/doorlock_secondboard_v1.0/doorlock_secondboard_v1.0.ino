#include <SPI.h>  //SPI통신 라이브러리
#include <MFRC522.h>  //RFID라이브러리

#include<SoftwareSerial.h>  //BT라이브러리
SoftwareSerial BTSerial(2, 3);  //BT에서 송·수신 핀 설정

#define SS_PIN 10  //Slave핀 설정
#define RST_PIN 9  //RST핀 설정
MFRC522 rc522(SS_PIN, RST_PIN);  //RFID핀 설정

const int doordatapin = 6;  //문열림 확인 값 핀번호 설정(1번에서 데이터 받음)
int doordata = 0;  //문열림 확인 변수(1번에서 데이터 받음)
const int RFIDdatapin = 5;  //RFID값 핀번호설정(1번에서 데이터 받음)
int RFIDdata = 0;  //RFID확인 변수(1번에서 데이터 받음)
const int checkdatapin = 4;  //RFID혹은 BlueTooth로 문이 열렸을때 1번보드로 데이터 전송

const int led = 7;  //문열림-LED켜기 문닫힘-LED끄기(2번에서 처리)
char bt;  //BlueTooth값 처리 변수(2번에서 처리)
int bluetooth;
const int alarm=8;  //버저 설정(2번에서 처리)
int count1=0, count2=0;  //문이 열리거나 닫힐 때 소리가 한 번만 나게 해 주는 변수


void setup() 
{
  pinMode(doordatapin, INPUT);  //1번 아두이노로부터 문열림데이터 전송 받음
  pinMode(RFIDdatapin, INPUT);  //1번 아두이노로부터 RFID데이터 전송 받음
  pinMode(checkdatapin, OUTPUT);  //문 열림(닫힘) 상태를 1번 아두이노로 전송
  pinMode(alarm, OUTPUT);  //버저에서 입력받은 소리값 출력
  pinMode(led, OUTPUT);  //LED 출력
  SPI.begin();  //RFID 모듈과 SPI통신 시작
  rc522.PCD_Init();  //RFID리더 이름 설정
  Serial.begin(9600);  //컴퓨터와 아두이노 보드와의 통신 속도 설정
  BTSerial.begin(9600);  //블루투스와의 통신 속도 설정
}


void loop() 
{
  bluetooth=BB();  //BB함수 호출
  if(bluetooth==1)  //BB함수의 반환값이 1인 경우
  {
    digitalWrite(checkdatapin, HIGH);  //1번 아두이노로 HIGH 전송
                                       //문열림
    while(1)
    {
      delay(1000);
      if(count1==0)  //처음 한 번만 실행
      {
        opensound();  //opensound함수 호출
        count1=1;
        count2=0;
      }
      if(count1==0)  //처음 한 번만 실행
      {
        opensound();  //opensound함수 호출
        count1=1;
        count2=0;
      }
      Serial.println("문열림-블루투스"); 
      digitalWrite(led, HIGH);  //LED 켬
      digitalWrite(checkdatapin, HIGH);  //1번 아두이노로 HIGH 전송
      doordata=digitalRead(doordatapin);  //1번 아두이노로부터 데이터 전송 받음
      if(doordata == LOW)  //1번 아두이노로부터 0(LOW) 전송 받았을 경우
      {
        digitalWrite(led, LOW);  //LED 끔
        doordata=0;
        break;  //스위치가 눌렸다면 while문을 나가고 doordata를 0으로 설정
      }
    }
    digitalWrite(checkdatapin, LOW);  //1번 아두이노로 LOW 전송
    bluetooth=0;  //bluetooth=0으로 초기화
  }
  doordata = digitalRead(doordatapin);  //1번 아두이노로부터 ‘문열림’ 여부 전송 받음
  RFIDdata = digitalRead(RFIDdatapin);  //1번 아두이노로부터 ‘R’버튼 입력 여부 전송 받음
  if(doordata == 0)  //1번 아두이노로부터 ‘문닫힘’ 전송받을 경우
  {
    if(count2 == 0)  //처음 한 번만 실행
    {
      closesound();  //closesound함수 호출
      count2=1;
      count1=0;
    }       
    digitalWrite(led, LOW);  //LED 끔
    Serial.println("문닫힘");
    delay(2000);
  }
  else if(doordata == 1)  //1번 아두이노로부터 ‘문열림’ 전송받을 경우
  {
    if(count1 == 0)  //처음 한 번만 실행
    {
      opensound();  //opensound함수 호출
      count1=1;
      count2=0;
    }
    digitalWrite(led, HIGH);  //LED 켬
    Serial.println("문열림");
    delay(2000);
  }
  if(RFIDdata==1)  //1번 아두이노에서 ‘R’버튼을 입력했을 경우
  {
    while(1)
    {
      if(!rc522.PICC_IsNewCardPresent()) return;  //새로운 카드가 들어오지 않았다면 TRUE반환
      if(!rc522.PICC_ReadCardSerial()) return;  //새로운 카드의 데이터를 읽음
      if(rc522.uid.uidByte[0]==21 && rc522.uid.uidByte[1]==153 && rc522.uid.uidByte[2]==208 && rc522.uid.uidByte[3]==101)  //고유번호 일치 여부 판단
      {
          Serial.println("<< OK !!! >>  Registered card...");
          digitalWrite(checkdatapin, HIGH);  //1번 아두이노로 HIGH 전송
          doordata = digitalRead(doordatapin);  //1번 아두이노로부터 ‘문열림’ 여부 전송 받음
          while(doordata == 1)  //1번 아두이노로부터 ‘문열림’ 전송받을 경우
          {
            digitalWrite(led, HIGH); //LED 켬
            Serial.println("문열림");
            delay(2000);
            doordata = digitalRead(doordatapin);  //1번보드가 0을 송신하면 while문 나감
          }
          RFIDdata = 0;  //RFIDdata=0으로 초기화
          delay(2000);
          break;
      }
    }
    digitalWrite(checkdatapin, LOW);  //checkdatapin를 0으로 해서 1번 보드의 RFID처리를 끝냄
  }
}


int BB()
{
  if(BTSerial.available())  //만약 블루투스 앱으로 데이터를 송신하면
  {
    bt=BTSerial.read();  //데이터를 읽어서 bt값 설정
    if(bt=='1')
    {
      return 1;  //문열림버튼이 눌리면 1반환 : 블루투스 기능 시작
    }
    else
    {
      return 0;  //버튼이 눌리지 않거나 이상한 버튼을 누르면 블루투스 기능 시작 불가
    }
  }
}


void closesound()
{
  tone(alarm, 523, 500);
  delay(500);
  tone(alarm, 392, 500);
  delay(500);
  tone(alarm, 330, 500);
  delay(500);
  tone(alarm, 262, 500);
  delay(500);
}


void opensound()
{
  tone(alarm, 262, 500);
  delay(500);
  tone(alarm, 330, 500);
  delay(500);
  tone(alarm, 392, 500);
  delay(500);
  tone(alarm, 523, 500);
  delay(500);
}
