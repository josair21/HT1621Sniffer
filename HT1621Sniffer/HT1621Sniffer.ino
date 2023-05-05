#define DataPin 8  //Salida de datos de HT1621
#define WRPin 2    //Salida de clock de HT1621
#define CSPin 3    //Salida de control de HT1621
bool Data[10][8] = { 0 };
bool Address[10][8] = { 0 };
void IRAM_ATTR ReadBit();
void IRAM_ATTR WaitForData();
uint8_t State = 0;
uint32_t lastMillis = millis();
void setup() {
  pinMode(DataPin, INPUT_PULLUP);
  pinMode(WRPin, INPUT_PULLUP);
  pinMode(CSPin, INPUT_PULLUP);

  Serial.begin(9600);

  attachInterrupt(CSPin, ReadBit, RISING);
}

void loop() {
  if (millis() - lastMillis > 2 && State == 1) {
    detachInterrupt(CSPin);
    float Weight = 0.0;
    uint8_t DecimalPointIndex = 0;
    bool IsCorrectFormat = true;
    for (uint8_t i = 0; i < 6; i++) {
      if (BitsToByte(Address[i]) != 16 + i * 2) false;
      if (Data[i][7]) DecimalPointIndex = i;
      Weight += BitsToFloat(Data[i]) * pow(10, i);
    }
    Weight /= pow(10, DecimalPointIndex);
    if (IsCorrectFormat) Serial.print(Weight);
    State = 2;
    lastMillis = millis();
    attachInterrupt(CSPin, WaitForData, RISING);
  }

  if (State == 3 && millis() - lastMillis > 2) {
    attachInterrupt(CSPin, ReadBit, RISING);
    State = 0;
  }
}

void IRAM_ATTR ReadBit() {
  bool BitReaded = digitalRead(DataPin);
  static uint8_t BitCounter = 0;
  static uint8_t Index = 0;
  switch (BitCounter) {
    case 0:
    case 2:
      if (BitReaded != 1) BitCounter = 0;
      else BitCounter++;
      return;
    case 1:
      if (BitReaded != 0) BitCounter = 0;
      else BitCounter++;
      return;
    case 3 ... 8:
      Data[Index][BitCounter - 3] = BitReaded;
      BitCounter++;
      return;
    case 9 ... 15:
      Data[Index][BitCounter - 9];
      BitCounter++;
      return;
    case 16:
      Data[Index][BitCounter - 9];
      BitCounter = 0;
      lastMillis = millis();
      if(Index < 5) Index++;
      else Index = 0;
      State = 1;
      return;
  }
}

void IRAM_ATTR WaitForData() {
  lastMillis = millis();
  State = 3;
}

uint8_t BitsToByte(bool Bits[]) {
  uint8_t Byte = 0;
  for (uint8_t i = 0; i < 8; i++) {
    if (Bits[i]) Byte += pow(2, i);
  }
  return Byte;
}

float BitsToFloat(bool Bits[]) {
  switch (BitsToByte(Bits) & 0b01111111) {
    case 0b1111101:
      return 0;
    case 0b1100000:
      return 1;
    case 0b111110:
      return 2;
    case 0b1111010:
      return 3;
    case 0b1100011:
      return 4;
    case 0b1011011:
      return 5;
    case 0b1011111:
      return 6;
    case 0b1110000:
      return 7;
    case 0b1111111:
      return 8;
    case 0b1111011:
      return 9;
  }
}