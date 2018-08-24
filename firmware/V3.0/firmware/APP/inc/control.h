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
  status_empty,
  status_full,
} motor_status_t;

/* Exported constants --------------------------------------------------------*/
/*电机转动的时间，单位ms*/
#define MOTOR_ACTION_TIME     10
#define MOVE_ACTION_TIME     100
/*电机转动超时次数*/
#define MOTOR_OVERFLOW_TIMES  6000

/*电量检测的间隔时间,单位ms*/
#define BAT_CHECK_TIME        15000

extern  uint16_t BatVol;

extern motor_status_t Motor_staus;

#define LED_FLASH_CONTINUE    0xFFFF

/* Exported macro ------------------------------------------------------------*/

/*
LED定义： 
 
1. 状态指示灯
取伞或还伞:        长亮 
红外遮挡:          100ms亮, 100ms灭,闪烁30次
电机过流：         1s亮，500ms灭,闪烁5次
无伞可取或伞架满：   200ms亮, 1s灭,闪烁5次
 
1. 网络指示灯
蓝牙/MQTT收发包:  50ms亮灭闪烁三次 
无SIM卡：        2秒亮，3s灭,循环闪烁 
无GPRS网络:      200ms亮，3s灭,循环闪烁 
连接不上MQTT服务: 100ms亮，5s灭,循环闪烁   
 
*/

#define LED_IR_OVER_FLASH()				LED_STATUS_Flash_Start(100, 100, 30)
#define LED_MOTOR_OVER_FLASH()    LED_STATUS_Flash_Start(1000, 500, 5)
#define LED_MOTOR_NG()            LED_STATUS_Flash_Start(200, 1000, 5)

#define LED_NET_FLASH()           LED_NET_Flash_Start(50, 50, 3)
#define LED_NOCARD_FLASH()        LED_NET_Flash_Start(2000, 3000, LED_FLASH_CONTINUE)
#define LED_NONET_FLASH()         LED_NET_Flash_Start(200, 3000, LED_FLASH_CONTINUE)
#define LED_NOMQTT_FLASH()        LED_NET_Flash_Start(100, 5000, LED_FLASH_CONTINUE)

/* Exported functions --------------------------------------------------------*/
void Control_Init(void);

void Control_Polling(void);

void Borrow_Action(void);
void Repay_Action(void);
void Stop_Action(void);

void LED_NET_Flash_Start(uint16_t activetime, uint16_t idletime, uint16_t times);
void LED_STATUS_Flash_Start(uint16_t activetime, uint16_t idletime, uint16_t times);

void TTS_Play(char *text);

void WatchDog_Clear(void);
uint8_t RFID_ReadPoll(uint8_t addr, uint8_t** data);

#endif /* _Control_H */

