#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>

#ifndef STASSID
#define STASSID ""            //wifi name
#define STAPSK  ""            //wifi password
#endif

WiFiServer server(0000);    //port of device
WiFiClient c;
WiFiServer TelnetServer(23);
WiFiClient Telnet;
ESP8266WebServer WebServer;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 7200, 1000);

const char* ssid = STASSID;
const char* password = STAPSK;
bool ota_flag = false;
int formattedDate;
String formattedtime;
unsigned long lastMillis;

void setup() {
  
  pinMode(4, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);

  digitalWrite(4, HIGH);
  digitalWrite(14, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);
  
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

  ArduinoOTA.setHostname("FrontGate");
  ArduinoOTA.setPassword("");   //password for OTA
  ArduinoOTA.setPort(8877);

  ArduinoOTASetup();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  WebServer.on("/restart",[](){
    WebServer.send(200,"text/plain", "Front Gate: Restarting...");
    delay(1000);
    ESP.restart();
  });

    WebServer.on("/update",[](){
    WebServer.send(200,"text/plain", "Front Gate: ESP8266 is in updating mode please upload your code or go to adress/restart or adress/normal to restore normal function of device");
    delay(1000);
    ota_flag = true;
  });
      
     WebServer.on("/normal",[](){
     WebServer.send(200,"text/plain", "Front Gate: ESP8266 is now in normal mode");
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
    Serial.println(formattedtime + " Front Gate: No Connection");
    Telnet.println(formattedtime + " Front Gate: No Connection");
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
      Serial.println(formattedtime + " Front Gate: OTA enabled upload your code");
      Telnet.println(formattedtime + " Front Gate: OTA enabled upload your code");
    }
    yield();
  }
  
  c = server.available();
  if(c.connected())
  {
    Serial.print("Front Gate: Incomming connection!\n");
    Telnet.println("Front Gate: Incomming connection!");
    while(true)
    {
      handleTelnet();
      char sstr[1]={c.read()};
      //---------------------------- Continue only if first char has ASCII value 2 that represent Start of Text ------------------------
      if(sstr[0] == 2)
      {
        char x[1]={c.read()};
        char y[1]={c.read()};
        char z[1]={c.read()};
  
        Serial.print(x[0]);
        Serial.print(y[0]);
        Serial.print(z[0]);
        Serial.print("\n");
        //------------------------ Open gate only when incoming bytes are "706" ------------------------
        if ((x[0] == '9') and (y[0] == '9') and (z[0] == '9'))
        {
          //c.write("606");
          Serial.print("Opening nothing 1\n");
          Telnet.println("Opening nothing 1");  
          digitalWrite(4, LOW);
          delay(500);
          digitalWrite(4, HIGH);
          delay(500);
          c.write("999");
          c.stop();
          break;
        }
        else if ((x[0] == '9') and (y[0] == '9') and (z[0] == '8'))
        {
          //c.write("717");
          Serial.print("Opening nothing 2\n");
          Telnet.println("Opening nothing 2");                    
          digitalWrite(14, LOW);
          delay(500);
          digitalWrite(14, HIGH);
          delay(500);
          c.write("998");
          c.stop();
          break;
        }
        else if ((x[0] == '6') and (y[0] == '0') and (z[0] == '6'))
        {
          //c.write("828");
          Serial.print("Opening whole gate\n");
          Telnet.println("Opening whole gate");
          digitalWrite(D5, LOW);
          delay(500);
          digitalWrite(D5, HIGH);
          delay(500);
          c.write("606");
          c.stop();
          break;
        }
        else if ((x[0] == '7') and (y[0] == '1') and (z[0] == '7'))
        {
          //c.write("939");
          Serial.print("Opening half gate\n");
          Telnet.println("Opening half gate");                      
          digitalWrite(D6, LOW);
          delay(500);
          digitalWrite(D6, HIGH);
          delay(500);
          c.write("717");
          c.stop();
          break;
        }
        else
        {
          Serial.print("Front Gate: Invalid command\n");
          Telnet.println("Front Gate: Invalid command");
          break;
        }
        Serial.print("\n");
        }
        if (!c.connected()) 
        {
          Serial.print("Front Gate: Lost Connection");
          Serial.print("\n");
          Telnet.println("Front Gate: Lost Connection");
          break;
        }
    }
  }
  
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
