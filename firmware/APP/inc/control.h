/**
  ******************************************************************************
  * @file    control.c
  * @author  宋阳
  * @version V1.0
  * @date    2015.12.4
  * @brief   led driver.
  * @brief   Header file of Control
  ******************************************************************************
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _Control_H
#define _Control_H

/* Includes ------------------------------------------------------------------*/
#include "includes.h"

/** @addtogroup firmwave_F2_BLE
  * @{
  */

/** @addtogroup Control
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup Control_Exported_Types Control Exported Types
  * @{
  */

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/

/** @defgroup Control_Exported_Constants Control Exported Constants
  * @{
  */
#define BUZ_TIMER           NRF_TIMER1

#define DELAY_TIMER         NRF_TIMER2

/*电机转动的时间，单位ms*/
#define CMD_ACTION_TIME   200
#define RFID_ACTION_TIME   1500
#define  MOTOR_ACTION_TIME   3000

#define MOTOR_STOP_TIME   	5000

/*电量检测的间隔时间,单位ms*/
#define BAT_CHECK_TIME      15000


extern uint16_t BatVol;
extern  uint8_t RFID_SendCmd_Flag;
extern  uint8_t  RFID_ReceiveData_Flag;
/**
  * @}
  */


/* Exported macro ------------------------------------------------------------*/
/** @defgroup Control_Exported_Macros Control Exported Macros
  * @{
  */
typedef struct
{
    uint16_t activetime;
    uint16_t idletime;
    uint16_t times;
    uint16_t index;
    BOOL active;
} Period_Block_t;



#define LED_SETTING_FLASH()				LED_FLAG_Flash_Start(100, 400, 10)

typedef void (*USDelay_CB)(void);

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup Control_Exported_Functions
  * @{
  */
void Control_Init(void);
//void WatchDog_Clear(void);

void Delay_Us(uint16_t us, USDelay_CB cb);
void RFID_ReceiveDataCheck(uint8_t *dat);
void Borrow_Action(uint8_t *dat);
void Repay_Action(void);
void Stop_Action(void);


/**
  * @}
  */



/**
  * @}
  */

/**
  * @}
  */



#endif /* _Control_H */

