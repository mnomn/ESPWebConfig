#include "ParamStore.h"
#include <EEPROM.h>
#include "detail/ewc_d.h"

char* ParamStore::GetParameterById(const int id) {
  ESPWC_PRINT("GetParameterById ");
  ESPWC_PRINTLN(id);
  if (id <= 0) {
    return NULL;
  }
  uint8_t* tmp = eepromData;
  uint8_t* end = tmp + 511;
  while(*tmp != id) {
    tmp++;
    if (tmp == end) {
      ESPWC_PRINTLN("Param not found");
      return NULL;
    }
  }
  // Found index, skip one to point to string
  tmp++;
  ESPWC_PRINT("Param found:");
  ESPWC_PRINTLN((char*)tmp);
  return (char*)tmp;
}

/* Read all config strings to memory, return true if valid config */
bool ParamStore::ReadConfig()
{
  EEPROM.begin(512);
  eepromData[0] = EEPROM.read(0);
#ifdef ESPWC_DEBUG
  Serial.printf("EEPROM 0: 0x%X\n", _eepromData[0]);
#endif
  if (eepromData[0] == CONFIG_VALID ||
      eepromData[0] == CONFIG_ERASED) {

    int eeprom_address = 1;
    // Read all eeprom into memory
    while (eeprom_address < 512) {
      eepromData[eeprom_address] = EEPROM.read(eeprom_address);
#ifdef ESPWC_DEBUG
      if (eeprom_address < 64) {
        byte az = _eepromData[eeprom_address];
        if (' ' <  az && az < 'z') Serial.print((char)az);
        else if (az < 10) Serial.printf("<%d>", az);
        delay(1);
      }
#endif
      eeprom_address++;
    }
  }
  return eepromData[0] == CONFIG_VALID;
}

bool ParamStore::Restore() {
  return eepromData[0] == 0x1B;
}
