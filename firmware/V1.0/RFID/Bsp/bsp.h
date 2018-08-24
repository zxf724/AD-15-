/*
  **********************************  STM8S  ***********************************
  * @文件名     ： bsp.h
  * @作者       ： Huang Fugui
  * @库版本     ： V2.2.0
  * @文件版本   ： V1.0.0
  * @日期       ： 2016年09月02日
  * @摘要       ： 板级支持包头文件
  ******************************************************************************/

/* 定义防止递归包含 ----------------------------------------------------------*/
#ifndef _BSP_H
#define _BSP_H

/* 包含的头文件 --------------------------------------------------------------*/
#include "stm8s.h"


/* 宏定义 --------------------------------------------------------------------*/

/* LED */
#define LED_RED_Pin             GPIO_PIN_2
#define LED_RED_GPIO_Port       GPIOD

#define LED_GREEN_Pin           GPIO_PIN_1
#define LED_GREEN_GPIO_Port     GPIOA

/* SET 按键*/
#define PORT_KEY1               GPIOB
#define PORT_KEY2               GPIOB
#define PORT_KEY3               GPIOA
#define PORT_KEY4               GPIOD
#define PIN_KEY1                GPIO_PIN_4
#define PIN_KEY2                GPIO_PIN_5
#define PIN_KEY3                GPIO_PIN_3
#define PIN_KEY4                GPIO_PIN_4
#define KEY1                    (GPIO_ReadInputPin(PORT_KEY1, (GPIO_Pin_TypeDef)PIN_KEY1) == 0)
#define KEY2                    (GPIO_ReadInputPin(PORT_KEY2, (GPIO_Pin_TypeDef)PIN_KEY2) == 0)
#define KEY3                    (GPIO_ReadInputPin(PORT_KEY3, (GPIO_Pin_TypeDef)PIN_KEY3) == 0)
#define KEY4                    (GPIO_ReadInputPin(PORT_KEY4, (GPIO_Pin_TypeDef)PIN_KEY4) == 0)

/* RS485 EN */
#define PORT_485EN              GPIOD
#define PIN_485EN               GPIO_PIN_3
#define EN485_Recevie_OFF       (PORT_485EN->ODR |= (uint8_t)PIN_485EN)
#define EN485_Recevie_ON        (PORT_485EN->ODR &= (uint8_t)(~PIN_485EN))

#define IO_READ(x)              ((x##_GPIO_Port->IDR & x##_Pin) ? 1 : 0)
#define IO_H(x)                 (x##_GPIO_Port->ODR |= x##_Pin)
#define IO_L(x)                 (x##_GPIO_Port->ODR &= (uint8_t)(~x##_Pin))
#define IO_TOGGLE(x)            (x##_GPIO_Port->ODR ^= x##_Pin)

#define LED_ON(x)               IO_H(LED_##x)
#define LED_OFF(x)              IO_L(LED_##x)
#define LED_TOGGLE(x)           IO_TOGGLE(LED_##x)

#define GPIO_H(port, pin)       (port->ODR |= pin)
#define GPIO_L(port, pin)       (port->ODR &= (uint8_t)(~pin))

#define TS_INIT(ts)             do {ts = TS_Index;}while(0)
#define TS_IS_OVER(ts, over)    ((uint32_t)TS_Index - (uint32_t)ts >= over)
#define TS_COUNT(ts)            ((uint32_t)TS_Index - (uint32_t)ts)
#define TS_DELAY(delay)         do {uint32_t volatile ts; TS_INIT(ts); while(!TS_IS_OVER(ts, delay));}while(0)

#define IO_INIT_OUT_LOW(x)      GPIO_Init(x##_GPIO_Port, x##_Pin, GPIO_MODE_OUT_PP_LOW_FAST)
#define IO_INIT_OUT_PP(x)       GPIO_Init(x##_GPIO_Port, x##_Pin, GPIO_MODE_OUT_PP_HIGH_FAST)
#define IO_INIT_IN_PU(x)        GPIO_Init(x##_GPIO_Port, x##_Pin, GPIO_MODE_IN_PU_NO_IT)

extern uint32_t TS_Index;

/* 函数申明 ------------------------------------------------------------------*/
void CLK_Configuration(void);
void GPIO_Basic_Configuration(void);
void BSP_Initializes(void);


#endif /* _BSP_H */

/***** Copyright (C)2016 HuangFugui. All Rights Reserved ***** END OF FILE *****/
