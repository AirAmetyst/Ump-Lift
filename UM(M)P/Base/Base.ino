// подключите один мотор к клемме: M1+ и M1-
// а второй к клемме: M2+ и M2-
// Motor shield использует четыре контакта 4, 5, 6, 7 для управления моторами 
// 4 и 7 — для направления, 5 и 6 — для скорости
#define SPEED_1      5 
#define DIR_1        4
 
#define SPEED_2      6
#define DIR_2        7
 
void setup() {
  // настраиваем выводы платы 4, 5, 6, 7 на вывод сигналов 
  for (int i = 4; i < 8; i++) {     
    pinMode(i, OUTPUT);
  }
} 
 
void loop() {
  // устанавливаем направление мотора «M1» в одну сторону
  
  
  
 
  // устанавливаем направление мотора «M1» в другую сторону
  digitalWrite(DIR_1, LOW);
  analogWrite(SPEED_1, 255);
  digitalWrite(DIR_2, HIGH);
  analogWrite(SPEED_2, 255);
  // ждём одну секунду
  delay(1000);
  // выключаем первый мотор
  analogWrite(SPEED_1, 0);
  analogWrite(SPEED_2, 0);
 
  
}