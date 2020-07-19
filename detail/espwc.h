#ifndef ESPWC_H
#define ESPWC_H

#ifdef ESPWC_DEBUG
#define ESPWC_PRINT(x) (Serial.print(x))
#define ESPWC_PRINTLN(x) (Serial.println(x))
#else
#define ESPWC_PRINT(x)
#define ESPWC_PRINTLN(x)
#endif

#endif
