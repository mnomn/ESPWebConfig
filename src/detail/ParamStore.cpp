//#include "Arduino.h"
#include "ParamStore.h"
#include "ewc_d.h"
#include <EEPROM.h>

constexpr uint8_t configValid = 0x1a;
constexpr uint8_t configCleared = 0x1b;

constexpr int eepromSize = 512;
constexpr int maxExternalParamSize = 8;
constexpr int maxParamSize = maxExternalParamSize + ParamStore::internalParamSize;

/* Read all config strings to memory, return true if valid config */
bool ParamStore::Initialize(const int noOfExternalParameters) {
  if (noOfExternalParameters > maxExternalParamSize) return false;

  m_noOfParameters = internalParamSize + noOfExternalParameters;
  return ReadAll();
}

bool ParamStore::IsConfigured() {
  if (m_noOfParameters <= 0) return false;

  return m_status == configValid;
}

/* Read all config strings to memory, return true if valid config */
bool ParamStore::ReadAll()
{
  EEPROM.begin(eepromSize);
  m_status = EEPROM.read(0);
  return m_status == configValid;
}

// Get value is returning a pointer to the data held by the eeprom lib
const char* ParamStore::GetValue(const int id) {
  ESPWC_PRINTF("GetParameterById %d\n", id);

  if (m_status != configCleared && m_status != configValid) {
    return nullptr;
  }

  if (id <= 0 || id > m_noOfParameters) {
    return nullptr;
  }

  // Get pointer to index 0;
  const uint8_t* eepromData = EEPROM.getConstDataPtr();
  const uint8_t *end = eepromData + eepromSize - 1;

  while (*eepromData != id) {
    eepromData++;
    if (eepromData == end) {
      ESPWC_PRINT("GetParameterById NOT FOUND\n");
      return nullptr;
    }
  }

  // Found id. Verify that there is an end/null termination
  eepromData++;
  const uint8_t* tmp = eepromData;
  while (tmp != end) {
    if (*tmp == '\0') {
      ESPWC_PRINT("GetParameterById FOUND\n");
      return reinterpret_cast<const char*>(eepromData);
    }
    tmp++;
  }

  ESPWC_PRINT("GetParameterById END NOT FOUND\n");
  return nullptr;
}

bool ParamStore::StoreParameter(const int id, const String& val) {
  ESPWC_PRINTF("StoreParameter: %d, %s\n", id, val.c_str());
  if (id < 1 || m_noOfParameters < id) {
    return false;
  }

  m_saveMap[id] = val;
  return true;
}

bool ParamStore::Commit() {
  auto eepromData = EEPROM.getDataPtr();
  auto eepromEnd = eepromData + eepromSize;
  eepromData++; // First item holds the status

  ESPWC_PRINTF("Commit parameters: %d\n", m_noOfParameters);

  for (auto& param: m_saveMap) {
    int id = param.first;
    String val = param.second;

    Serial.printf("Commit param:  %d %s\n", id, val.c_str());

    if (id < 1 || (m_noOfParameters) < id ) return false;

    // First write index
    *eepromData = (uint8_t)id;
    eepromData++;

    // Ten write string including zero term.
    auto copySize = val.length() + 1; // String plus nul termination
    if (copySize >= (unsigned int)(eepromEnd - eepromData)) return false;

    val.getBytes(eepromData, copySize);
    eepromData += copySize;
  }

  EEPROM.write(0, configValid);
  m_status = configValid;

#if 0
  // For debug: Print eeprom
  for (int i = 0; i < 60; i+= 10) {
    Serial.printf("%X %X %X %X %X %X %X %X %X %X\n",
      EEPROM.read(i), EEPROM.read(i+1), EEPROM.read(i+2), EEPROM.read(i+3), EEPROM.read(i+4),
      EEPROM.read(i+5), EEPROM.read(i+6), EEPROM.read(i+7), EEPROM.read(i+8), EEPROM.read(i+9)
    );
  }
#endif

  return EEPROM.commit();
}

void ParamStore::Clear() {
  if (m_status == configCleared) return;

  EEPROM.write(0, configCleared);
  EEPROM.commit();
  m_status = configCleared;
}
