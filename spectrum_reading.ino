#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "AS726X.h"

struct SpectrumReadings {
  int ts;
  float v;
  float b;
  float g;
  float y;
  float o;
  float r;
  float temp;
};

// const char* ssid = "iPhone-YJL"; //輸入wifi ssid
const char* ssid = ""; //輸入wifi ssid
const char* password = ""; 
const char* serverUrl = "";
AS726X sensor;//Creates the sensor object
byte GAIN = 3; // 0: 1x 1: 3.7x 2: 16x 3: 64x (power-on default)
byte MEASUREMENT_MODE = 0;  // 0: Continuous reading of VBGY (Visible) / STUV (IR)
                            // 1: Continuous reading of GYOR (Visible) / RTUX (IR)
                            // 2: Continuous reading of all channels
                            // 3: One-shot reading of all channels (power-on default)

void sendSpectrumReading(SpectrumReadings sr, const char* host = serverUrl) {
  WiFiClientSecure *client = new WiFiClientSecure;
  if(client) {
    client->setInsecure();
    String query = getQueryString(sr, host);
    HTTPClient https;
    Serial.println("[GET] " + query);
    if (https.begin(*client, query)) {
      int httpCode = https.GET();
      if (httpCode > 0) {
       Serial.printf("[STATUS CODE] %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.printf("[RESPONSE] %s\n", payload);
        }
      }
      else {
        Serial.printf("[ERROR]: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
    }
  }
  else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  Serial.println();
  delay(10000);
}

void setup() {
  Serial.begin(115200);
  connectWifi();
  Wire.begin();
  sensor.begin(Wire, GAIN, MEASUREMENT_MODE);//Initializes the sensor with non default values
}

void loop() {
  Serial.print("reading spectrum: ");
  Serial.print("timestamp=");
  Serial.println(millis()/1000);
  SpectrumReadings sr = getSpectrumReading();
  sendSpectrumReading(sr);
  delay(1000);
}

void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("STAIP address: ");
  Serial.println(WiFi.localIP());  
}


void testSpectrumReading() {
  SpectrumReadings test;
  test.v = 1.0;
  test.b = 2.0;
  test.g = 3.0;
  test.y = 4.0;
  test.o = 5.0;
  test.r = 6.0;
  test.temp = 777.0;
  sendSpectrumReading(test, "127.0.0.1:9002/spectrum");
}

SpectrumReadings getSpectrumReading() {
  SpectrumReadings sr;
  sensor.takeMeasurements();
  if (sensor.getVersion() == SENSORTYPE_AS7262)
  {
    sr.v = sensor.getCalibratedViolet();
    sr.b = sensor.getCalibratedBlue();
    sr.g = sensor.getCalibratedGreen();
    sr.y = sensor.getCalibratedYellow();
    sr.o = sensor.getCalibratedOrange();
    sr.r = sensor.getCalibratedRed();
    sr.temp = sensor.getTemperatureF();
  }
  return sr;
}


String getQueryString(SpectrumReadings sr, const char* host) {
  // sprintf 不支援浮點數，暫時棄用
  String query = String(host) 
    + "?ts=" + String(sr.ts)
    + "&v=" + String(sr.v)
    + "&b=" + String(sr.b)
    + "&g=" + String(sr.g)
    + "&y=" + String(sr.y)
    + "&o=" + String(sr.o)
    + "&r=" + String(sr.r);
  return query;
}
