#ifndef _GENERAL_CONFIG_H
#define _GENERAL_CONFIG_H

#define GP_EPD_RST_PIN 21
#define GP_EPD_DC_PIN 16
#define GP_EPD_BUSY_PIN 20
#define GP_EPD_CS_PIN 17
#define GP_EPD_CLK_PIN 18
#define GP_EPD_MOSI_PIN 19
#define DC_SPI_PORT spi0

#define KEY0 (27)
#define KEY1 (22)
#define KEY2 (26)

#ifdef __cplusplus
extern "C" {
#endif

void general_sleep();

#ifdef __cplusplus
}
#endif

#endif
