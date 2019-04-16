/**
  ******************************************************************************
  * @file    touchpad.h
  * @author  宋阳
  * @version V1.0
  * @date    2017.12.15
  * @brief   业务逻辑函数头文件
  ******************************************************************************
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _PROFESSION_H
#define _PROFESSION_H

/* Includes ------------------------------------------------------------------*/
#include "prjlib.h"

/* Exported define -----------------------------------------------------------*/

/*定时M1卡循环唤醒周期,单位ms*/
#define M1_CARD_WAKEUP_PERIOD			500

/*M1卡上电延时时间，单位us*/
#define MFRC_POWERON_DELAY				500

/*指纹模块上电延时时间，单位ms*/
#define FPG_POWERON_DELAY					200

/* Exported types ------------------------------------------------------------*/

/*系统状态定义*/
typedef enum
{
	m_idle,
	m_open_FPG,
	m_open_M1,
	m_open_PWD,
	m_open_BLE,
	m_open_Empty,
	m_set_FPG,
	m_set_M1,
	m_set_PWD,
} MStatus_t;



/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/
extern MStatus_t mStatus;

/* Exported functions --------------------------------------------------------*/
void Profession_init(void);

void Profession_Polling(void);

int Search_PasswordIndex(uint8_t *pwd, uint8_t len);
int Search_M1Card(uint32_t uid); 

#endif


/************************ (C)  *****END OF FILE****/

