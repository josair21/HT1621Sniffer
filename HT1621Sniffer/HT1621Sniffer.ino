#include "BluetoothSerial.h"
BluetoothSerial SerialBT;
#define DataPin 15  //Salida de datos de HT1621
#define WRPin 4     //Salida de clock de HT1621
#define CSPin 3     //Salida de control de HT1621
bool Data[10][8] = { 0 };
bool Address[10][8] = { 0 };
void IRAM_ATTR ReadBit();
void IRAM_ATTR WaitForData();
uint8_t State = 0;
uint8_t Index = 0;
uint32_t lastMillis = millis();
void setup() {
  pinMode(DataPin, INPUT_PULLUP);
  pinMode(WRPin, INPUT_PULLUP);
  pinMode(CSPin, INPUT_PULLUP);
  Serial.begin(9600);
  Serial2.begin(9600);
  SerialBT.begin("EQUILAB CS004");

  attachInterrupt(WRPin, ReadBit, RISING);
}

void loop() {
  if (millis() - lastMillis > 2 && State == 1) {
    detachInterrupt(CSPin);
    Index = 0;
    float Weight = 0.0;
    uint8_t DecimalPointIndex = 0;
    bool IsCorrectFormat = true;
    for (uint8_t i = 0; i < 5; i++) {
      //Serial.print(BitsToByte(Address[i]));
      //Serial.print("    ");
      //Serial.println(BitsToByte(Data[i]), BIN);
      if (BitsToByte(Address[i]) != 16 + i * 2) IsCorrectFormat = false;
      if (Data[i][7]) DecimalPointIndex = i;
      Weight += BitsToFloat(Data[i]) * pow(10, i);
    }
    Weight /= pow(10, DecimalPointIndex);
    if (IsCorrectFormat) {
      SerialBT.println(Weight);
      Serial.println(Weight);
      Serial2.println(Weight);
    }
    State = 2;
    ClearData();
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
  //Serial.println(BitReaded);
  static uint8_t BitCounter = 0;
  switch (BitCounter) {
    case 0:
    case 2:
      if (!BitReaded) BitCounter = 0;
      else BitCounter++;
      return;
    case 1:
      if (BitReaded) BitCounter = 0;
      else BitCounter++;
      return;
    case 3 ... 8:
      Address[Index][5 - BitCounter + 3] = BitReaded;
      BitCounter++;
      return;
    case 9 ... 15:
      Data[Index][7 - BitCounter + 9] = BitReaded;
      BitCounter++;
      return;
    case 16:
      Data[Index][7 - BitCounter + 9] = BitReaded;
      BitCounter = 0;
      lastMillis = millis();
      Index++;
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
    default:
    case 0b1111101:
      return 0.0;
    case 0b1100000:
      return 1.0;
    case 0b111110:
      return 2.0;
    case 0b1111010:
      return 3.0;
    case 0b1100011:
      return 4.0;
    case 0b1011011:
      return 5.0;
    case 0b1011111:
      return 6.0;
    case 0b1110000:
      return 7.0;
    case 0b1111111:
      return 8.0;
    case 0b1111011:
      return 9.0;
  }
}
void ClearData() {
  for (uint8_t i = 0; i < 10; i++) {
    for (uint8_t j = 0; j < 8; j++) {
      Address[i][j] = false;
      Data[i][j] = false;
    }
  }
}