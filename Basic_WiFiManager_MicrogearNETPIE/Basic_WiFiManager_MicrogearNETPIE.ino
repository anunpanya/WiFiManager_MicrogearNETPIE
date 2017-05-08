/*  NETPIE ESP8266 basic sample                            */
/*  More information visit : https://netpie.io             */

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <MicroGear.h>

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

//for LED status
#include <Ticker.h>
Ticker ticker;

#define APPID   "APPID"
#define KEY     "APPKEY"
#define SECRET  "APPSECRET"
#define ALIAS   "esp8266"
#define AP_NAME "AP-XXXXX"

WiFiClient client;

int timer = 0;
MicroGear microgear(client);

void tick()
{
  //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);  // ticker ทำงานเรียกใช้ฟังก์ชั่น tick ทุกๆ 200ms
}

/* If a new message arrives, do this */
void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
    Serial.print("Incoming message --> ");
    msg[msglen] = '\0';
    Serial.println((char *)msg);
}

/* When a microgear is connected, do this */
void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
    Serial.println("Connected to NETPIE...");
    /* Set the alias of this microgear ALIAS */
    microgear.setAlias(ALIAS);
    ticker.attach(1, tick);  // ticker ทำงานเรียกใช้ฟังก์ชั่น tick ทุกๆ 1s
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  /* Add Event listeners */
  /* Call onMsghandler() when new message arraives */
  microgear.on(MESSAGE,onMsghandler);

  /* Call onConnected() when NETPIE connection is established */
  microgear.on(CONNECTED,onConnected);
  
  //set led pin as output
  pinMode(BUILTIN_LED, OUTPUT);
  ticker.attach(0.6, tick); // ticker ทำงานเรียกใช้ฟังก์ชั่น tick ทุกๆ 600ms 

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings(); // 

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.startConfigPortal(AP_NAME)) {  
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();  // สำหรับ library ตัวนี้จะไม่ใช้คำสั่ง ESP.reset()
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  ticker.detach();  // ยกเลิกคำสั่ง ticker
  //keep LED on
  digitalWrite(BUILTIN_LED, LOW);

  /* Initial with KEY, SECRET and also set the ALIAS here */
  microgear.init(KEY,SECRET,ALIAS);

  /* connect to NETPIE to a specific APPID */
  microgear.connect(APPID);
}

void loop() {
    /* To check if the microgear is still connected */
    if (microgear.connected()) {
        Serial.println("connected");

        /* Call this method regularly otherwise the connection may be lost */
        microgear.loop();

        if (timer >= 1000) {
            Serial.println("Publish...");

            /* Chat with the microgear named ALIAS which is myself */
            microgear.chat(ALIAS,"Hello");
            timer = 0;
        } 
        else timer += 100;
    }
    else {
        Serial.println("connection lost, reconnect...");
        if (timer >= 5000) {
            microgear.connect(APPID);
            timer = 0;
        }
        else timer += 100;
        ticker.detach();  // ยกเลิกคำสั่ง ticker
    }
    delay(100);
}
