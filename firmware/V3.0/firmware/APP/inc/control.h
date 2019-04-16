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
typedef enum {
    k_status_idle,
    //output unbrella
    k_status_output_unbrella_success,
    k_status_take_the_unbrella_soon,
    k_status_have_no_unbrella,
    k_status_start_output_unbrella,
    //input unbrella
    k_status_start_input_unbrella,
    k_status_input_unbrella_success,
    k_status_do_not_occlusion_door,
    k_status_input_unbrella_soon,
    k_status_full_unbrella,
    //input breakdown unbrella
    k_status_input_breakdown_unbrella,
    k_status_restart_ouput,
    k_status_report_breakdown,
    //other situation
    k_status_motor_stuck,
    k_status_ir_stuck,
    k_status_timeout,
} motor_status_Enum;

/* Exported constants --------------------------------------------------------*/
/*���ת����ʱ�䣬��λms*/
#define MOTOR_ACTION_TIME     10
#define MOVE_ACTION_TIME     100
#define TMP_TIME             500
#define TIME_RFID_READ       800

/*���ת����ʱ����*/
#define MOTOR_OVERFLOW_TIMES  6000

/*�������ļ��ʱ��,��λms*/
#define BAT_CHECK_TIME        15000

extern motor_status_Enum Motor_staus;

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
void ControlInit(void);

void ControlPolling(void);

void Borrow_Action(void);
void Repay_Action(void);
void Stop_Action(uint8_t num);

void LED_NET_Flash_Start(uint16_t activetime, uint16_t idletime, uint16_t times);
void LED_STATUS_Flash_Start(uint16_t activetime, uint16_t idletime, uint16_t times);

void TTS_Play(char *text);

void WatchDogClear(void);
uint8_t gs_RFID_ReadPoll(uint8_t addr, uint8_t** data);

void Breakdown_Repay(void);
void Motor_Re_Fun(void);
void TestFun(void);
void Reset(void);
void InitFlag(void);

#endif /* _Control_H */

