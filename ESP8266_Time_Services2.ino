#include <ESP8266WiFi.h>
#include "time.h"

String Local_Date_Time;

const char* ssid     = "";
const char* password = "";

void setup() {
  Serial.begin(115200);               // For serial diagnostic prints
  // We start by connecting to a WiFi network
  Serial.println("Connecting to " + String(ssid));
  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {delay(500);Serial.print(".");}
  configTime(0, 0, "0.pool.ntp.org");
  // See  https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv  for your timezone
  setenv("TZ", "EST5EDT,M3.2.0,M11.1.0", 1);
  delay(1000); // Wait for time to start
}

void loop() {
  GetLocalTime();  // You must call this to get the current date-time
  Serial.println(Local_Date_Time);
  delay(5000);
}

String GetLocalTime() {
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];
  time (&rawtime);
  timeinfo = localtime (&rawtime);
  // See here for time format codes http://www.cplusplus.com/reference/ctime/strftime/
  strftime (buffer,80,"%D %I:%M:%S %p",timeinfo); // US Format 01-31-19 17:59:22 PM
  //strftime (buffer,80,"%I:%M:%S %p",timeinfo);    // US Format time only 17:59:22 PM
  //strftime (buffer,80,"Date is : %D",timeinfo); // US format date 'Date is: 01-31-19'
  //strftime (buffer,80,"%T",timeinfo); // 24-Hour time format
  //strftime (buffer,80,"Time is : %T",timeinfo); // 24-Hour time format 'Time is: 17:19:22'
  
  //Serial.println(buffer);
  Local_Date_Time = String(buffer);
}
