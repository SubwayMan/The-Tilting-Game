#include <MPU6050_tockn.h>
#include <Wire.h>

uint64_t test = 0b1111111011111111111111111000111111111111111110000111111111111111LL;

int nLevels = 4;
int levelData[4][8] = {
  {
    0b11111111,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10001111,
    0b10000000,
    0b10000000,
    0b11111111
  },
  {
    0b11111111,
    0b10000000,
    0b10001000,
    0b10001000,
    0b11001111,
    0b10000000,
    0b10010000,
    0b11111111
  },
  {
    0b11111111,
    0b10000001,
    0b10111111,
    0b10000001,
    0b11111101,
    0b10000001,
    0b10000001,
    0b11111111
  },
  {
    0b11111111,
    0b10100011,
    0b10001001,
    0b11110101,
    0b10000101,
    0b10111101,
    0b10000001,
    0b11111111
  }
};

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

MPU6050 mpu6050(Wire);

const int clockPin = 12, dataPin = 11, latchPin = 10;
float accX, accY, accZ;
float rotX, rotY, rotZ;
int ticks = 0;

//death mark
int ldRow=-1, ldCol=-1;

struct Level {
  int startRow, startCol;
  int exitRow, exitCol;
  int levelData[8];

  Level(int startRow, int startCol, int exitRow, int exitCol, int *mask) {
    this->startRow = startRow;
    this->startCol = startCol;
    this->exitRow = exitRow;
    this->exitCol = exitCol;
    for (int i=0; i<8; ++i) {
      this->levelData[i] = *(mask+i);
    }
  }
};

Level levels[4] = {
  Level(5, 1, 1, 1, levelData[0]),
  Level(5, 0, 3, 1, levelData[1]),
  Level(5, 6, 1, 1, levelData[2]),
  Level(1, 6, 3, 3, levelData[3])
};

int cLevel = 0;

int playerRow = levels[0].startRow, playerCol = levels[0].startCol;
int posX = playerRow*125+60, posY = playerCol*125+60;

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
  posY = playerRow*125+63, posX = playerCol*125+63;

}



void setup() {
  Serial.begin(9600); 
  // pins 2-9 row headers, pins 10-12 for shift register
  for (int i = 2; i <= 12; ++i)
    pinMode(i, OUTPUT);

  sendBits(0, 0);
  for (int i = 2; i <= 9; ++i) digitalWrite(i, HIGH);
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);

  while (test) {
    Serial.print((int)test&1);
    test >>= 1;
  }
  Serial.println();

  setPos(levels[cLevel].startRow, levels[cLevel].startCol);
}

void loop() {

  // victory check
  if (cLevel == nLevels) {
    for (int r=0; r<8; ++r) {
      sendBits(victoryScreen[r], victoryScreen[r]^255);
      digitalWrite(r + 2, HIGH);
      delay(1);
      digitalWrite(r + 2, LOW);
    }
    return;
  }

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

    if (levels[cLevel].levelData[playerRow]&(1<<playerCol)) {
      ldRow = playerRow;
      ldCol = playerCol;
      setPos(levels[cLevel].startRow, levels[cLevel].startCol);
    }

    if (ticks % 400 == 0) {
      Serial.print("x rotation "); Serial.print(rotX);
      Serial.print(" y rotation "); Serial.println(rotY);    
      char s[50]; sprintf(s, "xpos: %d, ypos: %d", (int)posX, (int)posY);
      Serial.println(s);
    }

  }


  // reset the green bitmask
  for (int r = 0; r < 8; ++r) {
    int green=0;    
    if (r == playerRow) green |= 1<<playerCol; 
    if (r == levels[cLevel].exitRow && (ticks/80)%2 == 0) green |= 1 << levels[cLevel].exitCol;
    if (r == ldRow) green |= 1 << ldCol;
    
    sendBits(levels[cLevel].levelData[r]^255, green^255); // reverse bitmask, since 0 is ON
    digitalWrite(r + 2, HIGH);
    delay(1);
    digitalWrite(r + 2, LOW);
  }
  if (playerRow == levels[cLevel].exitRow && playerCol == levels[cLevel].exitCol) {
    cLevel++;
    ldRow=-1;
    ldCol=-1;
    if (cLevel < (sizeof(levels)/sizeof(levels[0])))
      setPos(levels[cLevel].startRow, levels[cLevel].startCol);
  }


}
