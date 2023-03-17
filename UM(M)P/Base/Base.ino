
int IN1=8;
int IN2=9;
int IN3=7;
int IN4=5;
int ENA=10;
int ENB=6;

void setup()
{
 for (int i = 5; i <11; i ++) 
   {
     pinMode(i, OUTPUT);   
   }
  
}
void loop()
{
  
  digitalWrite(IN1,HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA,255); 
  digitalWrite(IN3,HIGH);
  digitalWrite(IN4,LOW); 
  analogWrite(ENB,255); 
  delay(10000);
  analogWrite(ENA,0); 
  analogWrite(ENB,0); 
  delay(5000); 
  digitalWrite(IN1,LOW);
  digitalWrite(IN2,HIGH);
  analogWrite(ENA,255); 
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,HIGH); 
  analogWrite(ENB,255); 
  delay(10000);
  analogWrite(ENA,0); 
  analogWrite(ENB,0); 
  delay(5000); 
  
  
  
  
}
