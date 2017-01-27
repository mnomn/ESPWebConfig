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

class ESPConfig
{
public:
    /* Create ESPConfig object.
       ConfigPassword: Password to login to the config AP.
                       Null for no password.
       ResetPin:       Pin to use to reset config.
                       Longpress to reset and reboot into config mode.
       ParamNames: List of parameters to connfigure in in the web interface.
       NoOfParameters: Number of parameters in ParamNames list
       */
    ESPConfig(char* ConfigPassword, int ResetPin,
              String* ParamNames, int NoOfParameters);
    /* Call fron arduino setup function.
       Will read config. If not configured start web config.
       Return true if system is configured.
       */
    bool setup(ESP8266WebServer& server);
    /* Call this to clear the config. Will nor restart device.
       Call ESP.restart() from main program to restart. */
    void clearConfig();
    /* After config, call this to read parameter values. All values are char*
       null if not found. The fumnction is quite slow, so only call it once.
       */
    char* getParameter(const char *name);

private:
    byte _eepromData[512];
    int _noOfParameters = 0;
    String* _paramNames;
    char* _configPassword;

    void _setupConfig(ESP8266WebServer& server);
    byte _nameToId(const char* name);
    bool _readConfig();
};
#endif // ESPWEBCONFIG_H
