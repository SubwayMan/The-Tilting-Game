#include <MPU6050_tockn.h>
#include <Wire.h>

int victoryScreen[8] = {
  0b00011100,
  0b00100100,
  0b10111110,
  0b11111110,
  0b11111110,
  0b10111110,
  0b00100100,
  0b00011100
};

int redMask[8] = {
  0b11111111,
  0b10000000,
  0b10000000,
  0b10000000,
  0b10001111,
  0b10000000,
  0b10000000,
  0b11111111
};

int greenMask[8] = {0, 0, 0, 0, 0, 0, 0, 0};

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

void setPos(int nrow, int ncol) {
  // Used for instantaneously updating player position
  playerRow = nrow;
  playerCol = ncol;
  posX = playerRow*125-63, posY = playerCol*125-63;

}

struct Level {
  int startRow, startCol;
  int exitRow, exitCol;
  int redMask[8];
  int greenMask[8];

  Level(int startRow, int startCol, int exitRow, int exitCol, int *mask1, int *mask2) {
    this->startRow = startRow;
    this->startCol = startCol;
    this->exitRow = exitRow;
    this->exitCol = exitCol;
    for (int i=0; i<8; ++i) {
      this->redMask[i] = *(mask1+i);
      this->greenMask[i] = *(mask2+i);
    }
    setPos(startRow, startCol);
  }
};

Level cLevel(6, 7, 1, 1, redMask, greenMask);
bool victoryFlag = false;

void setup() {
  Serial.begin(9600); 
  // pins 2-9 row headers, pins 10-12 for shift register
  for (int i = 2; i <= 12; ++i)
    pinMode(i, OUTPUT);

  sendBits(255, 255);
  for (int i = 2; i <= 9; ++i) digitalWrite(i, HIGH);
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
}

void loop() {
  // tick rate, 8ms for display refresh
  ticks += 8;
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

    if (redMask[playerRow]&(1<<playerCol)) {
      setPos(cLevel.startRow, cLevel.startCol);
    }

  }

  // victory screen
  if (victoryFlag) {
    for (int r=0; r<8; ++r) {
      sendBits(victoryScreen[r], victoryScreen[r]^255);
      digitalWrite(r + 2, HIGH);
      delay(1);
      digitalWrite(r + 2, LOW);
    }
    return;
  }

  // reset the green bitmask
  for (int i=0; i<8; ++i) greenMask[i] = cLevel.greenMask[i];
  for (int r = 0; r < 8; ++r) {
    if (r == playerRow) greenMask[r] |= 1<<playerCol; 
    if (r == cLevel.exitRow && (ticks/80)%2 == 0) greenMask[r] |= 1 << cLevel.exitCol;
    
    sendBits(cLevel.redMask[r]^255, greenMask[r]^255); // reverse bitmask, since 0 is ON
    digitalWrite(r + 2, HIGH);
    delay(1);
    digitalWrite(r + 2, LOW);
  }
  if (playerRow == cLevel.exitRow && playerCol == cLevel.exitCol) {
    victoryFlag=true;
  }


}
