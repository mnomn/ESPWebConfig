#ifndef ESPWEBCONFIG_H
#define ESPWEBCONFIG_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include "detail/ParamStore.h"

class ESPWebConfig
{
public:
   ESPWebConfig();
   ESPWebConfig(const char *configPassword, const char *helpText, std::vector<const char*> parameters);

   /* Call from arduino setup function.
      Read configuration parameters and connects to wifi.
      If no configuration is found, the function blocks and starts the web configuration.

      Return: true if wifi is connected.
             false if wifi not connected.
      */
   bool setup();

   /***** Optional methods *****/

   /* Start config mode without erasing old configuration, for example when a button is pressed.
      After config the device will restart.
      If the device is restarted without submitting the web form, it will keep its old values.

      timeout: How long to be in config mode. After time out the device will restart and resume normal mode. 0 = no timeout.*/
   void startConfig(unsigned long timeoutMs = 0);

   /* Clear the config. Next time the device starts, it will be unconfigured and go into config mode.*/
   void clearConfig();

   /* After config, call this to read parameter values.
      name: Use same string as in constructor.
      return: parameter value as const char*, or nullptr if not found. */
   const char* getParameter(const char *name);

private:
   const char *m_configPassword;
   const char *m_helpText;
   std::vector<const char*> m_parameters;
   ParamStore m_paramStore;
   std::unique_ptr<ESP8266WebServer> m_ewc_server;
   byte nameToId(const char *name);
   bool startWifi();
   // Blocking until user action or timeout. Device will restart.
   void startWebConfiguration(unsigned long timeoutMs = 0);
   void handleServe();
   void handleGetParameters();
   void handlePostParameters();
   const char* getParameterHelper(int index, bool getValue);
};
#endif // ESPWEBCONFIG_H
