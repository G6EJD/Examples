#include "FS.h";

int variable1, variable2, variable3, variable4;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");
  bool ok = SPIFFS.begin();
  if (ok) Serial.println("Spiffs started"); else Serial.println("SPIFFS.failed");
  pinMode(D5, OUTPUT); // e.g. Pin D5 on ESP8266 
  pinMode(D6, OUTPUT); // e.g. Pin D6 on ESP8266 
  pinMode(D7, OUTPUT); // e.g. Pin D7 on ESP8266 
  pinMode(D8, OUTPUT); // e.g. Pin D8 on ESP8266 
  variable1 = 1;       // All Relays OFF 0 = OFF 1 = ON is the assumption
  variable2 = 1;       // All Relays OFF 0 = OFF 1 = ON is the assumption
  variable3 = 0;       // All Relays OFF 0 = OFF 1 = ON is the assumption
  variable4 = 1;       // All Relays OFF 0 = OFF 1 = ON is the assumption
  Save_Data();
}

void loop() {
  Read_Data();
  // Your application amends 'varable1..variable4' as required
  if (variable1 == 1) digitalWrite(D5, HIGH); else digitalWrite(D5, LOW);  // Switch Relay1 ON if variable = 1 otherwise OFF
  if (variable2 == 1) digitalWrite(D6, HIGH); else digitalWrite(D6, LOW);  // Switch Relay2 ON if variable = 1 otherwise OFF
  if (variable3 == 1) digitalWrite(D7, HIGH); else digitalWrite(D7, LOW);  // Switch Relay3 ON if variable = 1 otherwise OFF
  if (variable4 == 1) digitalWrite(D8, HIGH); else digitalWrite(D8, LOW);  // Switch Relay4 ON if variable = 1 otherwise OFF
  Serial.printf("var1: %d, var2: %d, var3: %d, var4: %d", variable1, variable2, variable3, variable4);
  Serial.println();
  Save_Data();
  delay(10000); // wait and then do it all again
  variable1 = 0;  // Clear the values to prove it's reading back from EEPROM
  variable2 = 0;  // Clear the values to prove it's reading back from EEPROM
  variable3 = 0;  // Clear the values to prove it's reading back from EEPROM
  variable4 = 0;  // Clear the values to prove it's reading back from EEPROM
}

void Read_Data(){
  char filename [] = "datalog.txt";                     // Assign a filename or use the format e.g. SD.open("datalog.txt",...);
  String line_input;
  File dataFile = SPIFFS.open(filename, "r");           // Open the file again, this time for reading
  if (!dataFile) Serial.println("file open failed");    // Check for errors
  while (dataFile.available()) {
    line_input = dataFile.readStringUntil('\n'); variable1 = line_input.toInt();
    line_input = dataFile.readStringUntil('\n'); variable2 = line_input.toInt();
    line_input = dataFile.readStringUntil('\n'); variable3 = line_input.toInt();
    line_input = dataFile.readStringUntil('\n'); variable4 = line_input.toInt();
  }
  dataFile.close();                                     // Close the file
}

void Save_Data(){
  // Assign a file name e.g. 'names.dat' or 'data.txt' or 'data.dat' try to use the 8.3 file naming convention format could be 'data.d'
  char filename [] = "datalog.txt";                     // Assign a filename or use the format e.g. SD.open("datalog.txt",...);
  if (SPIFFS.exists(filename)) SPIFFS.remove(filename); // First in this example check to see if a file already exists, if so delete it
  File dataFile = SPIFFS.open(filename, "a+");          // Open a file for reading and writing (appending)
  if (!dataFile) Serial.println("file open failed");    // Check for errors
  else {
    dataFile.println(variable1);                        // Write data to file
    dataFile.println(variable2);                        // Write data to file
    dataFile.println(variable3);                        // Write data to file
    dataFile.println(variable4);                        // Write data to file
    dataFile.close();                                   // Close the file
  }
}



