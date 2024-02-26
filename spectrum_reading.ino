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

const int delayMillis = 5000; // 設定每次傳送資料到 Google 試算表後要休息多少毫秒 (請勿設定小於 1000)
const char* ssid = "CSHS_T36_AP_2G"; //輸入wifi ssid
const char* password = "51685168"; //輸入wifi 密碼WiFi.begin(ssid, password);
const char* serverUrl = "https://goattl.tw/cshs/hackathon/spectrum";
AS726X sensor;//Creates the sensor object
byte GAIN = 3; // 0: 1x 1: 3.7x 2: 16x 3: 64x (power-on default)
byte MEASUREMENT_MODE = 0;  // 0: Continuous reading of VBGY (Visible) / STUV (IR)
                            // 1: Continuous reading of GYOR (Visible) / RTUX (IR)
                            // 2: Continuous reading of all channels
                            // 3: One-shot reading of all channels (power-on default)

int bufferIndex = 0;
int bufferSize = 10;
SpectrumReadings* spectrumBuffer;

void sendSpectrumReading(const char* host = serverUrl) {
  WiFiClientSecure *client = new WiFiClientSecure;
  if(client) {
    client->setInsecure();
    String query = getQueryString(host);
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
}

void setup() {
  Serial.begin(115200);
  connectWifi();
  Wire.begin();
  sensor.begin(Wire, GAIN, MEASUREMENT_MODE);//Initializes the sensor with non default values
  spectrumBuffer = new SpectrumReadings[bufferSize];
}

void loop() {
  Serial.print("reading spectrum: ");
  Serial.print("timestamp=");
  Serial.print(millis()/1000);

  SpectrumReadings sr = getSpectrumReading();
  showSpectrumReading(sr);
  updateBuffer(sr);
  if (bufferIndex == bufferSize) {
    sendSpectrumReading();
    bufferIndex = 0;
  }
  delay(delayMillis);
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

  delay(1000);
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("STAIP address: ");
  Serial.println(WiFi.localIP());  
}


void showSpectrumReading(SpectrumReadings sr) {
  Serial.print("  violet="); Serial.print(sr.v);
  Serial.print("  blue="); Serial.print(sr.b);
  Serial.print("  green="); Serial.print(sr.g);
  Serial.print("  yellow="); Serial.print(sr.y);
  Serial.print("  orange="); Serial.print(sr.o);
  Serial.print("  red="); Serial.println(sr.r);
  // sendSpectrumReading(test, "127.0.0.1:9002/spectrum");
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
  sr.ts = millis() / 1000;
  return sr;
}

void updateBuffer(SpectrumReadings sr) {
  spectrumBuffer[bufferIndex] = sr;
  bufferIndex++;
}


String getQueryString(const char* host) {
  // sprintf 不支援浮點數，暫時棄用
  String dataString = "";
  for (int i=0; i<bufferSize; i++) {
    SpectrumReadings sr = spectrumBuffer[i];
    dataString = dataString + String(sr.ts)
    + "," + String(sr.v)
    + "," + String(sr.b)
    + "," + String(sr.g)
    + "," + String(sr.y)
    + "," + String(sr.o)
    + "," + String(sr.r)
    + ";";
  }

  String query = String(host) + "?s=" + dataString;
  return query;
}

