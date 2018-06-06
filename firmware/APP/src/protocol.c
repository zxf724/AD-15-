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
#include "datasave.h"
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
BOOL isTimeOut = FALSE;
uint32_t AuthOK_TS = 0;
uint8_t mulitiBuf[MULITI_PACKET_LEN_MAX];
uint16_t authRunIndex = 0, authStep = 0;
uint8_t Random_Running[8];
//CommParam_t  CommParam;
// EF E1 F2 9C B4 A5 CD B7 D8 C9 1B 1E BC BD AE 29
uint8_t Key_Default[] = { 0xEF, 0xE1, 0xF2, 0x9C, 0xB4, 0xA5, 0xCD, 0xB7, 0xD8, 0xC9, 0x1B, 0x1E, 0xBC, 0xBD, 0xAE, 0x29 };
uint8_t Key_Default_exam[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };

static uint8_t recStack[PROTOCOL_REC_STACK_MAX][BLE_NUS_MAX_DATA_LEN];

static protocol_fifo_t  ProtocolFifo;


/* Private function prototypes -----------------------------------------------*/
static void Protocol_Analyse(uint8_t* dat, uint8_t len);
//static void Protocol_Auth(uint8_t* dat, uint8_t len);
static void Protocol_Cmd_Analy(uint8_t* dat, uint8_t len);


/* Exported functions ---------------------------------------------------------*/


/**
 * 初始化命令处理.
 */
void Protocol_Init(void) {
  /* 初始化FIFO */
  if (IS_POWER_OF_TWO(PROTOCOL_REC_STACK_MAX)) {
    ProtocolFifo.sizeMask = PROTOCOL_REC_STACK_MAX - 1;
    ProtocolFifo.rpos = 0;
    ProtocolFifo.wpos = 0;
    ProtocolFifo.pStack = recStack;
  }
}

/**
    * @brief  命令接收处理轮询.
    */
void Protocol_DateProcPoll(void)
{
  uint8_t* p = NULL, len = 0;
  
  if (STACK_LENGTH()) 
  {
     
    p = &(ProtocolFifo.pStack[ProtocolFifo.rpos & ProtocolFifo.sizeMask][0]);
    len = (uint8_t)*p++;
    ProtocolFifo.rpos++;
    Protocol_Analyse(p, len);
    DBG_LOG("CMD Handle...");
    DBG_LOG("CMD Handle %02u.number", ProtocolFifo.rpos);
    AuthOK_TS = RTC_ReadCount();
  }
  else
  {
        /*重置时间戳*/
    if(BLE_Connect && RTC_ReadCount() - AuthOK_TS > BLE_CONNECT_TIMEOUT)
    {
      AuthOK_TS = RTC_ReadCount();
      DBG_LOG("AuthOK_TS %02u.s", AuthOK_TS);
      user_BLE_Disconnected();
      DBG_LOG("Timeout disconnected.");
    }
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
    memcpy(p, dat, len);
    ProtocolFifo.wpos++;
    DBG_LOG("CMD Receiving...");
    DBG_LOG("CMD Handle %02u.number", ProtocolFifo.wpos);
  }
}

/**
 * 发送命令
 * 
 * @param cmd    命令
 * @param arg    参数内容
 * @param len    参数长度
 */
void Send_Cmd(uint8_t cmd, uint8_t* arg, uint8_t len) 
{ uint8_t buf[20];

  DBG_LOG("Send_Cmd:%#x, len:%u", cmd, len);
  buf[0] = cmd << 2;
  buf[0] |= MSG_TYPE_CMD;
  if (arg != NULL && len > 0 && len <= 19) {
    memcpy(&buf[1], arg, len);
  }
  len += 1;
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
void Send_CmdDefaultkey(uint8_t cmd, uint8_t* arg, uint8_t len) {
  nrf_ecb_hal_data_t ecb_data;
  uint8_t buf[20];

  DBG_LOG("Send_Cmd:%#x, len:%u", cmd, len);
  buf[0] = cmd << 2;
  buf[0] |= MSG_TYPE_CMD;
  if (arg != NULL && len > 0 && len <= 19) {
    memcpy(&buf[1], arg, len);
  }
  len += 1;

  if (len > 16) {
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
void Send_MulitiPacket(uint16_t index, uint8_t* data, uint8_t len) {

  uint8_t buf[20];

  buf[0] = (uint8_t)(index << 2);
  buf[0] |= MSG_TYPE_CMD;
  buf[1] = (uint8_t)(index >> 6);

  if (data != NULL && len > 0 && len <= 18) {
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

  for(i = 0; i < 8; i++) 
  {
    Key_Running[i] = WDATA_KEYS[random[i] % 128];
    Key_Running[i + 8] = WDATA_KEYS[(random[i] + random[3]) % 128];
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
  nrf_drv_rng_rand(Random_Running, 4);
  return Random_Running;
}

/**
 * AES解密数据
 * 
 * @param data   数据内容
 * @param key    密钥
 * @param len    数据长度，需为16的倍数
 */
void AesData_decrypt(uint8_t* data,uint8_t * key, uint16_t len) 
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
  DBG_LOG("handle the data:");
//    for (i = 0; i < len; i++) 
//    {
//      DBG_LOG("0x%02X.", *(dat + i));
//    }
    /*单包命令*/
    if (dat[0] == 0) 
    {
      dat++;
      len--;
#if USE_AES == 1
      AesData_decrypt((uint8_t*)dat, (uint8_t*)Key_Default, 16);
      DBG_LOG("AES Decrypt:");
      for (i = 0; i < len; i++) 
      {
        DBG_LOG("0x%02X.",(uint8_t) *(dat + i));
      }
#endif
      Protocol_Cmd_Analy((uint8_t*)dat, (uint8_t)len);
    }

}
/*   加密函数
*  *dat   数据
*  len    数据长度
*/
void encrypt_data(uint8_t *dat,uint8_t len)
{
    nrf_ecb_hal_data_t ecb_data;
    uint8_t buf[16];
    
    memcpy(buf,&dat[0],16);
    memset(ecb_data.cleartext, 0, 16);
    memcpy(ecb_data.cleartext, (uint8_t*)buf,16);
    memcpy(ecb_data.key,(uint8_t *) Key_Default, 16);
    sd_ecb_block_encrypt(&ecb_data);
    DBG_LOG("encrypt beginning the data");
    memcpy(&dat[0],ecb_data.ciphertext,16);
     for (uint8_t i = 0; i <len; i++) 
    {
      DBG_LOG("0x%02X.", *(dat + i));
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
  uint8_t  run,sup[9] = {0},i;
    nrf_ecb_hal_data_t ecb_data;
    dat[0]= 0x00;
    memcpy((uint8_t*)&run, &dat[5], 2);
    /*比较同步计数值*/
    if (run - authRunIndex >= 1 &&  run - authRunIndex < 5)
    {
      dat[1] = cmd << 2;
      dat[1] |= MSG_TYPE_CMD;
      memcpy(&dat[2],(uint8_t*) Create_Random(), 4);
      memcpy(&dat[6],(uint8_t*) &run, 2);
      if(param != NULL)
      {
        if(len>9)       len = 9;
        memcpy(&dat[8],(uint8_t*)param,len);
      }
      else
      {
        memcpy(&dat[8],(uint8_t*)sup,9);
      }
            for (i = 0; i < 16; i++) 
      {
        DBG_LOG("0x%02X.", *(dat + i));
      }
//        user_BLE_Send(dat, 16);
      memset(ecb_data.cleartext, 0, 16);
      memcpy(ecb_data.cleartext, (uint8_t*) &dat[1], 16);
      memcpy(ecb_data.key,(uint8_t *) Key_Default, 16);
      sd_ecb_block_encrypt(&ecb_data);
      memcpy(&dat[1],ecb_data.ciphertext,16);
      user_BLE_Send(dat,17);
//      for (i = 0; i < 16; i++) 
//      {
//        DBG_LOG("0x%02X.", *(dat + i));
//      }
      DBG_LOG("Device auth handshake.");
    } 
    else 
    {
      dat[1] = cmd << 2;
      dat[1] |= MSG_TYPE_CMD;
      memcpy(&dat[2],(uint8_t*) Create_Random(), 4);
      dat[6] = 0x00;
      dat[7] = 0x00;
      if(param != NULL)
      {
        if(len>9)       len = 9;
        memcpy(&dat[8],(uint8_t*)param,len);
      }
      else
      {
        memcpy(&dat[8],(uint8_t*)sup,9);
      }
            for (i = 0; i < 16; i++) 
      {
        DBG_LOG("0x%02X.", *(dat + i));
      }
//        user_BLE_Send(dat, 16);
      memset(ecb_data.cleartext, 0, 16);
      memcpy(ecb_data.cleartext, (uint8_t*) &dat[1], 16);
      memcpy(ecb_data.key,(uint8_t *) Key_Default, 16);
      sd_ecb_block_encrypt(&ecb_data);
      memcpy(&dat[1],ecb_data.ciphertext,16);
      user_BLE_Send(dat,17);
//      for (i = 0; i < 16; i++) 
//      {
//        DBG_LOG("0x%02X.", *(dat + i));
//      }
      DBG_LOG("auth index falut, recevive:%u, store:%u", run, authRunIndex);
    }
    authRunIndex = run;
}
//static uint8_t data_cmp(uint8_t *str1,char *str2,uint8_t len)
//{
//  uint8_t  i = 0;
//  
//  while(len --)
//  {
//      if(*(str1 + i) != *(str2 + i))
//      {
//          return 0;
//      }
//      i++;
//  }
//  
//  return  1;
//}

static uint32_t getIntFromChar(uint8_t c) {  
    uint32_t result = (uint32_t) c;  
    return result & 0x000000ff;  
} 

uint32_t getWordFromStr(uint8_t *str) {  
    uint32_t one = getIntFromChar(str[0]);  
    one = one << 24;  
    uint32_t two = getIntFromChar(str[1]);  
    two = two << 16;  
    uint32_t three = getIntFromChar(str[2]);  
    three = three << 8;  
    uint32_t four = getIntFromChar(str[3]);  
    return one | two | three | four;  
} 
/**
 * 单包命令处理
 * 
 * @param dat    输入的数据
 * @param len    数据长度
 */
static void Protocol_Cmd_Analy(uint8_t* dat, uint8_t len)
 {

  uint8_t cmd = 0, utc[4];
  uint8_t param[9]={0};
  uint8_t device_id[4];
  uint32_t F_id,S_id;
  
//  lock_t  lock;

  /*命令处理*/
  cmd = dat[0] >> 2;
//  cmd = dat[0];
  // dat++;
  // len--;
  DBG_LOG("Receive command 0x%X.", (uint8_t)cmd);

  switch (cmd) 
  {
    /*校时*/
    case CMD_TIME_RALIB:
        DBG_LOG("Enter timing...");
        F_id = getWordFromStr(&dat[7]);
        memcpy(utc, (uint8_t*)&dat[11], 4);
        RTC_SetCount(*(uint32_t*)utc);
        S_id =uatoi(CommParam.mqtt.MQTT_ClientID);;
        DBG_LOG("F_id = %u",F_id);
        DBG_LOG("S_id = %u",S_id);
        param[4] = 0x01;
        if(F_id == S_id)
        {
          memcpy(&param[0],CommParam.mqtt.MQTT_ClientID,4);
          param[5] = 0x1b;
          param[6] = CommParam.UmbrellaCount;
        }
        ReBack(dat,0x19,param,7);
        break;
      /*借伞*/
    case CMD_BORROW_UMBRELLA:
        DBG_LOG("Enter Opening Lock ...");
        memcpy(device_id, (uint8_t*)&dat[7], 4);
        Borrow_Action(dat);
      break;
      /*固件升级*/
    case CMD_DFU:
          DBG_LOG("Enter bootloadering ...");
          /*重启进入bootloader*/
          memcpy((uint8_t*)utc, (uint8_t*)&dat[7], (uint8_t)1);
          if (*(uint16_t*)utc > VERSION) 
          {
            WDATA_DFU_T.newver = *(uint8_t*)utc;
            memcpy(utc, &dat[8], 4);
            WDATA_DFU_T.size = *(uint32_t*)utc;
            memcpy(utc, &dat[12], 4);
            WDATA_DFU_T.crc = *(uint32_t*)utc;
            WDATA_DFU_SAVE();
            param[0]= 0x65;
            ReBack((uint8_t*)dat,(uint8_t)0X2C,param,(uint8_t)1);
            DBG_LOG("versions allow upgrade...");
            nrf_delay_ms(50);
            user_BLE_Disconnected();
            nrf_delay_ms(10);
            NVIC_SystemReset();
          }
          else             
          {
            DBG_LOG("versions not allow upgrade...");
            param[0]=0x64;
            ReBack((uint8_t*)dat,(uint8_t)0X2C,param,(uint8_t)1);
          }
      break;
    default:
      break;
  }
}
