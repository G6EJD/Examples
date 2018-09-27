#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid     = "";
const char* password = "";

bool done = false;

typedef struct WGAstronomy {
  String moonPctIlum;
  String moonAge;
  String moonPhase;
  String sunriseTime;
  String sunsetTime;
  String moonriseTime;
  String moonsetTime;
} WGAstronomy;

int moon_percent;

WGAstronomy astronomyData;

char bufferJson[128];

// setup
void setup() {
  Serial.begin(115200);
  while (!Serial) continue;

  Serial.println();
  Serial.println("Connecting to: " + String(ssid));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.println("WiFi connected at: " + WiFi.localIP().toString());
  Serial.println();
}


// loop
void loop() {
  if (!done) {
    Serial.println(F("WUnderground"));
    String jsonStr = Request_WUnderground();

    Serial.println();
    Serial.println(F("reponse: "));
    jsonStr.trim();
    Serial.println(jsonStr);
    Serial.println();

    DecodeJson(jsonStr);

    done = true;
  }
}

String Request_WUnderground() {
  bool ok = false;
  String respString = "";

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");

    // configure server and url
    http.begin("http://api.wunderground.com/api/7b20aec43affb7b6/astronomy/q/CANADA/MONTREAL.json");
    //http.begin("http://newsapi.org/v2/everything?q=bitcoin&apiKey=2ee8e9e7ebea498b9ce519389e71aae8");

    //http.begin("192.168.1.12", 80, "/test.html");

    Serial.print("[HTTP] GET...n");
    // start connection and send HTTP header
    int httpCode = http.GET();
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %dn", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        respString = http.getString();
        ok = true;

        http.end();
      }
    }
    else {
      ok = false;
    }
  }
  if (ok) {
    Serial.println(F("ok."));
    return respString;
  }
  else {
    Serial.println(F("nothing."));
    return "";
  }
}

void DecodeJson(String jsonStr) {
  const size_t bufferSize = JSON_OBJECT_SIZE(1) + 8 * JSON_OBJECT_SIZE(2) + 2 * JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(9) + 440;
  DynamicJsonBuffer jsonBuffer(bufferSize);

  //const char* json = "{"response":{"version":"0.1","termsofService":"http://www.wunderground.com/weather/api/d/terms.html","features":{"astronomy":1}},"moon_phase":{"percentIlluminated":"96","ageOfMoon":"17","phaseofMoon":"Waning Gibbous","hemisphere":"North","current_time":{"hour":"23","minute":"12"},"sunrise":{"hour":"6","minute":"46"},"sunset":{"hour":"18","minute":"44"},"moonrise":{"hour":"19","minute":"54"},"moonset":{"hour":"8","minute":"05"}},"sun_phase":{"sunrise":{"hour":"6","minute":"46"},"sunset":{"hour":"18","minute":"44"}}}";

  const int jsonSize = jsonStr.length();
  char bufferJson[jsonSize];
  jsonStr.toCharArray(bufferJson, jsonSize);

  JsonObject& root = jsonBuffer.parseObject(jsonStr);
  if (!root.success()) {
    Serial.println(F("json parse failed."));
  }
  else {
    Serial.println(F("json parse ok."));
  }
  
  // Once you have a response, you can use the JSON coder see here: https://arduinojson.org/v5/assistant/
  // Paste the JSON respone into the left side, it produces the code for you
  
  JsonObject& response = root["response"];
  const char* response_version = response["version"]; // "0.1"
  const char* response_termsofService = response["termsofService"]; // "http://www.wunderground.com/weather/api/d/terms.html"

  int response_features_astronomy = response["features"]["astronomy"]; // 1

  JsonObject& moon_phase = root["moon_phase"];
  int moon_phase_percentIlluminated = moon_phase["percentIlluminated"]; // "95"
  int moon_phase_ageOfMoon = moon_phase["ageOfMoon"]; // "17"
  const char* moon_phase_phaseofMoon = moon_phase["phaseofMoon"]; // "Waning Gibbous"
  const char* moon_phase_hemisphere = moon_phase["hemisphere"]; // "North"

  int         moon_phase_current_time_hour = moon_phase["current_time"]["hour"]; // "4"
  int         moon_phase_current_time_minute = moon_phase["current_time"]["minute"]; // "38"

  int         moon_phase_sunrise_hour = moon_phase["sunrise"]["hour"]; // "6"
  int         moon_phase_sunrise_minute = moon_phase["sunrise"]["minute"]; // "47"

  int         moon_phase_sunset_hour = moon_phase["sunset"]["hour"]; // "18"
  int         moon_phase_sunset_minute = moon_phase["sunset"]["minute"]; // "42"

  int         moon_phase_moonrise_hour = moon_phase["moonrise"]["hour"]; // "20"
  int         moon_phase_moonrise_minute = moon_phase["moonrise"]["minute"]; // "21"

  int         moon_phase_moonset_hour = moon_phase["moonset"]["hour"]; // "9"
  int         moon_phase_moonset_minute = moon_phase["moonset"]["minute"]; // "11"

  int         sun_phase_sunrise_hour = root["sun_phase"]["sunrise"]["hour"]; // "6"
  int         sun_phase_sunrise_minute = root["sun_phase"]["sunrise"]["minute"]; // "47"

  int         sun_phase_sunset_hour = root["sun_phase"]["sunset"]["hour"]; // "18"
  int         sun_phase_sunset_minute = root["sun_phase"]["sunset"]["minute"]; // "42"
  //Now you have some local variables within this function scope that you need to assign to a global variable to use elsewhere
  moon_percent = moon_phase_percentIlluminated;
  Serial.println("Moon Illumination = " + String(moon_phase_percentIlluminated));
  Serial.println("Moon age          = " + String(moon_phase_ageOfMoon));
  Serial.println("Moon phase        = " + String(moon_phase_phaseofMoon));
  // And so-on
}
