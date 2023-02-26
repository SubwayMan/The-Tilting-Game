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

void sendBits(int data) {
  // helper function that makes it simple to set the contents of the shift register
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, data);
  digitalWrite(latchPin, HIGH);
}
void setup() {
  //pins 2-9 row headers, pins 10-12 for shift register
  for (int i=2; i<=12; ++i)
    pinMode(i, OUTPUT);

}

void loop() {
  for (int r=0; r<8; ++r) {
    int reg=0;
    for (int c=0; c<8; ++c) reg |= (mask[r][c]<<c);
    sendBits(reg);
    digitalWrite(r+2, HIGH);
    delay(1);
    digitalWrite(r+2, LOW);

  }

  // put your main code here, to run repeatedly:

}
