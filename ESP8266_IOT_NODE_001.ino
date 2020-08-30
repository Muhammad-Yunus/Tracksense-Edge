#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include <OneWire.h>
#include <DallasTemperature.h>


/************************* Dallas Sensor Init *********************************/
#define ONE_WIRE_BUS D6
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress addr_temp;


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "AP_IOT"
#define WLAN_PASS       "yunusggg"

/**************************** MQTT Setup ***********************************/

#define MQTT_SERVER      "192.168.0.101"
#define MQTT_PORT         1883
#define TOPIC             "sensor/data/temperature"
#define CLIENT_ID         "DEVICE_001"
String data = "";

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT);
//Adafruit_MQTT_Subscribe pan_tilt = Adafruit_MQTT_Subscribe(&mqtt, TOPIC);
Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, TOPIC);

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

      sensors.begin();
      Serial.print("Parasite power is: "); 
      if (sensors.isParasitePowerMode()) Serial.println("ON");
      if (!sensors.getAddress(addr_temp, 0)) Serial.println("Unable to find address for Device 0");
      Serial.print("Device 0 Address: ");
      printAddress(addr_temp);
      Serial.println();
      sensors.setResolution(addr_temp, 9);
      Serial.print("Device 0 Resolution: ");
      Serial.print(sensors.getResolution(addr_temp), DEC); 
      Serial.println();
}

void loop() {
      MQTT_connect();
      
      sensors.requestTemperatures();
      float tempC = sensors.getTempC(addr_temp);

      if (! photocell.publish(tempC)) {
        Serial.println(F("Failed"));
      } else {
        Serial.print("Temp C: ");
        Serial.print(tempC);
        Serial.println(F(" OK!"));
      }
      delay(60000);
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

void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
