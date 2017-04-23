#ifndef HTTPCONFIGHANDLER_H
#define HTTPCONFIGHANDLER_H

#include <ESP8266WebServer.h>
#include <EEPROM.h>

// EEprom defines
#define CONFIG_VALID 0x1a
#define STRING_END 0
#define SSID_ID 1
#define PASS_ID 2
#define NO_OF_INTERNAL_PARAMS 2
#define USER_PARAM_ID 3

#define DEBUG_PRINT 0

/*
Eeprom format: CONFIG_VALID, id1, chars, 0, id2, 0, ... 
CONFIG_VALID,1,'v','a','l','1',0,2,'v','a','l','2',0
id 1 and 2 is for wifi config. id 3 and higher is for user defined parameters.
*/
class HttpConfigHandler : public RequestHandler {
  public:
    HttpConfigHandler(const char* uri = "config", const String* ParamNames = {},
    int NoOfParameters = 0, char* HelpText = NULL)
    : _uri(uri)
    {
      paramNames = ParamNames;
#if DEBUG_PRINT
      for (int i = 0; i<NoOfParameters; i++) {
        Serial.print("Param ");
        Serial.println(paramNames[i]);
      }
#endif
      noOfParams = NoOfParameters;
      helpText = HelpText;
      Serial.println(uri);
    }

    bool canHandle(HTTPMethod method, String uri) {
      return true;
    }

    bool handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) override { 
      if (requestUri != _uri) {
        return false;
      }
      if (requestMethod == HTTP_GET) {
        String label;
        String inputtype;
        String out =
        F("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
        "<style type=text/css>"
        "body { margin:5%; font-family: Arial;} form p label {display:block;float:left;width:100px;}"
        "</style></head>"
        "<body><h1>Configure me!</h1><form action=\"/\" method=\"post\">"
        "<h3>Wifi configuration</h3>"
        "<p><label>SSID* </label><input name=1 type=\"text\" required/></p>"
        "<p><label>Password* </label><input name=2 type=\"password\" required/></p>\n");

        if (noOfParams) {
          out += F("<h3>Parameters</h3>");
          if (helpText) {
            out += "<p>";
            out += helpText;
            out += "</p>";
          }
          for (int i = 0; i<noOfParams; i++) {
            int ix = paramNames[i].indexOf('|');
            if ( ix > 0) {
              label = paramNames[i].substring(0, ix);
              inputtype = paramNames[i].substring(ix+1);
              inputtype.trim();
            } else {
              label = paramNames[i];
            }

            out += "<p><label>";
            out += label;
            out +=  " </label><input ";
            if (inputtype && inputtype.length() > 0) {
              out += "type=\"";
              out += inputtype;
              out += "\" ";
            }
            // The name in is a number
            out += "name=\"";
            out += String(i + USER_PARAM_ID);

            out += "\" ";
            if (label.endsWith("*")) {
              out += "required ";
            }
            out += "/></p>\n";
          }
        }
        out += F("<p><input type=\"submit\" /></p></form><br></body></html>");
        server.send(200, "text/html", out);
      } else if (requestMethod == HTTP_POST) {
        char argName[8];
        int address = 0;
        const char* c;
        EEPROM.write(address, CONFIG_VALID);
        address++;
        for (int i = 1; i <= (noOfParams + NO_OF_INTERNAL_PARAMS); i++) {
          if (!itoa(i, argName, 10)) {
              break;
          }
          String val = server.arg(argName);
          c = val.c_str();
#if DEBUG_PRINT
          Serial.print(" ARG ");
          Serial.print(i);
          Serial.println(c);
#endif
          EEPROM.write(address, i);
          address++;

          while(*c) {
            EEPROM.write(address, *c);
            address++;
            c++;
          }
          EEPROM.write(address, 0);
          address++;
	    }
        EEPROM.commit();
        server.send(200, "text/html", F("<html><body><h1>Configuration done. Restarting.</h1></body></html>"));
        ESP.restart();
      } else {
        return false;
      }
      return true;
    }

  protected:
    String _uri;
    const String* paramNames;
    int noOfParams;
    char* helpText;
};

#endif