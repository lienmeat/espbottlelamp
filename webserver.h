#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

typedef std::function<boolean (const char *ssid, const char *password)> ConnectToWifiCB;

ConnectToWifiCB connect_cb;

ESP8266WebServer server = ESP8266WebServer(80);




const char* serverUpload = "<form method='POST' action='/doupdate' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path){
  if(path == "/favicon.ico") {
    server.send(404, "text/html", "");
  }
//  USE_SERIAL.println(path);
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleConfigRoot() {
    String page = "<html><body><form method=\"POST\" action=\"/login\"><input name=\"ssid\" placeholder=\"ssid\"><br /><input name=\"password\" placeholder=\"password\"><br /><input type=\"submit\"></form></body></html>";
    server.send(200, "text/html", page);
}

void handleRoot() {
  if(WiFi.status() == WL_CONNECTED) {
    handleFileRead("/index.html");
  }
  else{
    handleConfigRoot();  
  }
}

void handleNotFound() {
  if(!handleFileRead(server.uri()) && WiFi.status() != WL_CONNECTED) {
    //serve up the config page (captive portal reasons)
    handleConfigRoot();
  }
}

void handleWiFiConfigPOST() {
    String ssid;
    String password;
//    String message = "<div><h1>Working</h1><p></p></div>";
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

    String out = "something";//htmlTemplate(message, "<title>Login Error</title>");
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
    //we might serve files, need this
    SPIFFS.begin();
    
    // handle index
    server.on("/", handleRoot);

    server.on("/login", handleWiFiConfigPOST);

//the example sketch does very bad things to my esp, and so does this... :(
  
    server.on("/update", HTTP_GET, [](){
      server.sendHeader("Connection", "close");
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "text/html", serverUpload);
    });
    
    server.on("/doupdate", HTTP_POST, [](){
      server.sendHeader("Connection", "close");
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
      ESP.restart();
    },[](){
      HTTPUpload& upload = server.upload();
      if(upload.status == UPLOAD_FILE_START){
//        Serial.setDebugOutput(true);
        WiFiUDP::stopAll();
//        Serial.printf("Update: %s\n", upload.filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if(!Update.begin(maxSketchSpace)){//start with max available size
          Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_WRITE){
        if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
          Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_END){
        if(Update.end(true)){ //true to set the size to the current progress
//          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }

      }
      yield();
    });

    server.onNotFound(handleNotFound);

    server.begin();
}

void webserverLoop() {
  server.handleClient();
}

#endif // WEBSERVER_H
