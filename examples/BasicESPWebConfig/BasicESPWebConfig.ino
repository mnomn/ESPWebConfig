#include <ESP8266WiFi.h>
#include <ESPWebConfig.h>
#include <ESP8266HTTPClient.h>

/*
 Configure Wifi after boot. No need to "hard code" SSID and password in code.

 First boot (or after reset):
 Device will be an access point named ESP_<ipnumber>, which you access to
 configure normal WIFI and other parameters.
 User opens <ipnumber> in browser and enter Wifi name and password.

 After config and restart the device will get new IP. Now you can write networking code in sketch (WiFiClient, mqtt, etc)
*/

/* Connfigure a pin that will reset config if grounded (button pressed) */
int resetPin = 0;

ESPWebConfig espConfig;

void setup() {
  Serial.begin(74880);
  while(!Serial) {
    delay(1);
  }
  Serial.println("");
  Serial.println("Starting ...");

  /* Read config values. If no config values it will block and
     create an access point and serve the confg web UI. */
  espConfig.setup();

  // Print ip so we do not need to find it in the router.
  Serial.print("Configuration done!");
  Serial.println(WiFi.localIP());

  /* Configure a reset pin. Connect resetPin to ground to clear config */
  pinMode(resetPin, INPUT_PULLUP);
}

void loop() {
  // Restart by pressing a button
  if (digitalRead(resetPin) == LOW) {
    Serial.print("Clear configuration");
    espConfig.clearConfig();
    ESP.restart();
  }

  // Verify that wifi works
  static bool first = true;
  if (first) {
    WiFiClient client;
    HTTPClient http;

    first = false;
    http.begin(client, "http://google.com/robots.txt");
    int httpCode = http.GET();
    Serial.println("Test wifi connection");
    Serial.print("Google responded with code ");
    Serial.print(httpCode);
    http.end();
  }
}
