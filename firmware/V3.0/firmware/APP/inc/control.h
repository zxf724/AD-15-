/**
  ******************************************************************************
  * @file    control.c
  * @author  ����
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
/*���ת����ʱ�䣬��λms*/
#define MOTOR_ACTION_TIME     10
#define MOVE_ACTION_TIME     100
/*���ת����ʱ����*/
#define MOTOR_OVERFLOW_TIMES  6000

/*�������ļ��ʱ��,��λms*/
#define BAT_CHECK_TIME        15000

extern  uint16_t BatVol;

extern motor_status_t Motor_staus;

#define LED_FLASH_CONTINUE    0xFFFF

/* Exported macro ------------------------------------------------------------*/

/*
LED���壺 
 
1. ״ָ̬ʾ��
ȡɡ��ɡ:        ���� 
�����ڵ�:          100ms��, 100ms��,��˸30��
���������         1s����500ms��,��˸5��
��ɡ��ȡ��ɡ������   200ms��, 1s��,��˸5��
 
1. ����ָʾ��
����/MQTT�շ���:  50ms������˸���� 
��SIM����        2������3s��,ѭ����˸ 
��GPRS����:      200ms����3s��,ѭ����˸ 
���Ӳ���MQTT����: 100ms����5s��,ѭ����˸   
 
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

