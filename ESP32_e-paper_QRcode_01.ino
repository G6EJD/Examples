/* ESP QRcode display on an e-paper
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
#include <SPI.h>               // Built-in 
#include "EPD_WaveShare.h"     // Copyright (c) 2017 by Daniel Eichhorn https://github.com/ThingPulse/minigrafx
#include "EPD_WaveShare_42.h"  // Copyright (c) 2017 by Daniel Eichhorn https://github.com/ThingPulse/minigrafx
#include "MiniGrafx.h"         // Copyright (c) 2017 by Daniel Eichhorn https://github.com/ThingPulse/minigrafx
#include "DisplayDriver.h"     // Copyright (c) 2017 by Daniel Eichhorn https://github.com/ThingPulse/minigrafx
#include "qrcode.h"            // Copyright (c) //https://github.com/ricmoo/qrcode/

#define SCREEN_WIDTH  400.0    // Set for landscape mode, don't remove the decimal place!
#define SCREEN_HEIGHT 300.0
#define BITS_PER_PIXEL 1
#define EPD_BLACK 0
#define EPD_WHITE 1
uint16_t palette[] = { 0, 1 };

// pins_arduino.h, e.g. LOLIN32 
static const uint8_t EPD_BUSY = 4;
static const uint8_t EPD_SS   = 5;
static const uint8_t EPD_RST  = 16;
static const uint8_t EPD_DC   = 17;
static const uint8_t EPD_SCK  = 18;
static const uint8_t EPD_MISO = 19; // Master-In Slave-Out not used, as no data from display
static const uint8_t EPD_MOSI = 23;

EPD_WaveShare42 epd(EPD_SS, EPD_RST, EPD_DC, EPD_BUSY);
MiniGrafx gfx = MiniGrafx(&epd, BITS_PER_PIXEL, palette);

//################# LIBRARIES ##########################
String version = "1.0";       // Version of this program
//################ VARIABLES  ##########################

QRCode qrcode;

//#########################################################################################
void setup() {
  gfx.init();
  gfx.setRotation(0);
  gfx.fillBuffer(EPD_WHITE);
}

//#########################################################################################
void loop() {
  //Display_QRcode(x,y,element_size,QRcode Size,Error Detection/Correction Level,"string for encoding");
  gfx.drawString(150,10,"QRcode Generation Example");
  Display_QRcode(25,25,3,3,3,"Hello World!\n\nG6EJD");
  Display_QRcode(25,175,3,4,1,"http://example.com/");
  Display_QRcode(225,75,2,12,1,"Chapter-1 The quick brown fox jumps over the lazy dog's back\n\rChapter-2 The quick brown fox jumps over the lazy dog's back again\n\rChapter-3 The quick brown fox jumps over the lazy dog's back again and again");
  gfx.commit();
  delay(30000); // Delay before we do it again
  Clear_Screen();
}

//#########################################################################################
void Display_QRcode(int offset_x, int offset_y, int element_size, int QRsize, int ECC_Mode, const char* Message){
  // QRcode capacity examples Size-12  65 x 65 LOW      883 535 367 
  //                                           MEDIUM   691 419 287 
  //                                           QUARTILE 489 296 203 
  //                                           HIGH     374 227 155 
  uint8_t qrcodeData[qrcode_getBufferSize(QRsize)];
  //ECC_LOW, ECC_MEDIUM, ECC_QUARTILE and ECC_HIGH. Higher levels of error correction sacrifice data capacity, but ensure damaged codes remain readable.
  if (ECC_Mode%4 == 0) qrcode_initText(&qrcode, qrcodeData, QRsize, ECC_LOW, Message);
  if (ECC_Mode%4 == 1) qrcode_initText(&qrcode, qrcodeData, QRsize, ECC_MEDIUM, Message);
  if (ECC_Mode%4 == 2) qrcode_initText(&qrcode, qrcodeData, QRsize, ECC_QUARTILE, Message);
  if (ECC_Mode%4 == 3) qrcode_initText(&qrcode, qrcodeData, QRsize, ECC_HIGH, Message);
  for (int y = 0; y < qrcode.size; y++) {
    for (int x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        gfx.setColor(EPD_BLACK);
        gfx.fillRect(x*element_size+offset_x,y*element_size+offset_y,element_size,element_size);
      }
      else 
      {
        gfx.setColor(EPD_WHITE);
        gfx.fillRect(x*element_size+offset_x,y*element_size+offset_y,element_size,element_size);
      }
    }
  }
}
//#########################################################################################
void Clear_Screen(){
  gfx.fillBuffer(EPD_WHITE);
  gfx.commit();
  delay(4000);
}

