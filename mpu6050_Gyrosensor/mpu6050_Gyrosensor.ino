/*자이로센서 GyroSensor*/

#include <Wire.h> //IC2통신할  때 필요한 파일. 

void setup() {
  Serial.begin(115200); //통신 속도 설정

  Wire.begin(); //IC2통신 기능 활성화
  Wire.setClock(400000); //통신 속도 설정. 400KHz(400Kbps)로 설정
  Wire.beginTransmission(0x68); //I2C통신을 위해서 주소 필요. -> 68번지 주소 불러줌. 통신 시작
  Wire.write(0x6b); //전송하고자 하는 1바이트 데이터를 내부 메모리 큐에 저장하는 역할
  Wire.write(0x0); // 0 sleep모드 해지
  Wire.endTransmission(true); //통신 끝
}


void loop() {
  Wire.beginTransmission(0x68); //68번지 통신 시작
  Wire.write(0x45);// 45번지 : roll값
  Wire.endTransmission(false); //아직 통신 다 끝나지 않음 -> 인자로 false값 -> 데이터 전송 후 통신 재시작 메세지.
  Wire.requestFrom(0x68, 2, true); // 추가적 데이터 요구.
  int16_t GyYH = Wire.read(); // high값 읽음
  int16_t GyYL = Wire.read(); // low값 읽음
  int16_t GyY = GyYH << 8 | GyYL; // 두 값(high, low)을 합침.

  /* [추가] Roll 각속도와 각도 구하기 start */
  static int32_t GyYSum = 0;
  static double GyYOff = 0.0;
  static int cnt_sample = 1000;
  if (cnt_sample > 0) {
    GyYSum += GyY;
    cnt_sample --;
    if (cnt_sample == 0)GyYOff = GyYSum / 1000.0;
    delay(1);
    return;
  }
  double GyYD = GyY - GyYOff; // 값을 보정
  double GyYR = GyYD / 131.0; // [추가] 회전 각속도(GyYR)
  // 1초 동안 1도 회전할 경우의 GyY는 (32768/250 = 131)
  /* Roll 각속도와 각도 구하기 end */

  /* 주기 계산 start */
  static unsigned long t_pre = 0;
  unsigned long t_now = micros();
  double dt = (t_now - t_pre) / 1000000.0;
  t_pre = t_now;
  /* 주기 계산 end */

  static int cnt_loop;
  cnt_loop++;
  if (cnt_loop % 100 != 0)return;

  Serial.print("GyY = "); Serial.print(GyY); // GyY : Roll(y) 값
  Serial.print(" GyYD = "); Serial.print(GyYD); // GyYD : 보정한 Roll(y) 값
  Serial.print(" GyYR = "); Serial.print(GyYR); // GyYR : 회전 각속도
  Serial.print(" dt= "); Serial.print(dt, 6); // dt : 주기(시간)
  Serial.println();

}
