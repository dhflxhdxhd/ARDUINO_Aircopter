void setup() {
  pinMode(5,OUTPUT);

}

void loop() {
  // 주기 1s 
  // Not working
  /* 
  digitalWrite(5,1);
  delay(0);
  digitalWrite(5,0);
  delay(1000); 
  */

  // working
  /*
  digitalWrite(5,1);
  delay(1000);
  digitalWrite(5,0);
  delay(0);
   */
}
