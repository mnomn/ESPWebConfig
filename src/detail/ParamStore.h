#ifndef EWC_PARAMSTORE_H
#define EWC_PARAMSTORE_H

#include <Arduino.h>
#include <map>
// #include "c_types.h"

class ParamStore
{
public:
  // Read all config strings to memory, return true if valid config
  bool Initialize(const int noOfExternalParameters);

  // Return true if valid configuration values are stored and read.
  bool IsConfigured();

  const char* GetValue(const int id);

  // Store parameter temporarily. Must be stored in order of the id.
  // After all parameters are stored, call Commit to finalize save.
  bool StoreParameter(const int id, const String& val);

  bool Commit();

  void Clear();

  static const int ssidIndex = 1;
  static const int passwordIndex = 2;
  static const int internalParamSize = 2;

private:
  // Read all config strings to memory, return true if valid config
  bool ReadAll();
  uint8_t m_status {0};
  int m_noOfParameters {0};

  // Sorted map used during configuring
  std::map<int, String> m_saveMap;
};

#endif //EWC_PARAMSTORE_H
