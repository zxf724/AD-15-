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
pstorage_handle_t  psLockLog,Bock0,Bock1,Bock2,Bock3,Bock4;

uint8_t WorkBuf[WORK_BUFFER_SIZE];
uint32_t volatile op = 0, res = 0;
uint32_t logBlock = 0;
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
  * @brief  初始化.
  * @param  none.
  * @retval none
  */
void WorkData_Init(void) 
{
  pstorage_module_param_t param;

  pstorage_init();

  /* 注册pstorage */
  param.block_size = sizeof(LockLog_t);
  param.block_count = 3650;
  param.cb = pstorage_cb_handler;
  pstorage_register(&param, &psLockLog);
  DBG_LOG("psLockLog id:%#x", psLockLog.block_id);

  param.block_size = 512;
  param.block_count = 8;
  param.cb = pstorage_cb_handler;
  pstorage_register(&param, &Bock0);
  DBG_LOG("Bock0 id:%#x", Bock0.block_id);
  pstorage_block_identifier_get(&Bock0, 1, &Bock1);
  DBG_LOG("Bock1 id:%#x", Bock1.block_id);
  pstorage_block_identifier_get(&Bock0, 2, &Bock2);
  DBG_LOG("Bock2 id:%#x", Bock2.block_id);
  pstorage_block_identifier_get(&Bock0, 3, &Bock3);
  DBG_LOG("Bock3 id:%#x", Bock3.block_id);
  pstorage_block_identifier_get(&Bock0, 4, &Bock4);
  DBG_LOG("Bock4 id:%#x", Bock4.block_id);

  DataBackInit(FALSE);

  CMD_ENT_DEF(workdata, funWorkBack);
  Cmd_AddEntrance(CMD_ENT(workdata));

  Get_LockLog_Index(NULL, &logBlock);
  DBG_LOG("Locklog Store Block:%u, logsize:%u.", logBlock, sizeof(LockLog_t));

  DBG_LOG("WorkData Init.");
}


/**
 * 等等flash擦写完成，用于非阻塞的方式实现flash操作，pstorage是非阻塞的
 * 
 * @param op_in  等待的动作
 * @return 返回结果
 */
uint32_t Wait_For_FlashOP(uint32_t op_in) 
{
  op = op_in;
  res = 0xFFFFFFF;

  while (res == 0xFFFFFFF);

  return res;
}

/**
 * 循环覆盖方式保存开锁日志
 * 
 * @param log    待存储的日志
 * @return 返回存储的块,存储失败返回-1
 */
int32_t Save_LockLog(LockLog_t* log) 
{
  int32_t ret = -1;

  if (IS_RTC_TIME_CALIB() && LockLog_SaveToBlock(logBlock, log)) 
  {
    ret = logBlock;
    logBlock++;
    /*回滚*/
    if (logBlock >= LOCKLOG_MAX) 
    {
      logBlock = 0;
    }
  }
  return ret;
}

/**
 * 开锁日志保存至存储块中
 * 
 * @param block  存储块
 * @param log    待存储的日志
 * @return 保存成功返回TRUE
 */
BOOL LockLog_SaveToBlock(uint16_t block, LockLog_t* log) 
{

  BOOL ret = FALSE;
  uint16_t crc = 0;
  pstorage_handle_t mps;

  crc = CRC_16(0, (uint8_t*)log, 9);
  log->crc = crc;
  log->res = 0;

  pstorage_block_identifier_get(&psLockLog, block, &mps);
  if (pstorage_update(&mps, (uint8_t*)log, sizeof(LockLog_t), 0) == NRF_SUCCESS) 
  {
    Wait_For_FlashOP(PSTORAGE_UPDATE_OP_CODE);
    ret = TRUE;
  }
  return ret;
}

/**
 * 从存储块中读出开锁日志
 * 
 * @param block  存储块
 * @param log    读出的日志
 * @return 日志正确返回TRUE
 */
BOOL LockLog_ReadFromBlock(uint16_t block, LockLog_t* log) 
{
  if (P_LOCKLOG(block)->crc != BIT16_MAX && P_LOCKLOG(block)->crc != 0 &&
      CRC_16(0, (uint8_t*)P_LOCKLOG(block), 9) == P_LOCKLOG(block)->crc)
  {
    if (log != NULL) 
    {
      *log = *P_LOCKLOG(block);
    }
    return TRUE;
  }
  return FALSE;
}

/**
 * 获取开锁日志记录记数
 * 
 * @param start  返回开始的块
 * @param end    返回结束的块
 */
void Get_LockLog_Index(uint32_t* start, uint32_t* insert) 
{
  uint32_t istart = 0, iend = 0, i;
  uint32_t tsmin = BIT32_MAX, tsmax = 0;

  for (i = 0; i < LOCKLOG_MAX; i++) 
  {
    if (P_LOCKLOG(i)->crc != BIT16_MAX && P_LOCKLOG(i)->crc != 0 &&
        CRC_16(0, (uint8_t*)P_LOCKLOG(i), 9) == P_LOCKLOG(i)->crc) 
    {
      if (P_LOCKLOG(i)->time <= tsmin) 
      {
        tsmin = P_LOCKLOG(i)->time;
        istart = i;
      }
      if (P_LOCKLOG(i)->time >= tsmax) 
      {
        tsmax = P_LOCKLOG(i)->time;
        iend = i + 1;
        if (iend >= LOCKLOG_MAX) 
        {
          iend = 0;
        }
      }
    }
  }
  if (start != NULL) 
  {
    *start = istart;
  }
  if (insert != NULL) 
  {
    *insert = iend;
  }
}

/**
  * @brief  通用数据初始化.
  */
void DataBackInit(BOOL reset)
{

  uint32_t ver = WDATA_VERSION;
  if (ver == (WDATA_VERSION_V - 0x435) && reset == FALSE) 
  {
    DBG_LOG("Load WDATA_VERSION:%u", WDATA_VERSION);
  } else if (reset == FALSE) {
    ver = 0;
    DBG_LOG("Work Data Break.");
  } else {
    ver = 0;
    DBG_LOG("Work Date Reset.");
  }

  if (ver != WORKPARAM_VERSION)
  {
    if (ver < 1) {
      pstorage_clear(&Bock0, 512);
      DBG_LOG("pstorage_clear bock0:%d.", Wait_For_FlashOP(PSTORAGE_CLEAR_OP_CODE));
      pstorage_clear(&Bock1, 512);
      DBG_LOG("pstorage_clear bock1:%d.", Wait_For_FlashOP(PSTORAGE_CLEAR_OP_CODE));
      pstorage_clear(&Bock2, 512);
      DBG_LOG("pstorage_clear bock2:%d.", Wait_For_FlashOP(PSTORAGE_CLEAR_OP_CODE));
      pstorage_clear(&Bock3, 512);
      DBG_LOG("pstorage_clear bock3:%d.", Wait_For_FlashOP(PSTORAGE_CLEAR_OP_CODE));
      pstorage_clear(&Bock4, 512);
      DBG_LOG("pstorage_clear bock4:%d.", Wait_For_FlashOP(PSTORAGE_CLEAR_OP_CODE));

      /*清空指纹库库*/

      nrf_delay_ms(FPG_POWERON_DELAY);
//      FPG_PS_Empty();


      WDATA_FUNCTION_T = 0x0F;
      WDATA_FUNCTION_SAVE();

      WDATA_VERSION_T = 1;
      WDATA_VERSION_SAVE();

      WDATA_DFU_T.size = 0;
      WDATA_DFU_T.crc = 0;
      WDATA_DFU_T.newver = VERSION;
      WDATA_DFU_SAVE(); 

//      memcpy(WDATA_KEYS_T, Keys_Defult, 128);
      WDATA_KEYS_SAVE();

      DBG_LOG("Work version1 default.");
    }
  } else {
    DBG_LOG("Work data init right.");
  }

  /*APP成功运行时刷新版本*/
  if (WDATA_DFU.newver != VERSION) {
    DBG_LOG("DFU refresh version:%u.", VERSION);
    WDATA_DFU_T.size = 0;
    WDATA_DFU_T.crc = 0;
    WDATA_DFU_T.newver = VERSION;
    WDATA_DFU_SAVE();
  }
}

/** @addtogroup WorkData_Private_Functions
  * @{
  */

static void pstorage_cb_handler(pstorage_handle_t* p_handle,
                                uint8_t op_code, uint32_t result, uint8_t* p_data, uint32_t data_len) 
{
  if (op == op_code) 
  {
    res = result;
  }
}

/**
  * @brief  备份数据调试命令.
  */
static void funWorkBack(int argc, char* argv[]) 
{
  int i = 0;
  LockLog_t log;

  argv++;
  argc--;

  if (ARGV_EQUAL("version")) 
  {
    DBG_LOG("WDATA_VERSION:%u", WDATA_VERSION);
  } 
  else if (ARGV_EQUAL("m1uid"))
  {
    i = uatoi(argv[1]);
    if (argv[2] != NULL) 
    {
      WDATA_M1_UID_T = uatoix(argv[2]);
      WDATA_M1_UID_SAVE(i);
    }
    DBG_LOG("M1_UID%d:%#x", i, WDATA_M1_UID(i));
  } 
  else if (ARGV_EQUAL("m1ts")) 
  {
    i = uatoi(argv[1]);
    if (argv[2] != NULL)
    {
      WDATA_M1_TS_T = uatoi(argv[2]);
      WDATA_M1_TS_SAVE(i);
    }
    DBG_LOG("M1_TS%d:%u", i, WDATA_M1_TS(i));
  } 
  else if (ARGV_EQUAL("pwdts")) 
  {
    i = uatoi(argv[1]);
    if (argv[2] != NULL) 
    {
      WDATA_PWD_TS_T = uatoi(argv[2]);
      WDATA_PWD_TS_SAVE(i);
    }
    DBG_LOG("PWD_TS%d:%u", i, WDATA_PWD_TS(i));
  } 
  else if (ARGV_EQUAL("fpgts")) 
  {
    i = uatoi(argv[1]);
    if (argv[2] != NULL) 
    {
      WDATA_FPG_TS_T = uatoi(argv[2]);
      WDATA_FPG_TS_SAVE(i);
    }
    DBG_LOG("FPG_TS%d:%u", i, WDATA_FPG_TS(i));
  } 
  else if (ARGV_EQUAL("pwd")) 
  {
    i = uatoi(argv[1]);
    if (argv[2] != NULL) 
    {
      strncpy(WDATA_PWD_T, argv[2], PWD_SIZE_MAX);
      WDATA_PWD_SAVE(i);
    }
    DBG_LOG("PWD%d:%.*s", i, PWD_SIZE_MAX, WDATA_PWD(i));
  } 
  else if (ARGV_EQUAL("function"))
  {
    if (argv[1] != NULL) 
    {
      WDATA_FUNCTION_T = uatoix(argv[1]);
      WDATA_FUNCTION_SAVE();
    }
    DBG_LOG("FUNCTION:%#x", WDATA_FUNCTION);
  } else if (ARGV_EQUAL("readlog")) 
  {
    if (LockLog_ReadFromBlock(uatoi(argv[1]), &log)) {
      DBG_LOG("LockLog_ReadFromBlock type:%u, time:%u, arg:%u.", log.opentype, log.time, log.arg);
    } else {
      DBG_LOG("LockLog_ReadFromBlock failed.");
    }
  } else if (ARGV_EQUAL("writelog")) {
    log.time = RTC_ReadCount();
    log.opentype = uatoi(argv[2]);
    log.arg = uatoi(argv[3]);
    DBG_LOG("LockLog_SaveToBlock:%u", LockLog_SaveToBlock(uatoi(argv[1]), &log));
  } else if (ARGV_EQUAL("dfuinfo")) {
    DBG_LOG("DFU version:%u, size:%u, crc:%#x, ", WDATA_DFU.newver, WDATA_DFU.size, WDATA_DFU.crc);
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
