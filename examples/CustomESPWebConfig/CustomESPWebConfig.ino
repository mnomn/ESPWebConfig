#include <ESP8266WiFi.h>
#include <ESPWebConfig.h>

/*
 Starter project for Arduino/ESP8266, ESPWebConfig demo.

 First boot (or after longpress): Device will be an access point named
 ESP_<ipnumber>, which you access to configure normal WIFI and other parameters.
 User opens <ipnumber> in browser. Enter Wifi name and password. Also configure
  user defined parameters, in this example "Greeting" and "Name".

 After config and restart:
 Lib will handle wifi login. The device will get new IP. In this example,
 the custom parameters will be printed when browsing to new ip number.

 You arduino sketch must define its own url handlers.

 (Long press reset not implemented)
*/

// Strings are both used to display UI and find value after config is done.
const char* GREETING_KEY = "Greeting";
const char* NAME_KEY = "Name";

// Convenient pointers to parameter values, so they only need to be read once.
char* greeting;
char* name;

// TODO: can server and begin moove into ESPConfig
ESP8266WebServer server(80);
String parameters[] = {GREETING_KEY, NAME_KEY};
int resetPin = -1; // No reset pin configured (not implemented in lib)
ESPWebConfig espConfig(resetPin, "configpass", parameters, 2);

void handleRoot() {
  String out = F("<html><body><h1>HelloESPWebConfig</h1>");
  out = "<h3>" + String(greeting) + ", nice to meet you " + String(name) +
    ".</h3></body></html>";
  server.send(200, "text/html", out);
}

void setup() {
  Serial.begin(115200);
  while(!Serial) {
    delay(1);
  }
  Serial.println(F("Starting ..."));

  if (espConfig.setup(server)) {
    Serial.print(F("Normal boot: "));
    Serial.println(WiFi.localIP());

    // Get config parameters and print them
    greeting = espConfig.getParameter(GREETING_KEY);
    name = espConfig.getParameter(NAME_KEY);
    Serial.print(GREETING_KEY);
    Serial.println(greeting?greeting:"NULL");
    Serial.print(NAME_KEY);
    Serial.println(name?name:"NULL");

    // Try to get unknown parameter, for testing purpose.
    char* unk = espConfig.getParameter("unk");
    Serial.print("Undefined: ");
    Serial.println(unk?unk:"NULL");

    // Configure
    server.on ("/configreset", HTTP_POST, handleConfigReset);
    server.on ("/restart", HTTP_POST, handleRestart);
    server.on ("/", handleRoot);
  } else {
    Serial.println(F("Config mode"));
  }

  server.begin();
}

void loop() {
  espConfig.checkReset();
  server.handleClient();
}

/*
 Nifty debug handlers
 Use  curl -X POST http://192.168.1.45/configreset
 Use  curl -X POST http://192.168.1.45/restart
*/
void handleConfigReset() {
  // For development and debug.
  // Probably a bad idea to reset with URL post. Better to use hardware button.
  // Only handles POST to make accidental reset less likely.
  server.send(200, "text/html", "Config reset!\n");
  Serial.println(F("Clear config"));
  espConfig.clearConfig();
  ESP.restart();
}

void handleRestart() {
  server.send(200, "text/html", F("Restart...\n"));
  Serial.println(F("Clear config"));
  ESP.restart();

}
