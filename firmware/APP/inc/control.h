/**
  ******************************************************************************
  * @file    control.c
  * @author  宋阳
  * @version V1.0
  * @date    2015.12.4
  * @brief   led driver.
  * @brief   Header file of Control
  ******************************************************************************

  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _Control_H
#define _Control_H

/* Includes ------------------------------------------------------------------*/
#include "prjlib.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  status_idle,
  status_borrow,
  status_repay,
  status_borrow_complite,
  status_repay_complite,
  status_motor_stuck,
  status_ir_stuck,
  status_timeout, 
} motor_status_t;

/* Exported constants --------------------------------------------------------*/
/*电机转动的时间，单位ms*/
#define MOTOR_ACTION_TIME     5
/*电机转动超时次数*/
#define MOTOR_OVERFLOW_TIMES  1000

/*电量检测的间隔时间,单位ms*/
#define BAT_CHECK_TIME        15000

extern  uint16_t BatVol;

extern motor_status_t Motor_staus;


#define LED_FLASH_CONTINUE          0xFFFF
/* Exported macro ------------------------------------------------------------*/

/*
LED定义： 
 
1. 状态指示灯
取伞或还伞:    长亮 
红外遮挡:      200ms亮, 500ms灭,循环闪烁 
电机过流：     1s亮，500ms灭,循环闪烁 
 
1. 网络指示灯
蓝牙/MQTT收发包:  50ms亮灭闪烁三次 
无SIM卡：        2秒亮，1s灭,循环闪烁 
无GPRS网络:      200ms亮，1s灭,循环闪烁 
连接不上MQTT服务: 100ms亮，3s灭,循环闪烁   
 
*/

#define LED_IR_OVER_FLASH()				LED_STATUS_Flash_Start(200, 500, 30)
#define LED_MOTOR_OVER_FLASH()    LED_STATUS_Flash_Start(1000, 500, 3)

#define LED_NET_FLASH()           LED_NET_Flash_Start(50, 50, 3)
#define LED_NOCARD_FLASH()        LED_NET_Flash_Start(2000, 1000, LED_FLASH_CONTINUE)
#define LED_NONET_FLASH()         LED_NET_Flash_Start(200, 1000, LED_FLASH_CONTINUE)
#define LED_NOMQTT_FLASH()        LED_NET_Flash_Start(100, 3000, LED_FLASH_CONTINUE)

/* Exported functions --------------------------------------------------------*/
void Control_Init(void);

void Borrow_Action(uint8_t* dat);
void Repay_Action(void);
void Stop_Action(void);

void LED_NET_Flash_Start(uint16_t activetime, uint16_t idletime, uint16_t times);
void LED_STATUS_Flash_Start(uint16_t activetime, uint16_t idletime, uint16_t times);

void WatchDog_Clear(void);

#endif /* _Control_H */

