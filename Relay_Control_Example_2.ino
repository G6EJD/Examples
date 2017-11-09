#include <EEPROM.h>

byte variable1, variable2, variable3, variable4;
int address;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");
  EEPROM.begin(8);     // Only need 4 bytes for the 4 8-bit numbers

  pinMode(D5, OUTPUT); // e.g. Pin D5 on ESP8266
  pinMode(D6, OUTPUT); // e.g. Pin D6 on ESP8266
  pinMode(D7, OUTPUT); // e.g. Pin D7 on ESP8266
  pinMode(D8, OUTPUT); // e.g. Pin D8 on ESP8266

  variable1 = 1;       // All Relays OFF 0 = OFF 1 = ON is the assumption
  variable2 = 0;       // All Relays OFF 0 = OFF 1 = ON is the assumption
  variable3 = 1;       // All Relays OFF 0 = OFF 1 = ON is the assumption
  variable4 = 0;       // All Relays OFF 0 = OFF 1 = ON is the assumption
  Save_Data();
}

void loop() {
  Read_Data();
  if (variable1 == 1) digitalWrite(D5, HIGH); else digitalWrite(D5, LOW);
  if (variable1 == 1) digitalWrite(D6, HIGH); else digitalWrite(D6, LOW);
  if (variable1 == 1) digitalWrite(D7, HIGH); else digitalWrite(D7, LOW);
  if (variable1 == 1) digitalWrite(D8, HIGH); else digitalWrite(D8, LOW);

  Serial.printf("var1: %d, var2: %d, var3: %d, var4: %d", variable1, variable2, variable3, variable4);
  Serial.println();
  Save_Data();

  variable1 = 0;  // Clear the values to prove it's reading back from EEPROM
  variable2 = 0;  // Clear the values to prove it's reading back from EEPROM
  variable3 = 0;  // Clear the values to prove it's reading back from EEPROM
  variable4 = 0;  // Clear the values to prove it's reading back from EEPROM
  Serial.printf("var1: %d, var2: %d, var3: %d, var4: %d", variable1, variable2, variable3, variable4);
  Serial.println();
  
  delay(10000); // wait and then do it all again
}

void Read_Data() {
  Serial.println("Reading datas");
  address = 0;
  variable1 = EEPROM.read(address);
  address   = address + 1;
  variable2 = EEPROM.read(address);
  address   = address + 1;
  variable3 = EEPROM.read(address);
  address   = address + 1;
  variable4 = EEPROM.read(address);
}

void Save_Data() {
  Serial.println("Saving datas");
  address = 0;
  EEPROM.write(address, variable1);
  address   = address + 1;
  EEPROM.write(address, variable2);
  address   = address + 1;
  EEPROM.write(address, variable3);
  address   = address + 1;
  EEPROM.write(address, variable4);
  address   = address + 1;
  EEPROM.commit();
}

