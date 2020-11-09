#ifdef ESP8266 // NODEMCU based on ESP8266

// CONSIDERATIONS FOR THE CIRCUIT 

// From 
// - https://tttapa.github.io/ESP8266/Chap04%20-%20Microcontroller.html
// - https://i.pinimg.com/originals/f7/81/ef/f781efedf44a3b9673f1e88d2d8253c8.png

// GPIO15  GPIO0   GPIO2
//     0       0       1   UART BOOTLOADER
//     0       1       1   RUN MODE (in every boot, heads up or may unflash/corrupt your firmware)
//
// Heads up when having pull-up/pull-down resistors in such pins!
//
// ....
// [testing if needed all below] use resistors of 500Kohm for such
// Ensure:
//   VDD-----220uF---GND
//   EN------500kohm---VDD
//   RST--+--500kohm---VDD
//        \--470pf-----GND
//   GPIO0---500kohm---VDD
//   GPIO2---500kohm---VDD
//   GPIO15--500kohm---VDD
//   GPIO16------------RST (only in case of deep sleep) 


#define GPIO0_PIN 0 // pull-up?, participates in boot mode
#define GPIO1_PIN 1 // pull-up?
#define GPIO2_PIN 2 // pull-up?, participates in boot mode
#define GPIO3_PIN 3 // pull-up?
#define GPIO4_PIN 4 // pull-up
#define GPIO5_PIN 5 // pull-up
//#define GPIO6_PIN 6 // pull-up, can't be used as connected to flash
//#define GPIO7_PIN 7 // pull-up, can't be used as connected to flash
//#define GPIO8_PIN 8 // pull-up, can't be used as connected to flash
//#define GPIO9_PIN 9 // pull-up, can't be used as connected to flash
//#define GPIO10_PIN 10 // pull-up, can't be used as connected to flash
//#define GPIO11_PIN 11 // pull-up, can't be used as connected to flash
#define GPIO12_PIN 12 // pull-up
#define GPIO13_PIN 13 // pull-up
#define GPIO14_PIN 14 // pull-up
#define GPIO15_PIN 15 // pull-up, participates in boot mode
#define GPIO16_PIN 16 // pull-down
#define A0_PIN 17

#endif // ESP8266
