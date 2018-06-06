/**
    ******************************************************************************
    * @file    protocol.c
    * @author  宋阳
    * @version V1.0
    * @date    2015.11.31
    * @brief   命令处理相关函数.
    *
    ******************************************************************************
    */

/* Includes ------------------------------------------------------------------*/
#include "includes.h"
#include "ble_nus.h"
#include "bootloader.h"

/** @addtogroup firmwave_F2_BLE
    * @{
    */



/** @defgroup protocol
    * @brief 命令处理函数
    * @{
    */


/* Private typedef -----------------------------------------------------------*/

/** @defgroup protocol_Private_Typedef protocol Private Typedef
    * @{
    */
typedef struct
{
  uint8_t(* pStack)[BLE_NUS_MAX_DATA_LEN];
  uint16_t  sizeMask;
  uint16_t  rpos;
  uint16_t  wpos;
} protocol_fifo_t;

/**
    * @}
    */

/* Private define ------------------------------------------------------------*/


/* Private macros ------------------------------------------------------------*/

#define STACK_LENGTH()   (ProtocolFifo.wpos - ProtocolFifo.rpos)


/* Private variables ---------------------------------------------------------*/


BOOL isAuthOK = FALSE;
uint32_t AuthOK_TS = 0;
uint8_t mulitiBuf[2048];
uint16_t authRunIndex = 0;
uint8_t Random_Running[8];
//  AF 22 02 93 04 12 C5 07 E8 09 1A 1B 1C 1D 1E 2E
uint8_t Key_Default[] = { 0xAF, 0x22, 0x02, 0x93, 0x04, 0x12, 0xC5, 0x07, 0xE8, 0x09, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x2E };

static uint8_t recStack[PROTOCOL_REC_STACK_MAX][BLE_NUS_MAX_DATA_LEN], mulitiCMD = 0;
static uint16_t multiIndex = 0, dfuBufIndex = 0, dfuIndex = 0;
static protocol_fifo_t  ProtocolFifo;


/* Private function prototypes -----------------------------------------------*/
static void Protocol_Analyse(uint8_t* dat, uint8_t len);
static void Protocol_Cmd_Analy(uint8_t* dat, uint8_t len);
static void Protocol_Packet_Analy(uint8_t* dat, uint8_t len);

/* Exported functions ---------------------------------------------------------*/


/**
 * 初始化命令处理.
 */
void Protocol_Init(void)
{
  /* 初始化FIFO */
  if (IS_POWER_OF_TWO(PROTOCOL_REC_STACK_MAX)) 
  {
    ProtocolFifo.sizeMask = PROTOCOL_REC_STACK_MAX - 1;
    ProtocolFifo.rpos = 0;
    ProtocolFifo.wpos = 0;
    ProtocolFifo.pStack = recStack;
  }
}

    
/**
 * 应答处理
 * 
 * @param dat    输入的数据
 * @param cmd     应答命令
 * @param param   应答数据
 * @param len    应答数据长度
 */

void ReBack(uint8_t *dat,uint8_t cmd,uint8_t * param,uint8_t len)
{
    uint8_t  run,sup[9] = {0};
    nrf_ecb_hal_data_t ecb_data;
    
      memcpy((uint8_t*)&run, &dat[5], 2);
    /*比较同步计数值*/
    if (run - authRunIndex >= 1 &&  run - authRunIndex < 5)
    {
      dat[0] = cmd << 2;
      dat[0] |= MSG_TYPE_CMD;
      memcpy(&dat[1], Create_Random(), 4);
      memcpy(&dat[5],(uint8_t*) &run, 2);
      if(param != NULL)
      {
        if(len>9)       len = 9;
        memcpy(&dat[7],param,len);
      }
      else
      {
        memcpy(&dat[7],sup,9);
      }
      memset(ecb_data.cleartext, 0, 16);
      memcpy(ecb_data.cleartext, dat, 16);
      memcpy(ecb_data.key, Key_Default, 16);
      sd_ecb_block_encrypt(&ecb_data);
      user_BLE_Send(ecb_data.ciphertext, 16);

      DBG_LOG("Device auth handshake.");
    } 
    else 
    {
      DBG_LOG("auth index falut, recevive:%u, store:%u", run, authRunIndex);
    }
    authRunIndex = run;
}  


/**
    * @brief  命令接收处理轮询.
    */
void Protocol_DateProcPoll(void) 
{
  uint8_t* p = NULL, len = 0,buf[20]={0},param[9] = {0};
  static BOOL volatile connect = FALSE;
  
/*上报BOOT就绪*/
  if (BLE_Connect&&isAuthOK == FALSE) 
  {
    isAuthOK = TRUE;
    param[0] = 0x67;
    ReBack((uint8_t*)buf,(uint8_t)0X2C,param,(uint8_t)1);
    DBG_LOG("bootloardering already...");
  }
  if (STACK_LENGTH()) 
  {
    p = &(ProtocolFifo.pStack[ProtocolFifo.rpos & ProtocolFifo.sizeMask][0]);
    len = *p++;
    ProtocolFifo.rpos++;
    Protocol_Analyse((uint8_t*)p, (uint8_t)len);
    DBG_LOG("CMD Handle...");
    DBG_LOG("CMD Handle 0x%02u.number", ProtocolFifo.rpos);
  }

}

/**
    * @brief  命令接收入队.
    */
void Protocol_NewDate(uint8_t* dat, uint8_t len) 
{
  uint8_t* p = NULL;

  if (STACK_LENGTH() <= ProtocolFifo.sizeMask)
  {
    p = &(ProtocolFifo.pStack[ProtocolFifo.wpos & ProtocolFifo.sizeMask][0]);
    *p++ = len;
    memcpy((uint8_t*)p, (uint8_t*)dat, (uint8_t)len);
    ProtocolFifo.wpos++;
    DBG_LOG("CMD Receiving...");
    DBG_LOG("CMD Handle 0x%02u.number", ProtocolFifo.wpos);
  }
}

/**
 * 发送命令
 * 
 * @param cmd    命令
 * @param arg    参数内容
 * @param len    参数长度
 */
void Send_Cmd(uint8_t cmd, uint8_t* arg, uint8_t len) {
  uint8_t buf[20];

  DBG_LOG("Send_Cmd:%#x, len:%u", cmd, len);
  buf[0] = cmd << 2;
  buf[0] |= MSG_TYPE_CMD;
  if (arg != NULL && len > 0 && len <= 19) {
    memcpy(&buf[1], arg, len);
  }
  len += 1;
  AuthOK_TS = RTC_ReadCount();
#if USE_AES == 1
  nrf_ecb_hal_data_t ecb_data;

  if (len > 16) {
    len = 16;
  }
  memset(ecb_data.cleartext, 0, 16);
  memcpy(ecb_data.cleartext, buf, len);
  memcpy(ecb_data.key, Key_Running, 16);
  sd_ecb_block_encrypt(&ecb_data);
  user_BLE_Send(ecb_data.ciphertext, 16);
#else
  user_BLE_Send(buf, len);
#endif
}

/**
 * 使用默认密钥发送命令
 * 
 * @param cmd    命令
 * @param arg    参数内容
 * @param len    参数长度
 */
void Send_CmdDefaultkey(uint8_t cmd, uint8_t* arg, uint8_t len) 
{
  nrf_ecb_hal_data_t ecb_data;
  uint8_t buf[20];

  DBG_LOG("Send_Cmd:%#x, len:%u", cmd, len);
  buf[0] = cmd << 2;
  buf[0] |= MSG_TYPE_CMD;
  if (arg != NULL && len > 0 && len <= 19) 
  {
    memcpy(&buf[1], arg, len);
  }
  len += 1;
  AuthOK_TS = RTC_ReadCount();

  if (len > 16) 
  {
    len = 16;
  }
  memset(ecb_data.cleartext, 0, 16);
  memcpy(ecb_data.cleartext, buf, len);
  memcpy(ecb_data.key, Key_Default, 16);
  sd_ecb_block_encrypt(&ecb_data);
  user_BLE_Send(ecb_data.ciphertext, 16);
}

/**
 * 发送多包数据
 * 
 * @param index  包序号
 * @param data   单包数据内容
 * @param len    单包数据长度
 */
void Send_MulitiPacket(uint16_t index, uint8_t* data, uint8_t len)
{

  uint8_t buf[20];

  buf[0] = (uint8_t)(index << 2);
  buf[0] |= MSG_TYPE_CMD;
  buf[1] = (uint8_t)(index >> 6);

  if (data != NULL && len > 0 && len <= 18)
  {
    memcpy(&buf[1], data, len);
  }
  user_BLE_Send(buf, len + 1);
}

/**
 * 根据随机数从密码池中生成新的密钥
 * 
 * @param random 8byte随机数数组
 */
uint8_t* Create_Key(uint8_t* random) 
{
  int i;

  for (i = 0; i < 8; i++) 
  {
    Key_Running[i] = WDATA_KEY[random[i] % 128];
    Key_Running[i + 8] = WDATA_KEY[(random[i] + random[3]) % 128];
  }
  return Key_Running;
}


/**
 * 生成8BYTE随机数
 * 
 * @return 返回随机数指针
 */
uint8_t* Create_Random(void) 
{
  uint8_t  available = 0;

  while (available < 8) 
  {
    nrf_drv_rng_bytes_available(&available);
  }
  nrf_drv_rng_rand(Random_Running, 8);
  return Random_Running;
}

/**
 * AES解密数据
 * 
 * @param data   数据内容
 * @param key    密钥
 * @param len    数据长度，需为16的倍数
 */
void AesData_decrypt(uint8_t* data, uint8_t* key, uint16_t len) 
{
  int i;
  uint8_t buf[16];

  if (data != NULL && key != NULL && len >= 16 && len % 16 == 0) 
  {
    for (i = 0; i < len; i += 16)
    {
      AES128_ECB_decrypt(data, key, buf);
      memcpy(data, buf, 16);
      data += 16;
    }
    DBG_LOG("Decryption success.....");
  }
}

/**
    * @brief  命令接收处理轮询.
    * @param  none.
    * @retval none
    */
static void Protocol_Analyse(uint8_t* dat, uint8_t len)
{
     int16_t  i;
     uint8_t H_dat;
    /*多包数据*/
    if ((dat[0] != 0)&&(dat[0] & 0x03) == MSG_TYPE_DATA)
    {
      Protocol_Packet_Analy((uint8_t*)dat, (uint8_t)len);
    }
    /*单包命令*/
    else if (dat[0] == 0)
    {
      dat++;
      len--;
    for (i = 0; i < len; i++) 
    {
      DBG_LOG("0x%02X.", *(dat + i));
    }
#if USE_AES == 1
      AesData_decrypt((uint8_t*)dat,(uint8_t*) Key_Default, (uint8_t)16);
      DBG_LOG("AES Decrypt:");
      for (i = 0; i < len; i++) 
      {
        DBG_LOG("0x%02X.", *(dat + i));
      }
#endif
      H_dat =(uint8_t) (0x03&dat[0]);
      if(H_dat == 0x01)
      Protocol_Cmd_Analy((uint8_t*)dat, (uint8_t)len);
    }
    /*重置时间戳*/
//    AuthOK_TS = RTC_ReadCount();
}

/**
 * 单包命令处理
 * 
 * @param dat    输入的数据
 * @param len    数据长度
 */
static void Protocol_Cmd_Analy(uint8_t* dat, uint8_t len) 
{

  uint8_t cmd = 0, utc[4],param[9]={0};

  /*命令处理*/
  cmd = dat[0] >> 2;
  dat++;
  len--;
  DBG_LOG("Receive command 0x%X.", cmd);
  
  switch (cmd)
 {
    case CMD_DFU_CLEAR_F:
      memcpy(utc, &dat[7], 2);
      DFUInfo.newver = *(uint16_t*)utc;
      memcpy(utc, &dat[8], 4);
      DFUInfo.size = *(uint32_t*)utc;
      memcpy(utc, &dat[12], 4);
      DFUInfo.crc = *(uint32_t*)utc;

      DBG_LOG("DFU start, version:%u, size:%u, crc:%#x, ",
              DFUInfo.newver, DFUInfo.size, DFUInfo.crc);

      DFU_Clear(DFUInfo.size);
      DBG_LOG("flash already Clear.");
      param[0]=0x68;
      ReBack(dat,0X2C,param,1);
      Save_DFU_Info();

      /*准备接收多包数据*/
      mulitiCMD = CMD_DFU_CLEAR_F;
      multiIndex = 0;
      dfuBufIndex = 0;
      dfuIndex = 0;
    default:
      break;
  }
}

/**
 * 多包数据处理
 * 
 * @param dat    接收到的数据
 * @param len    数据长度
 */
static void Protocol_Packet_Analy(uint8_t* dat, uint8_t len) 
{
  uint8_t param[1];
  int16_t index = 0, i;

  index = (dat[0] >> 2) | (dat[1] << 6);
  dat += 2;
  len -= 2;
  DBG_LOG("Receive Order Number: 0x%X.", index);
  if (index == multiIndex) 
  {
    multiIndex++;
    if (dfuBufIndex <= 2048 - len) 
    {
      memcpy((uint8_t*)&mulitiBuf[dfuBufIndex], (uint8_t*)dat, (uint8_t)len);
      dfuBufIndex += len;
    }
    /*写入到flash*/
    if (mulitiCMD == CMD_DFU_CLEAR_F) 
    {
      /*最后一个包*/
      if (DFUInfo.size - dfuIndex == dfuBufIndex && dfuBufIndex <= 1024) 
      {
#if USE_AES == 1
        AesData_decrypt((uint8_t*)mulitiBuf, (uint8_t*)Key_Default, (uint8_t)dfuBufIndex);
#endif
        DFU_Write_Data((uint8_t*)mulitiBuf, (uint8_t)dfuBufIndex);
        i = Bootloader_app_status();
        DBG_LOG("DFU receive OK, verify status:%#x", i);
        DBG_LOG("Image crc:%#x, DFU crc:%#x", CRC_32(0, (uint8_t*)APP_ADDRESS, DFUInfo.size), DFUInfo.crc);
        if (i == DFU_COMPLETE)
        {
          param[0] =0x55;
          ReBack((uint8_t*)dat,(uint8_t)0X2C,(uint8_t*)param,(uint8_t)1);
          /*重启进入APP*/
          DBG_LOG("Restart enter APP");
          nrf_delay_ms(50);
          user_BLE_Disconnected();
          nrf_delay_ms(10);
          NVIC_SystemReset();
        } 
      } 
      else if (dfuBufIndex >= 1024) 
      {
        DBG_SEND(">", 1);
#if USE_AES == 1
        AesData_decrypt(mulitiBuf, Key_Default, 1024);
#endif
        DFU_Write_Data((uint8_t*)mulitiBuf, (uint8_t)1024);
        nrf_delay_ms(20);
        dfuIndex += 1024;
        dfuBufIndex -= 1024;
        if (dfuBufIndex > 0) 
        {
          memmove((uint8_t*)&mulitiBuf[0], (uint8_t*)&mulitiBuf[1024], dfuBufIndex);
        }
      }
    }
  } 
}



/************************ (C) COPYRIGHT  *****END OF FILE****/
