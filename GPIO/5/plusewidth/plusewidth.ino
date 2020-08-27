#define PIN3 3

void setup()
{
  pinMode(PIN3,INPUT);
  Serial.begin(115200);
}
void loop()
{
  unsigned long duration;
  duration=pulseIn(PIN3,HIGH);
  
  Serial.print("Pulse width = ");
  Serial.print(duration);
  Serial.println("msec");
  delay(500);
}
