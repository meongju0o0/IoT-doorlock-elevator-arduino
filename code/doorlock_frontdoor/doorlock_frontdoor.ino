const int alarmPin = 22;  //버저 핀번호
const int trigPin = 24;  //초음파 센서 trig핀번호
const int echoPin = 25;  //초음파 센서 echo핀번호
#include <Servo.h>  //서보모터 헤더파일 인클루드
Servo myservo;
#include <Wire.h>  //LCD I2C 핀 위치
#include <LiquidCrystal_I2C.h>  //LCD I2C모듈 헤더파일 인클루드
LiquidCrystal_I2C lcd(0x3F,16,2);  //LCD I2C 통신 설정(주소, 글자수, 줄수)
  /* << 4×4키패드 배열 설정 >> */
const int numRows = 4;  //keypad 입력 번호
const int numCols = 4;  //keypad 출력 번호
const int rowPins[] = {42,44,46,48};  //keypad 핀 번호
const int colPins[] = {40,38,36,34};  //keypad 핀 번호
const char keypad[numRows][numCols] = {
        {'1','2','3','A'},
        {'4','5','6','B'},
        {'7','8','9','R'},
        {'#','0','*','S'}
        };  // 'A' = 관리자, 'B' = Bluetooth, 'R' = RFID, 'S' = 저장, '#' = 취소, '*' = 확인
/*                                   << RFID설정 >>
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             49        D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */
#include <SPI.h>  //SPI통신 헤더파일 인클루드
#include <MFRC522.h>  //RFID 헤더파일 인클루드
#define SS_PIN 53
#define RST_PIN 49
#define DOOR_PIN 8
MFRC522 rc522(SS_PIN, RST_PIN);  //RFID핀 설정
int first_RFIDnum[]={0,0,0,0};  //RFID카드 저장, 1층
int second_RFIDnum[]={0,0,0,0};  //RFID카드 저장, 2층
int third_RFIDnum[]={0,0,0,0};  //RFID카드 저장, 3층
int fourth_RFIDnum[]={0,0,0,0};  //RFID카드 저장, 4층
int admin_RFIDnum[]={0,0,0,0};  //RFID카드 저장, 관리자
  /* << BlueTooth설정 >> */
/*#include<SoftwareSerial.h>
SoftwareSerial BTSerial(1, 0);*/

int first_digit = 4;  //비밀번호 자릿수 세팅(초기 비밀번호 4자리), 비밀번호 1~9자리, 1층
int second_digit = 4;  //비밀번호 자릿수 세팅(초기 비밀번호 4자리), 비밀번호 1~9자리, 2층
int third_digit = 4;  //비밀번호 자릿수 세팅(초기 비밀번호 4자리), 비밀번호 1~9자리, 3층
int fourth_digit = 4;  //비밀번호 자릿수 세팅(초기 비밀번호 4자리), 비밀번호 1~9자리, 4층
int admin_digit = 4;  //비밀번호 자릿수 세팅(초기 비밀번호 4자리), 비밀번호 1~9자리, 관리자
char first_password[] = {'0','0','0','0','0','0','0','0','0'};  //저장된 비밀번호 저장 변수, 초기비밀번호 : 0000, 1층
char second_password[] = {'0','0','0','0','0','0','0','0','0'};  //저장된 비밀번호 저장 변수, 초기비밀번호 : 0000, 2층
char third_password[] = {'0','0','0','0','0','0','0','0','0'};  //저장된 비밀번호 저장 변수, 초기비밀번호 : 0000, 3층
char fourth_password[] = {'0','0','0','0','0','0','0','0','0'};  //저장된 비밀번호 저장 변수, 초기비밀번호 : 0000, 4층
char admin_password[] = {'0','0','0','0','0','0','0','0','0'};  //저장된 비밀번호 저장 변수, 초기비밀번호 : 0000, 관리자
int wrong_count = 0;  //비밀번호 실패횟수 셈
char keyread = 0;  //keypadread함수로부터 key값 받을 변수
char floor_set = 0;  //층수확인 변수
int previous_distance = 0;  //ultrasonic함수에서 이전 거리 저장할 변수
char pass = 'F';  //pass함수에서 'T', 'F', 'N'값 받음
char bluetoothpass = 'F'; //bluetoothpass함수에서 'T', 'F'값 받음
char active = 'N';  //char함수에서 'Y', 'N'값 받음
char chk_RFID = 'N';


void setup()
{
  pinMode(alarmPin, OUTPUT);  //버저에서 소리값 출력
  pinMode(trigPin, OUTPUT);  //초음파 센서 trig핀을 OUTPUT으로 설정
  pinMode(echoPin, INPUT);  //초음파 센서 echo핀을 INPUT으로 설정
  myservo.attach(23);  //23번핀에 서보 연결
                   /*keypad 입출력 설정*/
  for (int i = 0 ; i < numRows; i++)
  {
    pinMode(rowPins[i],INPUT_PULLUP);  //2~5번 핀을 INPUT, PULL_UP으로 설정
  }
  for (int i = 0 ; i < numCols; i++)
  {
    pinMode(colPins[i],OUTPUT);  //6~9번 핀을 OUTPUT으로 설정
    digitalWrite(colPins[i],HIGH);  //6~9번의 출력상태를 HIGH로 설정
  }
                 /*LCD 설정 && 초기 출력*/
  lcd.init();  //LCD I2C 통신 초기화 설정
  lcd.backlight();  //배경화면 빛 설정
  lcd.init();
  lcd.setCursor(0,0);
  lcd.print("IoT front door");
  delay(3000);
  lcd.init();
  lcd.setCursor(0,0);
  lcd.print("made by");
  lcd.setCursor(0,1);
  lcd.print("controller club");
  delay(3000);
                      /*RFID 설정*/
  SPI.begin();
  rc522.PCD_Init();
                    /*BlueTooth 설정*/
  /*BTSerial.begin(9600);*/
  Serial.begin(9600);
  start_sound();
}


void loop()
{
  active = func_active();  //active함수 호출
  while(active == 'N')  //active함수에서 'N' 반환 되면
  {
    active = func_active();  //active함수 호출
  }
  /*active함수에서 'Y' 반환되면*/
  if(floor_set == 'A')  //관리자 모드이면
  {
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Administrator");
    lcd.setCursor(0,1);
    lcd.print("Mode");
    delay(1000);
  }
  lcd.init();
  lcd.setCursor(0,0);
  lcd.print("Activated");
  keyread = func_keypadread();  //keypadread함수 호출
  while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
  {
    keyread = func_keypadread();  //keypadread함수 호출
    delay(10);
    if(keyread != 0)
    {
      if(keyread == 'B')  //'B'입력 시 bluetooth
      {
        /*bluetoothpass = func_bluetoothpass();
        if(pass == 'F'){  //비밀번호가 일치하지 않는 경우
        }
        if(pass == 'T')  //비밀번호가 일치하는 경우
        {
          Serial.println("Open");
          Switch=func_Switch();  //Switch함수 호출
          if(Switch == 'C')  //스위치가 눌러 졌을 때
          {
            Switch = 'O';  //Swtich값 'O'으로 초기화(문열림)
            Serial.println("Closed");
          }
        }*/
      }
      if(keyread == 'R')  //'R'입력 시 saveRFID함수 호출
      {
        if(floor_set == '1')
        {
          int previous_RFIDnum[] = {0,0,0,0};
          for(int i = 0; i < 4; i++)
          {
            previous_RFIDnum[i] = first_RFIDnum[i];
          }
          func_saveRFID();
          if(previous_RFIDnum[0] == first_RFIDnum[0] && previous_RFIDnum[1] == first_RFIDnum[1] &&
             previous_RFIDnum[2] == first_RFIDnum[2] && previous_RFIDnum[3] == first_RFIDnum[3])
          {
            lcd.init();
            lcd.setCursor(0,0);
            lcd.print("Card not changed :(");
            error_sound();
            delay(1000);
          }
          else
          {
            lcd.init();
            lcd.setCursor(0,0);
            lcd.print("<< OK !!! >>");
            lcd.setCursor(0,1);
            lcd.print("Card registered");
          }
          break;
        }

        if(floor_set == '2')
        {
          int previous_RFIDnum[] = {0,0,0,0};
          for(int i = 0; i < 4; i++)
          {
            previous_RFIDnum[i] = second_RFIDnum[i];
          }
          func_saveRFID();
          if(previous_RFIDnum[0] == second_RFIDnum[0] && previous_RFIDnum[1] == second_RFIDnum[1] &&
             previous_RFIDnum[2] == second_RFIDnum[2] && previous_RFIDnum[3] == second_RFIDnum[3])
          {
            lcd.init();
            lcd.setCursor(0,0);
            lcd.print("Card not changed :(");
            error_sound();
            delay(1000);
          }
          else
          {
            lcd.init();
            lcd.setCursor(0,0);
            lcd.print("<< OK !!! >>");
            lcd.setCursor(0,1);
            lcd.print("Card registered");
          }
          break;
        }

        if(floor_set == '3')
        {
          int previous_RFIDnum[] = {0,0,0,0};
          for(int i = 0; i < 4; i++)
          {
            previous_RFIDnum[i] = third_RFIDnum[i];
          }
          func_saveRFID();
          if(previous_RFIDnum[0] == third_RFIDnum[0] && previous_RFIDnum[1] == third_RFIDnum[1] &&
             previous_RFIDnum[2] == third_RFIDnum[2] && previous_RFIDnum[3] == third_RFIDnum[3])
          {
            lcd.init();
            lcd.setCursor(0,0);
            lcd.print("Card not changed :(");
            error_sound();
            delay(1000);
          }
          else
          {
            lcd.init();
            lcd.setCursor(0,0);
            lcd.print("<< OK !!! >>");
            lcd.setCursor(0,1);
            lcd.print("Card registered");
          }
          break;
        }

        if(floor_set == '4')
        {
          int previous_RFIDnum[] = {0,0,0,0};
          for(int i = 0; i < 4; i++)
          {
            previous_RFIDnum[i] = fourth_RFIDnum[i];
          }
          func_saveRFID();
          if(previous_RFIDnum[0] == fourth_RFIDnum[0] && previous_RFIDnum[1] == fourth_RFIDnum[1] &&
             previous_RFIDnum[2] == fourth_RFIDnum[2] && previous_RFIDnum[3] == fourth_RFIDnum[3])
          {
            lcd.init();
            lcd.setCursor(0,0);
            lcd.print("Card not changed :(");
            error_sound();
            delay(1000);
          }
          else
          {
            lcd.init();
            lcd.setCursor(0,0);
            lcd.print("<< OK !!! >>");
            lcd.setCursor(0,1);
            lcd.print("Card registered");
          }
          break;
        }

        if(floor_set == 'A')
        {
          int previous_RFIDnum[] = {0,0,0,0};
          for(int i = 0; i < 4; i++)
          {
            previous_RFIDnum[i] = admin_RFIDnum[i];
          }
          func_saveRFID();
          if(previous_RFIDnum[0] == admin_RFIDnum[0] && previous_RFIDnum[1] == admin_RFIDnum[1] &&
             previous_RFIDnum[2] == admin_RFIDnum[2] && previous_RFIDnum[3] == admin_RFIDnum[3])
          {
            lcd.init();
            lcd.setCursor(0,0);
            lcd.print("Card not changed :(");
            error_sound();
            delay(1000);
          }
          else
          {
            lcd.init();
            lcd.setCursor(0,0);
            lcd.print("<< OK !!! >>");
            lcd.setCursor(0,1);
            lcd.print("Card registered");
          }
          break;
        }
      }
      if(keyread == 'S')  //'S'입력 시 비밀번호 변경 시작
      {
        func_savepassword();  //savepassword함수 호출
        break;
      }
      if(keyread == '0' || keyread == '1' || keyread == '2' || keyread == '3' ||
         keyread == '4' || keyread == '5' || keyread == '6' || keyread == '7' ||
         keyread == '8' || keyread == '9')  //'0~9’ 입력 시 비밀번호 잠금 해제 시작
      {
        pass = func_pass('A');  //pass함수 호출
        if(pass == 'F'){  //비밀번호가 일치하지 않는 경우
        }
        if(pass == 'T')  //비밀번호가 일치하는 경우
        {
          door();
        }
        if(pass == 'N'){  //비밀번호 입력 중 취소한 경우
        }
        pass = 'F';
        break;
      }
    }
  }
}


char func_active()  //활성(층수 확인) 함수
{
  active = 'N';
  floor_set = 0;
  lcd.init();
  lcd.setCursor(0,0);
  lcd.print("Floor : ");
  keyread = func_keypadread();  //keypadread함수 호출
  while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
  {
    func_RFID();  //RFID함수 호출
    if(chk_RFID == 'Y')
    {
      chk_RFID = 'N';
      break;  //while문 탈출 => loop함수 => 비활성
    }
    keyread = func_keypadread();  //keypadread함수 호출
    delay(10);
    if(keyread != 0)  //키패드를 눌렀을 때
    {
      if(keyread == '1' || keyread == '2' || keyread == '3' || keyread == '4' || keyread == 'A')  //'1~4', 'A'입력시 층수 저장
      {
        floor_set = keyread;
        lcd.setCursor(8,0);
        lcd.print(floor_set);
        keyread = func_keypadread();
        while(keyread == 0)
        {
          keyread = func_keypadread();
          if(keyread != 0)
          {
            if(keyread == '*')  //'*'누르면 활성
            {
              return 'Y';  //활성 반환
            }
          }
        }
      }
      if(keyread == '0' || keyread == '5' || keyread == '6' || keyread == '7' || 
         keyread == '8' || keyread == '9' || keyread == 'B' || keyread == 'R' || 
         keyread == 'S' || keyread == '#' || keyread == '*')  //다른 키를 누른 경우
      {
        active = 'N';
        floor_set = 0;
        error_sound();
        lcd.init();
        lcd.setCursor(0,0);
        lcd.print("Please write");
        lcd.setCursor(0,1);
        lcd.print("only floor :(");
        delay(1000);
        return 'N';  //비활성 반환
      }
    }
  }
}


char func_pass(char A_B)  //비밀번호 일치 확인 함수
{
  char inkey[9]={};  //입력받은 번호 저장할 배열
  if(floor_set == '1')
  {
    if(A_B == 'A')
    {
      inkey[0] = keyread;  //0번째 배열에 번호 저장(비밀번호를 입력하기 위해 처음 친 숫자 저장)
      lcd.init();
      lcd.setCursor(0,0);
      lcd.print("Entered number");
      lcd.setCursor(0,1);
      lcd.print(keyread);
      for(int i = 1; i < first_digit; i++)
      {
        lcd.setCursor(i,1);
        lcd.print("*");
      }
      for(int i=1; i < first_digit; i++)  //자릿수-1만큼 반복문 실행
      {
        keyread = func_keypadread();  //keypadread함수 호출
        while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
        {
          keyread = func_keypadread();  //keypadread함수 호출
          delay(10);
          if(keyread != 0)  //키패드에서 0이아닌값이 반환되면 반환값 inkey에 저장
          {
            if(keyread == '0' || keyread == '1' || keyread == '2' || keyread == '3' ||
               keyread == '4' || keyread == '5' || keyread == '6' || keyread == '7' ||
               keyread == '8' || keyread == '9')  //숫자를 누른 경우
            {
              inkey[i] = keyread;  //i번째 배열에 비밀번호 저장
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Entered number");
              for(int j = 0; j < first_digit; j++)
              {
                if(j != i)
                {
                  lcd.setCursor(j,1);
                  lcd.print("*");
                }
              }
              lcd.setCursor(i,1);
              lcd.print(keyread);
            }
            if(keyread == '#')  //비밀번호 입력중 취소하면
            {
              return 'N';  //'N'반환 => 비활성
            }
            if(keyread == 'A' || keyread == 'B' || keyread == 'R' || keyread == 'S' || keyread == '*')
            {
              error_sound();
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Please write");
              lcd.setCursor(0,1);
              lcd.print("only number :(");
              delay(1000);
              return 'N';  //'N'반환 => 비활성
            }
          }
        }
      }
    }
  
    if(A_B == 'B')
    {
      for(int i = 0; i < first_digit; i++)  //자릿수만큼 반복문 실행
      {
        keyread = func_keypadread();  //keypadread함수 호출
        while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
        {
          keyread = func_keypadread();  //keypadread함수 호출
          delay(10);
          if(keyread != 0)  //키패드에서 0이아닌값이 반환되면 반환값 inkey에 저장
          {
            if(keyread == '#')  //비밀번호 입력중 취소하면
            {
              return 'N';  //'N'반환 => 비활성
            }
            if(keyread == 'A' || keyread == 'B' || keyread == 'R' || keyread == 'S' || keyread == '*')
            {
              error_sound();
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Please write");
              lcd.setCursor(0,1);
              lcd.print("only numbe :(");
              delay(1000);
              return 'N';  //'N'반환 => 비활성
            }
            if(keyread == '0' || keyread == '1' || keyread == '2' || keyread == '3' ||
               keyread == '4' || keyread == '5' || keyread == '6' || keyread == '7' ||
               keyread == '8' || keyread == '9')  //숫자를 누른 경우
            {
              inkey[i] = keyread;  //i번째 배열에 비밀번호 저장
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Entered number");
              for(int j = 0; j < first_digit; j++)
              {
                if(j != i)
                {
                  lcd.setCursor(j,1);
                  lcd.print("*");
                }
              }
              lcd.setCursor(i,1);
              lcd.print(keyread);
            }
          }
        }
      }
    }
  }

  if(floor_set == '2')
  {
    if(A_B == 'A')
    {
      inkey[0] = keyread;  //0번째 배열에 번호 저장(비밀번호를 입력하기 위해 처음 친 숫자 저장)
      lcd.init();
      lcd.setCursor(0,0);
      lcd.print("Entered number");
      lcd.setCursor(0,1);
      lcd.print(keyread);
      for(int i = 1; i < second_digit; i++)
      {
        lcd.setCursor(i,1);
        lcd.print("*");
      }
      for(int i=1; i < second_digit; i++)  //자릿수-1만큼 반복문 실행
      {
        keyread = func_keypadread();  //keypadread함수 호출
        while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
        {
          keyread = func_keypadread();  //keypadread함수 호출
          delay(10);
          if(keyread != 0)  //키패드에서 0이아닌값이 반환되면 반환값 inkey에 저장
          {
            if(keyread == '0' || keyread == '1' || keyread == '2' || keyread == '3' ||
               keyread == '4' || keyread == '5' || keyread == '6' || keyread == '7' ||
               keyread == '8' || keyread == '9')  //숫자를 누른 경우
            {
              inkey[i] = keyread;  //i번째 배열에 비밀번호 저장
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Entered number");
              for(int j = 0; j < second_digit; j++)
              {
                if(j != i)
                {
                  lcd.setCursor(j,1);
                  lcd.print("*");
                }
              }
              lcd.setCursor(i,1);
              lcd.print(keyread);
            }
            if(keyread == '#')  //비밀번호 입력중 취소하면
            {
              return 'N';  //'N'반환 => 비활성
            }
            if(keyread == 'A' || keyread == 'B' || keyread == 'R' || keyread == 'S' || keyread == '*')
            {
              error_sound();
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Please write");
              lcd.setCursor(0,1);
              lcd.print("only number :(");
              delay(1000);
              return 'N';  //'N'반환 => 비활성
            }
          }
        }
      }
    }
  
    if(A_B == 'B')
    {
      for(int i = 0; i < second_digit; i++)  //자릿수만큼 반복문 실행
      {
        keyread = func_keypadread();  //keypadread함수 호출
        while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
        {
          keyread = func_keypadread();  //keypadread함수 호출
          delay(10);
          if(keyread != 0)  //키패드에서 0이아닌값이 반환되면 반환값 inkey에 저장
          {
            if(keyread == '#')  //비밀번호 입력중 취소하면
            {
              return 'N';  //'N'반환 => 비활성
            }
            if(keyread == 'A' || keyread == 'B' || keyread == 'R' || keyread == 'S' || keyread == '*')
            {
              error_sound();
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Please write");
              lcd.setCursor(0,1);
              lcd.print("only numbe :(");
              delay(1000);
              return 'N';  //'N'반환 => 비활성
            }
            if(keyread == '0' || keyread == '1' || keyread == '2' || keyread == '3' ||
               keyread == '4' || keyread == '5' || keyread == '6' || keyread == '7' ||
               keyread == '8' || keyread == '9')  //숫자를 누른 경우
            {
              inkey[i] = keyread;  //i번째 배열에 비밀번호 저장
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Entered number");
              for(int j = 0; j < second_digit; j++)
              {
                if(j != i)
                {
                  lcd.setCursor(j,1);
                  lcd.print("*");
                }
              }
              lcd.setCursor(i,1);
              lcd.print(keyread);
            }
          }
        }
      }
    }
  }

  if(floor_set == '3')
  {
    if(A_B == 'A')
    {
      inkey[0] = keyread;  //0번째 배열에 번호 저장(비밀번호를 입력하기 위해 처음 친 숫자 저장)
      lcd.init();
      lcd.setCursor(0,0);
      lcd.print("Entered number");
      lcd.setCursor(0,1);
      lcd.print(keyread);
      for(int i = 1; i < third_digit; i++)
      {
        lcd.setCursor(i,1);
        lcd.print("*");
      }
      for(int i=1; i < third_digit; i++)  //자릿수-1만큼 반복문 실행
      {
        keyread = func_keypadread();  //keypadread함수 호출
        while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
        {
          keyread = func_keypadread();  //keypadread함수 호출
          delay(10);
          if(keyread != 0)  //키패드에서 0이아닌값이 반환되면 반환값 inkey에 저장
          {
            if(keyread == '0' || keyread == '1' || keyread == '2' || keyread == '3' ||
               keyread == '4' || keyread == '5' || keyread == '6' || keyread == '7' ||
               keyread == '8' || keyread == '9')  //숫자를 누른 경우
            {
              inkey[i] = keyread;  //i번째 배열에 비밀번호 저장
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Entered number");
              for(int j = 0; j < third_digit; j++)
              {
                if(j != i)
                {
                  lcd.setCursor(j,1);
                  lcd.print("*");
                }
              }
              lcd.setCursor(i,1);
              lcd.print(keyread);
            }
            if(keyread == '#')  //비밀번호 입력중 취소하면
            {
              return 'N';  //'N'반환 => 비활성
            }
            if(keyread == 'A' || keyread == 'B' || keyread == 'R' || keyread == 'S' || keyread == '*')
            {
              error_sound();
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Please write");
              lcd.setCursor(0,1);
              lcd.print("only number :(");
              delay(1000);
              return 'N';  //'N'반환 => 비활성
            }
          }
        }
      }
    }
  
    if(A_B == 'B')
    {
      for(int i = 0; i < third_digit; i++)  //자릿수만큼 반복문 실행
      {
        keyread = func_keypadread();  //keypadread함수 호출
        while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
        {
          keyread = func_keypadread();  //keypadread함수 호출
          delay(10);
          if(keyread != 0)  //키패드에서 0이아닌값이 반환되면 반환값 inkey에 저장
          {
            if(keyread == '#')  //비밀번호 입력중 취소하면
            {
              return 'N';  //'N'반환 => 비활성
            }
            if(keyread == 'A' || keyread == 'B' || keyread == 'R' || keyread == 'S' || keyread == '*')
            {
              error_sound();
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Please write");
              lcd.setCursor(0,1);
              lcd.print("only numbe :(");
              delay(1000);
              return 'N';  //'N'반환 => 비활성
            }
            if(keyread == '0' || keyread == '1' || keyread == '2' || keyread == '3' ||
               keyread == '4' || keyread == '5' || keyread == '6' || keyread == '7' ||
               keyread == '8' || keyread == '9')  //숫자를 누른 경우
            {
              inkey[i] = keyread;  //i번째 배열에 비밀번호 저장
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Entered number");
              for(int j = 0; j < third_digit; j++)
              {
                if(j != i)
                {
                  lcd.setCursor(j,1);
                  lcd.print("*");
                }
              }
              lcd.setCursor(i,1);
              lcd.print(keyread);
            }
          }
        }
      }
    }
  }

  if(floor_set == '4')
  {
    if(A_B == 'A')
    {
      inkey[0] = keyread;  //0번째 배열에 번호 저장(비밀번호를 입력하기 위해 처음 친 숫자 저장)
      lcd.init();
      lcd.setCursor(0,0);
      lcd.print("Entered number");
      lcd.setCursor(0,1);
      lcd.print(keyread);
      for(int i = 1; i < fourth_digit; i++)
      {
        lcd.setCursor(i,1);
        lcd.print("*");
      }
      for(int i=1; i < fourth_digit; i++)  //자릿수-1만큼 반복문 실행
      {
        keyread = func_keypadread();  //keypadread함수 호출
        while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
        {
          keyread = func_keypadread();  //keypadread함수 호출
          delay(10);
          if(keyread != 0)  //키패드에서 0이아닌값이 반환되면 반환값 inkey에 저장
          {
            if(keyread == '0' || keyread == '1' || keyread == '2' || keyread == '3' ||
               keyread == '4' || keyread == '5' || keyread == '6' || keyread == '7' ||
               keyread == '8' || keyread == '9')  //숫자를 누른 경우
            {
              inkey[i] = keyread;  //i번째 배열에 비밀번호 저장
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Entered number");
              for(int j = 0; j < fourth_digit; j++)
              {
                if(j != i)
                {
                  lcd.setCursor(j,1);
                  lcd.print("*");
                }
              }
              lcd.setCursor(i,1);
              lcd.print(keyread);
            }
            if(keyread == '#')  //비밀번호 입력중 취소하면
            {
              return 'N';  //'N'반환 => 비활성
            }
            if(keyread == 'A' || keyread == 'B' || keyread == 'R' || keyread == 'S' || keyread == '*')
            {
              error_sound();
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Please write");
              lcd.setCursor(0,1);
              lcd.print("only number :(");
              delay(1000);
              return 'N';  //'N'반환 => 비활성
            }
          }
        }
      }
    }
  
    if(A_B == 'B')
    {
      for(int i = 0; i < fourth_digit; i++)  //자릿수만큼 반복문 실행
      {
        keyread = func_keypadread();  //keypadread함수 호출
        while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
        {
          keyread = func_keypadread();  //keypadread함수 호출
          delay(10);
          if(keyread != 0)  //키패드에서 0이아닌값이 반환되면 반환값 inkey에 저장
          {
            if(keyread == '#')  //비밀번호 입력중 취소하면
            {
              return 'N';  //'N'반환 => 비활성
            }
            if(keyread == 'A' || keyread == 'B' || keyread == 'R' || keyread == 'S' || keyread == '*')
            {
              error_sound();
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Please write");
              lcd.setCursor(0,1);
              lcd.print("only numbe :(");
              delay(1000);
              return 'N';  //'N'반환 => 비활성
            }
            if(keyread == '0' || keyread == '1' || keyread == '2' || keyread == '3' ||
               keyread == '4' || keyread == '5' || keyread == '6' || keyread == '7' ||
               keyread == '8' || keyread == '9')  //숫자를 누른 경우
            {
              inkey[i] = keyread;  //i번째 배열에 비밀번호 저장
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Entered number");
              for(int j = 0; j < fourth_digit; j++)
              {
                if(j != i)
                {
                  lcd.setCursor(j,1);
                  lcd.print("*");
                }
              }
              lcd.setCursor(i,1);
              lcd.print(keyread);
            }
          }
        }
      }
    }
  }

  if(floor_set == 'A')
  {
    if(A_B == 'A')
    {
      inkey[0] = keyread;  //0번째 배열에 번호 저장(비밀번호를 입력하기 위해 처음 친 숫자 저장)
      lcd.init();
      lcd.setCursor(0,0);
      lcd.print("Entered number");
      lcd.setCursor(0,1);
      lcd.print(keyread);
      for(int i = 1; i < admin_digit; i++)
      {
        lcd.setCursor(i,1);
        lcd.print("*");
      }
      for(int i=1; i < admin_digit; i++)  //자릿수-1만큼 반복문 실행
      {
        keyread = func_keypadread();  //keypadread함수 호출
        while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
        {
          keyread = func_keypadread();  //keypadread함수 호출
          delay(10);
          if(keyread != 0)  //키패드에서 0이아닌값이 반환되면 반환값 inkey에 저장
          {
            if(keyread == '0' || keyread == '1' || keyread == '2' || keyread == '3' ||
               keyread == '4' || keyread == '5' || keyread == '6' || keyread == '7' ||
               keyread == '8' || keyread == '9')  //숫자를 누른 경우
            {
              inkey[i] = keyread;  //i번째 배열에 비밀번호 저장
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Entered number");
              for(int j = 0; j < admin_digit; j++)
              {
                if(j != i)
                {
                  lcd.setCursor(j,1);
                  lcd.print("*");
                }
              }
              lcd.setCursor(i,1);
              lcd.print(keyread);
            }
            if(keyread == '#')  //비밀번호 입력중 취소하면
            {
              return 'N';  //'N'반환 => 비활성
            }
            if(keyread == 'A' || keyread == 'B' || keyread == 'R' || keyread == 'S' || keyread == '*')
            {
              error_sound();
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Please write");
              lcd.setCursor(0,1);
              lcd.print("only number :(");
              delay(1000);
              return 'N';  //'N'반환 => 비활성
            }
          }
        }
      }
    }
  
    if(A_B == 'B')
    {
      for(int i = 0; i < admin_digit; i++)  //자릿수만큼 반복문 실행
      {
        keyread = func_keypadread();  //keypadread함수 호출
        while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
        {
          keyread = func_keypadread();  //keypadread함수 호출
          delay(10);
          if(keyread != 0)  //키패드에서 0이아닌값이 반환되면 반환값 inkey에 저장
          {
            if(keyread == '#')  //비밀번호 입력중 취소하면
            {
              return 'N';  //'N'반환 => 비활성
            }
            if(keyread == 'A' || keyread == 'B' || keyread == 'R' || keyread == 'S' || keyread == '*')
            {
              error_sound();
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Please write");
              lcd.setCursor(0,1);
              lcd.print("only numbe :(");
              delay(1000);
              return 'N';  //'N'반환 => 비활성
            }
            if(keyread == '0' || keyread == '1' || keyread == '2' || keyread == '3' ||
               keyread == '4' || keyread == '5' || keyread == '6' || keyread == '7' ||
               keyread == '8' || keyread == '9')  //숫자를 누른 경우
            {
              inkey[i] = keyread;  //i번째 배열에 비밀번호 저장
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Entered number");
              for(int j = 0; j < admin_digit; j++)
              {
                if(j != i)
                {
                  lcd.setCursor(j,1);
                  lcd.print("*");
                }
              }
              lcd.setCursor(i,1);
              lcd.print(keyread);
            }
          }
        }
      }
    }
  }

  if(floor_set == '1')
  {
    for(int i=0; i < first_digit; i++)  //비밀번호 일치 확인
    {
      if(inkey[i] != first_password[i])  //입력된 번호와 저장한 번호가 일치하지 않으면
      {
        wrong_count++;  // 틀린횟수 1증가
        lcd.init();
        lcd.setCursor(0,0);
        lcd.print("Rewrite correct");
        lcd.setCursor(0,1);
        lcd.print("password");
        if(wrong_count > 9) //10번이상 오류시
        {
          lcd.init();
          lcd.setCursor(0,0);
          lcd.print("Password incorrect");
          lcd.setCursor(0,1);
          lcd.print("10 times!!");
          delay(60000);  //60초 대기
          wrong_count = 0;  //wrong count = 0으로 초기화
        }
        return 'F';
      }
    }
    wrong_count = 0;  //wrong count = 0으로 초기화
    return 'T';  //다맞은경우 'T' 반환
  }
  
  if(floor_set == '2')
  {
    for(int i=0; i < second_digit; i++)  //비밀번호 일치 확인
    {
      if(inkey[i] != second_password[i])  //입력된 번호와 저장한 번호가 일치하지 않으면
      {
        wrong_count++;  // 틀린횟수 1증가
        lcd.init();
        lcd.setCursor(0,0);
        lcd.print("Rewrite correct");
        lcd.setCursor(0,1);
        lcd.print("password");
        if(wrong_count > 9) //10번이상 오류시
        {
          lcd.init();
          lcd.setCursor(0,0);
          lcd.print("Password incorrect");
          lcd.setCursor(0,1);
          lcd.print("10 times!!");
          delay(60000);  //60초 대기
          wrong_count = 0;  //wrong count = 0으로 초기화
        }
        return 'F';
      }
    }
    wrong_count = 0;  //wrong count = 0으로 초기화
    return 'T';  //다맞은경우 'T' 반환
  }
  
  if(floor_set == '3')
  {
    for(int i=0; i < third_digit; i++)  //비밀번호 일치 확인
    {
      if(inkey[i] != third_password[i])  //입력된 번호와 저장한 번호가 일치하지 않으면
      {
        wrong_count++;  // 틀린횟수 1증가
        lcd.init();
        lcd.setCursor(0,0);
        lcd.print("Rewrite correct");
        lcd.setCursor(0,1);
        lcd.print("password");
        if(wrong_count > 9) //10번이상 오류시
        {
          lcd.init();
          lcd.setCursor(0,0);
          lcd.print("Password incorrect");
          lcd.setCursor(0,1);
          lcd.print("10 times!!");
          delay(60000);  //60초 대기
          wrong_count = 0;  //wrong count = 0으로 초기화
        }
        return 'F';
      }
    }
    wrong_count = 0;  //wrong count = 0으로 초기화
    return 'T';  //다맞은경우 'T' 반환
  }
  
  if(floor_set == '4')
  {
    for(int i=0; i < fourth_digit; i++)  //비밀번호 일치 확인
    {
      if(inkey[i] != fourth_password[i])  //입력된 번호와 저장한 번호가 일치하지 않으면
      {
        wrong_count++;  // 틀린횟수 1증가
        lcd.init();
        lcd.setCursor(0,0);
        lcd.print("Rewrite correct");
        lcd.setCursor(0,1);
        lcd.print("password");
        if(wrong_count > 9) //10번이상 오류시
        {
          lcd.init();
          lcd.setCursor(0,0);
          lcd.print("Password incorrect");
          lcd.setCursor(0,1);
          lcd.print("10 times!!");
          delay(60000);  //60초 대기
          wrong_count = 0;  //wrong count = 0으로 초기화
        }
        return 'F';
      }
    }
    wrong_count = 0;  //wrong count = 0으로 초기화
    return 'T';  //다맞은경우 'T' 반환
  }

  if(floor_set == 'A')
  {
    for(int i=0; i < admin_digit; i++)  //비밀번호 일치 확인
    {
      if(inkey[i] != admin_password[i])  //입력된 번호와 저장한 번호가 일치하지 않으면
      {
        wrong_count++;  // 틀린횟수 1증가
        lcd.init();
        lcd.setCursor(0,0);
        lcd.print("Rewrite correct");
        lcd.setCursor(0,1);
        lcd.print("password");
        if(wrong_count > 9) //10번이상 오류시
        {
          lcd.init();
          lcd.setCursor(0,0);
          lcd.print("Password incorrect");
          lcd.setCursor(0,1);
          lcd.print("10 times!!");
          delay(60000);  //60초 대기
          wrong_count = 0;  //wrong count = 0으로 초기화
        }
        return 'F';
      }
    }
    wrong_count = 0;  //wrong count = 0으로 초기화
    return 'T';  //다맞은경우 'T' 반환
  }
}


void func_savepassword()  //비밀번호 저장 함수
{
  if(floor_set == '1')
  {
    char previous_password[9];
    int previous_digit=0;
    for(int i = 0; i < 9; i++)
    {
      previous_password[i] = first_password[i];
    }
    previous_digit = first_digit;
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Change password");
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Write previous");
    lcd.setCursor(0,1);
    lcd.print("password");
    pass = func_pass('B');  //pass함수 호출
    if(pass == 'N')
    {
      return 0;  //함수 종료 => loop함수 => active함수 호출
    }
    if(pass == 'F')  //입력한 번호가 기존 번호와 일치하지 않는 경우
    {
      return 0;  //함수 종료 => loop함수 => active함수 호출
    }
    if(pass == 'T')  //입력한 번호가 기존 번호와 일치하는 경우
    {
      lcd.init();
      lcd.setCursor(0,0);
      lcd.print("Enter");
      lcd.setCursor(0,1);
      lcd.print("password digits");
      keyread = func_keypadread();  //keypadread함수 호출
      while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
      {
        keyread = func_keypadread();  //keypadread함수 호출
        delay(10);
        if(keyread != 0)  //자릿수(0-9) 설정
        {
          if(keyread == '1')
            first_digit = 1;
          if(keyread == '2')
            first_digit = 2;
          if(keyread == '3')
            first_digit = 3;
          if(keyread == '4')
            first_digit = 4;
          if(keyread == '5')
            first_digit = 5;
          if(keyread == '6')
            first_digit = 6;
          if(keyread == '7')
            first_digit = 7;
          if(keyread == '8')
            first_digit = 8;
          if(keyread == '9')
            first_digit = 9;
          if(keyread == '#')
          {
            first_digit = previous_digit;
            return 0;  //함수 종료 => loop함수 => active함수 호출
          }
          if(keyread == 'A' || keyread == 'B' || keyread == 'R' || keyread == 'S' || keyread == '*')  //숫자가 아닌 경우
          {
            error_sound();
            lcd.init();
            lcd.setCursor(0,0);
            lcd.print("Please write");
            lcd.setCursor(0,1);
            lcd.print("only number :(");
            delay(1000);
            first_digit = previous_digit;
            return 0;  //함수 종료 => loop함수 => active함수 호출
          }
        }
      }
      lcd.init();
      lcd.setCursor(0,0);
      lcd.print("digit : ");
      lcd.print(first_digit);  //자리수 확인 끝
                 /*비밀번호 저장 시작*/
      lcd.init();
      lcd.setCursor(0,0);
      lcd.print("Enter password");
      lcd.setCursor(0,1);
      lcd.print("to change");
      for(int i = 0; i < first_digit; i++)
      {
        keyread = func_keypadread();  //keypadread함수 호출
        while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
        {
          keyread = func_keypadread();  //keypadread함수 호출
          delay(10);
          if(keyread != 0)
          {
            if(keyread == '0' || keyread == '1' || keyread == '2' || keyread == '3' ||
               keyread == '4' || keyread == '5' || keyread == '6' || keyread == '7' ||
               keyread == '8' || keyread == '9')  //숫자를 누른 경우
            {
              first_password[i] = keyread; //password 배열의 i번째 행에 저장
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Entered number");
              for(int j = 0; j < first_digit; j++)
              {
                if(j != i)
                {
                  lcd.setCursor(j,1);
                  lcd.print("*");
                }
              }
              lcd.setCursor(i,1);
              lcd.print(keyread);
            }
            if(keyread == '#')  //취소를 누른 경우
            {
              for(int i = 0; i < 9; i++)
              {
                first_password[i] = previous_password[i];
              }
              first_digit = previous_digit;
              return 0;  //함수 종료 => loop함수 => active함수 호출
            }
            if(keyread == 'A' || keyread == 'B' || keyread == 'R' || keyread == 'S' || keyread == '*')  //숫자가 아닌 경우
            {
              for(int i = 0; i < 9; i++)
              {
                first_password[i] = previous_password[i];
              }
              first_digit = previous_digit;
              error_sound();
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Please write");
              lcd.setCursor(0,1);
              lcd.print("only number :(");
              delay(1000);
              return 0;  //함수 종료 => loop함수 => active함수 호출
            }
          }
        }
      }
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Password saved");
    }
    pass = 'F';
  }

  if(floor_set == '2')
  {
    char previous_password[9];
    int previous_digit=0;
    for(int i = 0; i < 9; i++)
    {
      previous_password[i] = second_password[i];
    }
    previous_digit = second_digit;
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Change password");
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Write previous");
    lcd.setCursor(0,1);
    lcd.print("password");
    pass = func_pass('B');  //pass함수 호출
    if(pass == 'N')
    {
      return 0;  //함수 종료 => loop함수 => active함수 호출
    }
    if(pass == 'F')  //입력한 번호가 기존 번호와 일치하지 않는 경우
    {
      return 0;  //함수 종료 => loop함수 => active함수 호출
    }
    if(pass == 'T')  //입력한 번호가 기존 번호와 일치하는 경우
    {
      lcd.init();
      lcd.setCursor(0,0);
      lcd.print("Enter");
      lcd.setCursor(0,1);
      lcd.print("password digits");
      keyread = func_keypadread();  //keypadread함수 호출
      while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
      {
        keyread = func_keypadread();  //keypadread함수 호출
        delay(10);
        if(keyread != 0)  //자릿수(0-9) 설정
        {
          if(keyread == '1')
            second_digit = 1;
          if(keyread == '2')
            second_digit = 2;
          if(keyread == '3')
            second_digit = 3;
          if(keyread == '4')
            second_digit = 4;
          if(keyread == '5')
            second_digit = 5;
          if(keyread == '6')
            second_digit = 6;
          if(keyread == '7')
            second_digit = 7;
          if(keyread == '8')
            second_digit = 8;
          if(keyread == '9')
            second_digit = 9;
          if(keyread == '#')
          {
            second_digit = previous_digit;
            return 0;  //함수 종료 => loop함수 => active함수 호출
          }
          if(keyread == 'A' || keyread == 'B' || keyread == 'R' || keyread == 'S' || keyread == '*')  //숫자가 아닌 경우
          {
            error_sound();
            lcd.init();
            lcd.setCursor(0,0);
            lcd.print("Please write");
            lcd.setCursor(0,1);
            lcd.print("only number :(");
            delay(1000);
            second_digit = previous_digit;
            return 0;  //함수 종료 => loop함수 => active함수 호출
          }
        }
      }
      lcd.init();
      lcd.setCursor(0,0);
      lcd.print("digit : ");
      lcd.print(second_digit);  //자리수 확인 끝
                 /*비밀번호 저장 시작*/
      lcd.init();
      lcd.setCursor(0,0);
      lcd.print("Enter password");
      lcd.setCursor(0,1);
      lcd.print("to change");
      for(int i = 0; i < second_digit; i++)
      {
        keyread = func_keypadread();  //keypadread함수 호출
        while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
        {
          keyread = func_keypadread();  //keypadread함수 호출
          delay(10);
          if(keyread != 0)
          {
            if(keyread == '0' || keyread == '1' || keyread == '2' || keyread == '3' ||
               keyread == '4' || keyread == '5' || keyread == '6' || keyread == '7' ||
               keyread == '8' || keyread == '9')  //숫자를 누른 경우
            {
              second_password[i] = keyread; //password 배열의 i번째 행에 저장
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Entered number");
              for(int j = 0; j < second_digit; j++)
              {
                if(j != i)
                {
                  lcd.setCursor(j,1);
                  lcd.print("*");
                }
              }
              lcd.setCursor(i,1);
              lcd.print(keyread);
            }
            if(keyread == '#')  //취소를 누른 경우
            {
              for(int i = 0; i < 9; i++)
              {
                second_password[i] = previous_password[i];
              }
              second_digit = previous_digit;
              return 0;  //함수 종료 => loop함수 => active함수 호출
            }
            if(keyread == 'A' || keyread == 'B' || keyread == 'R' || keyread == 'S' || keyread == '*')  //숫자가 아닌 경우
            {
              for(int i = 0; i < 9; i++)
              {
                second_password[i] = previous_password[i];
              }
              second_digit = previous_digit;
              error_sound();
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Please write");
              lcd.setCursor(0,1);
              lcd.print("only number :(");
              delay(1000);
              return 0;  //함수 종료 => loop함수 => active함수 호출
            }
          }
        }
      }
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Password saved");
    }
    pass = 'F';
  }

  if(floor_set == '3')
  {
    char previous_password[9];
    int previous_digit=0;
    for(int i = 0; i < 9; i++)
    {
      previous_password[i] = third_password[i];
    }
    previous_digit = third_digit;
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Change password");
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Write previous");
    lcd.setCursor(0,1);
    lcd.print("password");
    pass = func_pass('B');  //pass함수 호출
    if(pass == 'N')
    {
      return 0;  //함수 종료 => loop함수 => active함수 호출
    }
    if(pass == 'F')  //입력한 번호가 기존 번호와 일치하지 않는 경우
    {
      return 0;  //함수 종료 => loop함수 => active함수 호출
    }
    if(pass == 'T')  //입력한 번호가 기존 번호와 일치하는 경우
    {
      lcd.init();
      lcd.setCursor(0,0);
      lcd.print("Enter");
      lcd.setCursor(0,1);
      lcd.print("password digits");
      keyread = func_keypadread();  //keypadread함수 호출
      while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
      {
        keyread = func_keypadread();  //keypadread함수 호출
        delay(10);
        if(keyread != 0)  //자릿수(0-9) 설정
        {
          if(keyread == '1')
            third_digit = 1;
          if(keyread == '2')
            third_digit = 2;
          if(keyread == '3')
            third_digit = 3;
          if(keyread == '4')
            third_digit = 4;
          if(keyread == '5')
            third_digit = 5;
          if(keyread == '6')
            third_digit = 6;
          if(keyread == '7')
            third_digit = 7;
          if(keyread == '8')
            third_digit = 8;
          if(keyread == '9')
            third_digit = 9;
          if(keyread == '#')
          {
            third_digit = previous_digit;
            return 0;  //함수 종료 => loop함수 => active함수 호출
          }
          if(keyread == 'A' || keyread == 'B' || keyread == 'R' || keyread == 'S' || keyread == '*')  //숫자가 아닌 경우
          {
            error_sound();
            lcd.init();
            lcd.setCursor(0,0);
            lcd.print("Please write");
            lcd.setCursor(0,1);
            lcd.print("only number :(");
            delay(1000);
            third_digit = previous_digit;
            return 0;  //함수 종료 => loop함수 => active함수 호출
          }
        }
      }
      lcd.init();
      lcd.setCursor(0,0);
      lcd.print("digit : ");
      lcd.print(third_digit);  //자리수 확인 끝
                 /*비밀번호 저장 시작*/
      lcd.init();
      lcd.setCursor(0,0);
      lcd.print("Enter password");
      lcd.setCursor(0,1);
      lcd.print("to change");
      for(int i = 0; i < third_digit; i++)
      {
        keyread = func_keypadread();  //keypadread함수 호출
        while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
        {
          keyread = func_keypadread();  //keypadread함수 호출
          delay(10);
          if(keyread != 0)
          {
            if(keyread == '0' || keyread == '1' || keyread == '2' || keyread == '3' ||
               keyread == '4' || keyread == '5' || keyread == '6' || keyread == '7' ||
               keyread == '8' || keyread == '9')  //숫자를 누른 경우
            {
              third_password[i] = keyread; //password 배열의 i번째 행에 저장
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Entered number");
              for(int j = 0; j < third_digit; j++)
              {
                if(j != i)
                {
                  lcd.setCursor(j,1);
                  lcd.print("*");
                }
              }
              lcd.setCursor(i,1);
              lcd.print(keyread);
            }
            if(keyread == '#')  //취소를 누른 경우
            {
              for(int i = 0; i < 9; i++)
              {
                third_password[i] = previous_password[i];
              }
              third_digit = previous_digit;
              return 0;  //함수 종료 => loop함수 => active함수 호출
            }
            if(keyread == 'A' || keyread == 'B' || keyread == 'R' || keyread == 'S' || keyread == '*')  //숫자가 아닌 경우
            {
              for(int i = 0; i < 9; i++)
              {
                third_password[i] = previous_password[i];
              }
              third_digit = previous_digit;
              error_sound();
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Please write");
              lcd.setCursor(0,1);
              lcd.print("only number :(");
              delay(1000);
              return 0;  //함수 종료 => loop함수 => active함수 호출
            }
          }
        }
      }
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Password saved");
    }
    pass = 'F';
  }

  if(floor_set == '4')
  {
    char previous_password[9];
    int previous_digit=0;
    for(int i = 0; i < 9; i++)
    {
      previous_password[i] = fourth_password[i];
    }
    previous_digit = fourth_digit;
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Change password");
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Write previous");
    lcd.setCursor(0,1);
    lcd.print("password");
    pass = func_pass('B');  //pass함수 호출
    if(pass == 'N')
    {
      return 0;  //함수 종료 => loop함수 => active함수 호출
    }
    if(pass == 'F')  //입력한 번호가 기존 번호와 일치하지 않는 경우
    {
      return 0;  //함수 종료 => loop함수 => active함수 호출
    }
    if(pass == 'T')  //입력한 번호가 기존 번호와 일치하는 경우
    {
      lcd.init();
      lcd.setCursor(0,0);
      lcd.print("Enter");
      lcd.setCursor(0,1);
      lcd.print("password digits");
      keyread = func_keypadread();  //keypadread함수 호출
      while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
      {
        keyread = func_keypadread();  //keypadread함수 호출
        delay(10);
        if(keyread != 0)  //자릿수(0-9) 설정
        {
          if(keyread == '1')
            fourth_digit = 1;
          if(keyread == '2')
            fourth_digit = 2;
          if(keyread == '3')
            fourth_digit = 3;
          if(keyread == '4')
            fourth_digit = 4;
          if(keyread == '5')
            fourth_digit = 5;
          if(keyread == '6')
            fourth_digit = 6;
          if(keyread == '7')
            fourth_digit = 7;
          if(keyread == '8')
            fourth_digit = 8;
          if(keyread == '9')
            fourth_digit = 9;
          if(keyread == '#')
          {
            fourth_digit = previous_digit;
            return 0;  //함수 종료 => loop함수 => active함수 호출
          }
          if(keyread == 'A' || keyread == 'B' || keyread == 'R' || keyread == 'S' || keyread == '*')  //숫자가 아닌 경우
          {
            error_sound();
            lcd.init();
            lcd.setCursor(0,0);
            lcd.print("Please write");
            lcd.setCursor(0,1);
            lcd.print("only number :(");
            delay(1000);
            fourth_digit = previous_digit;
            return 0;  //함수 종료 => loop함수 => active함수 호출
          }
        }
      }
      lcd.init();
      lcd.setCursor(0,0);
      lcd.print("digit : ");
      lcd.print(fourth_digit);  //자리수 확인 끝
                 /*비밀번호 저장 시작*/
      lcd.init();
      lcd.setCursor(0,0);
      lcd.print("Enter password");
      lcd.setCursor(0,1);
      lcd.print("to change");
      for(int i = 0; i < fourth_digit; i++)
      {
        keyread = func_keypadread();  //keypadread함수 호출
        while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
        {
          keyread = func_keypadread();  //keypadread함수 호출
          delay(10);
          if(keyread != 0)
          {
            if(keyread == '0' || keyread == '1' || keyread == '2' || keyread == '3' ||
               keyread == '4' || keyread == '5' || keyread == '6' || keyread == '7' ||
               keyread == '8' || keyread == '9')  //숫자를 누른 경우
            {
              fourth_password[i] = keyread; //password 배열의 i번째 행에 저장
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Entered number");
              for(int j = 0; j < fourth_digit; j++)
              {
                if(j != i)
                {
                  lcd.setCursor(j,1);
                  lcd.print("*");
                }
              }
              lcd.setCursor(i,1);
              lcd.print(keyread);
            }
            if(keyread == '#')  //취소를 누른 경우
            {
              for(int i = 0; i < 9; i++)
              {
                fourth_password[i] = previous_password[i];
              }
              fourth_digit = previous_digit;
              return 0;  //함수 종료 => loop함수 => active함수 호출
            }
            if(keyread == 'A' || keyread == 'B' || keyread == 'R' || keyread == 'S' || keyread == '*')  //숫자가 아닌 경우
            {
              for(int i = 0; i < 9; i++)
              {
                fourth_password[i] = previous_password[i];
              }
              fourth_digit = previous_digit;
              error_sound();
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Please write");
              lcd.setCursor(0,1);
              lcd.print("only number :(");
              delay(1000);
              return 0;  //함수 종료 => loop함수 => active함수 호출
            }
          }
        }
      }
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Password saved");
    }
    pass = 'F';
  }

  if(floor_set == 'A')
  {
    char previous_password[9];
    int previous_digit=0;
    for(int i = 0; i < 9; i++)
    {
      previous_password[i] = admin_password[i];
    }
    previous_digit = admin_digit;
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Change password");
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Write previous");
    lcd.setCursor(0,1);
    lcd.print("password");
    pass = func_pass('B');  //pass함수 호출
    if(pass == 'N')
    {
      return 0;  //함수 종료 => loop함수 => active함수 호출
    }
    if(pass == 'F')  //입력한 번호가 기존 번호와 일치하지 않는 경우
    {
      return 0;  //함수 종료 => loop함수 => active함수 호출
    }
    if(pass == 'T')  //입력한 번호가 기존 번호와 일치하는 경우
    {
      lcd.init();
      lcd.setCursor(0,0);
      lcd.print("Enter");
      lcd.setCursor(0,1);
      lcd.print("password digits");
      keyread = func_keypadread();  //keypadread함수 호출
      while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
      {
        keyread = func_keypadread();  //keypadread함수 호출
        delay(10);
        if(keyread != 0)  //자릿수(0-9) 설정
        {
          if(keyread == '1')
            admin_digit = 1;
          if(keyread == '2')
            admin_digit = 2;
          if(keyread == '3')
            admin_digit = 3;
          if(keyread == '4')
            admin_digit = 4;
          if(keyread == '5')
            admin_digit = 5;
          if(keyread == '6')
            admin_digit = 6;
          if(keyread == '7')
            admin_digit = 7;
          if(keyread == '8')
            admin_digit = 8;
          if(keyread == '9')
            admin_digit = 9;
          if(keyread == '#')
          {
            admin_digit = previous_digit;
            return 0;  //함수 종료 => loop함수 => active함수 호출
          }
          if(keyread == 'A' || keyread == 'B' || keyread == 'R' || keyread == 'S' || keyread == '*')  //숫자가 아닌 경우
          {
            error_sound();
            lcd.init();
            lcd.setCursor(0,0);
            lcd.print("Please write");
            lcd.setCursor(0,1);
            lcd.print("only number :(");
            delay(1000);
            admin_digit = previous_digit;
            return 0;  //함수 종료 => loop함수 => active함수 호출
          }
        }
      }
      lcd.init();
      lcd.setCursor(0,0);
      lcd.print("digit : ");
      lcd.print(admin_digit);  //자리수 확인 끝
                 /*비밀번호 저장 시작*/
      lcd.init();
      lcd.setCursor(0,0);
      lcd.print("Enter password");
      lcd.setCursor(0,1);
      lcd.print("to change");
      for(int i = 0; i < admin_digit; i++)
      {
        keyread = func_keypadread();  //keypadread함수 호출
        while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keyread값이 0인동안 계속 반복
        {
          keyread = func_keypadread();  //keypadread함수 호출
          delay(10);
          if(keyread != 0)
          {
            if(keyread == '0' || keyread == '1' || keyread == '2' || keyread == '3' ||
               keyread == '4' || keyread == '5' || keyread == '6' || keyread == '7' ||
               keyread == '8' || keyread == '9')  //숫자를 누른 경우
            {
              admin_password[i] = keyread; //password 배열의 i번째 행에 저장
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Entered number");
              for(int j = 0; j < admin_digit; j++)
              {
                if(j != i)
                {
                  lcd.setCursor(j,1);
                  lcd.print("*");
                }
              }
              lcd.setCursor(i,1);
              lcd.print(keyread);
            }
            if(keyread == '#')  //취소를 누른 경우
            {
              for(int i = 0; i < 9; i++)
              {
                admin_password[i] = previous_password[i];
              }
              fourth_digit = previous_digit;
              return 0;  //함수 종료 => loop함수 => active함수 호출
            }
            if(keyread == 'A' || keyread == 'B' || keyread == 'R' || keyread == 'S' || keyread == '*')  //숫자가 아닌 경우
            {
              for(int i = 0; i < 9; i++)
              {
                admin_password[i] = previous_password[i];
              }
              admin_digit = previous_digit;
              error_sound();
              lcd.init();
              lcd.setCursor(0,0);
              lcd.print("Please write");
              lcd.setCursor(0,1);
              lcd.print("only number :(");
              delay(1000);
              return 0;  //함수 종료 => loop함수 => active함수 호출
            }
          }
        }
      }
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Password saved");
    }
    pass = 'F';
  }
}


void func_saveRFID()  //RFID키 저장 함수
{
  if(floor_set == '1')
  {
    int previous_RFIDnum[]={0,0,0,0};
    for(int i = 0; i < 4; i++)
    {
      previous_RFIDnum[i] = first_RFIDnum[i];
    }
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Change RFIDcard");
    lcd.setCursor(0,1);
    lcd.print("Write password");
    pass = func_pass('B');  //pass함수 호출
    if(pass == 'N'){  
      return 0;  //함수 종료 => loop함수 => active함수 호출
    }
    if(pass == 'F'){  //입력한 번호가 기존 번호와 일치하지 않는 경우
      return 0;  //함수 종료 => loop함수 => active함수 호출
    }
    if(pass == 'T')  //입력한 번호가 기존 번호와 일치하는 경우
    {
      if(!rc522.PICC_IsNewCardPresent()) return;  //새로운 카드가 들어오지 않았다면 TRUE반환
      if(!rc522.PICC_ReadCardSerial()) return;  //새로운 카드의 데이터를 읽음
      for(int i = 0; i < 4; i++)
      {
        first_RFIDnum[i] = rc522.uid.uidByte[i];
      }
    }
    pass = 'F';
  }
  
  if(floor_set == '2')
  {
    int previous_RFIDnum[]={0,0,0,0};
    for(int i = 0; i < 4; i++)
    {
      previous_RFIDnum[i] = second_RFIDnum[i];
    }
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Change RFIDcard");
    lcd.setCursor(0,1);
    lcd.print("Write password");
    pass = func_pass('B');  //pass함수 호출
    if(pass == 'N'){  
      return 0;  //함수 종료 => loop함수 => active함수 호출
    }
    if(pass == 'F'){  //입력한 번호가 기존 번호와 일치하지 않는 경우
      return 0;  //함수 종료 => loop함수 => active함수 호출
    }
    if(pass == 'T')  //입력한 번호가 기존 번호와 일치하는 경우
    {
      if(!rc522.PICC_IsNewCardPresent()) return;  //새로운 카드가 들어오지 않았다면 TRUE반환
      if(!rc522.PICC_ReadCardSerial()) return;  //새로운 카드의 데이터를 읽음
      for(int i = 0; i < 4; i++)
      {
        second_RFIDnum[i] = rc522.uid.uidByte[i];
      }
    }
    pass = 'F';
  }
  
  if(floor_set == '3')
  {
    int previous_RFIDnum[]={0,0,0,0};
    for(int i = 0; i < 4; i++)
    {
      previous_RFIDnum[i] = third_RFIDnum[i];
    }
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Change RFIDcard");
    lcd.setCursor(0,1);
    lcd.print("Write password");
    pass = func_pass('B');  //pass함수 호출
    if(pass == 'N'){  
      return 0;  //함수 종료 => loop함수 => active함수 호출
    }
    if(pass == 'F'){  //입력한 번호가 기존 번호와 일치하지 않는 경우
      return 0;  //함수 종료 => loop함수 => active함수 호출
    }
    if(pass == 'T')  //입력한 번호가 기존 번호와 일치하는 경우
    {
      if(!rc522.PICC_IsNewCardPresent()) return;  //새로운 카드가 들어오지 않았다면 TRUE반환
      if(!rc522.PICC_ReadCardSerial()) return;  //새로운 카드의 데이터를 읽음
      for(int i = 0; i < 4; i++)
      {
        third_RFIDnum[i] = rc522.uid.uidByte[i];
      }
    }
    pass = 'F';
  }
  
  if(floor_set == '4')
  {
    int previous_RFIDnum[]={0,0,0,0};
    for(int i = 0; i < 4; i++)
    {
      previous_RFIDnum[i] = fourth_RFIDnum[i];
    }
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Change RFIDcard");
    lcd.setCursor(0,1);
    lcd.print("Write password");
    pass = func_pass('B');  //pass함수 호출
    if(pass == 'N'){  
      return 0;  //함수 종료 => loop함수 => active함수 호출
    }
    if(pass == 'F'){  //입력한 번호가 기존 번호와 일치하지 않는 경우
      return 0;  //함수 종료 => loop함수 => active함수 호출
    }
    if(pass == 'T')  //입력한 번호가 기존 번호와 일치하는 경우
    {
      if(!rc522.PICC_IsNewCardPresent()) return;  //새로운 카드가 들어오지 않았다면 TRUE반환
      if(!rc522.PICC_ReadCardSerial()) return;  //새로운 카드의 데이터를 읽음
      for(int i = 0; i < 4; i++)
      {
        fourth_RFIDnum[i] = rc522.uid.uidByte[i];
      }
    }
    pass = 'F';
  }
  
  if(floor_set == 'A')
  {
    int previous_RFIDnum[]={0,0,0,0};
    for(int i = 0; i < 4; i++)
    {
      previous_RFIDnum[i] = admin_RFIDnum[i];
    }
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("Change RFIDcard");
    lcd.setCursor(0,1);
    lcd.print("Write password");
    pass = func_pass('B');  //pass함수 호출
    if(pass == 'N'){  
      return 0;  //함수 종료 => loop함수 => active함수 호출
    }
    if(pass == 'F'){  //입력한 번호가 기존 번호와 일치하지 않는 경우
      return 0;  //함수 종료 => loop함수 => active함수 호출
    }
    if(pass == 'T')  //입력한 번호가 기존 번호와 일치하는 경우
    {
      if(!rc522.PICC_IsNewCardPresent()) return;  //새로운 카드가 들어오지 않았다면 TRUE반환
      if(!rc522.PICC_ReadCardSerial()) return;  //새로운 카드의 데이터를 읽음
      for(int i = 0; i < 4; i++)
      {
        admin_RFIDnum[i] = rc522.uid.uidByte[i];
      }
    }
    pass = 'F';
  }
}


char func_keypadread()  //keypad값 받아들이는 함수
{
  char key = 0;
  for (int i= 0 ; i < numCols ; i++)
  {
    digitalWrite(colPins[i], 0);
    for (int j= 0; j < numRows; j++)
    {
      if(digitalRead(rowPins[j]) == 0)
      {
        delay(100);
        while(digitalRead(rowPins[j])==0) {
        }
        key = keypad[j][i];
      }
    }
    digitalWrite(colPins[i],1);
  }
  return key; //key값 리턴
}


void func_RFID()  //RFID 인식 함수
{
  if(!rc522.PICC_IsNewCardPresent()) return;  //새로운 카드가 들어오지 않았다면 함수 종료
  if(!rc522.PICC_ReadCardSerial()) return;  //새로운 카드의 데이터를 읽음
  if(rc522.uid.uidByte[0]==first_RFIDnum[0] && rc522.uid.uidByte[1]==first_RFIDnum[1] && 
     rc522.uid.uidByte[2]==first_RFIDnum[2] && rc522.uid.uidByte[3]==first_RFIDnum[3])  //RFID카드가 일치하는 경우
  {
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("<< OK !!! >>");
    lcd.setCursor(0,1);
    lcd.print("Registered card");
    floor_set = '1';
    chk_RFID = 'Y';
    door();
  }
  if(rc522.uid.uidByte[0]==second_RFIDnum[0] && rc522.uid.uidByte[1]==second_RFIDnum[1] && 
     rc522.uid.uidByte[2]==second_RFIDnum[2] && rc522.uid.uidByte[3]==second_RFIDnum[3])  //RFID카드가 일치하는 경우
  {
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("<< OK !!! >>");
    lcd.setCursor(0,1);
    lcd.print("Registered card");
    floor_set = '2';
    chk_RFID = 'Y';
    door();
  }
  if(rc522.uid.uidByte[0]==third_RFIDnum[0] && rc522.uid.uidByte[1]==third_RFIDnum[1] && 
     rc522.uid.uidByte[2]==third_RFIDnum[2] && rc522.uid.uidByte[3]==third_RFIDnum[3])  //RFID카드가 일치하는 경우
  {
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("<< OK !!! >>");
    lcd.setCursor(0,1);
    lcd.print("Registered card");
    floor_set = '3';
    chk_RFID = 'Y';
    door();
  }
  if(rc522.uid.uidByte[0]==fourth_RFIDnum[0] && rc522.uid.uidByte[1]==fourth_RFIDnum[1] && 
     rc522.uid.uidByte[2]==fourth_RFIDnum[2] && rc522.uid.uidByte[3]==fourth_RFIDnum[3])  //RFID카드가 일치하는 경우
  {
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("<< OK !!! >>");
    lcd.setCursor(0,1);
    lcd.print("Registered card");
    floor_set = '4';
    chk_RFID = 'Y';
    door();
  }
  if(rc522.uid.uidByte[0]==admin_RFIDnum[0] && rc522.uid.uidByte[1]==admin_RFIDnum[1] && 
     rc522.uid.uidByte[2]==admin_RFIDnum[2] && rc522.uid.uidByte[3]==admin_RFIDnum[3])  //RFID카드가 일치하는 경우
  {
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("<< OK !!! >>");
    lcd.setCursor(0,1);
    lcd.print("Registered card");
    floor_set = 'A';
    chk_RFID = 'Y';
    door();
  }
}


/*char func_bluetoothpass()
{
  char bluetooth_num[9]={};
  for(int i = 0; i < digit; i++)
  {
    if(BTSerial.available())  //만약 블루투스 앱으로 데이터를 송신하면
    {
      bluetooth_num[i] = BTSerial.read();  //데이터를 읽어서 bt값 설정
    }
  }
  
  for(int i=0; i < digit; i++)  //비밀번호 일치 확인
  {
    if(bluetooth_num[i] != password[i])  //입력된 번호와 저장한 번호가 일치하지 않으면
    {
      wrong_count++;  // 틀린횟수 1증가
      Serial.print("Rewrite correct password, Count : ");
      Serial.println(wrong_count);
      if(wrong_count > 9) //10번이상 오류시
      {
        Serial.println("Password incorrect 10 times!!");
        delay(60000);  //60초 대기
        wrong_count = 0;  //wrong count = 0으로 초기화
      }
      return 'F';
    }
  }
  return 'T';  //다맞은경우 'T'반환
}*/


void door()  //문 열고 닫는 함수
{
  open_sound();
  myservo.write(0);
  delay(2500);
  int distance = func_ultrasonic();
  while(distance < 12)
  {
    distance = func_ultrasonic();
  }
  myservo.write(90);
  delay(500);
  close_sound();
}


int func_ultrasonic()  //초음파 거리 함수
{
  int duration, distance;  //시간, 거리
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);  //trigPin을 10마이크로초 동안 HIGH
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;
  delay(100);
  if (distance >= 200 || distance <= 0)  //2m가 넘어가거나 값이 0 이하
  {
    lcd.init();
    lcd.setCursor(0,0);
    lcd.print("distance sensor");
    lcd.setCursor(0,1);
    lcd.print("error :(");
    return 5;  //5리턴 => 다시 ultrasonic함수 호출
  }
  else if(previous_distance == distance)  //0.1초 전에 잰 거리랑 현재 거리랑 같은 경우
  {
    previous_distance = distance;
    return distance;
  }
  else if(previous_distance != distance)  //다른 경우
  {
    previous_distance = distance;
    return 5;  //5리턴 => 다시 ultrasonic함수 호출
  }
}


void close_sound()
{
  tone(alarmPin, 523, 500);
  delay(500);
  tone(alarmPin, 392, 500);
  delay(500);
  tone(alarmPin, 330, 500);
  delay(500);
  tone(alarmPin, 262, 500);
  delay(500);
}


void open_sound()
{
  tone(alarmPin, 262, 500);
  delay(500);
  tone(alarmPin, 330, 500);
  delay(500);
  tone(alarmPin, 392, 500);
  delay(500);
  tone(alarmPin, 523, 500);
  delay(500);
}


void error_sound()
{
  tone(alarmPin, 1046, 500);
  delay(70);
  noTone(alarmPin);
  delay(70);
  tone(alarmPin, 1046, 500);
  delay(70);
  noTone(alarmPin);
}


void start_sound()
{
  tone(alarmPin, 262, 500);
  delay(50);
  tone(alarmPin, 294, 500);
  delay(50);
  tone(alarmPin, 330, 500);
  delay(50);
  tone(alarmPin, 349, 500);
  delay(50);
  tone(alarmPin, 392, 500);
  delay(50);
  tone(alarmPin, 440, 500);
  delay(50);
  tone(alarmPin, 494, 500);
  delay(50);
  tone(alarmPin, 523, 500);
  delay(50);
}
