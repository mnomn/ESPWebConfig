#include <ESPWebConfig.h>
#include <ESP8266WiFi.h>

char* ESPWebConfig::_helpText = NULL;
ParamStore ESPWebConfig::_paramStore;
const String* ESPWebConfig::_paramNames = NULL;
int ESPWebConfig::_paramNamesLength = 0;
boolean ESPWebConfig::_configurationDone = false;

// TODO: Only create server when web config starts.
ESP8266WebServer* ewc_server;

// EEprom defines
#define STRING_END 0
#define SSID_ID 1
#define PASS_ID 2
#define NO_OF_INTERNAL_PARAMS 2
#define USER_PARAM_ID 3

#define UNUSED(expr) do { (void)(expr); } while (0)

ESPWebConfig::ESPWebConfig(const char* configPassword, String* paramNames, int paramNamesLength, char* helpText) {
  _configPassword = configPassword;
  _paramNames = paramNames;
  _paramNamesLength = paramNamesLength;
  _helpText = helpText;
}

bool ESPWebConfig::setup() {
  bool cfgRead = false;

  ESPWC_PRINTLN("Setup");

  cfgRead = _paramStore.ReadConfig();
  if (cfgRead) {
    ESPWC_PRINTLN("Start wifi");
    if (this->_startWifi()) {
      ESPWC_PRINTLN("Wifi started");
      return true;
    }
  } else {
    ESPWC_PRINTLN("Not Configured");
  }

  // Configure device
  this->_startWebConfiguration();

  // One more try to read config and start wifi.
  if (_paramStore.ReadConfig() && this->_startWifi()) {
    return true;
  }
  return false;
}

char* ESPWebConfig::getParameter(const char *name) {
  ESPWC_PRINT("getParameter ");
  ESPWC_PRINTLN(name);
  byte id = this->_nameToId(name);

  return _paramStore.GetParameterById(id);
}

byte ESPWebConfig::getRaw(unsigned int address) {
  return _paramStore._eepromData[address];
}

void ESPWebConfig::setRaw(unsigned int address, byte val) {
  _paramStore._eepromData[address] = val;
  EEPROM.write(address, val);
  EEPROM.commit();
}

void ESPWebConfig::startConfig(unsigned long timeoutMs) {
  ESPWC_PRINTLN("Start config.");
  this->_startWebConfiguration(timeoutMs);
}

void ESPWebConfig::clearConfig() {
  if (_configCleard) return;
  ESPWC_PRINTLN("Clear config.");
  EEPROM.write(0, CONFIG_ERASED);
  EEPROM.commit();
  _configCleard = true;
}


///////// Private functions ///////////////////

/* Find id of variablename. 1, 2, ..., return 0 on failure. */
byte ESPWebConfig::_nameToId(const char* name) {
  if (!name || !_paramNames) {
    ESPWC_PRINTLN("_nameToId invalid");
    return 0;
  }

  for (int i = 0; i<_paramNamesLength; i++) {
    ESPWC_PRINT("Compare parameter ");
    ESPWC_PRINTLN(_paramNames[i]);
    if (strcmp(_paramNames[i].c_str(), name) == 0) {
      return i+USER_PARAM_ID;
    }
  }
  ESPWC_PRINT("No parameter with name ");
  ESPWC_PRINTLN(name);
  return 0;
}

bool ESPWebConfig::_startWifi() {
  WiFi.mode(WIFI_STA);
  char* ssid = _paramStore.GetParameterById(SSID_ID);
  char* pass = _paramStore.GetParameterById(PASS_ID);
  ESPWC_PRINT("_startWifi: ");
  ESPWC_PRINT(ssid?ssid:"NULL");
  ESPWC_PRINT(" ");
  ESPWC_PRINTLN(pass?pass:"NULL");
  WiFi.begin(ssid, pass);

  if(WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Connect Failed! ...");
    return false;
  }

  ESPWC_PRINTLN("_startWifi: OK!!!!");
  return true;
}

void ESPWebConfig::_handleSave() {
  int address = 0;
  const char* c;
  EEPROM.write(address, CONFIG_VALID);
  address++;
  for (int i = 1; i <= (_paramNamesLength + NO_OF_INTERNAL_PARAMS); i++) {

    String val = ewc_server->arg(String(i));

    c = val.c_str();

    EEPROM.write(address, i);
    address++;

    while(*c) {
      EEPROM.write(address, *c);
      address++;
      c++;
    }
    EEPROM.write(address, 0);
    address++;
  }
  EEPROM.commit();
  ewc_server->send(200, "text/html", F("<html><body><h1>Configuration done.</h1></body></html>"));
  ewc_server->close();
  delay(1000);
  _configurationDone = true;
}

void ESPWebConfig::_handleServe() {
    char inp[128];

    // Get chipId
    uint32_t chipid = ESP.getChipId();
    char chipStr[32] ={0};
    sprintf(chipStr, "<p>ChipId: %0X</p>", chipid);

    Serial.println("Handle config page");

    ewc_server->setContentLength(CONTENT_LENGTH_UNKNOWN);
    ewc_server->send(200, "text/html", "");
    ewc_server->sendContent("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
    "<style type=text/css>"
    "body { margin:5%; font-family: Arial;} form p label {display:block;float:left;width:100px;}"
    "</style></head>"
    "<body><h1>Configure device</h1><form action=\"/\" method=\"post\">"
    "<h3>Wifi configuration</h3>");

      ESPWebConfig::_generateInputField("SSID*", SSID_ID, inp, 128);
      ewc_server->sendContent(inp);
      ESPWebConfig::_generateInputField("password|Password", PASS_ID, inp, 128);
      ewc_server->sendContent(inp);

      if (_paramNamesLength) {
        sprintf(inp, "<h3>Parameters</h3><p>%s</p>", _helpText?_helpText:"");
        ewc_server->sendContent(inp);
        for (int i = 0; i < _paramNamesLength; i++) {
          ESPWebConfig::_generateInputField(_paramNames[i].c_str(), i + USER_PARAM_ID, inp, 128);
          ewc_server->sendContent(inp);
        }
      }
      ewc_server->sendContent("<p><input type=\"submit\" value=\"Save\"/></p></form><br>");
      // Print chipId, Could be useful in config.
      ewc_server->sendContent(chipStr);
      ewc_server->sendContent("</body></html>");
      ewc_server->sendContent("");
      ewc_server->client().stop();
}

void ESPWebConfig::_generateInputField(const char *legend, int id, char *html, int len)
{
  char *p = html;
  unsigned int left = len-1;// zero term
  int l = 0;
  char *val = NULL;
  if (_paramStore.Restore()) {
    val = _paramStore.GetParameterById(id);
  }
  const char *type = NULL;
  // Check for inpit type
  char *tmp = strchr(legend, '|');
  if (tmp) {
    type = legend;
    legend = tmp+1;
    *tmp = '\0';// Convert | to end of string
  }
  char *req = strchr(legend, '*');

  l = sprintf(p, "<p><label>%s</label><input name=%d", legend, id);
  p += l;
  left -= l;

  if (type && strlen(type) + 8 + 17 < left) {
    l = sprintf(p, " type=\"%s\"", type);
    left -= l;
    p += l;
  }
  if (val && strlen(val) + 9 + 17 < left) {
    l = sprintf(p, " value=\"%s\"", val);
    left -= l;
    p += l;
  }
  // End input tag
  if (req) {
    sprintf(p, " \"required\"/></p>");
  } else {
    sprintf(p, " /></p>");
  }
}


bool ESPWebConfig::_startWebConfiguration(unsigned long timeoutMs) {
  if (WiFi.isConnected()) WiFi.disconnect();

  ewc_server = new ESP8266WebServer(80);

//  ESP8266WebServer server(80);

  WiFi.mode(WIFI_AP);
  char ap_name[32];
  IPAddress myIP = WiFi.softAPIP();
  String ipstr;
  if (myIP) {
    ipstr = myIP.toString();
  } else{
    ipstr = "unknown_ip";
  }
  sprintf(ap_name, "ESP_%s", ipstr.c_str());

  if (_configPassword) {
    WiFi.softAP(ap_name, _configPassword);
  } else {
    WiFi.softAP(ap_name);
  }

  ESPWC_PRINTLN("Add handler /old ");

  ewc_server->on("/", HTTP_GET, this->_handleServe);
  ewc_server->on("/", HTTP_POST, this->_handleSave);

  ewc_server->begin();

  ESPWC_PRINTLN("Enter config mode.");
  // Serve the config page at "/" until config done.
  unsigned long startTime = millis();
  while(!_configurationDone) {
    // ESPWC_PRINTLN("HANDLE config mode.");
    ewc_server->handleClient();
    if (timeoutMs && (millis() - startTime) > timeoutMs) {
      ESPWC_PRINTLN("Config timeout, restart");
      delay(100);
      ESP.restart();
    }
  }

  return true;
}
