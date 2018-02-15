#ifndef HTTPCONFIGHANDLER_H
#define HTTPCONFIGHANDLER_H

#include <ESP8266WebServer.h>
#include "ParamStore.h"

// EEprom defines
#define STRING_END 0
#define SSID_ID 1
#define PASS_ID 2
#define NO_OF_INTERNAL_PARAMS 2
#define USER_PARAM_ID 3

#define UNUSED(expr) do { (void)(expr); } while (0)

/*
Eeprom format: CONFIG_VALID/ERASED, id1, chars, 0, id2, 0, ...
Example: 0x1a,1,'v','a','l','1',0,2,'v','a','l','2',0
id 1 and 2 is for wifi config. id 3 and higher is for user defined parameters.
*/
class HttpConfigHandler : public RequestHandler {
  public:
    static boolean ConfigurationStarted;
    static boolean ConfigurationDone;
    HttpConfigHandler(const char* uri = "config", const String* ParamNames = {},
    int NoOfParameters = 0, char* HelpText = NULL, ParamStore *ParamStore = NULL)
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
      paramStore = ParamStore;
      Serial.println(uri);
    }

    bool canHandle(HTTPMethod method, String uri) {
      UNUSED(method);
      UNUSED(uri);
      return true;
    }

    char *generateInputField(const char *legend, int id, char *html, int len)
    {
      char *p = html;
      int left = len-1;// zero trem
      int l = 0;
      char *val = NULL;
      if (paramStore->Restore()) {
        val = paramStore->GetParameterById(id);
      }
      const char *type = NULL;
      // Check for inpit type
      char *tmp = strchr(legend, '|');
      if (tmp) {
        type = legend;
        legend = tmp+1;
        *tmp = '\0';// Convert | to end of string
      }
      char *req = strchr(legend, '*');

      l = sprintf(p, "<p><label>%s</label><input name=%d", legend, id);
      p += l;
      left -= l;

      if (type && strlen(type) + 8 + 17 < left) {
        l = sprintf(p, " type=\"%s\"", type);
        left -= l;
        p += l;
      }
      if (val && strlen(val) + 9 + 17 < left) {
        l = sprintf(p, " value=\"%s\"", val);
        left -= l;
        p += l;
      }
      // End input tag
      if (req) {
        // Size of( "required"/></p>) = 17
        sprintf(p, " \"required\"/></p>");
      } else {
        sprintf(p, " /></p>");
      }
    }

    bool handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) override {
//      const char HTML_START[] PROGMEM = ;

//      const char HTML_END[] PROGMEM = "<p><input type=\"submit\" value=\"Save\"/></p></form><br></body></html>";

      if (requestUri != _uri) {
        return false;
      }
      if (requestMethod == HTTP_GET) {
        char inp[128];

        HttpConfigHandler::ConfigurationStarted = true;
        Serial.println("Handle config page");

        server.setContentLength(CONTENT_LENGTH_UNKNOWN);
        server.send(200, "text/html", "");
        server.sendContent("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
        "<style type=text/css>"
        "body { margin:5%; font-family: Arial;} form p label {display:block;float:left;width:100px;}"
        "</style></head>"
        "<body><h1>Configure me!</h1><form action=\"/\" method=\"post\">"
        "<h3>Wifi configuration</h3>");

        generateInputField("SSID*", SSID_ID, inp, 128);
        server.sendContent(inp);
        generateInputField("password|Password", PASS_ID, inp, 128);
        server.sendContent(inp);

        if (noOfParams) {
          sprintf(inp, "<h3>Parameters</h3><p>%s</p>", helpText?helpText:"");
          server.sendContent(inp);
          for (int i = 0; i < noOfParams; i++) {
            generateInputField(paramNames[i].c_str(), i + USER_PARAM_ID, inp, 128);
            server.sendContent(inp);
          }
        }
        server.sendContent("<p><input type=\"submit\" value=\"Save\"/></p></form><br></body></html>");
        server.sendContent("");
        server.client().stop();
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
        server.send(200, "text/html", F("<html><body><h1>Configuration done.</h1></body></html>"));
        HttpConfigHandler::ConfigurationDone = true;
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
    ParamStore *paramStore;
};
boolean HttpConfigHandler::ConfigurationDone = false;
boolean HttpConfigHandler::ConfigurationStarted = false;

#endif
