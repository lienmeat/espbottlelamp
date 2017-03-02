#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

typedef std::function<boolean (const char *ssid, const char *password)> ConnectToWifiCB;

ConnectToWifiCB connect_cb;

ESP8266WebServer server = ESP8266WebServer(80);

void handleConfigRoot() {
    String page = "<html><body><h1>WiFi Connection Setup</h1><form method=\"POST\" action=\"/login\"><input name=\"ssid\" placeholder=\"ssid\"><br /><input type=\"password\" name=\"password\" placeholder=\"password\"><br /><input type=\"submit\"></form></body></html>";
    server.send(200, "text/html", page);
}

void handleRoot() {
  if(WiFi.status() == WL_CONNECTED) {
    server.send(200, "text/html", "<html><body><h1>YEP</h1></body></html>");
  }
  else{
    handleConfigRoot();  
  }
}

void handleNotFound() {
  if(WiFi.status() != WL_CONNECTED) {
    //serve up the config page (captive portal reasons)
    handleConfigRoot();
  }else{
    server.send(404, "text/plain", "Nothing here, try again.");
  }
}

void handleWiFiConfigPOST() {
    String ssid;
    String password;
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      if(server.argName(i) == "ssid") {
          ssid = server.arg(i);
          Serial.println("");
          Serial.println(ssid);
      }
      if(server.argName(i) == "password") {
          password = server.arg(i);
          Serial.println("");
          Serial.println(password);
      }
    }

    String out = "<html><body><h1>Attempting to connect to " + ssid + "</h1></body></html>";
    server.send( 200, "text/html", out);

    //do connection to wifi here
    if (!connect_cb(ssid.c_str(), password.c_str())) {
        Serial.println("Failed to connect.");
    } else {
      //connected
      WiFi.mode(WIFI_STA);
    }
}

void setupWebserver(ConnectToWifiCB cb) {
    connect_cb = cb;
    
    // handle index
    server.on("/", handleRoot);

    server.on("/login", handleWiFiConfigPOST);

    server.onNotFound(handleNotFound);

    server.begin();
}

void webserverLoop() {
  server.handleClient();
}

#endif // WEBSERVER_H
