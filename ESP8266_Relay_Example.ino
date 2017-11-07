#include "FS.h";

int variable1, variable2, variable3, variable4;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");
  SPIFFS.begin();
  pinMode(D1, OUTPUT); // e.g. Pin D1 on ESP8266 
  pinMode(D2, OUTPUT); // e.g. Pin D2 on ESP8266 
  pinMode(D3, OUTPUT); // e.g. Pin D3 on ESP8266 
  pinMode(D4, OUTPUT); // e.g. Pin D4 on ESP8266 
  variable1 = 0;       // All Relays OFF 0 = OFF 1 = ON is the assumption
  variable2 = 0;       // All Relays OFF 0 = OFF 1 = ON is the assumption
  variable3 = 0;       // All Relays OFF 0 = OFF 1 = ON is the assumption
  variable4 = 0;       // All Relays OFF 0 = OFF 1 = ON is the assumption
  Save_Data();
}

void loop() {
  Read_Data();
  // Your application amends 'varable1..variable4' as required
  Save_Data();
  if (variable1 == 1) digitalWrite(D1, HIGH); else digitalWrite(D1, LOW);  // Switch Relay1 ON if variable = 1 otherwise OFF
  if (variable2 == 1) digitalWrite(D2, HIGH); else digitalWrite(D2, LOW);  // Switch Relay2 ON if variable = 1 otherwise OFF
  if (variable3 == 1) digitalWrite(D3, HIGH); else digitalWrite(D3, LOW);  // Switch Relay3 ON if variable = 1 otherwise OFF
  if (variable4 == 1) digitalWrite(D4, HIGH); else digitalWrite(D4, LOW);  // Switch Relay4 ON if variable = 1 otherwise OFF
  delay(10000);                                                            // wait and then do it all again
}

void Read_Data(){
  char filename [] = "datalog.txt";                     // Assign a filename or use the format e.g. SD.open("datalog.txt",...);
  File dataFile = SPIFFS.open(filename, "r");           // Open the file again, this time for reading
  if (!dataFile) Serial.println("file open failed");    // Check for errors
  while (dataFile.available()) {
    variable1 = dataFile.parseInt();
    variable2 = dataFile.parseInt();
    variable3 = dataFile.parseInt();
    variable4 = dataFile.parseInt();
  }
  dataFile.close();                                     // Close the file
}

void Save_Data(){
  // Assign a file name e.g. 'names.dat' or 'data.txt' or 'data.dat' try to use the 8.3 file naming convention format could be 'data.d'
  char filename [] = "datalog.txt";                     // Assign a filename or use the format e.g. SD.open("datalog.txt",...);
  if (SPIFFS.exists(filename)) SPIFFS.remove(filename); // First in this example check to see if a file already exists, if so delete it
  File dataFile = SPIFFS.open(filename, "a+");          // Open a file for reading and writing (appending)
  if (!dataFile)Serial.println("file open failed");     // Check for errors
  variable1 = 0; // Example value for testing, these would be varied by your application
  variable2 = 0; // Example value for testing, these would be varied by your application
  variable3 = 0; // Example value for testing, these would be varied by your application
  variable4 = 0; // Example value for testing, these would be varied by your application
  dataFile.println(variable1);                         // Write data to file
  dataFile.println(variable1);                         // Write data to file
  dataFile.println(variable1);                         // Write data to file
  dataFile.println(variable1);                         // Write data to file
  dataFile.close();                                    // Close the file
}

