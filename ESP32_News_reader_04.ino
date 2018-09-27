/* ESP8266 News Reader, obtains data from a News Server and displays the result on the Serial port
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
#include <WiFi.h> // Built-in
#include <WiFiClient.h>  // Built-in
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson NOTE: *** MUST BE Version-6 or above ***


//################  VERSION  ##########################
String version = "1";        // Version of this program
//################ VARIABLES ###########################

// Change to your WiFi credentials
const char* ssid       = "--";
const char* password   = "--";
int         news_index = 0;
String      apikey     = "";
String      news       = "";
String      rxtext     = "";
// feed URL's array
const char *rssFeedURL [] = {
  "http://newsapi.org/v2/top-headlines?country=us&apiKey=your_API_KEY",
  "http://newsapi.org/v2/top-headlines?sources=bbc-news&apiKey=your_API_KEY",
  "http://newsapi.org/v2/top-headlines?sources=bloomberg&apiKey=your_API_KEY",
  "http://newsapi.org/v2/top-headlines?sources=cbs-news&apiKey=your_API_KEY",
  "http://newsapi.org/v2/top-headlines?sources=cnn&apiKey=your_API_KEY"  
};

//################ PROGRAM VARIABLES and OBJECTS ################

WiFiClient client; // wifi client object

//#########################################################################################
void setup() {
  Serial.begin(115200);
  StartWiFi();
}
//#########################################################################################
void loop() {
  // Now cycle through the news sources see here: https://newsapi.org/docs/endpoints/sources 
  if (Get_News_Feed(rssFeedURL[news_index])) Serial.println("News successfully received\n\n");
  news_index++;
  if (news_index > 4 ) news_index = 0;
  delay(5000);
}
//#########################################################################################
bool Get_News_Feed(const char* url) {
  int    port   = 80;
  const char *ptr;
  char server[80];
  char request[100];

  // Clear string storage
  memset(server, 0, sizeof(server));
  memset(request, 0, sizeof(request));

  // Is there a protocol specified ?
  // If so skip it
  if ((ptr = strchr(url, ':')) != NULL)  {
    // Yes there was a protocol
    ptr += 3;
  } else  {
    // No protocol specified
    ptr = url;
  }
  // Search for host separator
  char *endPtr = strchr(ptr, '/');
  if (endPtr == NULL) {
    return false;
  }
  // Copy host string to host storage
  int index = 0;
  while (ptr != endPtr) {
    server[index++] = *ptr++;
  }
  // The remainder of the string is the path
  strcpy(request, endPtr);
  Serial.println("Connecting to '" + String(server) + " for current news using... "+String(request));
  client.stop(); // close connection before sending a new request
  int connected_status = client.connect(server, port); //-SUCCESS 1 -TIMED_OUT -1 -INVALID_SERVER -2 -TRUNCATED -3 -INVALID_RESPONSE -4
  if (connected_status) { // if the connection succeeds
    //Serial.println("connecting...");
    // send the HTTP GET request:
    client.print(String("GET ") + request + " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Connection: keep-alive" + "\r\n\r\n" +
                 "Accept:text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\n" +
                 "Accept-encoding: identity\r\n" +
                 "Accept-Language:en-GB,en-US;q=0.9,en;q=0.8\r\n" +
                 "User-Agent:Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/64.0.3282.186 Safari/537.36 \r\n" +
                 "Upgrade-Insecure-Requests:1\r\n" +
                 "Content-Type: application/json;charset=utf-8\r\n");
    char ch = 0;
    int timeoutvalue_mS = 3500;
    bool startJson = false;
    int jsonend = 0;
    bool finished = false;
    unsigned long timeoutmS = timeoutvalue_mS + millis();
    while (timeoutmS > millis()) {
      ch = client.read();
      // JSON formats contain an equal number of open and close curly brackets, so check that JSON is received correctly by counting open and close brackets
      if (ch == '{') {
        startJson = true; // set true to indicate JSON message has started
        jsonend++;
      }
      if (ch == '}') {
        jsonend--;
      }
      if (startJson == true) {
        rxtext += ch;  // Add in the received character
        //Serial.print(ch);
      }
      // if jsonend = 0 then we have have received equal number of curly braces
      if (jsonend == 0 && startJson == true) {
        //Serial.println("\nReceived OK...");
        //Serial.println("Decoding...");
        //Serial.println(rxtext.length());
        //Serial.println(rxtext); // Display received message
        Decode_news(rxtext);
        rxtext = "";
        client.stop();
        break;
      }
      timeoutmS = timeoutvalue_mS + millis();
      yield();
    }
    client.stop();
  }
  else {
    // if no connection was made:
    Serial.println("connection failed");
  }
}
//#########################################################################################
int StartWiFi() {
  Serial.print(F("\r\nConnecting to: ")); Serial.println(String(ssid));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED ) {
    delay(500); Serial.print(".");
  }
  Serial.println("WiFi connected at: " + String(WiFi.localIP()));
  return 1;
}

void Decode_news(String json) {
  //Serial.print(F("Creating object..."));
  DynamicJsonBuffer jsonBuffer (50 * 1024);
  JsonObject& root = jsonBuffer.parseObject(const_cast<char*>(json.c_str()));
  if (!root.success()) {
    Serial.print("ParseObject() failed");
  }
  const char* status = root["status"]; // "ok"
  int totalResults = root["totalResults"]; // e.g. 20
  JsonArray& articles = root["articles"];
  const char* title;
  const char* description;
  for (int i=0; i < totalResults; i++){
    JsonObject& articles0 = articles[i];
    title       = articles0["title"];
    description = articles0["description"];
    Serial.print("News item: "); Serial.print(i<10?"0":""); Serial.print(String(i+1)+" - ");
    Serial.println(title);
    if (description != "") Serial.println(description);
  }
}
