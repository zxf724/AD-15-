#include "MFRC522_SPI.h"


/*****************************************************
** @函数名：RC522SPI_Init
** @描述  ：RC522 模拟SPI接口初始化配置
** @参数  ：无
** @返回值：无
*****************************************************/
void RC522SPI_Init (void)
{    
  GPIO_Init(SPI_MISO_GPIO_PORT, (GPIO_Pin_TypeDef)SPI_MISO_GPIO_PINS, GPIO_MODE_IN_PU_NO_IT);
  /* MOSI 低速 推完输出 */
  GPIO_Init(SPI_MOSI_GPIO_PORT, (GPIO_Pin_TypeDef)SPI_MOSI_GPIO_PINS, GPIO_MODE_OUT_PP_HIGH_SLOW);
  /* CLK 低速 推完输出 */
  GPIO_Init(SPI_CLK_GPIO_PORT, (GPIO_Pin_TypeDef)SPI_CLK_GPIO_PINS, GPIO_MODE_OUT_PP_HIGH_SLOW);

  /* CS  推完输出 */
  GPIO_Init(SPI_CS_GPIO_PORT, (GPIO_Pin_TypeDef)SPI_CS_GPIO_PINS, GPIO_MODE_OUT_PP_HIGH_SLOW);

  /* RC523RST  推完输出 */
  GPIO_Init(RC523RST_GPIO_PORT, (GPIO_Pin_TypeDef)RC523RST_GPIO_PINS, GPIO_MODE_OUT_PP_HIGH_SLOW);
}

/*****************************************************
** @函数名：SPIReadByte
** @描述  ：RC522 模拟SPI接口 SPI读数据
** @参数  ：无
** @返回值：无
*****************************************************/
uint8_t SPIReadByte (void)
{
    uint8_t SPICount = 0;
    uint8_t SPIData  = 0;
    
    SPIData = 0;
    for (SPICount = 0; SPICount < 8; SPICount++)
    {
        SPIData <<= 1;
        CLR_SPI_SCK;
        __nop();
        __nop();
        if (STU_SPI_MISO)
        {
            SPIData |= 0x01;
        }
        SET_SPI_SCK;
        __nop();
        __nop();
    }
    
    return SPIData;
}

/*****************************************************
** @函数名：SPIWriteByte
** @描述  ：RC522 模拟SPI接口 SPI写数据
** @参数  ：无
** @返回值：无
*****************************************************/
void SPIWriteByte (uint8_t SPIData)
{
  uint8_t SPICount;                                       // Counter used to clock out the data
  
  for (SPICount = 0; SPICount < 8; SPICount++)
  {
    if (SPIData & 0x80) {
      SET_SPI_MOSI;
    } else {
      CLR_SPI_MOSI;
    } 
    __nop();
    __nop();
    CLR_SPI_SCK;
    __nop();
    __nop();
    SET_SPI_SCK;
		__nop();
    __nop();
    SPIData <<= 1;
  }        	
}

/*****************************************************
** @函数名：delay_us
** @描述  ：延时函数, 非标准us级别延时
** @参数  ：tus
** @返回值：无
*****************************************************/
void delay_us (uint16_t tus)
{
  uint16_t ii;
    
  while(tus)
  {
    tus--;
    for (ii = 0; ii < 100; ii++) 
    {
      __nop();
      __nop();
      __nop();
      __nop();
      __nop();
      __nop();
      __nop();
      __nop();
    }
  }
}


