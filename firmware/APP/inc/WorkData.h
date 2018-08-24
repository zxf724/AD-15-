/**
  ******************************************************************************
  * @file    WorkData.h
  * @author  ����
  * @version V1.0
  * @date    2015.12.31
  * @brief   Header file of WorkData
  ******************************************************************************
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _WorkData_H
#define _WorkData_H



/* Includes ------------------------------------------------------------------*/
#include "prjlib.h"
#include "pstorage.h"

/* Exported types ------------------------------------------------------------*/
/*�̼��汾,2BYTE*/
#define VERSION			        1
/*Ӳ���汾*/
#define VERSION_HARDWARE    "AD15_EVT"

/*�����汾*/
#define WORKDATA_VERSION	  1

/*������־������������*/
#define LOG_STORE_MAX				1280

/*ɡ�ܵ�Ĭ������*/
#define UMBRELLA_STOCK_DEF  16

/*Ĭ�Ϸ�������ַ*/
#define SERVER_DEF          "120.25.201.201"

/*Ĭ�϶˿�*/
#define PORT_DEF            9907

/*Ĭ���������*/
#define HEARTBEAT_DEF       60

/*Ĭ���豸ID*/
#define DEVICEID_DEF        80000000

/*�����豸�����ṹ,����4�ֽڶ���*/
typedef struct
{
  uint16_t  crc; 
  uint16_t  version;
  uint32_t	DeviceID;
  uint16_t  StockCount;
  uint16_t  StockMax;
  char      MQTT_Server[32];
  uint16_t  MQTT_Port;
  uint16_t  MQTT_PingInvt;
  uint16_t  StoreLog_In;
  uint16_t  StoreLog_Report;
} WorkParam_t;

/*���廹ɡ��¼�ṹ*/
typedef struct
{
  uint32_t time;
  uint32_t rfid;
} StoreLog_t;

/* Exported constants --------------------------------------------------------*/
extern WorkParam_t        WorkData;
extern pstorage_handle_t  psLog;

/* Exported macro ------------------------------------------------------------*/
#define STROE_LOG_POINT(i)				((StoreLog_t*)((uint8_t *)psLog.block_id + sizeof(StoreLog_t) * i))

/* Exported functions --------------------------------------------------------*/
void WorkData_Init(void);
void DataBackInit(BOOL reset);
BOOL WorkData_Update(void);

BOOL Write_StoreLog(uint32_t rfid);

uint32_t Wait_For_FlashOP(uint32_t op_in);

#endif /* _WorkData_H */

/************************ (C) *****END OF FILE****/

