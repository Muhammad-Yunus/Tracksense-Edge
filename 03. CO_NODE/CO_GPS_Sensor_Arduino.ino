#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>


TinyGPSPlus gps;  // The TinyGPS++ object

// REST API Endpoint
String RESTAPI  = "http://192.168.0.101:1880/api/co";
String Device_Id = "01500000A";

SoftwareSerial ss(4, 5); // The serial connection to the GPS device

#define ssid "AP_IOT"
#define password "yunusggg"
int r = 14; //D5
int g = 12; //D6
int b = 13; //D7
String latitude , longitude;
//int year , month , date, hour , minute , second;
String lat_str , lng_str, ispu_mgm3CO;
int Inter = 10000;
int Now;
int Prev = 0;
String Json_Data = "";
const long utcOffsetInSeconds = 3600 * 7;
//area ada di WIB Indonesia dan itu harus +7jam dari GMT sehingga program diatas harus dikalikan dengan 7 saja
char daysOfTheWeek[7][12] = {"Minggu", "Senin ", "Selasa", "Rabu  ", "Kamis ", "Jumat ", "Sabtu"};

//Definisi tool NTP nya
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

unsigned long epcohTime;
void setup()
{
  Serial.begin(9600);
  pinMode (r , OUTPUT);
  pinMode (g , OUTPUT);
  pinMode (b , OUTPUT);
  digitalWrite (r, LOW);
  digitalWrite (g, LOW);
  digitalWrite (b, LOW);
  ss.begin(9600);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  digitalWrite (r, LOW);
  digitalWrite (g, LOW);
  digitalWrite (b, HIGH);
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());

}


void sendDatatoRESTAPI(String Device_Id, String latitude, String longitude, float ispu_mgm3CO, float ugm3CO)
{
    timeClient.update();
    epcohTime =  timeClient.getEpochTime();
    WiFiClient client;
    HTTPClient http;
    http.begin(client, RESTAPI);
    http.addHeader("Content-Type", "application/json");
    Json_Data = "{\"DeviceId\":\"" + Device_Id + 
                 "\", \"Timestamp\": "+ String(epcohTime) + 
                 ", \"Latittude\": " + latitude + 
                 ", \"Longitude\": " + longitude + 
                 ", \"RawValue\": " + String(ugm3CO, 6) + 
                 ", \"Value\": " + String(ispu_mgm3CO, 6) +
                 "}";
    Serial.print (Json_Data);
    int httpCode = http.POST(Json_Data);
      
    // httpCode will be negative on error
    if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        const String& payload = http.getString();
        Serial.println("received payload:\n<<");
        Serial.println(payload);
        Serial.println(">>");
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  
}


void loop()
{
  
  float CO = analogRead(A0);
  float adcCO = (5 * CO) / 1023;
  float ppmCO = 1.9355 * adcCO + 20.00;
  float ugm3CO = ppmCO * 1000 * ((1 * 0.028) / (0.0821 * 298));
  float ispu_mgm3CO = (0.6974 * ugm3CO) - 13.01;
  
  Serial.print("Serial Status :");
  Serial.println(ss.available() > 0);
  
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
      displayInfo(Device_Id, ispu_mgm3CO, CO);

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
}

void displayInfo(String Device_Id, float ispu_mgm3CO, float ugm3CO)
{
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    digitalWrite (r, LOW);
    digitalWrite (g, HIGH);
    digitalWrite (b, LOW);
    latitude = String(gps.location.lat(), 6);
    longitude = String(gps.location.lng(), 6);
    Serial.print(ugm3CO);
    Serial.print(" ");
    //Serial.print(F(","));
    //Serial.print(longitude);
    sendDatatoRESTAPI(Device_Id, latitude, longitude, ispu_mgm3CO, ugm3CO);
  }
  else
  {
      digitalWrite (r, HIGH);
      digitalWrite (g, LOW);
      digitalWrite (b, LOW);
      Serial.print(F("INVALID"));
  }
  Serial.println();
  delay(60000);
}
