/**
 * **********************************************************************
 *             Copyright (c) 2016 temp. All Rights Reserved.
 * @file Process.h
 * @author ����
 * @version V1.0
 * @date 2016.8.31
 * @brief ҵ���߼�������ͷ�ļ�.
 *
 * **********************************************************************
 * @note
 *
 * **********************************************************************
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _PROCESS_H
#define _PROCESS_H


/* Includes ------------------------------------------------------------------*/
#include "prjlib.h"
#include "cjson.h"
#include "control.h"

/* Exported define -----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
void Process_Init(void);

BOOL CMD_Updata(char* cmd, cJSON* desired);
void Status_Updata(void);

BOOL Report_Umbrella_Borrow_Status(uint32_t rfid, motor_status_t status);
BOOL Report_Umbrella_Repy_Status(uint32_t rfid, motor_status_t status, uint32_t ts);

#endif
