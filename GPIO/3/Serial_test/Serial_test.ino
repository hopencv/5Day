void setup()
{
  Serial.begin(115200);  
}

void loop()
{
  char test='A';
  int i;
  for(i=0;i<16;i++){
    test=test+1;
  Serial.print(test);
  delay(500);
  }
}
