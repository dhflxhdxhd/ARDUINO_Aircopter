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

int throttle = 0;
void loop() {
  Wire.beginTransmission(0x68); //68번지 통신 시작
  Wire.write(0x43);// 43번지 : roll값
  Wire.endTransmission(false); //아직 통신 다 끝나지 않음 -> 인자로 false값 -> 데이터 전송 후 통신 재시작 메세지.
  Wire.requestFrom(0x68, 6, true); // 추가적 데이터 요구. 68번지에서 2바이트를 가져오겠다.
  int16_t GyXH = Wire.read(); // high값 읽음
  int16_t GyXL = Wire.read(); // low값 읽음
  int16_t GyYH = Wire.read(); // high값 읽음
  int16_t GyYL = Wire.read(); // low값 읽음
  int16_t GyZH = Wire.read(); // high값 읽음
  int16_t GyZL = Wire.read(); // low값 읽음
  int16_t GyX = GyXH << 8 | GyXL; // 두 값(high, low)을 합침.
  int16_t GyY = GyYH << 8 | GyYL; // 두 값(high, low)을 합침.
  int16_t GyZ = GyZH << 8 | GyZL; // 두 값(high, low)을 합침.

  /* Roll 각속도와 각도 구하기 start */
  static int32_t GyXSum, GyYSum, GyZSum = 0;
  static double GyXOff, GyYOff, GyZOff = 0.0;
  static int cnt_sample = 1000;
  if (cnt_sample > 0) {
    GyXSum += GyX , GyYSum += GyY, GyZSum += GyZ;
    cnt_sample --;
    if (cnt_sample == 0) {
      GyXOff = GyXSum / 1000.0;
      GyYOff = GyYSum / 1000.0;
      GyZOff = GyZSum / 1000.0;
    }
    delay(1);
    return;
  }
  double GyXD = GyX - GyXOff; // 값을 보정
  double GyYD = GyY - GyYOff;
  double GyZD = GyZ - GyZOff;
  double GyXR = GyXD / 131.0; // [추가] 회전 각속도(GyYR)
  double GyYR = GyYD / 131.0;
  double GyZR = GyZD / 131.0;
  // 1초 동안 1도 회전할 경우의 GyY는 (32768/250 = 131)
  /* Roll 각속도와 각도 구하기 end */

  /* 주기 계산 start */
  static unsigned long t_pre = 0;
  unsigned long t_now = micros(); // 시간함수
  double dt = (t_now - t_pre) / 1000000.0;
  t_pre = t_now;
  /* 주기 계산 end */

  /* 회전 각도 구하기 start */
  static double AngleX, AngleY, AngleZ = 0.0;
  //double Angle = GyYR * dt;
  AngleX += GyXR * dt; // "회전 각도(AngleY)=회전 각속도(GyYR)*주기(dt)" 누적
  AngleY += GyYR * dt;
  AngleZ += GyZR * dt;
  if (throttle == 0) AngleX = AngleY = AngleZ = 0.0;
  /* 회전 각도 구하기 end */

  /* 좌우 균형 값 찾기 start */
  static double tAngleX, tAngleY, tAngleZ = 0.0;
  double eAngleX = tAngleX - AngleX;
  double eAngleY = tAngleY - AngleY;
  double eAngleZ = tAngleZ - AngleZ;
  double Kp = 1.0; // 증폭 값을 저장할 변수
  // 여러가지 조건들에 의해 성능이 달라질 수 있으므로 기울어진 각도가 같더라도 서로 다른 힘으로 기울어진 각도 보정해야함.(Kp의 역할)
  // 좌우 균형 값(각도 보정 힘)=증폭값*(각도보정(-)*기운 각도)
  double BalX = Kp * eAngleX;
  double BalY = Kp * eAngleY;
  double BalZ = Kp * eAngleZ;
  /* 좌우 균형 값 찾기 end*/

  /* 회전 속도 상쇄 start */
  double Kd = 1.0;
  BalX += Kd * -GyXR;
  BalY += Kd * -GyYR;
  BalZ += Kd * -GyZR;
  if (throttle == 0)BalX = BalY = BalZ = 0.0;
  /* 회전 속도 상쇄 end */

  /* 목표 각도 도달하기 start */
  double Ki = 1.0; // 증폭값 저장 변수
  static double ResX , ResY, ResZ = 0.0; // 목표에 도달한 힘 값을 저장할 변수
  ResX += Ki * eAngleX * dt;
  ResY += Ki * eAngleY * dt;
  ResZ += Ki * eAngleZ * dt;
  if (throttle == 0)ResX = ResY = ResZ = 0.0;
  BalX += ResX;
  BalY += ResY;
  BalZ += ResX;
  /* 목표 각도 도달하기 end */

  if (Serial.available() > 0) {
    while (Serial.available() > 0) {
      char userInput = Serial.read();
      if (userInput > '0' && userInput <= '9') {
        throttle = (userInput - '0') * 25;
      }
    }
  }

  /* 모터 속도 계산 start */
  double speedA = throttle + BalY + BalX + BalZ;
  double speedB = throttle - BalY + BalX - BalZ;
  double speedC = throttle - BalY - BalX + BalZ;
  double speedD = throttle + BalY - BalX - BalZ;
  /* 모터 속도 계산 end */

  // 모터 A,B,C,D에 대한 속도를 0과 250 사이로 보정
  int iSpeedA = constrain((int)speedA, 0, 250);
  int iSpeedB = constrain((int)speedB, 0, 250);
  int iSpeedC = constrain((int)speedC, 0, 250);
  int iSpeedD = constrain((int)speedD, 0, 250);

  // 4개의 모터에 속도값 적용
  analogWrite(6, iSpeedA);
  analogWrite(10, iSpeedB);
  analogWrite(9, iSpeedC);
  analogWrite(5, iSpeedD);

  static int cnt_loop;
  cnt_loop++;
  if (cnt_loop % 100 != 0)return;


  Serial.print("GyY = "); Serial.print(GyY); // GyY : Roll(y) 값
  Serial.print(" |GyYD = "); Serial.print(GyYD); // GyYD : 보정한 Roll(y) 값
  Serial.print(" |GyYR = "); Serial.print(GyYR); // GyYR : 회전 각속도
  Serial.print(" |dt= "); Serial.print(dt, 6); // dt : 주기(시간)
  //Serial.print(" Angle= "); Serial.print(Angle);
  Serial.print(" |AngleY= "); Serial.print(AngleY); // AngleY : 회전각도
  Serial.print(" |BalY= "); Serial.print(BalY); // BalY : 좌우 균형 값
  Serial.print(" |A= "); Serial.print(speedA);
  Serial.print(" |B= "); Serial.print(speedB);
  Serial.print(" |C= "); Serial.print(speedC);
  Serial.print(" |D= "); Serial.print(speedD);
  Serial.println();

}
