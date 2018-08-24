#ifndef __MFRC522_SPI_H
#define __MFRC522_SPI_H

#include "stm8l15x.h"
/*********PORTD*********/

/* SPI 端口 */
#define SPI_MISO_GPIO_PORT        (GPIOB)
#define SPI_MOSI_GPIO_PORT        (GPIOB)
#define SPI_CLK_GPIO_PORT         (GPIOB)
#define SPI_CS_GPIO_PORT          (GPIOB)
#define RC523RST_GPIO_PORT        (GPIOC)                   //复位端口



#define SPI_MISO_GPIO_PinS        (GPIO_Pin_7)
#define SPI_MOSI_GPIO_PinS        (GPIO_Pin_6)
#define SPI_CLK_GPIO_PinS         (GPIO_Pin_5)
#define SPI_CS_GPIO_PinS          (GPIO_Pin_4)
#define RC523RST_GPIO_PinS        (GPIO_Pin_4)              //复位引脚


#define STU_SPI_MISO              GPIO_ReadInputDataBit(SPI_MISO_GPIO_PORT, (GPIO_Pin_TypeDef)SPI_MISO_GPIO_PinS)

#define SET_SPI_MOSI              GPIO_SetBits(SPI_MOSI_GPIO_PORT, (GPIO_Pin_TypeDef)SPI_MOSI_GPIO_PinS)
#define CLR_SPI_MOSI              GPIO_ResetBits(SPI_MOSI_GPIO_PORT, (GPIO_Pin_TypeDef)SPI_MOSI_GPIO_PinS)

#define SET_SPI_SCK               GPIO_SetBits(SPI_CLK_GPIO_PORT, (GPIO_Pin_TypeDef)SPI_CLK_GPIO_PinS)
#define CLR_SPI_SCK               GPIO_ResetBits(SPI_CLK_GPIO_PORT, (GPIO_Pin_TypeDef)SPI_CLK_GPIO_PinS)

#define SET_SPI_SDA               GPIO_SetBits(SPI_CS_GPIO_PORT, (GPIO_Pin_TypeDef)SPI_CS_GPIO_PinS)
#define CLR_SPI_SDA               GPIO_ResetBits(SPI_CS_GPIO_PORT, (GPIO_Pin_TypeDef)SPI_CS_GPIO_PinS)

#define SET_SPI_RST               GPIO_SetBits(RC523RST_GPIO_PORT, (GPIO_Pin_TypeDef)RC523RST_GPIO_PinS)
#define CLR_SPI_RST               GPIO_ResetBits(RC523RST_GPIO_PORT, (GPIO_Pin_TypeDef)RC523RST_GPIO_PinS)

#define __nop()                   nop()

/**********函数声明**********/
void        RC522SPI_Init (void);
uint8_t     SPIReadByte (void);
void        SPIWriteByte (uint8_t SPIData);
void        delay_us (uint16_t tus);


#endif

