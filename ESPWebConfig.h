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

class ESPWebConfig
{
public:
    /* Create ESPConfig object.
       paramNames:     List of parameters to connfigure in in the web interface.
       noOfParameters: Number of parameters in ParamNames list, null for no
                       custom parameters (Wifi parameters will always be shown).
       */
    ESPWebConfig(const char* configPassword = NULL, String* paramNames = {}, int noOfParameters = 0);

    /* Set text that will help the user understand what to write in the config. */
    void setHelpText(char* helpText);

    /* Call from arduino setup function.
       Read configuration parameter and connect to wifi.
       If no configuration data, function will block, create AP and serve config web.
       Return true if wifi connected.
              false if wifi not connected.
              (No return if no configuration, returns true false after config web filled in.)
       */
    bool setup();

    /* Call this to clear the config. Will not restart device.
       Call ESP.restart() from main program to restart. */
    void clearConfig();

    /* After config, call this to read parameter values.
       name: Use same string as in constructur.
       return: parameter value as char*, or null if not found. */
    char* getParameter(const char *name);

private:
    const char* _configPassword;
    const String* _paramNames;
    int _noOfParameters;

    byte _eepromData[512];
    char* _helpText;

    unsigned long _resetTime = 0;

    // Private because users do not know mumerical id
    char* _getParameterById(const int id);
    void _setupConfig(ESP8266WebServer& server);
    byte _nameToId(const char* name);
    bool _readConfig();
    bool _startWifi();
};
#endif // ESPWEBCONFIG_H
