#include <ESP8266WiFi.h>
#include <ESPWebConfig.h>

/*
 Configure Wifi after boot. No need to "hard code" SSID and password in code.

 First boot (or after reset):
 Device will be an access point named ESP_<ipnumber>, which you access to
 configure normal WIFI and other parameters.
 User opens <ipnumber> in browser and enter Wifi name and password.

 After config and restart the device will get new IP. Now you can write networking code in sketch (WiFiClient, mqtt, etc)
*/

/* Connfigure a pin that will reset config if grounded (button pressed) */
int resetPin = 4;

ESPWebConfig espConfig;

void setup() {
  Serial.begin(74880);
  while(!Serial) {
    delay(1);
  }
  Serial.println("");
  Serial.println("Starting ...");

  if (espConfig.setup()) {
    // Print ip so we do not need to find it in the router.
    Serial.print("Normal boot: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Config mode.");
  }

  /* Configure a reset pin. Connect resetPin to ground to clear config */
  pinMode(resetPin, INPUT_PULLUP);
}

void loop() {
  // Only needed if you do not have a server.
  espConfig.handleClient();

  // Restart by pressing a button
  if (!espConfig.isConfigMode() && digitalRead(resetPin) == LOW) {
    espConfig.clearConfig();
    ESP.restart();
  }
}
