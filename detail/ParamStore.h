#ifndef EWC_PARAMSTORE_H
#define EWC_PARAMSTORE_H

#define CONFIG_VALID 0x1a
#define CONFIG_ERASED 0x1b

#include "c_types.h"

class ParamStore
{
public:
  char* GetParameterById(const int id);

  /* Read all config strings to memory, return true if valid config */
  bool ReadConfig();

  bool Restore();
private:
  uint8_t eepromData[512];

};

#endif //EWC_PARAMSTORE_H
