#include <ESPWebConfig.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <detail/HttpConfigHandler.h>

ESPConfig::ESPConfig(char* configPassword, int resetPin,
                     String* paramNames, int noOfParameters) {
  _configPassword = configPassword;
  _paramNames = paramNames;
  _noOfParameters = noOfParameters;
}

bool ESPConfig::setup(ESP8266WebServer& server) {
  Serial.println(_paramNames[1].c_str());

  if (this->_readConfig()) {
    return true;
  }
  this->_setupConfig(server);
  return false;
}

char* ESPConfig::getParameter(const char *name) {
  byte id = this->_nameToId(name);
  if (!id) {
    return 0;
  }
  byte* tmp = _eepromData;
  byte* end = tmp + 512;
  while(*tmp != id) {
    tmp++;
    if (tmp == end) {
      return 0;
    }
  }
  // Found index, skip one top point to string
  tmp++;
  return (char*)tmp;
}

void ESPConfig::clearConfig() {
  EEPROM.write(0, 0);
  EEPROM.commit();
}

/* Set up Access point and handler for set up */
void ESPConfig::_setupConfig(ESP8266WebServer& server) {
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
  Serial.print("AP IP address: ");
  Serial.println(ipstr.c_str());
  sprintf(ap_name, "ESP_%s", ipstr.c_str());

  if (_configPassword) {
    WiFi.softAP(ap_name, _configPassword);
  } else {
    WiFi.softAP(ap_name);
  }
  server.addHandler(new HttpConfigHandler("/", _paramNames, _noOfParameters));
}

/* Find id of variablename. 1, 2, ..., return 0 on failure. */
byte ESPConfig::_nameToId(const char* name) {
  int i = 0;
  while(_paramNames[i]) {
    if (strcmp(_paramNames[i].c_str(), name) == 0) {
      return i+1;
    }
    i++;
  }
  return 0;
}

/* Read all config strings to memory, return true if valid config */
bool ESPConfig::_readConfig()
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
  }
  // Hard code end marker
  _eepromData[511] = 0;

  return true;
}
