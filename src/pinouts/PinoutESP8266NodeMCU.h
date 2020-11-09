#ifdef ESP8266
// NODEMCU based on ESP8266 (human names)

#include <pinouts/PinoutESP8266.h>

#define PIN_D0 GPIO16_PIN // working as OUTPUT, breaks deep sleep mode, NODE-MCU built-in !LED
#define PIN_D1 GPIO5_PIN
#define PIN_D2 GPIO4_PIN
#define PIN_D3 GPIO0_PIN  // working as OUTPUT
#define PIN_D4 GPIO2_PIN  // working as OUTPUT, ESP8266-12 built-in !LED
#define PIN_D5 GPIO14_PIN // working as OUTPUT
#define PIN_D6 GPIO12_PIN
#define PIN_D7 GPIO13_PIN  // RXD2
#define PIN_D8 GPIO15_PIN  // TXD2, working as OUTPUT
#define PIN_D9 GPIO3_PIN   // RXDO,  if used will break serial communication (uC <- PC), working as OUTPUT
#define PIN_D10 GPIO1_PIN  // TXDO, if used will break serial communication (uC -> PC), can work as OUTPUT
#define PIN_D11 GPIO9_PIN  // SDD2 / SD2 // cannot be used at all, internal
#define PIN_D12 GPIO10_PIN // SDD3 / SD3 // can be used as input only

#endif // ESP8266
