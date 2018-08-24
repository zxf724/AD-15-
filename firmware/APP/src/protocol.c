/**
    ******************************************************************************
    * @file    protocol.c
    * @author  ����
    * @version V1.0
    * @date    2015.11.31
    * @brief   �������غ���.
    *
    ******************************************************************************
    */

/* Includes ------------------------------------------------------------------*/
#include "includes.h"
#include "ble_nus.h"
/** @addtogroup firmwave_F2_BLE
    * @{
    */



/** @defgroup protocol
    * @brief �������
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
uint16_t authRunIndex = 0, authStep = 0;
uint8_t Random_Running[8];
uint8_t Key_Default[] = { 0xEF, 0xE1, 0xF2, 0x9C, 0xB4, 0xA5, 0xCD, 0xB7, 0xD8, 0xC9, 0x1B, 0x1E, 0xBC, 0xBD, 0xAE, 0x29 };

static uint8_t recStack[PROTOCOL_REC_STACK_MAX][BLE_NUS_MAX_DATA_LEN];

static protocol_fifo_t  ProtocolFifo;


/* Private function prototypes -----------------------------------------------*/
static BOOL Protocol_Analyse(uint8_t* dat, uint8_t len);
static BOOL Protocol_Cmd_Analy(uint8_t* dat, uint8_t len);


/* Exported functions ---------------------------------------------------------*/


/**
 * ��ʼ�������.
 */
void Protocol_Init(void) {
  /* ��ʼ��FIFO */
  if (IS_POWER_OF_TWO(PROTOCOL_REC_STACK_MAX)) {
    ProtocolFifo.sizeMask = PROTOCOL_REC_STACK_MAX - 1;
    ProtocolFifo.rpos = 0;
    ProtocolFifo.wpos = 0;
    ProtocolFifo.pStack = recStack;
  }
}

/**
    * @brief  ������մ�����ѯ.
    */
void Protocol_DateProcPoll(void) {
  uint8_t* p = NULL, len = 0;

  if (STACK_LENGTH()) {
    p = &(ProtocolFifo.pStack[ProtocolFifo.rpos & ProtocolFifo.sizeMask][0]);
    len = (uint8_t)*p++;
    ProtocolFifo.rpos++;
    AuthOK_TS = RTC_ReadCount();
    if (Protocol_Analyse(p, len) == FALSE) {
      DBG_LOG("Invalid BLE format, BLE disconnect.");
      user_BLE_Disconnected();
    }
  } else {
    /*��ʱ�����ݶϿ���������*/
    if (BLE_Connect) {} else {
      AuthOK_TS = RTC_ReadCount();
    }
    if (BLE_Connect && RTC_ReadCount() - AuthOK_TS > BLE_CONNECT_TIMEOUT) {
      AuthOK_TS = RTC_ReadCount();
      DBG_LOG("AuthOK_TS %02u.s", AuthOK_TS);
      user_BLE_Disconnected();
      DBG_LOG("Timeout disconnected.");
    }
  }
}

/**
 * �����ϱ�ȡɡ���
 * 
 * @param rfid   ��ɡ��RFID
 * @param status �����״̬
 */
void Protocol_Report_Umbrella_Borrow(uint32_t rfid, motor_status_t status) {
  uint8_t buf[5];

  if (BLE_Connect) {
    *(uint32_t*)buf = rfid;
    if (status == status_borrow_complite) {
      buf[4] = 0x55;
    } else if (status == status_ir_stuck) {
      buf[4] = 0xAA;
    } else if (status == status_motor_stuck) {
      buf[4] = 0xAB;
    } else if (status == status_timeout) {
      buf[4] = 0xAC;
    }
    Send_Cmd(0x3B, buf, 5);
  }
}

/**
    * @brief  ����������.
    */
void Protocol_NewDate(uint8_t* dat, uint8_t len) {
  uint8_t* p = NULL;

  if (STACK_LENGTH() <= ProtocolFifo.sizeMask) {
    p = &(ProtocolFifo.pStack[ProtocolFifo.wpos & ProtocolFifo.sizeMask][0]);
    *p++ = len;
    memcpy(p, dat, len);
    ProtocolFifo.wpos++;
    DataFlowCnt++;
  }
}

/**
 * ��������
 * 
 * @param cmd    ����
 * @param arg    ��������
 * @param len    ��������
 */
void Send_Cmd(uint8_t cmd, uint8_t* arg, uint8_t len) {
  static uint16_t sendIndex = 0;
  uint8_t buf[20];

  DBG_LOG("Send_Cmd:%#x, len:%u", cmd, len);

  /*��ͷ������*/
  buf[0] = 0;
  buf[1] = cmd << 2;
  buf[1] |= MSG_TYPE_CMD;

  /*�������*/
  memcpy(&buf[2], (uint8_t*)Create_Random(), 4);

  /*��������ֵ*/
  sendIndex++;
  buf[6] = sendIndex;
  buf[7] = sendIndex >> 8;

  /*�������*/
  if (arg != NULL && len > 0 && len <= 12) {
    memcpy(&buf[8], arg, len);
  }
  len += 8;
#if USE_AES == 1
  nrf_ecb_hal_data_t ecb_data;

  if (len > 17) {
    len = 17;
  }
  memset(ecb_data.cleartext, 0, 16);
  memcpy(ecb_data.cleartext, &buf[1], len - 1);
  memcpy(ecb_data.key, Key_Default, 16);
  sd_ecb_block_encrypt(&ecb_data);
  memcpy(&buf[1], ecb_data.ciphertext, 16);
  user_BLE_Send(buf, 17);
#else
  user_BLE_Send(buf, len);
#endif
  DataFlowCnt++;
}

/**
 * ����8BYTE�����
 * 
 * @return ���������ָ��
 */
uint8_t* Create_Random(void) {
  uint8_t  available = 0;

  while (available < 8) {
    nrf_drv_rng_bytes_available(&available);
  }
  nrf_drv_rng_rand(Random_Running, 4);
  return Random_Running;
}

/**
 * AES��������
 * 
 * @param data   ��������
 * @param key    ��Կ
 * @param len    ���ݳ��ȣ���Ϊ16�ı���
 */
void AesData_decrypt(uint8_t* data, uint8_t* key, uint16_t len) {
  int i;
  uint8_t buf[16];

  if (data != NULL && key != NULL && len >= 16 && len % 16 == 0) {
    for (i = 0; i < len; i += 16) {
      AES128_ECB_decrypt(data, key, buf);
      memcpy(data, buf, 16);
      data += 16;
    }
  }
}

/**
    * @brief  ������մ�����ѯ.
    * @param  none.
    * @retval none
    */
static BOOL Protocol_Analyse(uint8_t* dat, uint8_t len) {
  int16_t  i;
  DBG_LOG("handle the data:");

  DBG_LOG("Protocol_Analyse:");
  for (i = 0; i < len; i++) {
    DBG_LOG("0x%02X.", (uint8_t)*(dat + i));
  }

  /*��������*/
  if (dat[0] == 0) {
    dat++;
    len--;
#if USE_AES == 1
    AesData_decrypt((uint8_t*)dat, (uint8_t*)Key_Default, 16);
    DBG_LOG("AES Decrypt:");
    for (i = 0; i < len; i++) {
      DBG_LOG("0x%02X.", (uint8_t)*(dat + i));
    }
#endif
    return Protocol_Cmd_Analy((uint8_t*)dat, (uint8_t)len);
  }
  return FALSE;
}

/**
 * ���������
 * 
 * @param dat    ���������
 * @param len    ���ݳ���
 */
static BOOL Protocol_Cmd_Analy(uint8_t* dat, uint8_t len) {
  BOOL ret = FALSE;
  uint16_t run;
  uint8_t cmd = 0, temp[4];

  /*�Ƚϰ�ͷ*/
  if ((dat[0] & 0x03) != MSG_TYPE_CMD) {
    isAuthOK = FALSE;
    DBG_LOG("Command type invalid.");
    return FALSE;
  }

  /*�ȽϹ���ͬ������ֵ*/
  run = dat[5] | (dat[6] << 8);
  if (run - authRunIndex >= 1 &&  run - authRunIndex < 5) {
    ret = TRUE;
    /*�����*/
    cmd = dat[0] >> 2;
    DBG_LOG("Receive command 0x%X.", (uint8_t)cmd);

    switch (cmd) {
      /*Уʱ*/
      case CMD_TIME_RALIB:
        memcpy(temp, (uint8_t*)&dat[7], 4);
        /*�Ƚ��豸ID*/
        if (*(uint32_t*)temp == WorkData.DeviceID) {
          memcpy(temp, (uint8_t*)&dat[11], 4);
          RTC_SetCount(*(uint32_t*)temp);
          temp[0] = 1;
          temp[1] = VERSION;
          temp[2] = WorkData.StockMax;
          temp[3] = WorkData.StockCount;
          Send_Cmd(0x19, temp, 4);
        }
        break;
        /*��ɡ*/
      case CMD_BORROW_UMBRELLA:
        memcpy(temp, (uint8_t*)&dat[7], 4);
        /*�Ƚ��豸ID*/
        if (*(uint32_t*)temp == WorkData.DeviceID) {
          Borrow_Action();
        }
        break;
      default:
        break;
    }
  } else {
    DBG_LOG("Running index fault, store:%u, receive:%u", authRunIndex, run);
  }
  authRunIndex = run;
  return ret;
}
