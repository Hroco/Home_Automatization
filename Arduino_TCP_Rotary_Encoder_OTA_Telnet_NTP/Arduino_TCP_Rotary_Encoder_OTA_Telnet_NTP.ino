#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <stdio.h>
#include <stdlib.h>
#include <NTPClient.h>

#ifndef STASSID
#define STASSID ""            //wifi name
#define STAPSK  ""            //wifi password
#endif

#define outputA 4 //GPIO pin 4 na doske pin 2
#define outputB 5 //GPIO pin 5 na doske pin 1

WiFiServer server(0000);    //port of device
WiFiClient c;
WiFiServer TelnetServer(23);
WiFiClient Telnet;
ESP8266WebServer WebServer;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 7200, 10000);

int counter = 0; 
bool aState;
bool aLastState;
bool newConnection = true;
const char* ssid     = STASSID;
const char* password = STAPSK;
int pozicia = 0;
int i = 0;
int thousand, thousandOld;
bool ota_flag = false;
int formattedDate;
String formattedtime;
unsigned long lastMillis;

void setup() {
  pinMode (outputA,INPUT);
  pinMode (outputB,INPUT);
  aLastState = digitalRead(outputA);
  
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  IPAddress ip(, , , );         //local ip of device
  IPAddress gateway(, , , );    //gateway ...
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(ip, gateway, subnet);
  IPAddress dns(8, 8, 8, 8);
  WiFi.config(ip, gateway, subnet, dns);
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.setPort(8879);
  ArduinoOTA.setHostname("GarageSenzor");
  ArduinoOTA.setPassword("");   //password for OTA

  ArduinoOTASetup();
  
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  WebServer.on("/restart",[](){
    WebServer.send(200,"text/plain", "Garage Senzor: Restarting...");
    delay(1000);
    ESP.restart();
  });

    WebServer.on("/update",[](){
    WebServer.send(200,"text/plain", "Garage Senzor: ESP8266 is in updating mode please upload your code or go to adress/restart or adress/normal to restore normal function of device");
    delay(1000);
    ota_flag = true;
  });
      
     WebServer.on("/normal",[](){
     WebServer.send(200,"text/plain", "Garage Senzor: ESP8266 is now in normal mode");
     delay(1000);
     ota_flag = false;
  });
  
  timeClient.begin();
  server.begin();
  WebServer.begin();
  TelnetServer.begin();
  TelnetServer.setNoDelay(true);  
}

void loop() {
  timeClient.update();
  handleTelnet();
  WebServer.handleClient();
  reboot();
  formattedDate = timeClient.getEpochTime();
  formattedtime = timeClient.getFormattedTime();
  if (formattedDate - lastMillis >= 1)
  {
    lastMillis = formattedDate;
    Serial.println(formattedtime + " No Connection Poloha: " + counter);
    Telnet.println(formattedtime + " No Connection Poloha: " + counter);
  }
  
  while(ota_flag)
  {
    ArduinoOTA.handle();
    handleTelnet();
    WebServer.handleClient();
    timeClient.update();
    formattedtime = timeClient.getFormattedTime();
    formattedDate = timeClient.getEpochTime();
    if (formattedDate - lastMillis >= 1)
    {  
      lastMillis = formattedDate;
      Serial.println(formattedtime + " Garage Senzor: OTA enabled upload your code");
      Telnet.println(formattedtime + " Garage Senzor: OTA enabled upload your code");
    }
    yield();
  }
  MovementDetection(&aState, &aLastState, &counter);
  c = server.available();
  if (c.connected()) 
  {  
    Serial.print("Garage Senzor: Client Connected");
    Telnet.println("Garage Senzor: Client Connected");
    newConnection = true;    
    while(true)
    {
      handleTelnet();
      MovementDetection(&aState, &aLastState, &counter);
      i++;;
      //---------- Sending every thousand location  --------
      if(i==1000)
      {
        yield(); //reset watchdog timer
        i=0;
        if(!newConnection)
        {
          thousandOld = thousand;
        }
        else
        {
          thousandOld--;
          newConnection = false;
        }
        
        thousand = counter;
        if(thousand != thousandOld)
        {
          //---------- Sending location to xamarin via TCP -----
          char buffer[10];
          char startString[2];
          char endString[2];
          startString[0] = 2;
          startString[1] = '\0';
          endString[0] = 3;
          endString[1] = '\0';
          snprintf(buffer, 10, "%d", thousand);
          strcat(buffer, endString);
          strcat(startString, buffer);
          Telnet.println(thousand);
          Serial.print(startString);
          Serial.print("\n");
          c.write(startString);
        }
        if (!c.connected()) 
        {
          Serial.print("Garage Senzor: Lost Connection");
          Telnet.println("Garage Senzor: Lost Connection");
          Serial.print("\n");
          break;
        }
       }
     } 
   }           
  
}

int MovementDetection(bool* pState, bool* pLastState, int* ActualLocation)
{
 *pState = digitalRead(outputA);
 if (*pState != *pLastState)
 {     
   if (digitalRead(outputB) != *pState) 
   { 
     *ActualLocation=*ActualLocation+1;
   }
   else 
   {
     *ActualLocation=*ActualLocation-1; 
   } 
 }
 *pLastState = *pState;   
 return 0;
}


//--------------------------------------Setup----------------------------------

void ArduinoOTASetup()
{

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    Serial.printf("\n");
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();  
}

void handleTelnet()
{
  if(TelnetServer.hasClient())
  {
    if(!Telnet||!Telnet.connected())
    {
      if(Telnet) Telnet.stop();
      Telnet = TelnetServer.available();  
    } 
    else
    {
      TelnetServer.available().stop();  
    } 
  }  
}

void reboot()
{
   //reboot arduino at 4:00
    if(timeClient.getHours() == 4 && timeClient.getMinutes() == 0 && timeClient.getSeconds() == 0)
      ESP.restart();
}
