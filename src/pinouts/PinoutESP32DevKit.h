#ifdef ESP32 // NODEMCU based on DEVKIT

// FROM https://circuits4you.com/2018/12/31/esp32-devkit-esp32-wroom-gpio-pinout/

#define GPIO1_PIN 1 // CLOCK3 / U0_TXD
#define GPIO2_PIN 2 // CS / ADC2_2 / HSPI-WP / TOUCH2
#define GPIO3_PIN 3 // CLK2 / U0_RXD
#define GPIO4_PIN 4 // ADC2_0 / HSPI_HD / TOUCH0
#define GPIO5_PIN 5 // V_SPI_CS0

#define GPIO12_PIN 12 // ADC2_5 / HSPI_Q / TOUCH5
#define GPIO13_PIN 13 // ADC2_4 / HSPI_ID / TOUCH4
#define GPIO14_PIN 14 // ADC2_6 / HSPI_CLK / TOUCH6
#define GPIO15_PIN 15 // ADC2_3 / HSPI_CS0 / TOUCH3
#define GPIO16_PIN 16 // U2_RXD
#define GPIO17_PIN 17 // U2_TXD
#define GPIO18_PIN 18 // SCK / VSPI-CLK
#define GPIO19_PIN 19 // MISO / V_SPI_Q / U0_CTS

#define GPIO21_PIN 21 // SDA / V_SPI_HD
#define GPIO22_PIN 22 // SCL / V_SPI_WD / U0_RTS
#define GPIO23_PIN 23 // MOSI / V_SPI_D

#define GPIO25_PIN 25 // ADC2_8 / DAC1
#define GPIO26_PIN 26 // ADC2_9 / DAC2
#define GPIO27_PIN 27 // ADC2_7 / TOUCH7

#define GPIO32_PIN 32 // ADC1_4 / XTAL32 / TOUCH9
#define GPIO33_PIN 33 // ADC1_5 / XTAL32 / TOUCH8
#define GPIO34_PIN 34 // ADC1_6
#define GPIO35_PIN 35 // ADC1_7
#define GPIO36_PIN 36 // ADC1_0 / SENS_VP

#define GPIO39_PIN 39 // ADC1_3 / SENS_VN

#endif // ESP32