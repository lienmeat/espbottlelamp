//I have no idea what I need this for anymore...
#include <Arduino.h>
#include "filestorage.h"
#include "Lamp.h"
#include "AnimationManager.h"
#include "ota.h"

//animation libs
#include "lamputil.h"
#include "LampAnimation.h"
#include "RainbowFill.h"
#include "Fire.h"
#include "ColorAnim.h"
#include "Seahawks.h"

//webserver and endpoint handling
#include "webserver.h"

#include <PubSubClient.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>

#define USE_SERIAL Serial

#define LED_DATA_PIN 2
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

const char* mqtt_server = "lienmac";
WiFiClient espmqttclient;
PubSubClient mqtt(espmqttclient);

//mqtt topics
//"mode" of the lamp (animation name, solid color, off, on)
String topic_mode;
//color of the lamp in "Solid Color" mode
String topic_color;
//speed of lamp animation updates
String topic_speed;
//set mode topic to control the the lamp
String topic_set_mode;
//set speed topic to control the the lamp
String topic_set_speed;
//set color topic to control the the lamp
String topic_set_color;

Lamp lamp = Lamp(LED_DATA_PIN, NUM_LEDS, NUM_COLS); 
AnimationManager amngr = AnimationManager();

CRGB cur_color;
uint8_t cur_speed = 100;
uint8_t cur_animation = 0;

ColorAnim* cf;

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

void mqtt_publish_mode() {
  String payload = amngr.getCurrentAnimation()->getName();
  mqtt.publish(topic_mode.c_str(), payload.c_str());
}

void mqtt_publish_color() {
  String payload = colorToHex(cur_color);
  mqtt.publish(topic_color.c_str(), payload.c_str());   
}

void mqtt_publish_speed() {
  String payload = (String) cur_speed;
  mqtt.publish(topic_speed.c_str(), payload.c_str());   
}

void mqtt_set_mode(String payload) {
  int idx = amngr.switchAnimationByName(payload);
  if(idx >= 0) {
    cur_animation = idx;
    mqtt_publish_mode();  
  } 
}

void mqtt_set_color(String payload) {
  cur_color = hexToColor(payload);
  cf->setColor(cur_color);
  mqtt_publish_color();
}

void mqtt_set_speed(String payload) {
  cur_speed = atoi(payload.c_str());
  amngr.setSpeed(cur_speed);
  mqtt_publish_speed();
}

void mqtt_subscribe_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  char c[length+1];
  for(int j = 0; j < length; j++) {
    c[j] = (char) payload[j];
  }
  c[length] = '\0';
  String payload_s = c;
  
  if((String) topic == topic_set_mode) {
      //set the mode
      mqtt_set_mode(payload_s);
  }
  if((String) topic == topic_set_color) {
      //set the color
      mqtt_set_color(payload_s);
  }
  if((String) topic == topic_set_speed) {
      //set the speed
      mqtt_set_speed(payload_s);
  }
}

void setupMQTT() {
    String t_pfx = device + "/" + device_name;
    topic_mode = t_pfx + "/mode";
    topic_color = t_pfx + "/color";
    topic_speed = t_pfx + "/speed";
    topic_set_mode = t_pfx + "/set/mode";
    topic_set_color = t_pfx + "/set/color";
    topic_set_speed = t_pfx + "/set/speed";

    USE_SERIAL.println("mqtt publishing to: ");
    USE_SERIAL.println(topic_mode);
    USE_SERIAL.println(topic_color);
    USE_SERIAL.println(topic_speed);
    USE_SERIAL.println("mqtt subscribing to: ");
    USE_SERIAL.println(topic_set_mode);
    USE_SERIAL.println(topic_set_color);
    USE_SERIAL.println(topic_set_speed);
    
    mqtt.setServer(mqtt_server, 1883);
    mqtt.setCallback(mqtt_subscribe_callback);  
}

void reconnect_mqtt() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect(device_name.c_str())) {
      Serial.println("connected");
      // Once connected, publish current settings
      mqtt_publish_mode();
      mqtt_publish_color();
      mqtt_publish_speed();
      // resubscribe to topics
      mqtt.subscribe(topic_set_mode.c_str());
      mqtt.subscribe(topic_set_color.c_str());
      mqtt.subscribe(topic_set_speed.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setupLamp() {
    lamp.begin();
    //we keep a reference to colorfill, so that we can switch it's color
    //on the fly.
    cf = new ColorAnim(&lamp, CRGB::White, "Solid Color");
    amngr.addAnimation(new ColorAnim(&lamp, CRGB::White, "On"));
    amngr.addAnimation(new ColorAnim(&lamp, CRGB::Black, "Off"));
    amngr.addAnimation(cf);
    amngr.addAnimation(new RainbowFill(&lamp, "Rainbow"));
    amngr.addAnimation(new Seahawks(&lamp, "Seahawks"));
    amngr.addAnimation(new Fire(&lamp, "Fire"));
    amngr.begin();
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

    setupMQTT();
    
    /* Setup the DNS server redirecting all the domains to the apIP for the captive portal to work*/
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(DNS_PORT, "*", ap_ip);

    setupOTA();

    if(MDNS.begin(host)) {
        USE_SERIAL.println("MDNS responder started");
    }

    setupWebserver(connect);

    // Add service to MDNS
    MDNS.addService("http", "tcp", 80);
    
    setupLamp();
    
    USE_SERIAL.println("SETUP COMPLETE!!!");
}

void loop() {
  if(!wifiConnected()) {
    dnsServer.processNextRequest();
    webserverLoop();
  }else{
    if (!mqtt.connected()) {
      reconnect_mqtt();
    }
    mqtt.loop();
    otaLoop();
    amngr.loop();
  }
}

