#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "Credentials.h"

#define WIFI_SSID     "Google Nest WiFi"
#define WIFI_PSWD     "esp8266ex"

#define LAMP LED_BUILTIN
#define ID "3"

#define DEBUG

bool GoogleHomeLampState = false;
String OnOffPath = "/" + String(ID) + "/OnOff";
FirebaseData firebaseData1;

void setup() {
  Serial.begin(115200);
  pinMode(LAMP, OUTPUT); digitalWrite(LAMP, LOW);
  
  WiFi.begin(WIFI_SSID, WIFI_PSWD);
  Serial.println("Connecting to: " + (String)WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println("OK");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  if(!Firebase.beginStream(firebaseData1, OnOffPath)){
    Serial.println("Could not begin stream");
    Serial.println("Reason:" + firebaseData1.errorReason());
  } else {
    #ifdef DEBUG
    Serial.println("[FIREBASE] Stream began");
    #endif
  }
}

void loop() {
  if(!Firebase.readStream(firebaseData1)){
    Serial.println();
    Serial.println("Can't read stream data");
    Serial.println("Reason:" + firebaseData1.errorReason());
    Serial.println();
  }

  if(firebaseData1.streamTimeout()){
    // nothing to do
  }

  if(firebaseData1.streamAvailable()){
    if(Firebase.get(firebaseData1, OnOffPath)){
      if(firebaseData1.dataType() == "json"){
        DynamicJsonDocument json(250);
        deserializeJson(json, firebaseData1.jsonString());
        #ifdef DEBUG
        Serial.print("Json string:"); Serial.println(firebaseData1.jsonString());
        Serial.println("State: " + json["on"].as<String>());
        #endif
        if(GoogleHomeLampState != json["on"]){
          GoogleHomeLampState = json["on"];
          digitalWrite(LAMP, !GoogleHomeLampState);
        }
      } else {
        // Serial.println("Wrong type, found " + firebaseData1.dataType());
      }
    } else {
      // Serial.println("Error:" + firebaseData1.errorReason());
    }
  }
  delay(100);
}
