#include <ESPWebConfig.h>
#include <ESP8266WiFi.h>
#include <detail/HttpConfigHandler.h>

#define LONGPRESS_MS 5000

#define DEBUG_PRINT 0
ESPWebConfig::ESPWebConfig(int resetPin, const char* configPassword,
                           String* paramNames, int noOfParameters) {
  _configPassword = configPassword;
  _paramNames = paramNames;
  _noOfParameters = noOfParameters;
  _resetPin = resetPin;
}

bool ESPWebConfig::setup(ESP8266WebServer& server) {
  if (_resetPin >= 0) {
    pinMode(_resetPin,INPUT_PULLUP);
  }
  if (this->_readConfig()) {
    WiFi.mode(WIFI_STA);
    char* ssid = this->_getParameterById(SSID_ID);
    char* pass = this->_getParameterById(PASS_ID);
#if DEBUG_PRINT
    Serial.print("Setup: ");
    Serial.print(ssid?ssid:"NULL");
    Serial.println(pass?pass:"NULL");
#endif
    WiFi.begin (ssid, pass);

    if(WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("WiFi Connect Failed! ...");
    }
    return true;
  }
  this->_setupConfig(server);
  return false;
}

char* ESPWebConfig::getParameter(const char *name) {
#if DEBUG_PRINT
  Serial.print("getParameter for ");
  Serial.print(name);
#endif
  byte id = this->_nameToId(name);
#if DEBUG_PRINT
  Serial.print(id);
#endif
  return this->_getParameterById(id);
}

void ESPWebConfig::clearConfig() {
  Serial.println("Clear config.");
  EEPROM.write(0, 0);
  EEPROM.commit();
}

void ESPWebConfig::checkReset() {
  if (this->_resetPin < 0) {
    return;
  }
  int val = digitalRead(this->_resetPin);
  if (val == HIGH) {
    if (this->_resetTime > 0) {
      // Low -> High. Buttton released.
      // Calculate how long it was pressed.
      unsigned long now = millis();
      if (now - this->_resetTime  > LONGPRESS_MS) {
        this->clearConfig();
        ESP.restart();
      }
      this->_resetTime = 0;
    }
  } else {
    // Button pressed
    if (this->_resetTime == 0) {
      Serial.println("Reset button pressed");
      this->_resetTime = millis();
    }
  }
}


///////// Private functions ///////////////////

/* Set up Access point and handler for set up */
void ESPWebConfig::_setupConfig(ESP8266WebServer& server) {
  WiFi.mode(WIFI_AP);
  char ap_name[32];
  char* tmp;
  tmp = ap_name + 4;
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
  server.addHandler(new HttpConfigHandler("/", _paramNames, _noOfParameters));
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

/* Read all config strings to memory, return true if valid config */
bool ESPWebConfig::_readConfig()
{
  EEPROM.begin(512);
  _eepromData[0] = EEPROM.read(0);
  if (_eepromData[0] != CONFIG_VALID) {
    return false;
  }

  int eeprom_address = 1;
  // Read all eeprom into memory
  while (eeprom_address < 512) {
    _eepromData[eeprom_address] = EEPROM.read(eeprom_address);
    eeprom_address++;
#if DEBUG_PRINT
    Serial.print(_eepromData[eeprom_address]);
    Serial.print(" ");
    if (eeprom_address%32 == 0) {
      Serial.println();
    }
#endif
  }
  // Hard code end marker
  _eepromData[511] = 0;

  return true;
}

char* ESPWebConfig::_getParameterById(const int id) {
  if (id <= 0) {
    return 0;
  }
  byte* tmp = _eepromData;
  byte* end = tmp + 511;
  while(*tmp != id) {
    tmp++;
    if (tmp == end) {
      return 0;
    }
  }
  // Found index, skip one top point to string
  tmp++;
#if DEBUG_PRINT
  Serial.print("param found at ");
  Serial.println((int)(tmp - _eepromData));
#endif
  return (char*)tmp;
}
