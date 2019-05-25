/*****************************************
* ESP32 GPS VKEL 9600 Bds
******************************************/

#include <TinyGPS++.h>                       
TinyGPSPlus gps;                            
#include <SPI.h>
#include "GPSLoRa.h"

#define SCK     5     // GPIO5  -- SX1278's SCK
#define MISO    19    // GPIO19 -- SX1278's MISnO
#define MOSI    27    // GPIO27 -- SX1278's MOSI
#define SS      18    // GPIO18 -- SX1278's CS
#define RST     14    // GPIO14 -- SX1278's RESET
#define DI0     26    // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND    868E6 // 868Mhz

unsigned int counter = 0;

String packet ;

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, 12, 15);   //17-TX 18-RX State GPS
  Serial.println("LoRa Sender GPS Data Test");
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop(){
  Serial.print("Latitude  : ");
  Serial.println(gps.location.lat(), 5);
  Serial.print("Longitude : ");
  Serial.println(gps.location.lng(), 4);
  Serial.print("Satellites: ");
  Serial.println(gps.satellites.value());
  Serial.print("Altitude  : ");
  Serial.print(gps.altitude.feet() / 3.2808);
  Serial.println("M");
  Serial.print("Time      : ");
  Serial.print(gps.time.hour());
  Serial.print(":");
  Serial.print(gps.time.minute());
  Serial.print(":");
  Serial.println(gps.time.second());
  Serial.println("**********************");
  smartDelay(1000);                                      
  if (millis() > 5000 && gps.charsProcessed() < 10) Serial.println(F("No GPS data received: check wiring"));
  // send packet of data via LoRa
  LoRa.beginPacket();
  LoRa.print("GPS-Data ");
  LoRa.print("Latitude  : ");
  LoRa.print(gps.location.lat(), 5);
  LoRa.print("Longitude : ");
  LoRa.println(gps.location.lng(), 4);
  LoRa.endPacket();
}

static void smartDelay(unsigned long ms){
  unsigned long start = millis();
  do
  {
    while (Serial1.available())
      gps.encode(Serial1.read());
  } while (millis() - start < ms);
}
