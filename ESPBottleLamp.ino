//I have no idea what I need this for anymore...
#include <Arduino.h>
#include "filestorage.h"
#include "Lamp.h"
#include "AnimationManager.h"
#include "ota.h"

//animation libs
#include "LampAnimation.h"
#include "RainbowFill.h"
#include "Fire.h"
#include "ColorAnim.h"
#include "Seahawks.h"

//webserver and endpoint handling
#include "webserver.h"


#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <Hash.h>

#define USE_SERIAL Serial

#define LED_DATA_PIN 2
//#define p1 0
//#define p2 2
#define NUM_LEDS 4
#define NUM_COLS 4

String device = "bottlelamp";
String device_id = "";
String device_name = "";

const byte DNS_PORT = 53;
DNSServer dnsServer;

String ap_ssid = device + "_AP";
String ap_password = "";
IPAddress ap_ip = IPAddress(192, 168, 4, 1);

struct WiFiConfStruct
{
  char          ssid[64] = "somessid";
  char          pass[64] = "badpass";
} WiFiSettings;

const char* wifisettingsfile = "/wificonf.txt";

const char* host = "";

Lamp lamp = Lamp(LED_DATA_PIN, NUM_LEDS, NUM_COLS); 
AnimationManager amngr = AnimationManager();

WebSocketsServer webSocket = WebSocketsServer(81);

int hue = 0;
int hue_step = 2;
int cur_delay = 0;

uint8_t red = 0;
uint8_t green = 0;
uint8_t blue = 0;
//percent of normal speed
uint8_t cur_speed = 100;
uint8_t cur_animation = 0;
int8_t hue_shift = -1;

ColorAnim* cf;

long ms = 0;
long begin = 0;

/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

/** Is this an IP? */
boolean isIp(String str) {
  for (int i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

void setupConfigPortal(const char *ssid, const char* password) {
  Serial.println("Configuring access point... ");
  Serial.println(ssid);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ap_ip, ap_ip, IPAddress(255, 255, 255, 0));

  if (password != NULL) {
    WiFi.softAP(ssid, password); //password option
  } else {
    WiFi.softAP(ssid);
  }

  delay(500); // Without delay I've seen the IP address blank
  Serial.println("AP IP address: ");
  Serial.println(WiFi.softAPIP());
}

boolean  startConfigPortal(char const *ssid, char const *password) {
  Serial.println("SET AP");
  ap_ssid = ssid;
  ap_password = password;
  setupConfigPortal(ssid, password);
  return true;
}

boolean wifiConnected() {
  return (WiFi.status() == WL_CONNECTED);  
}

boolean connect(const char *ssid, char const *password) {
  // We start by connecting to a WiFi network
  USE_SERIAL.print("Connecting to ");
  USE_SERIAL.println(ssid);
  WiFi.hostname(host);
  WiFi.begin(ssid, password);
  int8_t tries = 20;
  while (!wifiConnected() && tries > 0) {
    delay(500);
    USE_SERIAL.print(".");
    tries--;
  }
  if(wifiConnected()) {
    strcpy(WiFiSettings.ssid, ssid);
    strcpy(WiFiSettings.pass, password);
    SaveToFile((char*) wifisettingsfile, 0, (byte*)&WiFiSettings, sizeof(struct WiFiConfStruct));
    USE_SERIAL.println("");
    USE_SERIAL.println("WiFi connected");
    USE_SERIAL.println("SSID: ");
    USE_SERIAL.println(WiFiSettings.ssid);  
    USE_SERIAL.println("IP address: ");
    USE_SERIAL.println(WiFi.localIP());
    return true;
  }
  USE_SERIAL.println("WiFi could not connect");
  return false;
}

String getStateString() {
  String state = "STATE:";
  state += "r:" + (String) red + ";";
  state += "g:" + (String) green + ";";
  state += "b:" + (String) blue + ";"; 
  state += "speed:" + (String) cur_speed + ";";
  state += "anim:" + (String) cur_animation + ";";
  state += "anims:";
  state += amngr.getAnimationNames();
  state+=";";
  return state;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t len) {
    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

            // send message to client
            webSocket.sendTXT(num, "Connected");
            String state = getStateString();
            webSocket.sendTXT(num, state);
            break;
        }
        case WStype_TEXT:
            String payload_str = (char *) payload;
            USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);

            if(payload_str.startsWith("color:")) {
                // we get RGB data
                // decode rgb data
                uint32_t rgb = (uint32_t) strtol((const char *) &payload[7], NULL, 16);
                red = ((rgb >> 16) & 0xFF);
                green = ((rgb >> 8) & 0xFF);
                blue = ((rgb >> 0) & 0xFF);
                //ColorAnim* colorfill = amngr.getAnimation(0);
                cf->setColor(CRGB(red, green, blue));
            }

            if(payload_str.startsWith("anim:")) {
                //animation
                cur_animation = (uint8_t) strtol((const char *) &payload[5], NULL, 10);
                amngr.switchAnimation(cur_animation);
            } 
            if(payload_str.startsWith("speed:")) {
                //animation speed
                cur_speed = (uint8_t) strtol((const char *) &payload[6], NULL, 10);
                amngr.setSpeed(cur_speed);
            }
            
            String state = getStateString();
            webSocket.broadcastTXT(state);
            
            break;
    }
}



void setup() {
    //this needs to happen IMMEDIATELY so everything else can use these values!
    device_id = ESP.getChipId();
    device_name = device + "_" + device_id;
    host = device_name.c_str();

    setupFilestorage();

    USE_SERIAL.begin(9600);
    delay(500);
    
    USE_SERIAL.println("Booting "+device_name);

    //get stored wifi settings if they have previously been saved:
    LoadFromFile((char*) wifisettingsfile, 0, (byte*)&WiFiSettings, sizeof(struct WiFiConfStruct));

    //if wifi fails to connect to an access point
    //we start up in AP mode so you can enter the wifi creds
    //and do setup
    if(!connect(WiFiSettings.ssid, WiFiSettings.pass)) {
      startConfigPortal(ap_ssid.c_str(), ap_password.c_str()); 
    }

    /* Setup the DNS server redirecting all the domains to the apIP for the captive portal to work*/
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(DNS_PORT, "*", ap_ip);

    setupOTA();

    // start webSocket server
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    if(MDNS.begin(host)) {
        USE_SERIAL.println("MDNS responder started");
    }

    setupWebserver(connect);

    // Add service to MDNS
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 81);
    lamp.begin();

    cf = new ColorAnim(&lamp, CRGB::White, "ColorFill");
    amngr.addAnimation(cf);
    amngr.addAnimation(new RainbowFill(&lamp, "Rainbow"));
    amngr.addAnimation(new Seahawks(&lamp, "Seahawks"));
    amngr.addAnimation(new Fire(&lamp, "Fire"));
    amngr.addAnimation(new ColorAnim(&lamp, CRGB::White, "On/White"));
    amngr.addAnimation(new ColorAnim(&lamp, CRGB::Black, "Off/Black"));
    amngr.begin();
    USE_SERIAL.println("SETUP COMPLETE!!!");
}

static inline void fps(const int seconds){
  // Create static variables so that the code and variables can
  // all be declared inside a function
  static unsigned long lastMillis;
  static unsigned long frameCount;
  static unsigned int framesPerSecond;
  
  // It is best if we declare millis() only once
  unsigned long now = millis();
  frameCount ++;
  if (now - lastMillis >= seconds * 1000) {
    framesPerSecond = frameCount / seconds;
    Serial.println(framesPerSecond);
    frameCount = 0;
    lastMillis = now;
  }
}

void loop() {
//    fps(1);
    if(!wifiConnected()) {
      dnsServer.processNextRequest();
    }
    else{
      otaLoop();  
      amngr.loop();
      webSocket.loop();
    }
    webserverLoop();
}

