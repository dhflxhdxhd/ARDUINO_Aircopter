//MSP 메세지 입력받기
void setup() {
  Serial1.begin(115200);
  Serial.begin(115200);
}

void loop() {
  if (Serial1.available() > 0) {
    while (Serial1.available() > 0) {
      //char -> uint8_t
      uint8_t msp_data = Serial1.read();

      Serial.print((char)msp_data);
      Serial.print('\t');
      Serial.print(msp_data, DEC);
      Serial.println();
    }
  }
}
