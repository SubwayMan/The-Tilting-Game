#include <Wire.h>

int mask[8][8] = {
  {0, 1, 1, 1, 0, 0, 0, 0},
  {1, 1, 0, 0, 0, 0, 0, 0},
  {1, 1, 1, 1, 0, 0, 0, 0},
  {0, 1, 0, 1, 0, 0, 0, 0},
  {0, 1, 0, 1, 0, 0, 0, 0},
  {0, 1, 0, 1, 0, 0, 0, 0},
  {0, 1, 0, 1, 0, 0, 0, 0},
  {0, 1, 0, 1, 0, 0, 0, 0}
};

const int clockPin=12, dataPin=11, latchPin=10;
const int MPU=0x68, MPU_ACCEL=0x3B, MPU_GYRO=0x43; // addresses from MPU-6050 pinout
int accX, accY, accZ, omegaX, omegaY, omegaZ;
int rotX=0, rotY=0, rotZ=0; // Since gyro readings are given in deg/s, change in angle needs to be stored
float lastTime, curTime; 

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
  // gyro setup https://howtomechatronics.com/tutorials/arduino/arduino-and-mpu6050-accelerometer-and-gyroscope-tutorial/
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0x00); //wake up sensor
  Wire.endTransmission(true);
  curTime = millis()/1000.0;    
}

void loop() {
  // receive sensor input
  // acceeration
  Wire.beginTransmission(MPU);
  Wire.write(MPU_ACCEL);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);
  // The values are stored in two bytes, so we need to union both values
  accX = (Wire.read()<<8 | Wire.read());
  accY = (Wire.read()<<8 | Wire.read());
  accZ = (Wire.read()<<8 | Wire.read());
  // gyro
  Wire.beginTransmission(MPU);
  Wire.write(MPU_GYRO);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);

  omegaX = (Wire.read()<<8 | Wire.read());
  omegaY = (Wire.read()<<8 | Wire.read());
  omegaZ = (Wire.read()<<8 | Wire.read());

  // sum angle
  lastTime = curTime;
  curTime = millis()/1000.0;
  rotX += (float)omegaX*(curTime-lastTime);
  rotY += (float)omegaY*(curTime-lastTime);
  rotZ = (int)(rotZ + 360 + omegaZ*(curTime-lastTime)) % 360;
  char s[100];
  sprintf(s, "Accx: %d, AccY: %d, AccZ: %d", accX, accY, accZ); Serial.println(s);
  sprintf(s, "gyro x: %d, gyro y: %d, gyro z: %d", omegaX, omegaY, omegaZ); Serial.println(s);
  sprintf(s, "angle x: %d, angle y: %d, angle z: %d", rotX, rotY, rotZ); Serial.println(s);

  
  for (int r=0; r<8; ++r) {
    int reg=0;
    for (int c=0; c<8; ++c) reg |= (mask[r][c]<<c);
    sendBits(reg);
    digitalWrite(r+2, HIGH);
    digitalWrite(r+2, LOW);
    delay(1);
  }
  

  // tick rate, subtract 8 since it takes 8ms to update display
  delay(492);

}
