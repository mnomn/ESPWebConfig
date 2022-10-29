#include <ESP8266WiFi.h>
#include <ESPWebConfig.h>
#include <ESP8266HTTPClient.h>

/*
 Configure Wifi after boot. No need to "hard code" SSID and password in code.

 First boot:
 The device will set up an access point called ESP_192.161.4.1 (or something simliar).
 Connect a phonw or laptop to that wifi network.
 Browae to 192.161.4.1 and fill in the wifi and password you want to use in the future.

 Next time the device will connect to your configured network. 
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

  Serial.print("Configuration done!");
  Serial.println(WiFi.localIP());

  /* Configure a reset pin. Connect resetPin to ground to clear config */
  pinMode(resetPin, INPUT_PULLUP);
}

void loop() {
  delay(100);
  // Restart by pressing a button
  if (digitalRead(resetPin) == LOW) {
    Serial.print("Start configuration");
    espConfig.startConfig();
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
