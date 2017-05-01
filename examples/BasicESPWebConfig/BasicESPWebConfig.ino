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
 
*/

/* Connfigure a pin that will reset config if grounded */
int resetPin = 4;

ESP8266WebServer server(80);
ESPWebConfig espConfig;

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
    // Print ip so we do not need to find it in the router.
    Serial.print("Normal boot: ");
    Serial.println(WiFi.localIP());

    // Set up hander for the normal web pages.
    server.on ("/", handleRoot);

  } else {
    Serial.println("Config mode.");
  }

  server.begin();
  /* Configure a reset pin. Connect resetPin to ground to clear config */
  pinMode(resetPin, INPUT_PULLUP);
}

void loop() {
  if (!espConfig.isConfigMode() && digitalRead(resetPin) == LOW) {
    espConfig.clearConfig();
    ESP.restart();
  }
  server.handleClient();
}
