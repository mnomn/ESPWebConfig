#ifndef ESPWEBCONFIG_H
#define ESPWEBCONFIG_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include "detail/ParamStore.h"

class ESPWebConfig
{
public:
   /* Create ESPConfig object.
      Always configures Wifi access point and password.
      Can also configure extra parameters and display a help text.
      */
   ESPWebConfig(const char *configPassword = nullptr, const char *helpText = nullptr, std::vector<const char*> parameters = {});

   /* Call from arduino setup function.
      Read configuration parameters and connect to wifi.
      If no configuration is found, the function blocks and starts the web configuration.

      Return true if wifi is connected.
             false if wifi not connected.
      */
   bool setup();

   /***** Optional methods *****/

   /* Start config mode without erasing old configuration.
      After config the device will restart.
      If the device is restarted without submitting the web form, it weill keep its old values.

      timeout: How long to be in config mode. After time out the device will restart and resume normal mode. */
   void startConfig(unsigned long timeoutMs = 0);

   /* Clear the config. After restart the device will be unconfigured and go into config mode. */
   void clearConfig();

   /* After config, call this to read parameter values.
      name: Use same string as in constructur.
      return: parameter value as char*, or null if not found. */
   char *getParameter(const char *name);

private:
   const char *m_configPassword;
   // static const char *s_helpText;
   const char *m_helpText;
   std::vector<const char*> m_parameters;
   bool configCleard = false;

   // Private because users do not know mumerical id
   byte nameToId(const char *name);
   bool startWifi();
   bool startWebConfiguration(unsigned long timeoutMs = 0);
   ParamStore m_paramStore;
   void handleServe();
   void handleGetParameters();
   void handlePutParameters();

   std::unique_ptr<ESP8266WebServer> m_ewc_server;
};
#endif // ESPWEBCONFIG_H
