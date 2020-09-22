#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <PMsensor.h>
#include "DHT.h"


unsigned long previousMillis = 0;
const long interval = 60000;  // 60 second

/*************************** Sensor *********************************/

PMsensor PM;
 
#define DHTPIN D3
#define DHTTYPE DHT11  
DHT dht(DHTPIN, DHTTYPE);

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "AP_IOT"
#define WLAN_PASS       "yunusggg"

/**************************** MQTT Setup ***********************************/

#define MQTT_SERVER      "192.168.0.101"
#define MQTT_PORT         1883
#define DUST              "/WEMOS/data/Dust"
#define TEMPERATURE       "/WEMOS/data/Temperature"
#define HUMIDITY          "/WEMOS/data/Humidity"
#define CLIENT_ID         "DEVICE_002"
String data = "";

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT);
//Adafruit_MQTT_Subscribe pan_tilt = Adafruit_MQTT_Subscribe(&mqtt, TOPIC);
Adafruit_MQTT_Publish dust_sensor = Adafruit_MQTT_Publish(&mqtt, DUST);
Adafruit_MQTT_Publish temp_sensor = Adafruit_MQTT_Publish(&mqtt, TEMPERATURE);
Adafruit_MQTT_Publish hum_sensor = Adafruit_MQTT_Publish(&mqtt, HUMIDITY);

void setup() {
      
      Serial.begin(115200);
      delay(10);
    
      Serial.println(); Serial.println();
      Serial.print("Connecting to ");
      Serial.println(WLAN_SSID);
    
      WiFi.begin(WLAN_SSID, WLAN_PASS);
      while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
      }
      Serial.println();
    
      Serial.println("WiFi connected");
      Serial.println("IP address: "); Serial.println(WiFi.localIP());

      PM.init(D7, A0);
      dht.begin();
}

void loop() {
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        
        MQTT_connect();
      
        float pm = PM.read(0.1);
         
        float h = dht.readHumidity();
  
        float t = dht.readTemperature();
  
        if (isnan(h) ) h = 0;
        if (isnan(t) ) t = 0;
        
        if (! dust_sensor.publish(pm)) {
          Serial.println(F("Failed"));
        }
        if (! hum_sensor.publish(h)) {
          Serial.println(F("Failed"));
        }
        if (! temp_sensor.publish(t)) {
          Serial.println(F("Failed"));
        }
  
        Serial.print("Dust : ");
        Serial.print(pm);
        Serial.print(" | Humidity : ");
        Serial.print(h);
        Serial.print(" | Temperature : ");
        Serial.println(t); 
        }
}

void MQTT_connect() {
      int8_t ret;
      if (mqtt.connected()) {
            return;
      }
    
      Serial.print("Connecting to MQTT... ");
    
      uint8_t retries = 3;
      while ((ret = mqtt.connect()) != 0) {
           Serial.println(mqtt.connectErrorString(ret));
           Serial.println("Retrying MQTT connection in 5 seconds...");
           mqtt.disconnect();
           delay(5000);
           retries--;
           if (retries == 0) {
                while (1);
           }
      }
      Serial.println("MQTT Connected!");
}
