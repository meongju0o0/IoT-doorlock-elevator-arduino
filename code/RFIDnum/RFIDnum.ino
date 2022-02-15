/*MOSI --> Pin 11
 *MISO --> Pin 12
 *SCK  --> Pin 13
 *SDA  --> Pin 10
 *RST  --> Pin 9
 */
 
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 53
#define RST_PIN 49
#define DOOR_PIN 8

MFRC522 rc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

int RFIDnum[4]={0,0,0,0};
void setup() 
{
  Serial.begin(9600);
  SPI.begin();
  rc522.PCD_Init();
  Serial.println("Scan card...\r\n");
}

void loop() 
{

}

int func_RFIDnum()
{
  for(int i = 0; i < 4; i++)
  {
    RFIDnum[i] = rc522.uid.uidByte[i];
  }
}
