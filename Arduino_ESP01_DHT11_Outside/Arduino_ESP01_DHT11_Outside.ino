#include <ESP8266WiFi.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include "SPI.h"

#ifndef STASSID
#define STASSID ""            //wifi name
#define STAPSK  ""            //wifi password
#endif

#define DHTTYPE DHT22
#define DHTPIN  2

DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;
MySQL_Connection conn(&client);
MySQL_Cursor* cursor;

char INSERT_SQL[] = "INSERT INTO x.y (entity) VALUES ('temp')";        //replace x with database name and y with table name
char query[256];

IPAddress server_addr(,,,);     //db local ip
char userDB[] = "";             //user name
char passwordDB[] = "";         //user password

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
  //IPAddress dns(8, 8, 8, 8);
  //WiFi.config(ip, gateway, subnet, dns);
  WiFi.config(ip, gateway, subnet);
  
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

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  delay(10000);
   Serial.println("Connecting to SQL...  ");
   if (conn.connect(server_addr, 3306, userDB, passwordDB))
   Serial.println("OK.");
   else
   {
    Serial.println("FAILED.");
    ESP.restart();
   }
    
  
  // create MySQL cursor object
  cursor = new MySQL_Cursor(&conn);
}

void loop(){ 
  unsigned long currentMillis = millis();
  //Serial.println(currentMillis);
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    float newT = dht.readTemperature();
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      t = newT;
      char INSERT_SQLT[] = "INSERT INTO x.y (entity, value, location) VALUES ('temp', %f , 'detska_izba')";    //replace x with database name and y with table name
      sprintf(query, INSERT_SQLT, t);
      MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
      /*cur_mem->execute(query);
        delete cur_mem;
        Serial.print("Temp: ");
        Serial.println(t);*/
      if(cur_mem->execute(query))
      {
        delete cur_mem;
        Serial.print("Temp: ");
        Serial.println(t);
      }
      else 
      {
        Serial.print("Restarting");
        ESP.restart();
      }
      
    }
     //Read Humidity
    float newH = dht.readHumidity();
    // if humidity read failed, don't change h value 
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      h = newH;
      char INSERT_SQLH[] = "INSERT INTO x.y (entity, value, location) VALUES ('hum', %f , 'detska_izba')";     //replace x with database name and y with table name
      sprintf(query, INSERT_SQLH, h);
      MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
      /*cur_mem->execute(query);
      delete cur_mem;
        Serial.print("Humi: ");
        Serial.println(h);*/
      if(cur_mem->execute(query))
      {
        delete cur_mem;
        Serial.print("Humi: ");
        Serial.println(h);
      }
      else 
      {
        Serial.print("Restarting");
        ESP.restart();
      }
    }

  }
}
