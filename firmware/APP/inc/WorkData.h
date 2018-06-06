/**
  ******************************************************************************
  * @file    WorkData.h
  * @author  宋阳
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
/*固件版本,2BYTE*/
#define VERSION			3

/*参数版本*/
#define WORKPARAM_VERSION	1

/*缓存大小，用于保存参数时临时写入，需大于最大的数据段*/
#define WORK_BUFFER_SIZE	256

/*开锁功能使能掩码*/
#define OPEN_FUN_FPG			0x01
#define OPEN_FUN_PWD			0x02
#define OPEN_FUN_M1				0x04
#define OPEN_FUN_BLE			0x08

/*定义开门卡的最大张数*/
#define M1_CARD_STORE_MAX	120

/*定义指纹库的最大数量*/
#define FPG_STORE_MAX			120

/*定义密码的最大数量*/
#define PWD_STORE_MAX			32

/*密码最大与最小长度*/
#define PWD_SIZE_MAX			12
#define PWD_SIZE_LIM			4

/*开锁日志保存的最大数量*/
#define LOCKLOG_MAX				3650

/*定义设备参数结构*/
/*未使用*/
typedef struct
{
  uint8_t version;
  uint8_t	openFunctionEn;
  uint8_t Password[PWD_STORE_MAX][PWD_SIZE_MAX];
  uint32_t Password_TS[PWD_SIZE_MAX];
  uint32_t M1_Card_UID[M1_CARD_STORE_MAX];
  uint32_t M1_Card_TS[M1_CARD_STORE_MAX];
  uint8_t  FPG_IndexUsed[FPG_STORE_MAX];
  uint32_t FPG_TS[FPG_STORE_MAX];
} WorkParam_t;

/*定义开锁记录结构*/
typedef struct
{
  uint32_t time;
  uint32_t arg;
  uint8_t opentype;
  uint8_t res;     /*补齐字对齐*/
  uint16_t crc;
} LockLog_t;

/*定义DFU记录结构*/
typedef struct
{
  uint32_t newver;
  uint32_t size;
  uint32_t crc;
} DFU_Info_t;

/* Exported constants --------------------------------------------------------*/
extern uint8_t WorkBuf[];
extern pstorage_handle_t  psLockLog,Bock0,Bock1,Bock2,Bock3,Bock4;


/* Exported macro ------------------------------------------------------------*/

#define WDATA_VERSION							(*(uint32_t*)(((uint8_t *)Bock0.block_id) + 0))
#define WDATA_VERSION_V						(*(uint32_t*)(((uint8_t *)Bock0.block_id) + 4))
#define WDATA_VERSION_T						(*(uint32_t*)&WorkBuf[0])
#define WDATA_VERSION_LENGTH			4
#define WDATA_VERSION_COPY()			(WDATA_VERSION_T = WDATA_VERSION)
#define WDATA_VERSION_SAVE()			do {if (pstorage_update(&Bock0, WorkBuf,\
																	WDATA_VERSION_LENGTH, 0)== NRF_SUCCESS)\
																	Wait_For_FlashOP(PSTORAGE_UPDATE_OP_CODE);\
																	WDATA_VERSION_T += 0x435;\
																	if (pstorage_update(&Bock0, WorkBuf,\
																	WDATA_VERSION_LENGTH, 4)== NRF_SUCCESS)\
																	Wait_For_FlashOP(PSTORAGE_UPDATE_OP_CODE);} while (0)

#define WDATA_FUNCTION						(*(uint32_t*)(((uint8_t *)Bock0.block_id) + 8))
#define WDATA_FUNCTION_T					(*(uint32_t*)&WorkBuf[0])
#define WDATA_FUNCTION_LENGTH			4
#define WDATA_FUNCTION_COPY()			(WDATA_FUNCTION_T = WDATA_FUNCTION)
#define WDATA_FUNCTION_SAVE()			do {if (pstorage_update(&Bock0, WorkBuf,\
																	WDATA_FUNCTION_LENGTH, 8)== NRF_SUCCESS)\
																	Wait_For_FlashOP(PSTORAGE_UPDATE_OP_CODE);} while (0)


#define WDATA_DFU									(*(DFU_Info_t*)(((uint8_t *)Bock0.block_id) + 12))
#define WDATA_DFU_T								(*(DFU_Info_t*)&WorkBuf[0])
#define WDATA_DFU_LENGTH					12
#define WDATA_DFU_COPY()					(WDATA_DFU_T = WDATA_DFU)
#define WDATA_DFU_SAVE()					do {if (pstorage_update(&Bock0, WorkBuf,\
																	WDATA_DFU_LENGTH, 12)== NRF_SUCCESS)\
																	Wait_For_FlashOP(PSTORAGE_UPDATE_OP_CODE);} while (0)

#define WDATA_KEYS								((uint8_t*)(((uint8_t *)Bock0.block_id) + 24))
#define WDATA_KEYS_T							((uint8_t*)&WorkBuf[0])
#define WDATA_KEYS_LENGTH					128
#define WDATA_KEYS_SAVE()					do {if (pstorage_update(&Bock0, WorkBuf,\
																	WDATA_KEYS_LENGTH, 24)== NRF_SUCCESS)\
																	Wait_For_FlashOP(PSTORAGE_UPDATE_OP_CODE);} while (0)


#define WDATA_PWD(i)							((char*)(((uint8_t *)Bock1.block_id) + WDATA_PWD_LENGTH * i))
#define WDATA_PWD_T								((char*)&WorkBuf[0])
#define WDATA_PWD_LENGTH     			PWD_SIZE_MAX
#define WDATA_PWD_COPY(i)					do {strncpy(WDATA_FPWD_T, WDATA_PWD(i), WDATA_PWD_LENGTH);} while (0)
#define WDATA_PWD_SAVE(i)					do {if (pstorage_update(&Bock1, WorkBuf,\
																	WDATA_PWD_LENGTH, WDATA_PWD_LENGTH * i)== NRF_SUCCESS)\
																	Wait_For_FlashOP(PSTORAGE_UPDATE_OP_CODE);} while (0)


#define WDATA_PWD_TS(i)						(*(uint32_t*)(((uint8_t *)Bock1.block_id) + 384 + WDATA_PWD_TS_LENGTH * i))
#define WDATA_PWD_TS_T						(*(uint32_t*)&WorkBuf[0])
#define WDATA_PWD_TS_LENGTH				4
#define WDATA_PWD_TS_COPY(i)			(WDATA_PWD_TS_T = WDATA_PWD_TS(i))
#define WDATA_PWD_TS_SAVE(i)			do {if (pstorage_update(&Bock1, WorkBuf,\
																	WDATA_PWD_TS_LENGTH, 384 + WDATA_PWD_TS_LENGTH * i)== NRF_SUCCESS)\
																	Wait_For_FlashOP(PSTORAGE_UPDATE_OP_CODE);} while (0)

#define WDATA_M1_UID(i)						(*(uint32_t*)(((uint8_t *)Bock2.block_id) + WDATA_M1_UID_LENGTH * i))
#define WDATA_M1_UID_T						(*(uint32_t*)&WorkBuf[0])
#define WDATA_M1_UID_LENGTH				4
#define WDATA_M1_UID_COPY(i)			(WDATA_M1_UID_T = WDATA_M1_UID(i))
#define WDATA_M1_UID_SAVE(i)			do {if (pstorage_update(&Bock2, WorkBuf,\
																	WDATA_M1_UID_LENGTH, WDATA_M1_UID_LENGTH * i)== NRF_SUCCESS)\
																	Wait_For_FlashOP(PSTORAGE_UPDATE_OP_CODE);} while (0)

#define WDATA_M1_TS(i)						(*(uint32_t*)(((uint8_t *)Bock3.block_id) + WDATA_M1_TS_LENGTH * i))
#define WDATA_M1_TS_T							(*(uint32_t*)&WorkBuf[0])
#define WDATA_M1_TS_LENGTH				4
#define WDATA_M1_TS_COPY(i)				(WDATA_M1_TS_T = WDATA_M1_TS(i))
#define WDATA_M1_TS_SAVE(i)				do {if (pstorage_update(&Bock3, WorkBuf,\
																	WDATA_M1_TS_LENGTH, WDATA_M1_TS_LENGTH * i)== NRF_SUCCESS)\
																	Wait_For_FlashOP(PSTORAGE_UPDATE_OP_CODE);} while (0)

#define WDATA_FPG_TS(i)						(*(uint32_t*)(((uint8_t *)Bock4.block_id) + WDATA_FPG_TS_LENGTH * i))
#define WDATA_FPG_TS_T						(*(uint32_t*)&WorkBuf[0])
#define WDATA_FPG_TS_LENGTH				4
#define WDATA_FPG_TS_COPY(i)			(WDATA_FPG_TS_T = WDATA_FPG_TS(i))
#define WDATA_FPG_TS_SAVE(i)			do {if (pstorage_update(&Bock4, WorkBuf,\
																	WDATA_FPG_TS_LENGTH, WDATA_FPG_TS_LENGTH * i)== NRF_SUCCESS)\
																	Wait_For_FlashOP(PSTORAGE_UPDATE_OP_CODE);} while (0)


#define P_LOCKLOG(i)							((LockLog_t*)(((uint8_t *)psLockLog.block_id) + sizeof(LockLog_t) * i))

/* Exported functions --------------------------------------------------------*/
void WorkData_Init(void);

uint32_t Wait_For_FlashOP(uint32_t op_in);

void DataBackInit(BOOL reset);

int32_t Save_LockLog(LockLog_t* log);
BOOL LockLog_SaveToBlock(uint16_t block, LockLog_t* log);
BOOL LockLog_ReadFromBlock(uint16_t block, LockLog_t* log);


void Get_LockLog_Index(uint32_t* start, uint32_t* insert);


#endif /* _WorkData_H */

/************************ (C) *****END OF FILE****/

