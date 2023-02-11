#include "OLED_I2C.h"
#include "GyverEncoder.h"

OLED myOLED(SDA, SCL, 8);

#define CLK 7
#define DT 6
#define SW 5
Encoder enc1(CLK, DT, SW, TYPE1);

extern uint8_t TinyFont[];
extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
extern uint8_t BigNumbers[];

int value = 0;
int i = 0;

void setup(){
   myOLED.begin();  // инициализируем экран
   Serial.begin(9600);
}

void loop(){
  enc1.tick();
  myOLED.clrScr();
  myOLED.setFont(BigNumbers);
  myOLED.print(String(i), CENTER, 25);
  myOLED.update();
  if (enc1.isRight()) {
       i++;
    }
  else if (enc1.isLeft()) {
       i--;
    }
}
