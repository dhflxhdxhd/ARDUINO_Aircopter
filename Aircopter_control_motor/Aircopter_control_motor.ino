void setup() {
  Serial1.begin(115200);
  pinMode(5, OUTPUT);
}

void loop() {
  /*
    if (Serial.available() > 0) {
    //char value = Serial.read();
    int value  = Serial.parseInt();
    //Serial.write(49); 아스키 값으로 출력 (binary data)
    Serial.print(value);

    if (value == 1) {
      Serial.println("ON");
    // analogWrite(6, 100);
    analogWrite(5,50)
      delay(500);
    } /*else {
      Serial.println("OFF");
      analogWrite(6, 0);
    }
    }
    if (Serial1.available()) {
    char blueData = Serial1.read();
    Serial.write(blueData);
    Serial.println();

    if (blueData == '0')
    analogWrite(5, 0);
    }
  */

  //  control motor speed
  if (Serial1.available() > 0) {
    char userInput = Serial1.read();
    Serial1.println(userInput);

    if (userInput >= '0' && userInput <= '9') {
      int throttle = (userInput - '0') * 10;
      analogWrite(6, throttle);
      analogWrite(10, throttle);
      analogWrite(9, throttle);
      analogWrite(5, throttle);
    }
  }
}
