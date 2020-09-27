#include <ESP8266WiFi.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include "SPI.h"

#ifndef STASSID
#define STASSID ""            //wifi name
#define STAPSK  ""            //wifi password
#endif

#define DHTPIN D13    
#define DHTTYPE    DHT11

DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;

float t = 0.0;
float h = 0.0;

unsigned long previousMillis = 0;
const long interval = 10000;

const char* ssid = STASSID;
const char* password = STAPSK;

void setup() {
  dht.begin(); 
  
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  IPAddress ip(, , , );         //local ip of device
  IPAddress gateway(, , , );    //gateway ...
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(8, 8, 8, 8);
  WiFi.config(ip, gateway, subnet, dns);
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED) 
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
      
}

void loop(){ 

  IPAddress server(, , , ); // my computer ip
  if (client.connect(server, 50000)) {
    while(true)
    {
      if(client.connected())
      {
        delay(10000);
        Serial.println("connected");
        delay(100);
        float newT = dht.readTemperature();
        float newH = dht.readHumidity();
        
        if (isnan(newT)||isnan(newH)) {
          Serial.println("Failed to read from DHT sensor!");
        }
        else
        {
          t = newT;
          h = newH;
          char output[] = "{t,%f,1}{h,%f,1}"; //1 is detska izba
          char query[256];
          sprintf(query, output, t, h);
          client.print(query);
          Serial.print("Temp: ");
          Serial.println(t);
          Serial.print("Humi: ");
          Serial.println(h);
        }
      }
      else
      {
        Serial.print("Closing connection");
        client.stop();
        break;
      }
    }

  }
}
