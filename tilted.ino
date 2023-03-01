#include <MPU6050_tockn.h>
#include <Wire.h>

int display[8][8] = {
  { 1, 1, 1, 1, 1, 1, 1, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 0 },
  { 1, 0, 0, 0, 0, 0, 0, 0 },
  { 1, 0, 0, 0, 0, 0, 0, 0 },
  { 1, 0, 0, 0, 1, 1, 1, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 0 },
  { 1, 0, 0, 0, 0, 0, 0, 0 },
  { 1, 1, 1, 1, 1, 1, 1, 1 }
};

int greenMask[8] = {0, 0, 15, 32, 0, 0, 0, 0};

MPU6050 mpu6050(Wire);

const int clockPin = 12, dataPin = 11, latchPin = 10;
float accX, accY, accZ;
float rotX, rotY, rotZ;
int ticks = 0;

int playerRow = 6, playerCol = 7;
int posX = playerRow*125-60, posY = playerCol*125-60;

void sendBits(int red_data, int green_data) {
  // helper function that makes it simple to set the contents of the shift register
  digitalWrite(latchPin, LOW);
  //shiftOut(dataPin, clockPin, MSBFIRST, (green_data<<8)|red_data);
  shiftOut(dataPin, clockPin, MSBFIRST, green_data);
  shiftOut(dataPin, clockPin, MSBFIRST, red_data);
  digitalWrite(latchPin, HIGH);
}

void setup() {
  Serial.begin(9600); 
  // pins 2-9 row headers, pins 10-12 for shift register
  for (int i = 2; i <= 12; ++i)
    pinMode(i, OUTPUT);

  sendBits(0, 255);
  for (int i = 2; i <= 9; ++i) digitalWrite(i, HIGH);
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
}

void setPos(int nrow, int ncol) {
  // Used for instantaneously updating player position
  playerRow = nrow;
  playerCol = ncol;
  posX = playerRow*125-63, posY = playerCol*125-63;

}

void loop() {
  // receive sensor input
  mpu6050.update();
  // read input only every 100 ticks
  if (ticks % 40 == 0) {
    // gyro
    rotX = mpu6050.getAngleX();
    rotY = mpu6050.getAngleY();

    posX = constrain(posX - rotX / 90.0 * 125, 0, 999);
    posY = constrain(posY + rotY / 90.0 * 125, 0, 999);
    playerRow = posY / 125;
    playerCol = posX / 125;

    if (display[playerRow][playerCol]) {
      setPos(6, 7);
    }

  }

  for (int r = 0; r < 8; ++r) {
    int reg = 0;
    for (int c = 0; c < 8; ++c) reg |= ((display[r][c]^1) << c);
    if (r == playerRow) reg &= 255 - (1<<playerCol);
    sendBits(reg, greenMask[r]^0b11111111);
    digitalWrite(r + 2, HIGH);
    delay(1);
    digitalWrite(r + 2, LOW);
  }


  // tick rate, 8ms for display refresh
  ticks += 8;
}
