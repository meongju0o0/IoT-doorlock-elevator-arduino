const int numRows = 4;
const int numCols = 4;
/* 4X4키패드 */
char keys[numRows][numCols]={
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'#','0','*','D'}
}; //키패드 설정
const int colPins[] = {45,43,41,39};
const int rowPins[] = {47,49,51,53};

void setup()
{
  for (int i = 0 ; i < numRows; i++)
  {
    pinMode(rowPins[i],INPUT_PULLUP);
  }
  for (int i = 0 ; i < numCols; i++)
  {
    pinMode(colPins[i],OUTPUT);
    digitalWrite(colPins[i],HIGH);
  } // keypad 출력 (pullup, output 설정)
  Serial.begin(9600);
}

void loop() 
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
        while(digitalRead(rowPins[j])==0){
        }
        key = keys[j][i];
      }
    }
    digitalWrite(colPins[i],1);
  }
  if(key != 0)
  {
    Serial.println(key);
  }
}
