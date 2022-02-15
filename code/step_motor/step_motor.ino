#include <Stepper.h>  //스텝모터 라이브러리

const int stepsPerRevolution = 2560;  //1바퀴 회전
Stepper stepper(stepsPerRevolution,11,9,10,8);  //연결한 핀

void setup() {
  stepper.setSpeed(8);  //속도 조정 (15가 무리 없이 적당)
}

void loop() {
  stepper.step(-128);  //1바퀴 회전
  while(1)
  {
  }
}
