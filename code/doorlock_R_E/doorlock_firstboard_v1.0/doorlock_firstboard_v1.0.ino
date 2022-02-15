const int doordatapin = 10;  //2번 아두이노로 문열림데이터 전송핀(열림=1, 닫힘=0)
const int RFIDdatapin = 11;  //2번 아두이노로 RFID를 눌렀을때 데이터 전송(RFID=1)
const int checkdatapin = 12;  //2번 아두이노에서 BT나 RFID로 문이 열렸을때 데이터 받음(열림=1)
int checkdata = 0;  //데이터 확인 변수

const int inswitchpin = 13;  //스위치 값 받을 핀번호
int reading;  //스위치 누름 값 받을 변수

//4×4키패드 배열 설정
const int numRows = 4;
const int numCols = 4;
int rowPins[]={5,4,3,2}; //아두이노 꼽을 번호
int colPins[]={6,7,8,9}; //아두이노 꼽을 번호
char keys[numRows][numCols]={
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','R'},
  {'#','0','*','D'}
}; //키패드 배열 설정
   //A = 활성, B = Bluetooth, R = RFID, D = 저장,  # = 취소, * = 확인

int wrong=0; //비밀번호 실패횟수 셈
int cipher=4; //비밀번호 자릿수 세팅(초기 비밀번호 4자리), 비밀번호 1~9자리
char savepassword[]={'0','0','0','0','0','0','0','0','0'}; //저장된 비밀번호 저장 변수, 초기비밀번호'0000'

char keyread=0; //keypad함수로부터 key값 받을 변수
char tf=0;  //pass함수에서 T,F 값 받음
int sw=2;  //switch함수 리턴값 받음


void setup()
{
  pinMode(doordatapin, OUTPUT);  //2번 아두이노로 문열림데이터 전송
  pinMode(RFIDdatapin, OUTPUT);  //2번 아두이노로 RFID데이터 전송
  pinMode(checkdatapin, INPUT);  //문이 열린 상태임을 2번 아두이노로부터 전송 받음
  pinMode(inswitchpin, INPUT_PULLUP);  //스위치값 PULL_UP설정

  for (int i = 0 ; i < numRows; i++)
  {
    pinMode(rowPins[i],INPUT_PULLUP);  //2-5번 핀을 INPUT, PULLUP으로 설정
  }
  for (int i = 0 ; i < numCols; i++)
  {
    pinMode(colPins[i],OUTPUT);  //6-9번 핀을 OUTPUT으로 설정
    digitalWrite(colPins[i],HIGH);  //6-9번의 출력상태를 HIGH로 설정
  }  //keypad 입출력 설정

  Serial.begin(9600);
}


void loop()
{
  not_activated: //여러 오류발생시 비활성으로 되돌리기
  digitalWrite(doordatapin, LOW);
  digitalWrite(RFIDdatapin, LOW);  //2번 아두이노로 0(LOW)전송
  Serial.println("비활성");

  keyread = keypadRead(); //keypad함수에서 key 받을 변수
  while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keypad값이 0인동안 계속 반복
  {
   checkdata = digitalRead(checkdatapin);  //문이 열렸는지 확인(열림:1, 닫힘:0)
    if(checkdata == 1)
    {
      digitalWrite(doordatapin, HIGH);  //문이 열렸으면 2번 보드로 HIGH 전송
      sw=Switch();  //Switch함수 호출, 반환된 값을 sw에 저장
      if(sw == 1)
      {
        digitalWrite(doordatapin, LOW);  //스위치가 눌렸으면 2번 보드로 LOW 전송
        delay(1000);
      }
    }
    keyread = keypadRead();  //keypadRead함수 호출
    if(keyread != 0)  //키패드를 눌렀을 때
    {
      if(keyread != 'A')
      {
        goto not_activated;
      }
      if(keyread == 'A')
      {
        Serial.println("활성");
      }
    }
  }
  keyread = keypadRead(); //keypadRead함수 호출
  while(keyread == 0)  //키패드를 누르지 않는 동안 keyread에 0이 반환
  {
    keyread = keypadRead();  //keypadRead함수 다시 호출
    if(keyread != 0)  //키패드에서 0이 아닌 값이 반환되면 아래코드 실행
      {
      if(keyread == 'D')  //활성 후 ‘D’ 입력 시 비밀번호 변경 시작
      {
        Serial.println("Change_Password");
        Serial.println("Write_Before_Password");
        tf = pass();  //비밀번호 일치 확인 함수 호출(일치 - 'T'반환, 불일치 - 'F'반환)
        if(tf != 0)
        {
          if(tf == 0)  //비밀번호 입력중 취소한 경우
          {
            goto not_activated;
          }
          if(tf == 'F')  //입력한 번호가 기존 번호와 일치하지 않는 경우
          {
            Serial.print("Write_Correct_Before_Password, 실패횟수 : ");
            Serial.println(wrong);
            goto not_activated;
          }
          if(tf == 'T')  //입력한 번호가 기존 번호와 일치하는 경우
          {
            Serial.println("입력할 비밀번호의 자릿수를 입력하세요");
            keyread = keypadRead();
            while(keyread == 0)
            {
              keyread = keypadRead();
              if(keyread != 0)  //자릿수(0-9) 설정
              {
                if(keyread == '0')
                  cipher=0;
                if(keyread == '1')
                  cipher=1;
                if(keyread == '2')
                  cipher=2;
                if(keyread == '3')
                  cipher=3;
                if(keyread == '4')
                  cipher=4;
                if(keyread == '5')
                  cipher=5;
                if(keyread == '6')
                  cipher=6;
                if(keyread == '7')
                  cipher=7;
                if(keyread == '8')
                  cipher=8;
                if(keyread == '9')
                  cipher=9;
                Serial.print("자릿수 : ");
                Serial.println(cipher);
              }
            }  //자리수 확인 끝
               //비밀번호 저장 시작
            Serial.println("변경할 비밀번호를 입력하세요");
            for(int i=0; i<cipher; i++)
            {
              keyread = keypadRead();
              while(keyread == 0)
              {
                keyread = keypadRead();
                if(keyread != 0)
                {
                  savepassword[i]=keyread; //savepassword 배열의 i번째 행에 저장
                  Serial.print(savepassword[i]);
                }
              }
            }
            Serial.println("");
            Serial.println("Saved Password");
            goto not_activated;  //not_activated(비활성)으로 이동
          }
        }
      }  //비밀번호 변경 끝

      if(keyread == '0' || keyread == '1' || keyread == '2' || keyread == '3' ||
         keyread == '4' || keyread == '5' || keyread == '6' || keyread == '7' ||
         keyread == '8' || keyread == '9')  //활성 후 ‘0-9’ 입력 시 비밀번호로 잠금 해제 시작
      {
        tf = pass();  //비밀번호 일치 확인 함수 호출
        if(tf == 0)  //비밀번호 입력중 취소한 경우
        {
          goto not_activated;
        }
        if(tf == 'F')  //비밀번호가 일치하지 않는 경우
        {
          Serial.print("Rewirte_Correct_Password, 실패횟수 : ");
          Serial.println(wrong);
          goto not_activated;
        }
        if(tf == 'T')  //비밀번호가 일치하는 경우
        {
          digitalWrite(doordatapin, HIGH);  //2번 아두이노로 HIGH 전송
                                            //문열림
          sw=Switch();  //Switch함수 호출
          if(sw==1)  //스위치가 눌러 졌을 때(문이 닫혔을 때 => 반환값 : 1)
          {
            digitalWrite(doordatapin, LOW);  //2번 아두이노로 0(LOW)전송
            sw=2;  //sw값 2로 초기화(아무런 의미 없는 수)
            goto not_activated;
          }
        }
      }
      if(keyread == 'R')  //‘R’(RFID)버튼을 누른 경우
      {
        int cnt = 0;
        Serial.println("RFID");
        digitalWrite(RFIDdatapin, HIGH);  //2번 아두이노로 1(HIGH)전송
        checkdata=digitalRead(checkdatapin); //2번 아두이노에서 데이터 받음(문열림:1, 문닫힘:0)
        while(checkdata==0)
        {
          cnt++;
          delay(1);  //1ms(0.01s)마다 cnt 1씩 증가
          checkdata=digitalRead(checkdatapin);  //데이터 다시 받음(문열림:1, 문닫힘:0)
          if(checkdata==1)
          {
            digitalWrite(doordatapin, HIGH);  //2번 아두이노로 HIGH 전송
                                              //문열림
            sw=Switch();  //Switch함수 호출
            if(sw==1)  //스위치가 눌러 졌을때(문이 닫혔을때 => 리턴값 : 1)
            {
              digitalWrite(doordatapin, LOW);  //2번 아두이노로 0(LOW)전송
              delay(500);
              sw=2;  //sw값 2로 초기화(아무런 의미 없는 수)
              goto not_activated;
            }
          }
          if(cnt>10000)  //10초동안 checkdata == 0일 경우
          {
            goto not_activated;  //비활성으로 이동
          }
        }
        checkdata=0;
      }
    }
  }
}


char keypadRead()  //keypad값 받아들이는 함수
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
        while(digitalRead(rowPins[j])==0)
        {
          
        }
        key = keys[j][i];
      }
    }
    digitalWrite(colPins[i],1);
  }
  return key; //key값 리턴
}


char pass()  //비밀번호 일치 확인 함수
{
  char inkey[9]={};  //입력받은 번호 저장할 배열
  for(int i=0; i<cipher; i++)  //자릿수반큼 반복문실행
  {
    keyread = keypadRead();  //키패드 함수 호출
    while(keyread == 0)  //키패드를 누르지 않는동안 0이 반환되기 때문에 keypad값이 0인동안 계속 반복
    {
      keyread = keypadRead();
      if(keyread != 0)  //키패드에서 0이아닌값이 반환되면 반환값 inkey에 저장
      {
        inkey[i] = keyread;  //i번째 배열에 비밀번호 저장
        Serial.print("입력된 번호 : ");
        Serial.println(keyread);
        if(keyread == '#')  //비밀번호 입력중 취소하면
        {
          return 0; //0값 반환(비활성으로 되돌아감)
        }
        if(keyread == 'A' || keyread == 'B' || keyread == 'R' || keyread == 'D')
        {
          Serial.println("ERROR!!!");
          Serial.println("Please Write Only Number");
          return 0; //비밀번호 확인 중 숫자가 아닌 값이 들어올 경우 비활성으로
        }
      }
    }
  }

  for(int i=0; i<cipher; i++)  //비밀번호 일치 확인
  {
    if(inkey[i] != savepassword[i])  //입력된 번호와 저장한 번호가 일치하지 않으면
    {
      wrong++;  // 틀린횟수 1증가
      if(wrong>9) //10번이상 오류시
      {
        Serial.println("10회이상 비밀번호 틀림");
        delay(30000);  //30초 대기
      }
      return 'F';  //'F'반환후
    }
  }
  return 'T';  //다맞은경우 'T'반환
}


int Switch()  //스위치 누름 확인함수
{
  int check = 1;
  reading = digitalRead(inswitchpin);
  while(check == 1)
  {
    if(reading==LOW)  //LOW값 들어오면 1반환
    {
      delay(10);
      check = 0;
    }
    reading = digitalRead(inswitchpin);
    delay(10);
  }
  return 1;
}
