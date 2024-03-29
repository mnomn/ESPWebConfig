#include <ESP8266WiFi.h>
#include <ESPWebConfig.h>

/*
 Starter project for Arduino/ESP8266, ESPWebConfig demo.

 First boot: Device will be an access point named ESP_<ipnumber>, which you access
 to configure normal WIFI and other parameters. User opens <ipnumber> in browser.
 Enter Wifi name and password. Also configure user defined parameters.

 After config and restart:
 Lib will handle wifi login. The device will get new IP. In this example,
 a server is set up and the custom parameters will be printed when browsing to new ip number.
*/

/* Strings are both used to display UI and find value after config is done.
 * Names ending with * will be mandatoy
 * Input types:
 * Specify type by appendng "|type", which will not be displayed
 * Input type can be anything the html defenition considers a type.
 * Default input type is text. Not all types are suitable, like radio and select/option for instance.
 */
const char* GREETING_KEY = "Greeting*";
const char* NAME_KEY = "Name";
const char* DATE_KEY = "Date*|date";
const char* CHECK_KEY = "Check|checkbox";

// Convenient pointers to parameter values, so they only need to be read once.
char* greeting;
char* name;
char* date;
char* checked;

// TODO: can server and begin moove into ESPConfig
ESP8266WebServer server(80);
String parameters[] = {GREETING_KEY, NAME_KEY, DATE_KEY, CHECK_KEY};
ESPWebConfig espConfig("configpass", parameters, 4);

void handleRoot() {
  String out = F("<html><body><h1>HelloESPWebConfig</h1>");
    out += "<h3>";
    out += greeting;
    out += ", nice to meet you ";
    out += name;
    out += "<br>Date: ";
    out += date?date:"NULL";
    out += ", Checked: ";
    out += checked?checked:"NULL";
    out += ".</h3></body></html>";
  server.send(200, "text/html", out);
}

void setup() {
  Serial.begin(74880);
  while(!Serial) {
    delay(1);
  }
  Serial.println(F("Starting ..."));

  if (!espConfig.setup()) {
    Serial.println(F("Failed to connect to Wifi."));
  }

  Serial.print(F("Booting: "));
  Serial.println(WiFi.localIP());

  // Get config parameters and print them
  greeting = espConfig.getParameter(GREETING_KEY);
  name = espConfig.getParameter(NAME_KEY);
  date = espConfig.getParameter(DATE_KEY);
  checked = espConfig.getParameter(CHECK_KEY);

  Serial.print(GREETING_KEY);
  Serial.println(greeting?greeting:"NULL");
  Serial.print(NAME_KEY);
  Serial.println(name?name:"NULL");
  Serial.print(DATE_KEY);
  Serial.println(date?date:"NULL");
  Serial.print(CHECK_KEY);
  Serial.println(checked?checked:"NULL");

  // Try to get unknown parameter, for testing purpose.
  char* unk = espConfig.getParameter("unk");
  Serial.print("Undefined: ");
  Serial.println(unk?unk:"NULL");

  // Configure
  server.on ("/configreset", HTTP_POST, handleConfigReset);
  server.on ("/restart", HTTP_POST, handleRestart);
  server.on ("/", handleRoot);

  server.begin();
}

void loop() {
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
