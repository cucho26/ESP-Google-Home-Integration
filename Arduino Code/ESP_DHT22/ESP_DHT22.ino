#include <FirebaseESP8266.h>
#include "DHT.h"
#include <WiFiManager.h>
#include "Credentials.h"

#define MINUTE 60e6

#define DEBUG

#define ID 5

#define WIFI_NAME "Google Nest WiFi"
#define WIFI_PSWD "esp8266ex"

String tempPath = "/" + String(ID) + "/TemperatureSetting/thermostatTemperatureAmbient";
String mode = "/" + String(ID) + "/TemperatureSetting/thermostatMode";
String humiPath = "/" + String(ID) + "/TemperatureSetting/thermostatHumidityAmbient";

FirebaseData fbData;
DHT dht(D2, DHT22);
WiFiManager wm;

void putToSleep(){
  /**
   * Put the board and the MAX17043 to sleep
   */  
  ESP.deepSleep(10 * MINUTE);
}

void setup() {
  #ifdef DEBUG
  Serial.begin(115200);
  Serial.println("\nStart");
  #endif
  bool res = wm.autoConnect();
  if(!res){
	  #ifdef DEBUG
	  Serial.println("Failed to connect...");
	  #endif
	  putToSleep();
  } else {
	  #ifdef DEBUG
	  Serial.println("Connected to WiFi");
	  #endif
	  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
	  Firebase.reconnectWiFi(true);
  }
  
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  h = 55.0;
  t = 17.5;
  
  if(isnan(h) or isnan(t)){
	  #ifdef DEBUG
	  Serial.println("Measures failed");
	  #endif
	  putToSleep();
  }
  
  #ifdef DEBUG
	Serial.println("Temperature: " + (String)t + "°C");
	Serial.println("Humidité: " + (String)h + "%\n");
  #endif
  
  // Setting temperature
  if(Firebase.setInt(fbData, tempPath, (int)t)){
	  #ifdef DEBUG
	  Serial.println("Temperature set");
	  #endif
  } else {
	  #ifdef DEBUG
	  Serial.println("Something when wrong while setting temperature");
	  Serial.println(fbData.errorReason());
	  #endif
  }
  
  // Setting humidité
  if(Firebase.setInt(fbData, humiPath, (int)h)){
	  #ifdef DEBUG
	  Serial.println("Humidity set");
	  #endif
  } else {
	  #ifdef DEBUG
	  Serial.println("Something when wrong while setting humidity");
	  Serial.println(fbData.errorReason());
	  #endif
  }
  Serial.println("n\End");
  putToSleep();
}
