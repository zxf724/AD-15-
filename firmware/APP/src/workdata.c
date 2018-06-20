/**
  ******************************************************************************
  * @file    WorkData.c
  * @author  宋阳
  * @version V1.0
  * @date    2016.1.4
  * @brief   工作数据存储相关函数.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "includes.h"
/** @addtogroup firmwave_F2_BLE
  * @{
  */



/** @defgroup WorkData
  * @{
  */


/* Private typedef -----------------------------------------------------------*/

/** @defgroup WorkData_Private_Typedef WorkData Private Typedef
  * @{
  */

/**
  * @}
  */

/* Private define ------------------------------------------------------------*/
/** @defgroup WorkData_Private_Constants WorkData Private Constants
  * @{
  */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup WorkData_Private_Macros WorkData Private Macros
  * @{
  */


/**
  * @}
  */
/* Private variables ---------------------------------------------------------*/

/** @defgroup WorkData_Private_Variables Private Variables
  * @{
  */
pstorage_handle_t  psLog, psWorkData;

uint32_t volatile op = 0, res = 0;
uint32_t logBlock = 0;

WorkParam_t WorkData;
/**
  * @}
  */

/* Private function prototypes -----------------------------------------------*/
/** @defgroup WorkData_Private_Functions WorkData Private Functions
  * @{
  */
static void pstorage_cb_handler(pstorage_handle_t* p_handle,
                                uint8_t op_code, uint32_t result, uint8_t* p_data, uint32_t data_len);

static void funWorkBack(int argc, char* argv[]);


/**
  * @}
  */

/* Exported functions ---------------------------------------------------------*/

/** @defgroup WorkData_Exported_Functions WorkData Exported Functions
  *  @brief   WorkData 外部接口函数
  * @{
  */

/**
  * @brief  工作数据初始化.
  * @param  none.
  * @retval none
  */
void WorkData_Init(void) {
  pstorage_module_param_t param;

  pstorage_init();

  /*注册pstorage */
  param.block_size = sizeof(StoreLog_t);
  param.block_count = LOG_STORE_MAX;
  param.cb = pstorage_cb_handler;
  pstorage_register(&param, &psLog);
  DBG_LOG("psLog id:%#x", psLog.block_id);

  param.block_size = PSTORAGE_FLASH_PAGE_SIZE;
  param.block_count = 1;
  param.cb = pstorage_cb_handler;
  pstorage_register(&param, &psWorkData);
  DBG_LOG("psWorkData id:%#x", psWorkData.block_id);

  DataBackInit(FALSE);

  CMD_ENT_DEF(workdata, funWorkBack);
  Cmd_AddEntrance(CMD_ENT(workdata));

  DBG_LOG("WorkData Init.");
}

/**
 * 工作数据存储更新
 */
BOOL WorkData_Update(void) {
  uint16_t crc = 0;

  crc = CRC_16(0, ((uint8_t*)&WorkData) + 2, sizeof(WorkData) - 2);
  WorkData.crc = crc;

  if (pstorage_update(&psWorkData, (uint8_t*)&WorkData, sizeof(WorkData), 0) == NRF_SUCCESS) {
    Wait_For_FlashOP(PSTORAGE_UPDATE_OP_CODE);
    DBG_LOG("Work Param Update OK.");
    return TRUE;
  }
  return FALSE;
}
/**
  * @brief  通用数据初始化.
  */
void DataBackInit(BOOL reset) {
  uint16_t crc = 0;
  uint16_t ver = 0;

  if (reset) {
    ver = 0;
    DBG_LOG("Work Data Factory Reset.");
  }
  if (pstorage_load((uint8_t*)&WorkData, &psWorkData, sizeof(WorkData), 0) == NRF_SUCCESS) {
    ver = WorkData.version;
    if (ver == 0xFFFF) {
      ver = 0;
      DBG_LOG("Work Data Not Init.");
    }
    crc = CRC_16(0, ((uint8_t*)&WorkData) + 2, sizeof(WorkData) - 2);
    if (crc != WorkData.crc) {
      ver = 0;
      DBG_LOG("Work Data CRC Fault.");
    }
    if (ver > WORKDATA_VERSION) {
      ver = 0;
    }
    if (ver != WORKDATA_VERSION) {
      if (ver < 1) {
        WorkData.version = WORKDATA_VERSION;
        WorkData.DeviceID = DEVICEID_DEF;
        WorkData.StockCount = 0;
        WorkData.StockMax = UMBRELLA_STOCK_DEF;
        strcpy(WorkData.MQTT_Server, SERVER_DEF);
        WorkData.MQTT_Port = PORT_DEF;
        WorkData.MQTT_PingInvt = HEARTBEAT_DEF;
        WorkData.StoreLog_In = 0;
        WorkData.StoreLog_Report = 0;

        WorkData_Update();
        DBG_LOG("Work version1 default.");
      }
    } else {
      DBG_LOG("Work data load OK.");
    }
  }
}

/**
 * 新增一条还伞记录
 * 
 * @param rfid   伞的RFID号
 * @return 返回存储结果
 */
BOOL Write_StoreLog(uint32_t rfid) {
  BOOL ret = FALSE;
  StoreLog_t log;
  pstorage_handle_t mps;
  uint16_t logBlock = WorkData.StoreLog_In;

  if (IS_RTC_TIME_CALIB()) {
    logBlock++;
    /*越界回滚*/
    if (logBlock >= LOG_STORE_MAX) {
      logBlock = 0;
    }
    log.time = RTC_ReadCount();
    log.rfid = rfid;
    pstorage_block_identifier_get(&psLog, logBlock, &mps);
    if (pstorage_update(&mps, (uint8_t*)&log, sizeof(StoreLog_t), 0) == NRF_SUCCESS) {
      Wait_For_FlashOP(PSTORAGE_UPDATE_OP_CODE);
      ret = TRUE;
      DBG_LOG("Write_StoreLog index:%u, time:%u, rfid:%#x.", logBlock, log.time, log.rfid);
      WorkData.StoreLog_In = logBlock;
      WorkData_Update();
    }
  }
  return ret;
}

/**
 * 等等flash擦写完成，用于非阻塞的方式实现flash操作，pstorage是非阻塞的
 * 
 * @param op_in  等待的动作
 * @return 返回结果
 */
uint32_t Wait_For_FlashOP(uint32_t op_in) {
  op = op_in;
  res = 0xFFFFFFF;

  while (res == 0xFFFFFFF);

  return res;
}

/** @addtogroup WorkData_Private_Functions
  * @{
  */

static void pstorage_cb_handler(pstorage_handle_t* p_handle,
                                uint8_t op_code, uint32_t result, uint8_t* p_data, uint32_t data_len) {
  if (op == op_code) {
    res = result;
  }
}

/**
  * @brief  备份数据调试命令.
  */
static void funWorkBack(int argc, char* argv[]) {
  argv++;
  argc--;

  StoreLog_t* log;
  if (ARGV_EQUAL("readlog")) {
    log = STROE_LOG_POINT(uatoi(argv[1]));
    DBG_LOG("readlog time:%u, rfid：%#x.",
            log->time, log->rfid);
  } else if (ARGV_EQUAL("writelog")) {
    Write_StoreLog(uatoix(argv[1]));
    DBG_LOG("writelog OK.");
  } else if (ARGV_EQUAL("logindex")) {
    DBG_LOG("Stroe Loog Record:%u, Report:%u.", WorkData.StoreLog_In, WorkData.StoreLog_Report);
  } else if (ARGV_EQUAL("deviceid")) {
    if (argv[1] != NULL) {
      WorkData.DeviceID = uatoi(argv[1]);
      WorkData_Update();
    }
    DBG_LOG("Device ID:%u", WorkData.DeviceID);
  } else if (ARGV_EQUAL("ip")) {
    if (argv[1] != NULL) {
      strcpy(WorkData.MQTT_Server, argv[1]);
      WorkData_Update();
    }
    DBG_LOG("MQTT connect server:%s", WorkData.MQTT_Server);
  } else if (ARGV_EQUAL("port")) {
    if (argv[1] != NULL) {
      WorkData.MQTT_Port = uatoi(argv[1]);
      WorkData_Update();
    }
    DBG_LOG("MQTT connect port:%u", WorkData.MQTT_Port);
  }
}


/**
  * @}
  */



/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT  *****END OF FILE****/
