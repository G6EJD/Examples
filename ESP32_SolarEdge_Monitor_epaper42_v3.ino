/* ESP32 Solar PV (solar Edge) Monitor using an EPD 4.2" Display, obtains data from Solaredge, decodes it and then displays it.
  ####################################################################################################################################
  This software, the ideas and concepts is Copyright (c) David Bird 2018. All rights to this software are reserved.

  Any redistribution or reproduction of any part or all of the contents in any form is prohibited other than the following:
  1. You may print or download to a local hard disk extracts for your personal and non-commercial use only.
  2. You may copy the content to individual third parties for their personal use, but only if you acknowledge the author David Bird as the source of the material.
  3. You may not, except with my express written permission, distribute or commercially exploit the content.
  4. You may not transmit it or store it in any other website or other form of electronic retrieval system for commercial purposes.

  The above copyright ('as annotated') notice and this permission notice shall be included in all copies or substantial portions of the Software and where the
  software use is visible to an end-user.

  THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT. FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY
  OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  See more at http://www.dsbird.org.uk
*/
#include "credentials.h"       // See 'credentials' tab and enter your OWM API key and set the Wifi SSID and PASSWORD
#include <ArduinoJson.h>       // https://github.com/bblanchon/ArduinoJson NOTE: *** MUST BE Version-6 or above ***
#include <WiFi.h>              // Built-in
#include <WiFiClientSecure.h>  // Built-in
#include "time.h"              // Built-in
#include <SPI.h>               // Built-in 
#include "EPD_WaveShare.h"     // Copyright (c) 2017 by Daniel Eichhorn https://github.com/ThingPulse/minigrafx
#include "EPD_WaveShare_42.h"  // Copyright (c) 2017 by Daniel Eichhorn https://github.com/ThingPulse/minigrafx
#include "MiniGrafx.h"         // Copyright (c) 2017 by Daniel Eichhorn https://github.com/ThingPulse/minigrafx
#include "DisplayDriver.h"     // Copyright (c) 2017 by Daniel Eichhorn https://github.com/ThingPulse/minigrafx
#include "ArialRounded.h"      // Copyright (c) 2017 by Daniel Eichhorn https://github.com/ThingPulse/minigrafx

#define SCREEN_WIDTH  400.0    // Set for landscape mode, don't remove the decimal place!
#define SCREEN_HEIGHT 300.0
#define BITS_PER_PIXEL 1
#define EPD_BLACK 0
#define EPD_WHITE 1
uint16_t palette[] = { 0, 1 };

// pins_arduino.h, e.g. LOLIN32 LITE
static const uint8_t EPD_BUSY = 4;
static const uint8_t EPD_SS   = 5;
static const uint8_t EPD_RST  = 16;
static const uint8_t EPD_DC   = 17;
static const uint8_t EPD_SCK  = 18;
static const uint8_t EPD_MISO = 19; // Master-In Slave-Out not used, as no data from display
static const uint8_t EPD_MOSI = 23;

EPD_WaveShare42 epd(EPD_SS, EPD_RST, EPD_DC, EPD_BUSY);
MiniGrafx gfx = MiniGrafx(&epd, BITS_PER_PIXEL, palette);

//################  VERSION  ##########################
String version = "1";        // Version of this program
//################ VARIABLES ###########################

const unsigned long UpdateInterval = (30L * 60L - 03) * 1000000L; // Update delay in microseconds, 13-secs is the time to update so compensate for that

String time_str, Day_time_str; // strings to hold time and received time data;
int    wifi_signal, wifisection, displaysection, start_time;

//################ PROGRAM VARIABLES and OBJECTS ################

float Production[12]      = {0};
float Consumption[12]     = {0};
float SelfConsumption[12] = {0};
float Purchased[12]       = {0};

float LifeTimeEnergy  = 0;
float Revenue         = 0;
float LastYearEnergy  = 0;
float LastMonthEnergy = 0;
float LastDayEnergy   = 0;

#define autoscale_on  true
#define autoscale_off false
#define barchart_on   true
#define barchart_off  falseco

WiFiClientSecure client; // wifi client object

//#########################################################################################
void setup() {
  start_time = millis();
  Serial.begin(115200);
  StartWiFi();
  wifi_signal = WiFi_Signal();
  SetupTime();
  bool Received_EnergyData_OK = false;
  Received_EnergyData_OK = (Obtain_Energy_Reading("Daily") && Obtain_Energy_Reading("Yearly"));
  // Now only refresh the screen if all the data was received OK, otherwise wait until the next timed check otherwise wait until the next timed check
  if (Received_EnergyData_OK) {
    StopWiFi(); // Reduces power consumption
    gfx.init();
    gfx.setRotation(0);
    gfx.setColor(EPD_BLACK);
    gfx.fillBuffer(EPD_WHITE);
    gfx.setTextAlignment(TEXT_ALIGN_LEFT);
    Display_Energy();
    gfx.commit();
    delay(2000);
    Serial.println("total time to update = " + String(millis() - start_time));
  }
  begin_sleep();
}
//#########################################################################################
void loop() { // this will never run!
}
//#########################################################################################
void begin_sleep() {
  esp_sleep_enable_timer_wakeup(UpdateInterval);
  Serial.println(F("Starting deep-sleep period..."));
#ifdef BUILTIN_LED
  pinMode(BUILTIN_LED, INPUT);    // In case it's on, turn output off, sometimes PIN-5 on some boards is used for SPI-SS
  digitalWrite(BUILTIN_LED, HIGH); // In case it's on, turn LED off, as sometimes PIN-5 on some boards is used for SPI-SS
#endif
  delay(2000);
  esp_deep_sleep_start(); // Sleep for e.g. 30 minutes
}
//#########################################################################################
void Display_Energy() {                          // 4.2" e-paper display is 400x300 resolution
  Display_Heading_Section();                     // Top line of the display
  Display_YearlyEnergySummary();
  Display_DailyEnergySummary();
  Display_Status_Section(200, 0, wifi_signal); // Wi-Fi signal strength and Battery voltage
}
//#########################################################################################
void Display_Heading_Section() {
  gfx.drawLine(0, 15, SCREEN_WIDTH, 15);
  gfx.setFont(ArialMT_Plain_10);
  gfx.setTextAlignment(TEXT_ALIGN_RIGHT);
  gfx.drawString(SCREEN_WIDTH - 3, 0, Day_time_str);
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.drawString(5, 0, time_str);
}
//#########################################################################################
void Display_DailyEnergySummary() {
  gfx.setFont(ArialMT_Plain_24);
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  float total_consumption = 0;
  float total_selfconsumption = 0;
  for (int r = 0; r <= 11; r++) {
    total_consumption     += Consumption[r];
    total_selfconsumption += SelfConsumption[r];
  }
  gfx.drawString(10, 20, "Total Production:"); Display_Power(200, 20, LifeTimeEnergy);
  gfx.drawString(10, 40, "Revenue:");
  gfx.drawString(200, 40, (String)"£" + (Revenue));
  Serial.println("Total Consumption: " + String(total_consumption, 1));
  Serial.println("Total SelfConsumption:   " + String(total_selfconsumption, 1));
  gfx.drawString(280, 40, (String)" + £" + (total_selfconsumption * 0.133245)); // Tarif as of Sept 2018 for Electricity
  gfx.drawString(10, 60,  "Power this year:");  Display_Power(200, 60, LastYearEnergy);
  gfx.drawString(10, 80,  "Power this month:"); Display_Power(200, 80, LastMonthEnergy);
  gfx.drawString(10, 100, "Power today:");      Display_Power(200, 100, LastDayEnergy / 1000); // Daily consumption is usually in the range 0-30KWhr
  gfx.setFont(ArialMT_Plain_10);
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.drawLine(0, 130, SCREEN_WIDTH, 130);
}
//#########################################################################################
void Display_YearlyEnergySummary() {
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.drawString(SCREEN_WIDTH / 2, 150, "Yearly Summary");
  gfx.setFont(ArialMT_Plain_10);
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  DrawGraph(25, 190, 150, 75, 0, 1000, "Consumption", Consumption, 12, autoscale_on, barchart_on);
  DrawGraph(230, 190, 150, 75, 0, 1000, "Self-Consumption", SelfConsumption, 12, autoscale_on, barchart_on);
}
//#########################################################################################
void Display_Power(int x, int y, float reading) {
  String units = "KWhr";
  int string_length;
  string_length = 7;
  if (reading >= 1000) {
    reading /= 1000;
    string_length = (String(reading, 1)).length();
    units    = "KWhr";
  }
  if (reading >= 1000000) {
    reading /= 1000000;
    string_length = (String(reading, 1)).length();
    units    = "MWhr";
  }
  if (reading < 50) gfx.drawString(x, y, String(reading, 2)); else gfx.drawString(x, y, String(reading, 0));
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.drawString(x + string_length * 9, y + 5, units);
  gfx.setFont(ArialMT_Plain_24);
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
}
//#########################################################################################
bool Obtain_Energy_Reading(String RequestType) {
  String rxtext = "";
  Serial.println("Connecting to server for " + RequestType);
  client.stop(); // close connection before sending a new request
  if (client.connect(server, 443)) { // if the connection succeeds
    Serial.println("connecting...");
    // send the HTTP PUT request:
    if (RequestType == "Yearly")
      client.println(YearlyRequest);
    else
      client.println(Daily_Request);
    client.println("Host: monitoringapi.solaredge.com");
    client.println("User-Agent: ESP OWM Receiver/1.1");
    client.println("Connection: close");
    client.println();
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return false;
      }
    }
    char c = 0;
    bool startJson = false;
    int jsonend = 0;
    while (client.available()) {
      c = client.read();
      Serial.print(c);
      // JSON formats contain an equal number of open and close curly brackets, so check that JSON is received correctly by counting open and close brackets
      if (c == '{') {
        startJson = true; // set true to indicate JSON message has started
        jsonend++;
      }
      if (c == '}') {
        jsonend--;
      }
      if (startJson == true) {
        rxtext += c;  // Add in the received character
      }
      // if jsonend = 0 then we have have received equal number of curly braces
      if (jsonend == 0 && startJson == true) {
        Serial.println("Received OK...");
        //Serial.println(rxtext);
        if (!DecodeEnergyData(rxtext, RequestType)) return false;
        client.stop();
        return true;
      }
    }
  }
  else {
    // if no connection was made:
    Serial.println("connection failed");
    return false;
  }
  rxtext = "";
  return true;
}
//#########################################################################################
// Problems with stucturing JSON decodes, see here: https://arduinojson.org/assistant/
bool DecodeEnergyData(String json, String Type) {
  Serial.print(F("Creating object...and "));
  DynamicJsonBuffer jsonBuffer (50 * 1024);
  JsonObject& root = jsonBuffer.parseObject(const_cast<char*>(json.c_str()));
  if (!root.success()) {
    Serial.print("ParseObject() failed");
    return false;
  }
  Serial.println(" Decoding " + Type + " data");
  if (Type == "Daily") {
    // All Serial.println statements are for diagnostic purposes and not required, remove if not needed
    JsonObject& overview = root["overview"];
    const char* overview_lastUpdateTime       = overview["lastUpdateTime"]; // "2018-09-20 10:48:56"
    long        overview_lifeTimeData_energy  = overview["lifeTimeData"]["energy"]; // 350820
    float       overview_lifeTimeData_revenue = overview["lifeTimeData"]["revenue"]; // 22.95513
    long        overview_lastYearData_energy  = overview["lastYearData"]["energy"]; // 350460
    long        overview_lastMonthData_energy = overview["lastMonthData"]["energy"]; // 225261
    int         overview_lastDayData_energy   = overview["lastDayData"]["energy"]; // 680
    int         overview_currentPower_power   = overview["currentPower"]["power"]; // 264
    const char* overview_measuredBy           = overview["measuredBy"]; // "METER"
    LifeTimeEnergy  = overview_lifeTimeData_energy;
    Revenue         = overview_lifeTimeData_revenue;
    LastYearEnergy  = overview_lastYearData_energy;
    LastMonthEnergy = overview_lastMonthData_energy;
    LastDayEnergy   = overview_lastDayData_energy;
  }
  if (Type == "Yearly")
  {
    //Serial.println(json);
    // Consumption
    // Purchased
    // FeedIn
    // Production
    // SelfConsumption
    JsonObject& energyDetails = root["energyDetails"];
    const char* energyDetails_timeUnit = energyDetails["timeUnit"]; // "MONTH"
    const char* energyDetails_unit = energyDetails["unit"]; // "Wh"
    JsonArray& energyDetails_meters = energyDetails["meters"];
    for (int type = 0; type < 5; type++) {
      String energyDetails_meters_type = energyDetails_meters[type]["type"]; // "Consumption","Purchased","FeedIn","Production" or "SelfConsumption" the order supplied varies!
      Serial.println(energyDetails_meters_type);
      if (energyDetails_meters_type == "Consumption") {
        JsonArray& energyDetails_meters0_values = energyDetails_meters[type]["values"];
        Consumption[0]  = energyDetails_meters0_values[0]["value"];
        Consumption[1]  = energyDetails_meters0_values[1]["value"];
        Consumption[2]  = energyDetails_meters0_values[2]["value"];
        Consumption[3]  = energyDetails_meters0_values[3]["value"];
        Consumption[4]  = energyDetails_meters0_values[4]["value"];
        Consumption[5]  = energyDetails_meters0_values[5]["value"];
        Consumption[6]  = energyDetails_meters0_values[6]["value"];
        Consumption[7]  = energyDetails_meters0_values[7]["value"];
        Consumption[8]  = energyDetails_meters0_values[8]["value"];
        Consumption[9]  = energyDetails_meters0_values[9]["value"];
        Consumption[10] = energyDetails_meters0_values[10]["value"];
        Consumption[11] = energyDetails_meters0_values[11]["value"];
      }
      if (energyDetails_meters_type == "SelfConsumption") {
        JsonArray& energyDetails_meters_values = energyDetails_meters[type]["values"];
        SelfConsumption[0]  = energyDetails_meters_values[0]["value"];
        SelfConsumption[1]  = energyDetails_meters_values[1]["value"];
        SelfConsumption[2]  = energyDetails_meters_values[2]["value"];
        SelfConsumption[3]  = energyDetails_meters_values[3]["value"];
        SelfConsumption[4]  = energyDetails_meters_values[4]["value"];
        SelfConsumption[5]  = energyDetails_meters_values[5]["value"];
        SelfConsumption[6]  = energyDetails_meters_values[6]["value"];
        SelfConsumption[7]  = energyDetails_meters_values[7]["value"];
        SelfConsumption[8]  = energyDetails_meters_values[8]["value"];
        SelfConsumption[9]  = energyDetails_meters_values[9]["value"];
        SelfConsumption[10] = energyDetails_meters_values[10]["value"];
        SelfConsumption[11] = energyDetails_meters_values[11]["value"];      
      }
    }
    for (int i = 0; i < 12; i++) {
      Serial.println(Consumption[i] / 1000);
      Consumption[i] /= 1000;
    }
    for (int i = 0; i < 12; i++) {
      Serial.println(SelfConsumption[i] / 1000);
      SelfConsumption[i]   /= 1000;
    }
  }
  return true;
}
//#########################################################################################
int StartWiFi() {
  int connAttempts = 0;
  Serial.print(F("\r\nConnecting to: ")); Serial.println(String(ssid1));
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid1, password1);
  while (WiFi.status() != WL_CONNECTED ) {
    delay(500); Serial.print(".");
    if (connAttempts > 20) {
      WiFi.disconnect();
      begin_sleep();
    }
    connAttempts++;
  }
  Serial.println("WiFi connected at: " + String(WiFi.localIP()));
  return 1;
}
//#########################################################################################
void StopWiFi() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  wifisection    = millis() - wifisection;
}
//#########################################################################################
int WiFi_Signal() {
  return WiFi.RSSI();
}
//#########################################################################################
void Display_Status_Section(int x, int y, int rssi) {
  Draw_RSSI(x - 50, y + 13, rssi);
  DrawBattery(x + 50, y - 3);
}
//#########################################################################################
void Draw_RSSI(int x, int y, int rssi) {
  int WIFIsignal = 0;
  int xpos = 1;
  for (int _rssi = -100; _rssi <= rssi; _rssi = _rssi + 20) {
    if (_rssi <= -20)  WIFIsignal = 20; //            <20dbm displays 5-bars
    if (_rssi <= -40)  WIFIsignal = 16; //  -40dbm to -21dbm displays 4-bars
    if (_rssi <= -60)  WIFIsignal = 12; //  -60dbm to -41dbm displays 3-bars
    if (_rssi <= -80)  WIFIsignal = 8;  //  -80dbm to -61dbm displays 2-bars
    if (_rssi <= -100) WIFIsignal = 4;  // -100dbm to -81dbm displays 1-bar
    gfx.fillRect(x + xpos * 5, y - WIFIsignal, 4, WIFIsignal);
    xpos++;
  }
  gfx.fillRect(x, y - 1, 4, 1);
}
//#########################################################################################
void DrawBattery(int x, int y) {
  uint8_t percentage = 100;
  float voltage = analogRead(35) / 4096.0 * 7.2;
  if (voltage >= 0 ) { // Only display if there is a valid reading
    Serial.println("Battery voltage = " + String(voltage, 2));
    if (voltage >= 4.19) percentage = 100;
    else if (voltage < 3.20) percentage = 0;
    else percentage = (voltage - 3.20) * 100 / (4.20 - 3.20);
    gfx.setColor(EPD_BLACK);
    gfx.drawRect(x - 22, y + 5, 19, 10);
    gfx.fillRect(x - 2, y + 7, 3, 6);
    gfx.fillRect(x - 20, y + 7, 17 * percentage / 100.0, 6);
    gfx.setFont(ArialMT_Plain_10);
    gfx.drawString(x - 48, y + 5, String(percentage) + "%");
  }
}
//#########################################################################################
void SetupTime() {
  configTime(0, 0, "0.uk.pool.ntp.org", "time.nist.gov");
  setenv("TZ", Timezone, 1);
  delay(500);
  UpdateLocalTime();
}
//#########################################################################################
void UpdateLocalTime() {
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println(F("Failed to obtain time"));
  }
  //See http://www.cplusplus.com/reference/ctime/strftime/
  //Serial.println(&timeinfo, "%a %b %d %Y   %H:%M:%S");     // Displays: Saturday, June 24 2017 14:05:49
  Serial.println(&timeinfo, "%H:%M:%S");                     // Displays: 14:05:49
  char output[30], day_output[30];
  strftime(day_output, 30, "%a  %d-%b-%y", &timeinfo);       // Displays: Sat 24/Jun/17
  strftime(output, 30, "(Updated: %H:%M:%S )", &timeinfo);   // Creates: '@ 14:05:49'
  Day_time_str = day_output;
  time_str     = output;
}
//#########################################################################################
String ConvertUnixTime(int unix_time) {
  struct tm *now_tm;
  int hour, min, second, day, month, year, wday;
  // timeval tv = {unix_time,0};
  time_t tm = unix_time;
  now_tm = localtime(&tm);
  hour   = now_tm->tm_hour;
  min    = now_tm->tm_min;
  second = now_tm->tm_sec;
  wday   = now_tm->tm_wday;
  day    = now_tm->tm_mday;
  month  = now_tm->tm_mon + 1;
  year   = 1900 + now_tm->tm_year; // To get just YY information
  time_str =  (hour < 10 ? "0" + String(hour) : String(hour)) + ":" + (min < 10 ? "0" + String(min) : String(min)) + ":" + "  ";                     // HH:MM   05/07/17
  time_str += (day < 10 ? "0" + String(day) : String(day)) + "/" + (month < 10 ? "0" + String(month) : String(month)) + "/" + (year < 10 ? "0" + String(year) : String(year)); // HH:MM   05/07/17
  //Serial.println(time_str);
  return time_str;
}
//#########################################################################################
void DrawGraph(int x_pos, int y_pos, int gwidth, int gheight, float Y1Min, float Y1Max, String title, float DataArray[], int readings, boolean auto_scale, boolean barchart_mode) {
#define auto_scale_margin 0 // Sets the autoscale increment, so axis steps up in units of e.g. 3
#define y_minor_axis 5      // 5 y-axis division markers
  int maxYscale = -10000;
  int minYscale =  10000;
  int last_x, last_y;
  float x1, y1, x2, y2;
  if (auto_scale == true) {
    for (int i = 1; i < readings; i++ ) {
      if (DataArray[i] >= maxYscale) maxYscale = DataArray[i];
      if (DataArray[i] <= minYscale) minYscale = DataArray[i];
    }
    maxYscale = round(maxYscale + auto_scale_margin); // Auto scale the graph and round to the nearest value defined, default was Y1Max
    Y1Max = round(maxYscale + 0.5);
    if (minYscale != 0) minYscale = round(minYscale - auto_scale_margin); // Auto scale the graph and round to the nearest value defined, default was Y1Min
    Y1Min = round(minYscale);
  }
  // Draw the graph
  last_x = x_pos + 1;
  last_y = y_pos + (Y1Max - constrain(DataArray[1], Y1Min, Y1Max)) / (Y1Max - Y1Min) * gheight;
  gfx.setColor(EPD_BLACK);
  gfx.drawRect(x_pos, y_pos, gwidth + 3, gheight + 2);
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.drawString(x_pos + gwidth / 2, y_pos - 18, title);
  gfx.setFont(ArialMT_Plain_10);
  gfx.setTextAlignment(TEXT_ALIGN_RIGHT);
  // Draw the data
  for (int gx = 1; gx < readings; gx++) {
    x1 = last_x;
    y1 = last_y;
    x2 = x_pos + gx * gwidth / (readings - 1) - 1 ; // max_readings is the global variable that sets the maximum data that can be plotted
    y2 = y_pos + (Y1Max - constrain(DataArray[gx], Y1Min, Y1Max)) / (Y1Max - Y1Min) * gheight + 1;
    if (barchart_mode) {
      gfx.fillRect(x2, y2, (gwidth / readings) - 1, y_pos + gheight - y2 + 1);
    } else {
      gfx.drawLine(last_x, last_y, x2, y2);
    }
    last_x = x2;
    last_y = y2;
  }
  //Draw the Y-axis scale
  for (int spacing = 0; spacing <= y_minor_axis; spacing++) {
#define number_of_dashes 20
    for (int j = 0; j < number_of_dashes; j++) { // Draw dashed graph grid lines
      if (spacing < y_minor_axis) gfx.drawHorizontalLine((x_pos + 3 + j * gwidth / number_of_dashes), y_pos + (gheight * spacing / y_minor_axis), gwidth / (2 * number_of_dashes));
    }
    if ( (Y1Max - (float)(Y1Max - Y1Min) / y_minor_axis * spacing) < 10) {
      gfx.drawString(x_pos - 2, y_pos + gheight * spacing / y_minor_axis - 5, String((Y1Max - (float)(Y1Max - Y1Min) / y_minor_axis * spacing + 0.01), 1));
    }
    else {
      if (Y1Min < 1 && Y1Max < 10) gfx.drawString(x_pos - 2, y_pos + gheight * spacing / y_minor_axis - 5, String((Y1Max - (float)(Y1Max - Y1Min) / y_minor_axis * spacing + 0.01), 1));
      else gfx.drawString(x_pos - 2, y_pos + gheight * spacing / y_minor_axis - 5, String((Y1Max - (float)(Y1Max - Y1Min) / y_minor_axis * spacing + 0.01), 0)); // +0.01 prevents -0.00 occurring
    }
  }
  for (int i = 1; i <= 12; i++) {
    if (i % 2 == 1) gfx.drawString(4 + x_pos + gwidth / 11 * i, y_pos + gheight + 3, String(i));
  }
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.drawString(x_pos + gwidth / 2, y_pos + gheight + 12, "Months");
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
}

