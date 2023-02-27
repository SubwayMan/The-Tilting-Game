#include <MPU6050_tockn.h>
#include <Wire.h>

int display[8][8] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 0},
  {0, 1, 0, 0, 0, 0, 1, 0},
  {0, 1, 0, 0, 0, 0, 1, 0},
  {0, 1, 0, 0, 0, 0, 1, 0},
  {0, 1, 0, 0, 0, 0, 1, 0},
  {0, 1, 1, 1, 1, 1, 1, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};

MPU6050 mpu6050(Wire);

const int clockPin=12, dataPin=11, latchPin=10;
float accX, accY, accZ;
float rotX, rotY, rotZ; 
int ticks=0;

void sendBits(int data) {
  // helper function that makes it simple to set the contents of the shift register
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, data);
  digitalWrite(latchPin, HIGH);
  
}

void setup() {
  Serial.begin(9600);
  // pins 2-9 row headers, pins 10-12 for shift register
  for (int i=2; i<=12; ++i)
    pinMode(i, OUTPUT);

  sendBits(0);
  for (int i=2; i<=9; ++i) digitalWrite(i, HIGH);
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
}

void loop() {
  // receive sensor input
  mpu6050.update();
  // acceeration
  accX = mpu6050.getAccX();
  accY = mpu6050.getAccY();
  accZ = mpu6050.getAccZ();
  // gyro
  rotX = mpu6050.getAngleX();
  rotY = mpu6050.getAngleY();
  rotZ = mpu6050.getAngleZ();

  if (ticks%400==0) {
    Serial.print("angleX : ");Serial.print(mpu6050.getAngleX());
    Serial.print("\tangleY : ");Serial.print(mpu6050.getAngleY());
    Serial.print("\tangleZ : ");Serial.println(mpu6050.getAngleZ());
  }

  
  for (int r=0; r<8; ++r) {
    int reg=0;
    for (int c=0; c<8; ++c) reg |= (display[r][c]<<c);
    sendBits(reg);
    digitalWrite(r+2, HIGH);
    delay(1);
    digitalWrite(r+2, LOW);
  }
  

  // tick rate, 8ms for display refresh
  ticks+=8;

}
