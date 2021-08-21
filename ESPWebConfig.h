#ifndef ESPWEBCONFIG_H
#define ESPWEBCONFIG_H

/**

Overview
--------
Configure your ESP project over web (local network)
At first boot it will create an access pont named "ESP_<ipnumber>"
Connect to the AP, go to <ipnumber> and fill in your variables.
When you click "Submit, the device will reboot and go to normal execution."

Preparation
-----------
Create an ESPConfig object. Optionally, set Access Point password, a list of extra parameters.

For example
const int noOfParams 2
String parameters[noOfParams] = {String("SSID"), String("Password")};
ESPConfig espConfig("configpass", parameters, noOfParams);

*/


#include <Arduino.h>
#include <ESP8266WebServer.h>
#include "detail/ParamStore.h"

class ESPWebConfig
{
public:
    /* Create ESPConfig object. Always configures Wifi access point and password. Can also configure extra parameters.
       paramNames:     List of extra parameters parameters to show in to configure in in the web interface
       paramNamesLength: Number of parameters in ParamNames list
       */
    ESPWebConfig(const char* configPassword = NULL, String* paramNames = {}, int paramNamesLength = 0, char* helpText = NULL);

    /* Call from arduino setup function.
       Read configuration parameters and connect to wifi.
       If no configuration is found, the function blocks and starts the web configuration.

       Return true if wifi is connected.
              false if wifi not connected.
       */
    bool setup();

    /***** Optional parameters *****/

    /* Start config mode. Go directly to config mode, do not erase old configuration.
       It is possible to restart without configuring, and device will keep its old configuration.
       After config the device will restart.

       timeout: How long to be in config mode. After time out device will restart and resume normal mode. */
    void startConfig(unsigned long timeoutMs = 0);

    /* Clear the config. After restart the device will be unconfigured and go into config mode. */
    void clearConfig();

    /* After config, call this to read parameter values.
       name: Use same string as in constructur.
       return: parameter value as char*, or null if not found. */
    char* getParameter(const char *name);

    /* Set/get a byte based on eeprom address (0 - 511).
       Used for runtime configuration after web config.
       Web config starts at 0, so get/setRaw shall typically use address
       511, 510, 509 etc. */
    byte getRaw(unsigned int address);
    void setRaw(unsigned int address, byte val);

private:
    const char* _configPassword;
    bool _configCleard = false;

    // Private because users do not know mumerical id
    byte _nameToId(const char* name);
    bool _readConfig();
    bool _startWifi();
    bool _startWebConfiguration(unsigned long timeoutMs = 0);
    static char* _helpText;
    static ParamStore _paramStore;
    static const String* _paramNames;
    static int _paramNamesLength;
    static boolean _configurationDone;
    static void _handleServe();
    static void _handleSave();
    static void _generateInputField(const char *legend, int id, char *html, int len);
};
#endif // ESPWEBCONFIG_H
