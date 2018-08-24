/*
  **********************************  STM8S  ***********************************
  * @文件名     ： bsp.c
  * @作者       ： Huang Fugui
  * @库版本     ： V2.2.0
  * @文件版本   ： V1.0.0
  * @日期       ： 2016年09月02日
  * @摘要       ： 板级支持包源文件
  ******************************************************************************/
/*----------------------------------------------------------------------------
  更新日志:
  2016-09-02 V1.0.0:初始版本
  ----------------------------------------------------------------------------*/
/* 包含的头文件 --------------------------------------------------------------*/
#include "bsp.h"

uint32_t TS_Index = 0;

/************************************************
函数名称 ： CLK_Configuration
功    能 ： 时钟配置
参    数 ： 无
返 回 值 ： 无
作    者 ： Huang Fugui
*************************************************/
void CLK_Configuration(void)
{
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1); //HSI = 16M (1分频)
}

/************************************************
函数名称 ： GPIO_Basic_Configuration
功    能 ： 基本IO引脚配置
参    数 ： 无
返 回 值 ： 无
作    者 ： Huang Fugui
*************************************************/
void GPIO_Basic_Configuration(void)
{
    GPIO_Init(LED_RED_GPIO_Port, (GPIO_Pin_TypeDef)LED_RED_Pin, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(LED_GREEN_GPIO_Port, (GPIO_Pin_TypeDef)LED_GREEN_Pin, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(PORT_485EN, (GPIO_Pin_TypeDef)PIN_485EN, GPIO_MODE_OUT_PP_LOW_FAST);

    GPIO_Init(PORT_KEY1, (GPIO_Pin_TypeDef)PIN_KEY1, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(PORT_KEY2, (GPIO_Pin_TypeDef)PIN_KEY2, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(PORT_KEY3, (GPIO_Pin_TypeDef)PIN_KEY3, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(PORT_KEY4, (GPIO_Pin_TypeDef)PIN_KEY4, GPIO_MODE_IN_PU_NO_IT);

    GPIO_Init(GPIOD, (GPIO_Pin_TypeDef)(GPIO_PIN_6), GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(GPIOD, (GPIO_Pin_TypeDef)GPIO_PIN_5, GPIO_MODE_OUT_PP_HIGH_FAST);
    /* 系统时钟输出测试 */
#if 0
    GPIO_Init(GPIOC,(GPIO_Pin_TypeDef)GPIO_PIN_4, GPIO_MODE_OUT_PP_HIGH_FAST);
    CLK_CCOCmd(ENABLE);
#endif
}

/************************************************
函数名称 ： BSP_Initializes
功    能 ： 板级支持包初始化
参    数 ： 无
返 回 值 ： 无
作    者 ： Huang Fugui
*************************************************/
void BSP_Initializes(void)
{
    CLK_Configuration();
    GPIO_Basic_Configuration();
}


/***** Copyright (C)2016 HuangFugui. All Rights Reserved ***** END OF FILE *****/
