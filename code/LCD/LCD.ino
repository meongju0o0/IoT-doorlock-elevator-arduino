#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F,16,2);  //LCD I2C 통신 설정(주소, 글자수, 줄수)

void setup() {
  lcd.init(); //LCD I2C 통신 초기화 설정
  lcd.backlight(); //배경화면 빛 설정
}

void loop() {
  lcd.print("Hello");
  delay(2000);
  lcd.init();
}
