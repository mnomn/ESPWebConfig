#include <ESP8266WiFi.h>
#include <ESPWebConfig.h>

/*
 Configure Wifi after boot. No need to "hard code" SSID and password in code.

 First boot (or after longpress):
 Device will be an access point named
 ESP_<ipnumber>, which you access to configure normal WIFI and other parameters.
 User opens <ipnumber> in browser. Enter Wifi name and password.

 After config and restart:
 The device will get new IP. When you browse to the new ip number you will
 see a "Hello world" greeting.
 
 (Long press reset not implemented)
*/

ESP8266WebServer server(80);
int resetPin = -1; // No reset pin configured (not implemented in lib)
ESPWebConfig espConfig(resetPin);

void handleRoot() {
  server.send(200, "text/html", "<html><body><h1>Hello ESPWebConfig</h1></body></html>");
}

void setup() {
  Serial.begin(115200);
  while(!Serial) {
    delay(1);
  }
  Serial.println("Starting ...");

  if (espConfig.setup(server)) {
    Serial.println("Normal boot");
    Serial.println(WiFi.localIP());
    server.on ("/", handleRoot);
  } else {
    Serial.println("Config mode");
  }

  server.begin();
}

void loop() {
  espConfig.checkReset();
  server.handleClient();
}
