#include "ESPWebConfig.h"
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "include/index.h"
#include "detail/ewc_d.h"

// EEprom defines
#define SSID_ID 1
#define PASS_ID 2
#define USER_PARAM_ID 3

ESPWebConfig::ESPWebConfig(const char* configPassword, const char* helpText, std::vector<const char*> parameters):
  m_configPassword(configPassword),
  m_helpText(helpText),
  m_parameters(parameters)
{
}

bool ESPWebConfig::setup()
{
  bool cfgRead = false;

  ESPWC_PRINTLN("Setup");

  cfgRead = m_paramStore.ReadConfig();
  if (cfgRead) {
    if (this->startWifi()) {
      return true;
    }
    return false;
  } else {
    ESPWC_PRINTLN("Not Configured");
  }

  // Configure device
  this->startWebConfiguration();

  // One more try to read config and start wifi.
  if (m_paramStore.ReadConfig() && this->startWifi()) {
    return true;
  }
  return false;
}

char* ESPWebConfig::getParameter(const char *name) {
  ESPWC_PRINT("getParameter ");
  ESPWC_PRINTLN(name);
  byte id = this->nameToId(name);

  return m_paramStore.GetParameterById(id);
}

void ESPWebConfig::startConfig(unsigned long timeoutMs) {
  ESPWC_PRINTLN("Start config.");
  this->startWebConfiguration(timeoutMs);
}

void ESPWebConfig::clearConfig() {
  if (configCleard) return;
  ESPWC_PRINTLN("Clear config.");
  EEPROM.write(0, CONFIG_ERASED);
  EEPROM.commit();
  configCleard = true;
}


///////// Private functions ///////////////////

/* Find id of variablename. 1, 2, ..., return 0 on failure. */
byte ESPWebConfig::nameToId(const char* name) {
  if (!name || m_parameters.size() > 0) {
    ESPWC_PRINTLN("_nameToId invalid");
    return 0;
  }

  int i = USER_PARAM_ID;
  for (auto paramName: m_parameters) {
    ESPWC_PRINT("Compare parameter ");
    ESPWC_PRINTLN(paramName);
    if (strcmp(paramName, name) == 0) {
      return i;
    }
    ++i;
  }
  ESPWC_PRINT("No parameter with name ");
  ESPWC_PRINTLN(name);
  return 0;
}

bool ESPWebConfig::startWifi() {
  WiFi.mode(WIFI_STA);
  char* ssid = m_paramStore.GetParameterById(SSID_ID);
  char* pass = m_paramStore.GetParameterById(PASS_ID);
  ESPWC_PRINT("_startWifi: ");
  ESPWC_PRINT(ssid?ssid:"NULL");
  ESPWC_PRINT(" ");
  ESPWC_PRINTLN(pass?pass:"NULL");
  WiFi.begin(ssid, pass);

  if(WiFi.waitForConnectResult() != WL_CONNECTED) {
    ESPWC_PRINTLN("WiFi Connect Failed! ...");
    return false;
  }

  ESPWC_PRINTLN("_startWifi: OK!!!!");
  return true;
}

void ESPWebConfig::handleServe() {
  m_ewc_server->send(200, "text/html", indexPage);
}

void ESPWebConfig::handleGetParameters() {
static const char formatParam[] PROGMEM = "%c{\"n\":\"%s\",\"v\":\"%s\"}";
constexpr int parameterSize = 256;
char parameterString[parameterSize] = {0};
#if 0
  Json format Format
    {
      "chipId": "ACBC123",
      "helpText": "Some text",
      // Using array for parameters,since it must be ordered.
      // Name (n), value (v). Value is optional. Name may contain type and "mandatory asterix".
      // The first two parameters are always present
      // Parameter 3 and on are userdefines
      "parameters": [
        {"n":"Username*", "v":"Ada"},
        {"n":"Password|password", "v":"s3cr3t"},
        {"n":"Expired|date", "v":""},
        {"n":"Comment"},
      ]
    }
#endif

  m_ewc_server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  m_ewc_server->send(200, "application/json", "{");

  snprintf_P(parameterString, parameterSize, "\"chipId\":\"%X\"", ESP.getChipId());
  m_ewc_server->sendContent(parameterString);

  snprintf_P(parameterString, parameterSize, ",\"helpText\":\"Help text help!\"");
  m_ewc_server->sendContent(parameterString);

  m_ewc_server->sendContent(",\"parameters\":[");

  // // First write two internal parameters
  int paramIndex = 1;
  snprintf_P(parameterString, parameterSize, formatParam, ' ', "Wifi*", m_paramStore.GetParameterById(paramIndex));
  m_ewc_server->sendContent(parameterString);
  paramIndex++;
  snprintf_P(parameterString, parameterSize, formatParam, ',', "Password*|password", m_paramStore.GetParameterById(paramIndex));
  m_ewc_server->sendContent(parameterString);


  for (auto& pname: m_parameters) {
    paramIndex++;
    auto pval = m_paramStore.GetParameterById(paramIndex);
    auto written = snprintf_P(parameterString, parameterSize, formatParam,' ',
      pname, pval?pval:"-");

    if (written >= parameterSize) {
      // Stringtruncated. Rewrite without value. Should work, paramnames cannot be more than X char.
      snprintf_P(parameterString, parameterSize, formatParam, ' ', pname, "");
    }

    m_ewc_server->sendContent(parameterString);
  }
  m_ewc_server->sendContent("]}");
}

// Save parameters. Use form data.
void ESPWebConfig::handlePutParameters() {
  int address = 0;
  int paramId = 1;

  EEPROM.write(address, CONFIG_VALID);
  address++;

  while (m_ewc_server->hasArg(String(paramId))) {
    auto val = m_ewc_server->arg(String(paramId));
    auto val_c = val.c_str();
    EEPROM.write(address, paramId);
    address++;

    while(*val_c) {
      EEPROM.write(address, *val_c);
      address++;
      val_c++;
    }
    EEPROM.write(address, 0);
    address++;

    paramId++;
  }

  EEPROM.commit();
  m_ewc_server->send(200, "text/html", F("<html><body><h1>Configuration done.</h1></body></html>"));
  m_ewc_server->close();
  delay(500);
  ESP.restart();
}


bool ESPWebConfig::startWebConfiguration(unsigned long timeoutMs) {
  // using namespace std::placeholders;  // for _1, _2, _3...
  if (WiFi.isConnected()) WiFi.disconnect();

  m_ewc_server = std::make_unique<ESP8266WebServer>(80);

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

  if (m_configPassword) {
  //  WiFi.softAP(ap_name, m_configPassword);
  } else {
    WiFi.softAP(ap_name);
  }

  m_ewc_server->on("/", HTTP_GET, std::bind(&ESPWebConfig::handleServe, this));
  m_ewc_server->on("/parameters", HTTP_GET, std::bind(&ESPWebConfig::handleGetParameters, this));
  m_ewc_server->on("/parameters", HTTP_PUT, std::bind(&ESPWebConfig::handlePutParameters, this));

  m_ewc_server->begin();

  ESPWC_PRINTLN("Enter config mode.");
  // Serve the config page at "/" until config done.
  unsigned long startTime = millis();
  while(1) {
    m_ewc_server->handleClient();
    if (timeoutMs && (millis() - startTime) > timeoutMs) {
      ESPWC_PRINTLN("Config timeout, restart");
      delay(500);
      ESP.restart();
    }
  }

  return true;
}
