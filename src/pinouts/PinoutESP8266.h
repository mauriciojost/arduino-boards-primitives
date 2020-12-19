#ifdef ESP8266 // NODEMCU based on ESP8266

#define GPIO0_PIN 0     // (*)
#define GPIO1_PIN 1     // 
#define GPIO2_PIN 2     // built-in LED (*) (?)
#define GPIO3_PIN 3     // 
#define GPIO4_PIN 4     // (@)
#define GPIO5_PIN 5     // (@)
//#define GPIO6_PIN 6   // can't be used as connected to flash
//#define GPIO7_PIN 7   // can't be used as connected to flash
//#define GPIO8_PIN 8   // can't be used as connected to flash
//#define GPIO9_PIN 9   // can't be used as connected to flash
//#define GPIO10_PIN 10 // can't be used as connected to flash
//#define GPIO11_PIN 11 // can't be used as connected to flash
#define GPIO12_PIN 12   // 
#define GPIO13_PIN 13   // 
#define GPIO14_PIN 14   // 
#define GPIO15_PIN 15   // (*)
#define GPIO16_PIN 16   // 
#define A0_PIN 17       // 

// BOOT TRANSIENTS
// (@) such pins are stable low digital outputs at boot time, and are the only ones that can be used reliably (no transients, well defined behabiour) to control loads like relays, motors, servos, etc.
// The ESP8266 is a complicated non-trivial uC when it comes to behavior of its GPIO at boot. Some pins are not usable, some are output for a few tens of ms (even before getting to the setup() routine call), etc. Better read: https://rabbithole.wwwdotorg.org/2017/03/28/esp8266-gpio.html (heads up, study done with NodeMCU, not with the raw ESP8266)

/*
+-----------+----+-----------------------------------+----------------------------------------------+-----------------------------------------------+
|GPIOESP8266|GPIO|flash-mode                         |dummy                                         |set-gpios-low                                  |
|node-mcu   |    |behaviour                          |application behaviour                         |application behaviour                          |
+-----------+----+-----------------------------------+----------------------------------------------+-----------------------------------------------+
|D0         |16  |high                               |high during boot, falls after ~110ms (to ~1V?)|high during boot, falls after ~110ms (to ~1V)  |
|D1         | 5  |low                                |low                                           |low                                            |
|D2         | 4  |low                                |low                                           |low                                            |
|D3         | 0  |low then oscillates                |varies, stabilizes high after ~100ms          |varies, stabilizes low after ~110ms            |
|D4         | 2  |varies, stabilizes high after ~60ms|varies, stabilizes high after ~70ms           |varies, stabilizes low after ~110ms            |
|D5         |14  |high                               |high, then low after ~110ms                   |high, then low after ~110ms                    |
|D6         |12  |high                               |high, then low after ~110ms                   |high, then low after ~110ms                    |
|D7         |13  |high                               |high, then low after ~110ms                   |high, then low after ~110ms                    |
|D8         |15  |low                                |low, with glitch ~110ms                       |low, with glitch ~110ms                        |
|D9         | 3  |low                                |low until ~50ms then high                     |low until ~50ms then high until ~110ms then low|
|D10        | 1  |low                                |low until ~50ms then high                     |low until ~50ms then high until ~110ms then low|
+-----------+----+-----------------------------------+----------------------------------------------+-----------------------------------------------+
*/

// BOOT MODE
// (*) such pin participates in boot mode, so pull ups/down resistors are needed.
// From 
// - https://tttapa.github.io/ESP8266/Chap04%20-%20Microcontroller.html
// - https://i.pinimg.com/originals/f7/81/ef/f781efedf44a3b9673f1e88d2d8253c8.png

// +-------+-------+--------+-------------------------------------
// | GPIO0 | GPIO2 | GPIO15 |
// +-------+-------+--------+-------------------------------------
// |    0  |    1  |    0   | UART BOOTLOADER
// |    1  |    1  |    0   | RUN MODE (in every boot, heads up or may unflash/corrupt your firmware)
// +-------+-------+--------+-------------------------------------

#endif // ESP8266
