#ifndef __MFRC522_SPI_H
#define __MFRC522_SPI_H

#include "stm8s.h"
/*********PORTD*********/

/* SPI 端口 */
#define SPI_MISO_GPIO_PORT        (GPIOC)
#define SPI_MOSI_GPIO_PORT        (GPIOC)
#define SPI_CLK_GPIO_PORT         (GPIOC)
#define SPI_CS_GPIO_PORT          (GPIOC)
#define RC523RST_GPIO_PORT        (GPIOC)                   //复位端口



#define SPI_MISO_GPIO_PINS        (GPIO_PIN_7)
#define SPI_MOSI_GPIO_PINS        (GPIO_PIN_6)
#define SPI_CLK_GPIO_PINS         (GPIO_PIN_5)
#define SPI_CS_GPIO_PINS          (GPIO_PIN_4)
#define RC523RST_GPIO_PINS        (GPIO_PIN_3)              //复位引脚


#define STU_SPI_MISO              GPIO_ReadInputPin(SPI_MISO_GPIO_PORT, (GPIO_Pin_TypeDef)SPI_MISO_GPIO_PINS)

#define SET_SPI_MOSI              GPIO_WriteHigh(SPI_MOSI_GPIO_PORT, (GPIO_Pin_TypeDef)SPI_MOSI_GPIO_PINS)
#define CLR_SPI_MOSI              GPIO_WriteLow(SPI_MOSI_GPIO_PORT, (GPIO_Pin_TypeDef)SPI_MOSI_GPIO_PINS)

#define SET_SPI_SCK               GPIO_WriteHigh(SPI_CLK_GPIO_PORT, (GPIO_Pin_TypeDef)SPI_CLK_GPIO_PINS)
#define CLR_SPI_SCK               GPIO_WriteLow(SPI_CLK_GPIO_PORT, (GPIO_Pin_TypeDef)SPI_CLK_GPIO_PINS)

#define SET_SPI_SDA               GPIO_WriteHigh(SPI_CS_GPIO_PORT, (GPIO_Pin_TypeDef)SPI_CS_GPIO_PINS)
#define CLR_SPI_SDA               GPIO_WriteLow(SPI_CS_GPIO_PORT, (GPIO_Pin_TypeDef)SPI_CS_GPIO_PINS)

#define SET_SPI_RST               GPIO_WriteHigh(RC523RST_GPIO_PORT, (GPIO_Pin_TypeDef)RC523RST_GPIO_PINS)
#define CLR_SPI_RST               GPIO_WriteLow(RC523RST_GPIO_PORT, (GPIO_Pin_TypeDef)RC523RST_GPIO_PINS)

#define __nop()                   nop()

/**********函数声明**********/
void        RC522SPI_Init (void);
uint8_t     SPIReadByte (void);
void        SPIWriteByte (uint8_t SPIData);
void        delay_us (uint16_t tus);


#endif

