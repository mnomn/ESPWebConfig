#include "ESPWebConfig.h"
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "include/index.h"
#include "detail/ewc_d.h"

ESPWebConfig::ESPWebConfig(const char* configPassword, const char* helpText, std::vector<const char*> parameters):
  m_configPassword(configPassword),
  m_helpText(helpText),
  m_parameters(parameters)
{
  m_paramStore.Initialize(m_parameters.size());
}

ESPWebConfig::ESPWebConfig():
  m_configPassword(nullptr),
  m_helpText(nullptr),
  m_parameters({})
{
  m_paramStore.Initialize(m_parameters.size());
}

bool ESPWebConfig::setup()
{
  if (m_paramStore.IsConfigured()) {
    ESPWC_PRINT("Setup->Configured\n");
    if (this->startWifi()) {
      return true;
    }
    return false;
  } else {
    ESPWC_PRINT("Setup->Not configured\n");
  }

  // Configure device
  this->startWebConfiguration();

  // After config, values and start wifi.
  if (m_paramStore.IsConfigured() && this->startWifi()) {
    return true;
  }
  return false;
}

const char* ESPWebConfig::getParameter(const char *name) {
  ESPWC_PRINTF("getParameter %s\n", name);
  byte id = this->nameToId(name);

  return m_paramStore.GetValue(id);
}

void ESPWebConfig::startConfig(unsigned long timeoutMs) {
  ESPWC_PRINT("Start config\n");
  this->startWebConfiguration(timeoutMs);
}

void ESPWebConfig::clearConfig() {
  m_paramStore.Clear();
}


///////// Private functions ///////////////////

/* Find id of variablename. 1, 2, ..., return 0 on failure. */
byte ESPWebConfig::nameToId(const char* name) {
  if (!name || m_parameters.size() <= 0) {
    ESPWC_PRINT("nameToId not possible\n");
    return 0;
  }

  int i = ParamStore::internalParamSize + 1;
  for (auto paramName: m_parameters) {
    ESPWC_PRINTF("Compare parameter %s %s\n", name, paramName);
    if (strcmp(paramName, name) == 0) {
      return i;
    }
    ++i;
  }
  ESPWC_PRINTF("No parameter with name %s", name);
  return 0;
}

bool ESPWebConfig::startWifi() {
  WiFi.mode(WIFI_STA);
  auto ssid = m_paramStore.GetValue(ParamStore::ssidIndex);
  auto pass = m_paramStore.GetValue(ParamStore::passwordIndex);
  ESPWC_PRINTF("_startWifi: %s %s\n", ssid?ssid:"NULL", pass?pass:"NULL");
  WiFi.begin(ssid, pass);

  if(WiFi.waitForConnectResult() != WL_CONNECTED) {
    ESPWC_PRINT("WiFi Connect Failed! ...\n");
    return false;
  }

  ESPWC_PRINT("_startWifi: OK!!!!\n");
  return true;
}

void ESPWebConfig::handleServe() {
  m_ewc_server->send(200, F("text/html"), indexPage);
}

// Always return a usable string (value or empty string), never null
const char* ESPWebConfig::getParameterHelper(int index, bool getValue) {
  if (!getValue) return "";

  const char* val = m_paramStore.GetValue(index);

  if (val) return val;

  return "";
}

void ESPWebConfig::handleGetParameters() {
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
        {"n":"Wifi*", "v":"Ada"},
        {"n":"Password|password", "v":"s3cr3t"},
        {"n":"Expired|date", "v":""},
        {"n":"Comment"},
      ]
    }
#endif

  bool getValues = m_ewc_server->hasArg("getValues");
  ESPWC_PRINTF("handlePostParameters getValues: %d\n", getValues);

  static const char formatParam[] PROGMEM = "%c{\"n\":\"%s\",\"v\":\"%s\"}";
  constexpr int parameterSize = 256;
  char parameterString[parameterSize] = {0};

  m_ewc_server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  m_ewc_server->send(200, "application/json", "{");

  snprintf_P(parameterString, parameterSize, "\"chipId\":\"%X\"", ESP.getChipId());
  m_ewc_server->sendContent(parameterString);

  snprintf_P(parameterString, parameterSize, ",\"helpText\":\"%s\"", m_helpText?m_helpText:"");
  m_ewc_server->sendContent(parameterString);

  m_ewc_server->sendContent(",\"parameters\":[");

  // // First write two internal parameters
  int paramIndex = 1;
  snprintf_P(parameterString, parameterSize, formatParam, ' ', "Wifi*", getParameterHelper(paramIndex, getValues));
  m_ewc_server->sendContent(parameterString);

  paramIndex++;
  snprintf_P(parameterString, parameterSize, formatParam, ',', "Password*|password", getParameterHelper(paramIndex, getValues));

  m_ewc_server->sendContent(parameterString);

  for (auto& pname: m_parameters) {
    paramIndex++;

    auto written = snprintf_P(parameterString, parameterSize, formatParam,',', pname,
      getParameterHelper(paramIndex, getValues));

    if (written >= parameterSize) {
      // Stringtruncated. Rewrite without value. Should work, paramnames cannot be more than X char.
      snprintf_P(parameterString, parameterSize, formatParam, ',', pname, "");
    }

    m_ewc_server->sendContent(parameterString);
  }
  m_ewc_server->sendContent("]}");
}

// Save parameters. Use form data.
void ESPWebConfig::handlePostParameters() {
  bool result {true};
  ESPWC_PRINT("handlePostParameters");

  auto th = F("text/html");
  unsigned int noOfParameters = ParamStore::internalParamSize + m_parameters.size();
  for (unsigned int i = 1; i <= noOfParameters; i++) {
    auto paramIdStr = String(i);
    ESPWC_PRINTF(" %s", paramIdStr.c_str());
    if (!m_ewc_server->hasArg(paramIdStr)) {
      ESPWC_PRINT("No such param\n");
      result = false;
      break;
    }
    auto val = m_ewc_server->arg(paramIdStr);
    auto stored = m_paramStore.StoreParameter(i, val);
    if (!stored) {
      result = false;
      break;
    }
  }

  if (result) {
    result = m_paramStore.Commit();
  }

  if (result) {
    m_ewc_server->send(200, th, F("<html><body><h1>Configuration done</h1></body></html>"));
  }
  else {
    m_ewc_server->send(200, th, F("<html><body><h1>Configuration failed</h1></body></html>"));
  }

  ESPWC_PRINTF("handlePostParameters result: %d", result);

  m_ewc_server->close();
  delay(500);
  ESP.restart();
}


void ESPWebConfig::startWebConfiguration(unsigned long timeoutMs) {
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
  m_ewc_server->on("/", HTTP_POST, std::bind(&ESPWebConfig::handlePostParameters, this));

  m_ewc_server->begin();

  ESPWC_PRINTF("Enter config mode\n");
  // Serve the config page at "/" until config done.
  unsigned long startTime = millis();
  while(1) {
    m_ewc_server->handleClient();
    if (timeoutMs && (millis() - startTime) > timeoutMs) {
      ESPWC_PRINTF("Config timeout, restart\n");
      break;
    }
  }

  delay(500);
  ESP.restart();
}
