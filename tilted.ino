int mask[4][8] = {
  {0, 1, 1, 1, 0, 0, 0, 0},
  {1, 1, 0, 0, 0, 0, 0, 0},
  {1, 1, 1, 1, 0, 0, 0, 0},
  {0, 1, 0, 1, 0, 0, 0, 0}
};

void setup() {
  // put your setup code here, to run once:
  for (int i=2; i<=13; ++i)
    pinMode(i, OUTPUT);

}

void loop() {
  for (int r=0; r<4; ++r) {
    for (int c=0; c<8; ++c) {
      digitalWrite(c+6, (mask[r][c])?LOW:HIGH);
    }
    digitalWrite(r+2, HIGH);
    delay(1);
    digitalWrite(r+2, LOW);
  }

  // put your main code here, to run repeatedly:

}
