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
typedef struct {
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

extern uint8_t g_mode;

BOOL isAuthOK = FALSE;
BOOL isTimeOut = FALSE;
uint32_t AuthOK_TS = 0;
uint16_t authRunIndex = 0, authStep = 0;
uint8_t Random_Running[8];
uint8_t Key_Default[] = { 0xEF, 0xE1, 0xF2, 0x9C, 0xB4, 0xA5, 0xCD, 0xB7, 0xD8, 0xC9, 0x1B, 0x1E, 0xBC, 0xBD, 0xAE, 0x29 };

static uint8_t recStack[PROTOCOL_REC_STACK_MAX][BLE_NUS_MAX_DATA_LEN];

static protocol_fifo_t  ProtocolFifo;


/* Private function prototypes -----------------------------------------------*/
static uint8_t Protocol_Analyse(uint8_t* dat, uint8_t len);
static uint8_t Protocol_Cmd_Analy(uint8_t* dat, uint8_t len);


/* Exported functions ---------------------------------------------------------*/


/**
 * 初始化命令处理.
 */
void ProtocolInit(void) {
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
void ProtocolDateProcPoll(void) {
    uint8_t* p = NULL, len = 0;
    if (STACK_LENGTH()) {
        p = &(ProtocolFifo.pStack[ProtocolFifo.rpos & ProtocolFifo.sizeMask][0]);
        len = (uint8_t) * p++;
        ProtocolFifo.rpos++;
        AuthOK_TS = RTC_ReadCount();
        DBG_LOG("AuthOK_TS %02u.s", AuthOK_TS);
        if (Protocol_Analyse(p, len) == 0) {
            DBG_LOG("Invalid BLE format, BLE disconnect.");
            user_BLE_Disconnected();
        }
        DBG_LOG("RTC_ReadCount() - AuthOK_TS = %02u.s", RTC_ReadCount() - AuthOK_TS);
    } else {
        /*超时无数据断开蓝牙连接*/
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
 * 蓝牙上报取伞结果
 *
 * @param rfid   雨伞的RFID
 * @param status 电机的状态
 */
void Protocol_Report_Umbrella(uint32_t rfid, motor_status_Enum status) {
    uint8_t buf[5];
    if (BLE_Connect) {
        *(uint32_t*)buf = rfid;
        if (status == k_status_output_unbrella_success) {
            buf[4] = 0x55;
        } else if (status == k_status_ir_stuck) {
            buf[4] = 0xAA;
        } else if (status == k_status_motor_stuck) {
            buf[4] = 0xAB;
        } else if (status == k_status_timeout) {
            buf[4] = 0xAC;
        }
        Send_Cmd(0x3B, buf, 5);
    }
}

/**
    * @brief  命令接收入队.
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
 * 发送命令
 *
 * @param cmd    命令
 * @param arg    参数内容
 * @param len    参数长度
 */
void Send_Cmd(uint8_t cmd, uint8_t* arg, uint8_t len) {
    static uint16_t sendIndex = 0;
    uint8_t buf[20];
    DBG_LOG("Send_Cmd:%#x, len:%u", cmd, len);
    /*包头与命令*/
    buf[0] = 0;
    buf[1] = cmd << 2;
    buf[1] |= MSG_TYPE_CMD;
    DBG_LOG("before encryption: buf[1] = 0X%x", buf[1]);
    /*真随机数*/
    memcpy(&buf[2], (uint8_t*)Create_Random(), 4);
    /*滚动计数值*/
    sendIndex++;
    buf[6] = sendIndex;
    buf[7] = sendIndex >> 8;
    /*命令参数*/
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
    DBG_LOG("buf[0] = 0x%x", buf[0]);
    DBG_LOG("after encryption: buf[1] = 0x%x", buf[1]);
    user_BLE_Send(buf, 17);
#else
    user_BLE_Send(buf, len);
#endif
    DataFlowCnt++;
}

/**
 * 生成8BYTE随机数
 *
 * @return 返回随机数指针
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
 * AES解密数据
 *
 * @param data   数据内容
 * @param key    密钥
 * @param len    数据长度，需为16的倍数
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
    * @brief  命令接收处理轮询.
    * @param  none.
    * @retval none
    */
static uint8_t Protocol_Analyse(uint8_t* dat, uint8_t len) {
    int16_t  i;
    DBG_LOG("handle the data:");
    DBG_LOG("Protocol_Analyse:");
    for (i = 0; i < len; i++) {
        DBG_LOG("0x%02X.", (uint8_t) * (dat + i));
    }
    /*单包命令*/
    if (dat[0] == 0) {
        //去除单包包头
        dat++;
        len--;
#if USE_AES == 1
       AesData_decrypt((uint8_t*)dat, (uint8_t*)Key_Default, 16);
        DBG_LOG("AES Decrypt:");
        for (i = 0; i < len; i++) {
            DBG_LOG("解密后的数据包：\n 0x%02X.", (uint8_t) * (dat + i));
        }
#endif
        return Protocol_Cmd_Analy((uint8_t*)dat, (uint8_t)len);
    }
    return 0;
}


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
static uint8_t Protocol_Cmd_Analy(uint8_t* dat, uint8_t len) {
    uint8_t ret = 0;
    uint16_t run;
    uint8_t cmd = 0, temp[4];
    DBG_LOG("Protocol_Cmd_Analy:");
    /*比较包头*/
    if ((dat[0] & 0x03) != MSG_TYPE_CMD) {
        isAuthOK = FALSE;
        DBG_LOG("Command type invalid.");
        return 0;
    }
    DBG_LOG("Command type valid！");
    dat[0] = dat[0] >> 2;
    DBG_LOG("here is the cmd = 0x%02X", dat[0]);
    /*比较滚动同步计数值*/
    run = (dat[6] << 8) | dat[5];
    if (run - authRunIndex >= 1 &&  run - authRunIndex < 5) {
        ret = 1;
        /*命令处理*/
        cmd = dat[0];
        DBG_LOG("Receive command 0x%X.", (uint8_t)cmd);
        switch (cmd) {
            /*校时*/
            case CMD_TIME_RALIB:
                memcpy(temp, (uint8_t*)&dat[7], 4);
                for(uint8_t i=0;i<=3;i++){
                    DBG_LOG("dat[%d] = %d",i,temp[i]);
                }
                //tmp = (dat[7] << 24) | (dat[8] << 16) | (dat[9] << 8) | dat[10];
                DBG_LOG("*(uint32_t*)temp = %d", *(uint32_t*)temp);
                /*比较设备ID*/
                WorkData.DeviceID = 1;  
                if (*(uint32_t*)temp == WorkData.DeviceID) {
                    DBG_LOG("timing .....");
                    memcpy(temp, (uint8_t*)&dat[11], 4);
                    RTC_SetCount(*(uint32_t*)temp);
                    if(WorkData.StockCount == 0) {
                        Motor_staus = k_status_have_no_unbrella;
                    }
                    temp[0] = 1;
                    temp[1] = VERSION;
                    temp[2] = WorkData.StockMax;
                    temp[3] = WorkData.StockCount;
                    AuthOK_TS = RTC_ReadCount();
                    Send_Cmd(0x19, temp, 4);
                    DBG_LOG("Running index timing, store:%u, receive:%u", authRunIndex, run);
                }
                break;
            /*借伞*/
            case CMD_BORROW_UMBRELLA:
                DBG_LOG("BorrowAction .....");
                memcpy(temp, (uint8_t*)&dat[7], 4);
                /*比较设备ID*/
                if (*(uint32_t*)temp == WorkData.DeviceID) {
                    BorrowAction();
                    Motor_staus = k_status_start_output_unbrella;
                    DBG_LOG("Running index borrowing , store:%u, receive:%u", authRunIndex, run);
                }
                break;
            /*还伞*/
            case CMD_RETURN_UMBRELLA:
                if((uint32_t*)temp == 0) {
                    Motor_staus = k_status_full_unbrella;
                    break;
                }else {
                    DBG_LOG("RepayInAction...");
                    Motor_staus = k_status_start_input_unbrella;
                    RepayAction();
                    break;
                }
            /*还故障伞*/
            case CMD_RETURN_BREAKDOWN_UMBRELLA:
                DBG_LOG("BreakDownAction...");
                Motor_staus = k_status_input_breakdown_unbrella;
                BreakdownRepay();
                break;
            default:
                break;
        }
    } else {
        Send_Cmd(0x25, NULL, 0);
        ret = 2;
        DBG_LOG("Running index fault, store:%u, receive:%u", authRunIndex, run);
    }
    authRunIndex = run;
    return ret;
}