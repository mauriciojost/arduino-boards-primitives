#ifdef ESP8266 // NODEMCU based on ESP8266

#define GPIO0_PIN 0 // *
#define GPIO1_PIN 1 // 
#define GPIO2_PIN 2 // *
#define GPIO3_PIN 3 // 
#define GPIO4_PIN 4 // @
#define GPIO5_PIN 5 // @
//#define GPIO6_PIN 6 // can't be used as connected to flash
//#define GPIO7_PIN 7 // can't be used as connected to flash
//#define GPIO8_PIN 8 // can't be used as connected to flash
//#define GPIO9_PIN 9 // can't be used as connected to flash
//#define GPIO10_PIN 10 // can't be used as connected to flash
//#define GPIO11_PIN 11 // can't be used as connected to flash
#define GPIO12_PIN 12 // 
#define GPIO13_PIN 13 // 
#define GPIO14_PIN 14 // 
#define GPIO15_PIN 15 // *
#define GPIO16_PIN 16 // 
#define A0_PIN 17  // 

// BOOT TRANSIENTS
// (@) such pins are stable low digital outputs at boot time, and are the only ones that can be used reliably (no transients, well defined behabiour) to control loads like relays, motors, servos, etc.
// The ESP8266 is a complicated non-trivial uC when it comes to behavior of its GPIO at boot. Some pins are not usable, some are output for a few tens of ms (even before getting to the setup() routine call), etc. Better read:
https://rabbithole.wwwdotorg.org/2017/03/28/esp8266-gpio.html

/*

NodeMCU
GPIOESP8266    GPIOBehaviour     Flash ModeBehaviour   Dummy Arduino AppBehaviour    Arduino Set GPIOs Low
D0 16   High   High during boot, falls after ~110ms (to ~1V?)   High during boot, falls after ~110ms (to ~1V)
D1 5 Low Low Low
D2 4 Low Low Low 
D3 0 Low then oscillatesVaries, stabilizes high after ~100msVaries, stabilizes low after ~110ms
D4 2 Varies, stabilizes high after ~60msVaries, stabilizes high after ~70msVaries, stabilizes low after ~110ms
D5 14 High High, then low after ~110msHigh, then low after ~110ms
D6 12 High High, then low after ~110msHigh, then low after ~110ms
D7 13 High High, then low after ~110msHigh, then low after ~110ms
D8 15 Low Low, with glitch ~110msLow, with glitch ~110ms
D9 3 Low Low until ~50ms then highLow until ~50ms then high until ~110ms then low
D10 1 Low Low until ~50ms then highLow until ~50ms then high until ~110ms then low

*/



// BOOT MODE
// (*) such pin participates in boot mode, so pull ups/down resistors are needed.
// From 
// - https://tttapa.github.io/ESP8266/Chap04%20-%20Microcontroller.html
// - https://i.pinimg.com/originals/f7/81/ef/f781efedf44a3b9673f1e88d2d8253c8.png

// GPIO0   GPIO2   GPIO15
//     0       1       0    UART BOOTLOADER
//    1       1        0    RUN MODE (in every boot, heads up or may unflash/corrupt your firmware)

#endif // ESP8266
