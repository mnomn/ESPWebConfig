#ifndef HTTPCONFIGHANDLER_H
#define HTTPCONFIGHANDLER_H

#include <ESP8266WebServer.h>
#include <EEPROM.h>

// EEprom defines
#define CONFIG_VALID 0x1b
#define STRING_END 0
/*
Eeprom format: CONFIG_VALID, id1, chars, 0, id2, 0, ... 
CONFIG_VALID,1,'v','a','l','1',0,2,'v','a','l','2',0
*/
class HttpConfigHandler : public RequestHandler {
  public:
    HttpConfigHandler(const char* uri = "config", String* ParamNames = {},
    int NoOfParameters = 0)
    : _uri(uri)
    {
      paramNames = ParamNames;
      for (int i = 0; i<NoOfParameters; i++) {
        Serial.print("Param ");
        Serial.println(paramNames[i]);
      }
      noOfParams = NoOfParameters;
      Serial.println(uri);
    }

    bool canHandle(HTTPMethod method, String uri) {
      Serial.print(uri);
      Serial.print(method);
      Serial.println(" Can handle true!");
      return true;
    }

    bool handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) override { 
      Serial.println("Handerling");
      if (requestUri != _uri) {
        return false;
      }
      if (requestMethod == HTTP_GET) {
        String out = "<body><h1>Configure device</h1><form action='/' method='post'>";
        for (int i = 0; i<noOfParams; i++) {
          out += "<div><label for=pwd>";
          out += paramNames[i];
          out += ": </label><input type=text name='";
          out += i;
          out += "' /></div>";
        }
        out += "<div><input type=submit /></div></form></body>";
        server.send(200, "text/html", out);
      } else if (requestMethod == HTTP_POST) {
        char argName[8];
        int address = 0;
        const char* c;
        EEPROM.write(address, CONFIG_VALID);
        address++;
       	for ( uint8_t i = 0; i < noOfParams; i++ ) {
          if (!itoa(i, argName, 10)) {
              break;
          }
          String val = server.arg(argName);
          c = val.c_str();
          EEPROM.write(address, (i+1));
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
        server.send(200, "text/html", "<html><body><h1>Config done. Restarting.</h1></body></html>");
        ESP.restart();
      } else {
        return false;
      }
      return true;
    }

  protected:
    String _uri;
    String* paramNames;
    int noOfParams;
};

#endif