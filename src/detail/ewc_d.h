#ifndef EWC_D_H
#define EWC_D_H

/*
To enable in Platform IO (platformio.ini file):

build_flags =
	'-DESPWC_DEBUG=1'
*/

#ifdef ESPWC_DEBUG
#define ESPWC_PRINTF(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#define ESPWC_PRINT(x) (Serial.print(x))
#else
#define ESPWC_PRINTF(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
//#define ESPWC_PRINTF(fmt, ...)
#define ESPWC_PRINT(x)
#endif

#endif // EWC_D_H