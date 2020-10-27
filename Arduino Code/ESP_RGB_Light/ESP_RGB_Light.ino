#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include "Credentials.h"

#define DEBUG

// WiFi Credentials
#define WIFI_SSID     "Google Nest WiFi"
#define WIFI_PSWD     "esp8266ex"

#define ID "1"

#define LED_NUM 30
#define LED_PIN D4

//EEPROM address definition
#define Start_Address 0
#define Bri_Address Start_Address + sizeof(int)
#define Clr_Address Bri_Address + sizeof(int)

// Strip LED object
Adafruit_NeoPixel strip(LED_NUM, LED_PIN, NEO_GBRW + NEO_KHZ800);


String BrightnessPath = "/" + String(ID) + "/Brightness/brightness";
String OnOffPath = "/" + String(ID) + "/OnOff/on";
String ColorPath = "/" + String(ID) + "/ColorSetting/color/spectrumRGB";
FirebaseData fbData;

struct ColorCode {
  int red;
  int green;
  int blue;
  int white = 0;
};

ColorCode OLE2RGB(int color) {
  Serial.println("OLE Color:" + (String)color);
  ColorCode rgb;
  rgb.red = color % 256;
  rgb.green = (color / 256) % 256;
  rgb.blue = (color / (int)pow(256, 2)) % 256;
  return rgb;
}

void saveBrightness(int newValue){
  EEPROM.write(Bri_Address, newValue);
}


int readBrightness(){
  return EEPROM.read(Bri_Address);
}

int readColor(){
  return EEPROM.read(Clr_Address);
}

void setup(){
    Serial.begin(115200);

    // Retrieve data stored on EEPROM
    EEPROM.begin(256);
    // EEPROM.get(Clr_Address, current_color);

    WiFi.begin(WIFI_SSID, WIFI_PSWD);
    Serial.println("Connecting to: " + (String)WIFI_SSID);
    while (WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(500);
    }
    Serial.println("OK");

    // Connect to Firebase database
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);

    // Initialize LED strip
    strip.begin();
    // strip.fill(current_color);
    strip.setBrightness(readBrightness());
    strip.show();

    if(!Firebase.beginStream(fbData, BrightnessPath)){
        Serial.println("Could not begin stream");
        Serial.println("Reason:" + fbData.errorReason());
    } else {
        #ifdef DEBUG
        Serial.println("[FIREBASE] Stream began");
        #endif
    }
}

void loop(){
    if(!Firebase.readStream(fbData)){
        Serial.println("\nCannot read stream data\n");
        Serial.println("Reason: " + fbData.errorReason() + "\n");
    }

    if(fbData.streamAvailable()){
      // On Brightness Change
      if(Firebase.get(fbData, BrightnessPath)){
        int brightness = fbData.intData();
        #ifdef DEBUG
        Serial.println("Type: " + fbData.dataType() + " Brightness Value: " + (String)brightness);
        #endif
        saveBrightness(brightness);
        strip.setBrightness(brightness); strip.show();
      }

      // On Color Change
      if(Firebase.get(fbData, ColorPath)){
        if(fbData.dataType() == "int"){
          ColorCode rgb = OLE2RGB(fbData.intData());
          Serial.println("R: " + (String)rgb.red + " g: " + (String)rgb.green + " b:" + (String)rgb.blue);
          strip.fill(strip.Color(rgb.red, rgb.green, rgb.blue, rgb.white)); strip.show();
          // strip.fill(strip.Color(255, 0, 0));
        } else {
          Serial.println("Expecting type: " + fbData.dataType() + " for colors");
        }
      }
      
      // On Power Change
      if(Firebase.get(fbData, OnOffPath)){
        if(fbData.dataType() == "boolean"){
          bool state = fbData.boolData();
          #ifdef DEBUG
          Serial.println("Type: " + fbData.dataType() + " State: " + (String)state);
          #endif;
          if(state){
            strip.setBrightness(readBrightness()); strip.show(); // Somehow brightness doesn't come back pnce leds are turned off
          } else {
            strip.setBrightness(0); strip.show();
          }
        }
      }
    }
    delay(10);
}
