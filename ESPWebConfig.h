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
Create an ESPConfig object. Set Access Point password, reset pin and
a list of parameters. 

For example
const int noOfParams 2
String parameters[noOfParams] = {String("SSID"), String("Password")};
ESPConfig espConfig("configpass", -1, parameters, noOfParams);

*/


#include <Arduino.h>
#include <ESP8266WebServer.h>

class ESPWebConfig
{
public:
    /* Create ESPConfig object.
       resetPin:       Pin to use to reset config. Typically connect a button between
                       resetPin in and gnd. Longpress 5 seconds to reset and reboot into
                       config mode. Device will reboot when button is released. that way you
                       can use GPIO 0 as button without device going to flash mode after reboot.
                       The library set resetPin to input with internal pull up.
       configPassword: Password to login to the config AP.
                       Null for no password.
       paramNames:     List of parameters to connfigure in in the web interface.
       noOfParameters: Number of parameters in ParamNames list, null for no
                       custom parameters (Wifi parameters will always be shown).
       */
    ESPWebConfig( int resetPin, const char* configPassword = NULL,
                 String* paramNames = {}, int noOfParameters = 0);
    /* Call from arduino setup function.
       Will read config. If not configured start web config.
       Return true if system is configured.
       */
    bool setup(ESP8266WebServer& server);
    /* Call this to clear the config. Will not restart device.
       Call ESP.restart() from main program to restart. */
    void clearConfig();
    /* After config, call this to read parameter values.
       name: Use same string as in constructir.
       return: parameter value as char*, or null if not found.
       */
    char* getParameter(const char *name);

    /* Call in arduino loop function.
       Device will clear config and restart if reset pin long pressed.
       */
    void checkReset();

private:
    byte _eepromData[512];
    int _noOfParameters;
    const String* _paramNames;
    const char* _configPassword;
    int _resetPin;
    unsigned long _resetTime = 0;

    // Private because users do not know mumerical id
    char* _getParameterById(const int id);
    void _setupConfig(ESP8266WebServer& server);
    byte _nameToId(const char* name);
    bool _readConfig();
};
#endif // ESPWEBCONFIG_H
