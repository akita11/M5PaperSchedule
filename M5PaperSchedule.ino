#include <M5EPD.h>
#include <Arduino_JSON.h>
#include "FS.h"
#include "SPIFFS.h"
#include <WiFi.h>
#include <HTTPClient.h>

// upoading file to SPIFFS
// https://www.mgo-tec.com/blog-entry-spiffs-uploader-plugin-arduino-esp32.html

// M5Paper library:
// https://github.com/m5stack/M5EPD

M5EPD_Canvas canvas(&M5.EPD);

const char* ssid     = "****"; // WiFi SSID
const char* password = "****"; // WiFi Password
String url = "http://.../get_sche.php"; // server URL

void setup() {
  M5.begin();

  String payload;

  wifiConnect();
  HTTPClient https;
  Serial.print("connect url :");
  Serial.println(url);

  Serial.print("[HTTPS] begin...\n");
  if (https.begin(url)) {  // HTTPS
    Serial.print("[HTTPS] GET...\n");
    // start connection and send HTTP header
    int httpCode = https.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      //Serial.println(https.getSize());

      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        payload = https.getString();
        Serial.println("HTTP_CODE_OK");
//        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  wifiDisconnect();

  delay(1000);

  JSONVar obj = JSON.parse(payload);
  JSONVar dat;
  JSONVar day_item;

  if (JSON.typeof(obj) == "undefined") Serial.println("Parsing input failed!");

  M5.TP.SetRotation(90);
  M5.EPD.SetRotation(90);
  M5.EPD.Clear(true);
  M5.RTC.begin();

  canvas.loadFont("/GenSenRounded-R.ttf", SD); // Load font files from SD Card

#define FONT_SIZE 32
#define DY 40
  canvas.createCanvas(540, 960);
  canvas.createRender(FONT_SIZE, 256); // font renderer, size=24, cache=256
  canvas.setTextSize(FONT_SIZE);

  int x, y = 10;
  String s;
  canvas.println("");
  if (obj.hasOwnProperty("data")) {
    dat = obj["data"];
    int i, j;
    for (i = 0; i < dat.length(); i++) {
      s = dat[i]["date"];
      canvas.setTextColor(15);
      canvas.println(" "+s);
      canvas.setTextColor(10);
      day_item = dat[i]["item"];
      for (j = 0; j < day_item.length(); j++) {
        s = day_item[j]["start"];
        canvas.print("   "+s);
        s = day_item[j]["end"];
        if (s != "") canvas.print("-"+s);
        canvas.print(": ");
        s = day_item[j]["summary"]; canvas.print(s);
        s = day_item[j]["description"];
        if (s != ""){ canvas.println(""); canvas.print("      ("+s.substring(0, 31)+")");}
        canvas.println("");
      }
    }
  }
  
  canvas.pushCanvas(0, 0, UPDATE_MODE_GC16);
  canvas.destoryRender(FONT_SIZE);

  delay(1000);
  M5.shutdown(60*60); // every 60min
}

void loop() {
}

void wifiDisconnect() {
  Serial.println("Disconnecting WiFi...");
  WiFi.disconnect(true); // disconnect & WiFi power off
}

void wifiConnect() {
  Serial.print("Connecting to " + String(ssid));

  //WiFi接続開始
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  //接続を試みる(30秒)
  for (int i = 0; i < 60; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      //接続に成功。IPアドレスを表示
      Serial.println();
      Serial.print("Connected! IP address: ");
      Serial.println(WiFi.localIP());
      break;
    } else {
      Serial.print(".");
      delay(500);
    }
  }

  // WiFiに接続出来ていない場合
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("");
    Serial.println("Failed, Wifi connecting error");
  }
}
