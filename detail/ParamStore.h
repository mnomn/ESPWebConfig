#ifndef EWC_PARAMSTORE_H
#define EWC_PARAMSTORE_H

#define CONFIG_VALID 0x1a
#define CONFIG_ERASED 0x1b

#include <EEPROM.h>
#include "espwc.h"

class ParamStore
{
public:
  char* GetParameterById(const int id) {
    ESPWC_PRINT("GetParameterById ");
    ESPWC_PRINTLN(id);
    if (id <= 0) {
      return '\0';
    }
    byte* tmp = _eepromData;
    byte* end = tmp + 511;
    while(*tmp != id) {
      tmp++;
      if (tmp == end) {
        ESPWC_PRINTLN("Param not found");
        return '\0';
      }
    }
    // Found index, skip one to point to string
    tmp++;
    ESPWC_PRINT("Param found:");
    ESPWC_PRINTLN((char*)tmp);
    return (char*)tmp;
  }

  /* Read all config strings to memory, return true if valid config */
  bool ReadConfig()
  {
    EEPROM.begin(512);
    _eepromData[0] = EEPROM.read(0);
  #ifdef ESPWC_DEBUG
    Serial.println("EEPROM 0:");
    Serial.print(_eepromData[0]);
  #endif
    if (_eepromData[0] == CONFIG_VALID ||
        _eepromData[0] == CONFIG_ERASED) {

      int eeprom_address = 1;
      // Read all eeprom into memory
      while (eeprom_address < 512) {
        _eepromData[eeprom_address] = EEPROM.read(eeprom_address);
    #ifdef ESPWC_DEBUG
        String dbg = " ";
        dbg += _eepromData[eeprom_address];
        Serial.print(dbg.c_str());
        if (eeprom_address%32 == 0) {
          Serial.println();
        }
    #endif
        eeprom_address++;
      }
    }
    ESPWC_PRINTLN("");
    return _eepromData[0] == CONFIG_VALID;
  }

  bool Restore() {
    return _eepromData[0] == 0x1B;
  }

  byte _eepromData[512];

};

#endif
