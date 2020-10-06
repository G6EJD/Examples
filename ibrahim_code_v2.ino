#include <QMC5883L.h>
#include <Adafruit_GFX.h>       //screen lib draw 
#include <MCUFRIEND_kbv.h>
#include <Wire.h>
#include <TouchScreen.h>
#define MINPRESSURE 200   // define variable for FSR
#define MAXPRESSURE 1000  // define variable for FSR
#define ir1 A9            // define the pin for IR   
#define model1 1080       // define model for IR  
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

QMC5883L compass;
MCUFRIEND_kbv tft;

const int XP = 6, XM = A2, YP = A1, YM = 7; //240x320 ID=0x9340

const int TS_LEFT = 119, TS_RT = 919, TS_TOP = 72, TS_BOT = 926;
int pixel_x, pixel_y;     //Touch_getXY() updates global vars

//TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
Adafruit_GFX_Button p1_btn, p2_btn, p3_btn, p4_btn, p5_btn;

int pr, foot, dis1, dis7, rak;    //variables

void setup() {
  Serial.begin(9600);
  Wire.begin();
  uint16_t ID = tft.readID();     //read screen id and store it in  ID variable, unsigned take only positive.
  Serial.print("TFT ID = 0x");
  Serial.println(ID, HEX);
  Serial.println("Calibrate for your Touch Panel");
  if (ID == 0xD3D3) ID = 0x9486; //change id
  tft.begin(ID);
  tft.setRotation(0);            //PORTRAIT
  tft.fillScreen(BLACK);
  compass.init();
  compass.setSamplingRate(500);
}

void loop() {
ww:
  foot = analogRead(A8); // this is force sensor resistor
  delay(100);
  if (foot > 500) { getkeblah(); }
}

void getkeblah() {
ss:
  tft.fillScreen(BLACK);
  tft.setTextSize(5);
  tft.setTextColor(BLUE, BLACK);
  tft.setCursor(60, 120);
  int heading = compass.readHeading();
  if (heading == 0) {}
  else {
    tft.print(heading);
  }
  delay(100);
  if ((285 <= heading) and (heading <= 300)) {
    return;
  }
  Draw_Compass(heading);
  goto ss;
}

void Draw_Compass(float angle){
  Draw_Compass_Rose(); 
  dx = (diameter * cos((angle-90)*3.14/180)) + centreX;    // calculate X position 
  dy = (diameter * sin((angle-90)*3.14/180)) + centreY;    // calculate Y position 
  arrow(last_dx,last_dy, centreX, centreY, 20, 20,BLACK);  // Erase last arrow      
  arrow(dx,dy, centreX, centreY, 20, 20,CYAN);             // Draw arrow in new position
  last_dx = dx; 
  last_dy = dy;
  //Print the angle on the LCD Screen
  tft.setCursor(0, 0);
  tft.print("Angle=" + String(angle,2));
  tft.setCursor(20, 0);
  tft.print("Heading: ");
  if((bearing < 22.5)  || (bearing > 337.5 ))  tft.print("North");
  if((bearing > 22.5)  && (bearing < 67.5 ))   tft.print("North-East");
  if((bearing > 67.5)  && (bearing < 112.5 ))  tft.print("East");
  if((bearing > 112.5) && (bearing < 157.5 ))  tft.print("South-East");
  if((bearing > 157.5) && (bearing < 202.5 ))  tft.print("South");
  if((bearing > 202.5) && (bearing < 247.5 ))  tft.print("SOuth-West");
  if((bearing > 247.5) && (bearing < 292.5 ))  tft.print("West");
  if((bearing > 292.5) && (bearing < 337.5 ))  tft.print("North-West");

  //Print the approximate direction
  Serial.print("\nYou are heading ");
  if((bearing > 337.5) || (bearing < 22.5))    Serial.print("North");
  if((bearing > 22.5)  && (bearing < 67.5 ))   Serial.print("North-East");
  if((bearing > 67.5)  && (bearing < 112.5 ))  Serial.print("East");
  if((bearing > 112.5) && (bearing < 157.5 ))  Serial.print("South-East");
  if((bearing > 157.5) && (bearing < 202.5 ))  Serial.print("South");
  if((bearing > 202.5) && (bearing < 247.5 ))  Serial.print("South-West");
  if((bearing > 247.5) && (bearing < 292.5 ))  Serial.print("West");
  if((bearing > 292.5) && (bearing < 337.5 ))  Serial.print("North-West");
  delay(100);
}

void arrow(int x2, int y2, int x1, int y1, int alength, int awidth, int colour) {
  float distance;
  int dx, dy, x2o,y2o,x3,y3,x4,y4,k;
  distance = sqrt(pow((x1 - x2),2) + pow((y1 - y2), 2));
  dx = x2 + (x1 - x2) * alength / distance;
  dy = y2 + (y1 - y2) * alength / distance;
  k = awidth / alength;
  x2o = x2 - dx;
  y2o = dy - y2;
  x3 = y2o * k + dx;
  y3 = x2o * k + dy;
  //
  x4 = dx - y2o * k;
  y4 = dy - x2o * k;
  tft.drawLine(x1, y1, x2, y2, colour);
  tft.drawLine(x1, y1, dx, dy, colour);
  tft.drawLine(x3, y3, x4, y4, colour);
  tft.drawLine(x3, y3, x2, y2, colour);
  tft.drawLine(x2, y2, x4, y4, colour);
} 

void Draw_Compass_Rose() {
  int dxo, dyo, dxi, dyi;
  tft.drawCircle(centreX,centreY,diameter,YELLOW);  // Draw compass circle
  for (float i = 0; i <360; i = i + 22.5) {
    dxo = diameter * cos((i-90)*3.14/180);
    dyo = diameter * sin((i-90)*3.14/180);
    dxi = dxo * 0.9;
    dyi = dyo * 0.9;
    tft.drawLine(dxi+centreX,dyi+centreY,dxo+centreX,dyo+centreY,YELLOW);   
  }
  display_item((centreX-5),(centreY-85),"N",RED,2);
  display_item((centreX-5),(centreY+70),"S",RED,2);
  display_item((centreX+80),(centreY-5),"E",RED,2);
  display_item((centreX-85),(centreY-5),"W",RED,2);
}
