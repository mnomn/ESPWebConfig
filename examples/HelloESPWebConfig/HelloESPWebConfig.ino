#include <ESP8266WiFi.h>
#include <ESPWebConfig.h>

/*
 Starter procect for ESP8266, ESPWebConfig demo.
 First boot or after longpress: enter access point mode to allow config,

 After config and restart:
 Log in to wifi from defined in config.
 Serve / and show param set in config

 (Long press reset not implemented)
*/

const char* SSID_KEY = "SSID";
const char* PWD_KEY = "Password";
const char* MESS_KEY = "Hello message";
char* mess;

// TODO: can server and begin moove into ESPConfig
ESP8266WebServer server(80);
String parameters[] = {SSID_KEY, PWD_KEY, MESS_KEY};

ESPConfig espConfig("configpass", -1, parameters, 3);

void configReset() {
  // TODO: Some key/pwd to reset?
  // Probably a bad idea to reset with API. Better to use hardware button.
  // Only handles POST to make accidental reset less likely.
  Serial.print("Clear config");
  espConfig.clearConfig();
  ESP.reset();
}

void handleRoot() {
  String out = "<html><body><h1>Hello " + String(mess) + "</h1></body></html>";
  server.send(200, "text/html", out);
}

void setup() {
  Serial.begin(115200);
  while(!Serial) {
    delay(1);
  }
  Serial.println();
  Serial.print("Starting ...");

  if (espConfig.setup(server)) {
    char* ssid;
    char* pwd;

    Serial.println();
    Serial.println("Normal boot");

    ssid = espConfig.getParameter(SSID_KEY);
    pwd = espConfig.getParameter(PWD_KEY);
    mess = espConfig.getParameter(MESS_KEY);
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("PWD: ");
    Serial.println(pwd);
    Serial.print("Mess: ");
    Serial.println(mess);

    /* Try to get unknown parameter, for testing purpose.*/
    char* unk = espConfig.getParameter("unk");
    Serial.print("UNK: ");
    Serial.println(unk?unk:"NULL");

    WiFi.mode(WIFI_STA);
  	WiFi.begin (ssid, pwd);

    if(WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("WiFi Connect Failed! ...");
    }
    Serial.println(WiFi.localIP());

  	server.on ("/configreset", HTTP_POST, configReset);
  	server.on ("/", handleRoot);
  } else {
    Serial.println("config mode");
 }

  server.begin();
}

void loop() {
	server.handleClient();
}
