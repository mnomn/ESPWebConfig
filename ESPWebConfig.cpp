#include <ESPWebConfig.h>
#include <ESP8266WiFi.h>
#include <detail/HttpConfigHandler.h>

ESPWebConfig::ESPWebConfig(const char* configPassword, String* paramNames, int noOfParameters) {
  _configPassword = configPassword;
  _paramNames = paramNames;
  _noOfParameters = noOfParameters;
}

bool ESPWebConfig::setup(unsigned configTimeIfNoWifi) {
  bool cfgRead = false;

  cfgRead = _paramStore.ReadConfig();
  if (cfgRead) {
    if (this->_startWifi()) {
      return true;
    }
  } else {
    // Not configured... long time config!
    configTimeIfNoWifi = 24*60*60;
  }

  if (configTimeIfNoWifi == 0) {
    // Configured, but no wifi and no configTimeIfNoWifi setting
    return false;
  }

  // Configure device
  ESP8266WebServer server(80);
  this->_setupConfig(server);
  Serial.println("Enter config mode.");
  // Serve the config page at "/" until config done.
  unsigned long startCfg = millis();
  while(!HttpConfigHandler::ConfigurationDone) {
    server.handleClient();
    if (!HttpConfigHandler::ConfigurationStarted &&
        millis() > startCfg + configTimeIfNoWifi * 1000) {
      // If config not started within _configIfNoWifi, give up.
      // _configIfNoWifi can be quite small, for power saving reasons
      break;
    }
  }

  // One more try to read config and start wifi.
  if (_paramStore.ReadConfig() && this->_startWifi()) {
    return true;
  }
  return false;
}

void ESPWebConfig::setHelpText(char* helpText) {
  _helpText = helpText;
}

char* ESPWebConfig::getParameter(const char *name) {
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

void ESPWebConfig::clearConfig() {
  Serial.println("Clear config.");
  EEPROM.write(0, CONFIG_ERASED);
  EEPROM.commit();
}


///////// Private functions ///////////////////

/* Set up Access point and handler for set up */
void ESPWebConfig::_setupConfig(ESP8266WebServer& server) {
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

  server.addHandler(new HttpConfigHandler("/", _paramNames, _noOfParameters,
                                          _helpText, &_paramStore));
  server.begin();
}

/* Find id of variablename. 1, 2, ..., return 0 on failure. */
byte ESPWebConfig::_nameToId(const char* name) {
  if (!name || !_paramNames) {
    return 0;
  }
  int i = 0;
  while(_paramNames[i]) {
    if (strcmp(_paramNames[i].c_str(), name) == 0) {
      return i+USER_PARAM_ID;
    }
    i++;
  }
  return 0;
}

bool ESPWebConfig::_startWifi() {
  WiFi.mode(WIFI_STA);
  char* ssid = _paramStore.GetParameterById(SSID_ID);
  char* pass = _paramStore.GetParameterById(PASS_ID);
#if DEBUG_PRINT
  Serial.print("Setup: ");
  Serial.print(ssid?ssid:"NULL");
  Serial.println(pass?pass:"NULL");
#endif
  WiFi.begin (ssid, pass);

  if(WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Connect Failed! ...");
    return false;
  }
  return true;
}
