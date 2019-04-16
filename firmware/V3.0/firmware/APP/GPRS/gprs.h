/**
 * **********************************************************************
 *             Copyright (c) 2016 temp. All Rights Reserved.
 * @file gprs.h
 * @author 宋阳
 * @version V1.0
 * @date 2016.4.1
 * @brief GPRS驱动函数头文件.
 *
 * **********************************************************************
 * @note
 *
 * **********************************************************************
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _GPRS_H
#define _GPRS_H


/* Includes ------------------------------------------------------------------*/
#include "prjlib.h"

/* Exported define -----------------------------------------------------------*/
#define GPRS_SEND_MAX_SIZE          512
#define GPRS_RECEIVE_MAX_SIZE       512

/*GPRS省电关机的时间，单位秒*/
#define GPRS_POWER_SAVE_TIME        60

/*定义GPRS模块控制选项*/
#define GPRS_OPT_RESET              0x01
#define GPRS_OPT_SET_SOCKET         0x04
#define GPRS_OPT_SET_APN            0x08

/*GPRS调试开关*/
#define GPRS_DEBUG                  2

/* Exported types ------------------------------------------------------------*/
typedef enum {
    gprs_status_poweroff = 0,
    gprs_status_powerkey,
    gprs_status_poweron,
    gprs_status_nocard,
    gprs_status_nonet,
    gprs_status_online,
    gprs_status_fault
} GPRS_Status_t;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
void GPRSInit(void);

void GPRSPolling(void);

void GPRS_ReStart(void);
void GPRS_SetOnOff(BOOL onoff);
void GPRS_SetSleep(BOOL sleepen);
GPRS_Status_t GPRS_ReadStatus(void);

uint8_t GPRS_ReadRSSI(void);
void GPRS_TTS(char* text);

int16_t GPRS_SocketSendData(uint8_t *data, uint16_t len);
int8_t GPRS_IsSocketConnect(void);
void GPRS_SetSocketParam(char *server, uint16_t port);
int16_t GPRS_SocketReadData(uint8_t* data, uint16_t len);
uint16_t GPRS_SocketRecDataLength(void);
uint32_t GPRS_ReadRFID(uint8_t num);

#endif


