/**
 * This file contains:
 * - entry point for arduino programs (setup and loop functions)
 * - declaration of HW specific functions (the definition is in a dedicated file)
 * - other functions that are not defined by HW specific but that use them, that are required by the module
 *   (so that it can be passed as callback).
 * The rest should be put in Module so that they can be tested regardless of the HW used behind.
 */

#include <Main.h>

#ifdef ARDUINO

#ifdef ESP8266 // on ESP8266
#include <ESP8266.h>
#endif // ESP8266

#ifdef ESP32 // on ESP32
#include <ESP32.h>
#endif // ESP32

#else // ARDUINO

#ifdef X86_64 // on PC
#include <X86_64.h>
#endif

void main() {
}

#endif // ARDUINO

}
