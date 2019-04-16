/**
 * *********************************************************************
 *             Copyright (c) 2016 temp. All Rights Reserved.
 * @file gprs.c
 * @version V1.0
 * @date 2016.4.1
 * @brief GPRS驱动函数文件,适用于SIM800L.
 *
 * *********************************************************************
 * @note
 * 2016.12.18 增加socket接收回调.
 *
 * *********************************************************************
 * @author 宋阳
 */



/* Includes ------------------------------------------------------------------*/
#include "includes.h"


/* Private typedef -----------------------------------------------------------*/
static const char* ipRetArray[] = {
    "IP INITIAL",
    "IP START",
    "IP CONFIG",
    "IP GPRSACT",
    "IP STATUS",
    "TCP CONNECTING",
    "CONNECT OK",
    "TCP CLOSING",
    "IP CLOSE",
    "PDP DEACT",
    NULL
};

/*TCPIP连接状态*/
typedef enum {
    ip_status_INITIAL = 0,
    ip_status_START,
    ip_status_CONFIG,
    ip_status_GPRSACT,
    ip_status_STATUS,
    ip_status_CONNECTING,
    ip_status_OK,
    ip_status_CLOSING,
    ip_status_CLOSED,
    PDP_DECT
} IP_Status_t;

typedef struct {
    char* APN;
    char* APN_User;
    char* APN_Pwd;
    char* ConnectAddress;
    uint16_t    ConnectPort;
    uint8_t     rssi;
    uint16_t    ErrorCount;
    uint16_t    ConnectFailCount;
    IP_Status_t IP_Status;
    GPRS_Status_t status;
    BOOL        powerOnOff;
} GPRS_Param_t;

/* Private define ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/
#define GPRS_PWR_ON()                   do { nrf_gpio_pin_clear(GSM_EN_PIN); nrf_gpio_cfg_output(GSM_EN_PIN); } while (0)
#define GPRS_PWR_OFF()                  nrf_gpio_cfg_input(GSM_EN_PIN, NRF_GPIO_PIN_NOPULL)

#define GPRS_KEY_ON()                   do { nrf_gpio_pin_clear(GSM_PWRKEY_PIN); nrf_gpio_cfg_output(GSM_PWRKEY_PIN); } while (0)
#define GPRS_KEY_OFF()                  nrf_gpio_cfg_input(GSM_PWRKEY_PIN, NRF_GPIO_PIN_NOPULL)


#define GPRS_SEND_DATA(dat, len)        do {UART_TX_PIN_SELECT(GSM_TXD_PIN);user_uart_SendData(dat, len);}while(0)
#define GPRS_SEND_AT(cmd)               do {UART_TX_PIN_SELECT(GSM_TXD_PIN);printf("AT+%s\r\n", cmd);}while(0)
#define GPRS_AT_PRINTF(format, ...)     do {UART_TX_PIN_SELECT(GSM_TXD_PIN);printf("AT+"format"\r\n", ##__VA_ARGS__);}while(0)

#define GPRS_WAIT_ACK(token, timeout)   WaitATRsp(token"\r\n", timeout)
#define GPRS_WAIT_TOKEN(token, timeout) WaitATRsp(token, timeout)

#define GPRS_OPT_SET(opt)               do {MASK_SET(GPRS_Opt, opt); } while(0)
#define GPRS_OPT_CLEAR(opt)             do {MASK_CLEAR(GPRS_Opt, opt); } while(0)
#define GPRS_OPT(opt)                   IS_MASK_SET(GPRS_Opt, opt)

/* Private variables ---------------------------------------------------------*/
static GPRS_Param_t GPRS_Param;
static uint32_t GPRS_Opt;
static uint8_t RspBuf[GPRS_RECEIVE_MAX_SIZE], GPRS_RecBuf[GPRS_RECEIVE_MAX_SIZE];
static uint16_t RspBufIndex = 0;

static BOOL RspTimeout = FALSE;
static FIFO_t SockRecFIFO;

APP_TIMER_DEF(TimerId_GPRS);
/* Private function prototypes -----------------------------------------------*/
static void GPRS_ManagerPoll(void);
static void GPRS_Intercept_Proc(void);
static void GPRS_TCPIP_ReceiveProc(char* pReceive);
static BOOL GPRS_ModuleInit(void);
static void GPRS_ModulePowerOn(void);
static BOOL GPRS_ModulePowerOff(void);
static BOOL ConnectShut(void);
static BOOL ConnectClose(void);
static BOOL ConnectStart(char* addr, uint16_t port);
static uint8_t GetRSSI(void);
static IP_Status_t GetIPStatus(void);
static BOOL GetNetWorkStatus(void);
static uint16_t TCPIP_Send(uint8_t* data, uint16_t len);
static void TTS_Play(char* text);
static char* WaitATRsp(char* token, uint16_t time);
static void TimerCB_GPRS(void* p_context);
static void GPRS_Console(int argc, char* argv[]);
static uint8_t GetRFID(uint32_t * rfid1, uint32_t *rfid2);
/* Exported functions --------------------------------------------------------*/
/**
 * GPRS驱动初始化
 */
void GPRSInit(void) {
    FIFO_Init(&SockRecFIFO, GPRS_RecBuf, GPRS_RECEIVE_MAX_SIZE);
    app_timer_create(&TimerId_GPRS, APP_TIMER_MODE_SINGLE_SHOT, TimerCB_GPRS);
    CMD_ENT_DEF(GPRS, GPRS_Console);
    Cmd_AddEntrance(CMD_ENT(GPRS));
    DBG_LOG("GPRS Init");
}

/**
 * GPRS任务
 * @param argument 初始化参数
 */
void GPRSPolling(void) {
    if (NRF_UART0->PSELRXD == GSM_RXD_PIN) {
        UART_MutexCount++;
        GPRS_ManagerPoll();
        if (UserUartRecLength() > 0 || RspBufIndex > 0) {
            GPRS_Intercept_Proc();
        }
        UART_MutexCount--;
    }
}

/**
 * GPRS模块重启
 */
void GPRS_ReStart(void) {
    if (!GPRS_OPT(GPRS_OPT_RESET)) {
        GPRS_OPT_SET(GPRS_OPT_RESET);
    }
}

/**
 * GPRS模块开关机
 */
void GPRS_SetOnOff(BOOL onoff) {
    if (GPRS_Param.powerOnOff != onoff) {
        GPRS_Param.powerOnOff = onoff;
        if (onoff == FALSE) {
            GPRS_OPT_SET(GPRS_OPT_RESET);
        }
    }
}

/**
 * 读模块的状态
 * @return 返回模块的状态
 */
GPRS_Status_t GPRS_ReadStatus(void) {
    return GPRS_Param.status;
}

/**
 * GPRS socket 发送数据
 * @param data 数据指针
 * @param len  数据的长度
 * @return 返回发送结果
 */
int16_t GPRS_SocketSendData(uint8_t* data, uint16_t len) {
    int rc = 0;
    if (len > GPRS_SEND_MAX_SIZE) {
        len = GPRS_SEND_MAX_SIZE;
    }
    if (GPRS_Param.IP_Status == ip_status_OK) {
#if GPRS_DEBUG > 1
        DBG_LOG("GPRS_SocketSendData:%.*s", len, data);
#endif
        rc = TCPIP_Send(data, len);
#if GPRS_DEBUG > 0
        DBG_LOG("TCPIP_Send %u, ret:%d", len, rc);
#endif
        return rc;
    }
    return -1;
}

/**
 * 读GPRS模块的RSSI
 * @return 返回RSSI的值
 */
uint8_t GPRS_ReadRSSI(void) {
    uint8_t csq = 0;
    csq = GetRSSI();
    GPRS_Param.rssi = csq;
    return csq;
}
/**
 * 读rfid的id
 * @return 返回ID的值,无rfid返回:0
 */
uint32_t GPRS_ReadRFID(uint8_t num) {
    uint32_t rfid1 = 0, rfid2 = 0;
    uint32_t ret = 0;
    ret = GetRFID(&rfid1, &rfid2);
    if(num == 1) {
        ret =  rfid1;
    } else if(num == 2) {
        ret =  rfid2;
    }
    return ret;
}
/**
 * GPRS播放TTS语音播报
 *
 * @param text   待播报的文本
 */
void GPRS_TTS(char* text) {
    if (GPRS_Param.status != gprs_status_fault) {
        TTS_Play(text);
    }
}

/**
 * 获取GPRS的连接状态
 * @retur  模块故障返回-1，
 *         无连接返回0,已连接返回1.
 */
int8_t GPRS_IsSocketConnect(void) {
    int8_t ret = -1;
    if (GPRS_Param.status != gprs_status_fault && GPRS_Param.status != gprs_status_nocard) {
        ret = 0;
    }
    if (GPRS_Param.status == gprs_status_online
            && GPRS_Param.IP_Status == ip_status_OK
            && !GPRS_OPT(GPRS_OPT_SET_SOCKET)) {
        ret = 1;
    }
    return ret;
}

/**
 * GPRS模块设置socket参数
 * @param server 服务器的IP地址或者域名名
 * @param port   服务器的端口号
 */
void GPRS_SetSocketParam(char* server, uint16_t port) {
    if (server != GPRS_Param.ConnectAddress || port != GPRS_Param.ConnectPort) {
        GPRS_Param.ConnectAddress = server;
        GPRS_Param.ConnectPort = port;
        GPRS_Param.IP_Status = ip_status_CLOSING;
        GPRS_Opt |= GPRS_OPT_SET_SOCKET;
    }
    if (server != NULL && port > 0) {
        /*GPRS开机*/
        GPRS_SetOnOff(TRUE);
    }
}

/**
 * socket读出数据
 * @param data 读出的数据指针
 * @param len  读出的长度
 * @return 返回实际读出的长度，设备故障返回-1
 */
int16_t GPRS_SocketReadData(uint8_t* data, uint16_t len) {
    int16_t rc = 0;
    rc = FIFO_Read(&SockRecFIFO, data, len);
    if (rc == 0 && GPRS_IsSocketConnect() != 1) {
        rc = -1;
    }
    return rc;
}

/**
 * 查询GPRS Socket接收的数据长度
 *
 * @return 返回数据长度
 */
uint16_t GPRS_SocketRecDataLength(void) {
    return  FIFO_Length(&SockRecFIFO);
}


/* Private function prototypes ----------------------------------------------*/
/**
 * GPRS管理轮询.
 */
static void GPRS_ManagerPoll(void) {
    static uint32_t tsNet = 0, tsConnect = 0, tsStatus = 0, tsPowerOn = 0;
    /*错误重启管理*/
    if (GPRS_Param.ErrorCount > 10 || GPRS_Param.ConnectFailCount > 10 || GPRS_OPT(GPRS_OPT_RESET)) {
        GPRS_OPT_CLEAR(GPRS_OPT_RESET);
        GPRS_Param.ErrorCount = 0;
        GPRS_Param.ConnectFailCount = 0;
        GPRS_ModulePowerOff();
        GPRS_Param.status = gprs_status_poweroff;
    }
    /*自动开机与状态管理*/
    switch (GPRS_Param.status) {
        case gprs_status_poweroff:
            if (GPRS_Param.powerOnOff) {
                GPRS_ModulePowerOn();
                TSEC_INIT(tsPowerOn);
                GPRS_Param.status = gprs_status_powerkey;
                DBG_LOG("GPRS module power on init.");
            }
            break;
        case gprs_status_powerkey:
            if (TSEC_IS_OVER(tsPowerOn, 12)) {
                TSEC_INIT(tsNet);
                GPRS_KEY_OFF();
                GPRS_SEND_DATA("AT\r", 3);
                GPRS_WAIT_ACK("OK", 100);
                GPRS_SEND_DATA("AT\r", 3);
                if (GPRS_WAIT_ACK("OK", 1000)) {
                    if (GPRS_ModuleInit()) {
                        DBG_LOG("GPRS_ModuleInit");
                        GPRS_Param.status = gprs_status_poweron;
                    } else {
                        GPRS_Param.status = gprs_status_nocard;
                    }
                } else {
                    DBG_LOG("GPRS module fault.");
                    GPRS_Param.status = gprs_status_fault;
                    GPRS_KEY_OFF();
                    GPRS_PWR_OFF();
                }
                if (GPRS_Param.status == gprs_status_nocard) {
                    DBG_LOG("GPRS module no SIM card.");
                    GPRS_KEY_OFF();
                    GPRS_PWR_OFF();
                }
            }
            break;
        case gprs_status_poweron:
            GPRS_Param.status = (GetNetWorkStatus() == TRUE) ? gprs_status_online : gprs_status_nonet;
            GPRS_Param.rssi =  GetRSSI();
            break;
        case gprs_status_nonet:
            if (TSEC_IS_OVER(tsNet, 30)) {
                TSEC_INIT(tsNet);
                if (GetNetWorkStatus() == FALSE) {
                    GPRS_Param.ConnectFailCount++;
                } else {
                    GPRS_Param.status = gprs_status_online;
                }
            }
            break;
        default:
            break;
    }
    if (GPRS_Param.status == gprs_status_online) {
        /*设置参数*/
        if (GPRS_OPT(GPRS_OPT_SET_SOCKET)) {
            if (GPRS_Param.IP_Status != ip_status_CLOSED && GPRS_Param.IP_Status != ip_status_INITIAL) {
                ConnectClose();
                TSEC_INIT(tsStatus);
                GPRS_Param.IP_Status = GetIPStatus();
                GPRS_OPT_CLEAR(GPRS_OPT_SET_SOCKET);
            } else {
                GPRS_OPT_CLEAR(GPRS_OPT_SET_SOCKET);
            }
        }
        /*维持TCP链路*/
        switch (GPRS_Param.IP_Status) {
            case ip_status_CLOSED:
            case ip_status_INITIAL:
                TSEC_INIT(tsConnect);
                if (GPRS_Param.ConnectAddress != NULL && GPRS_Param.ConnectPort > 0) {
                    ConnectStart(GPRS_Param.ConnectAddress, GPRS_Param.ConnectPort);
                    GPRS_Param.IP_Status = GetIPStatus();
                }
                break;
            case ip_status_STATUS:
            case PDP_DECT:
                ConnectShut();
                GPRS_Param.IP_Status = ip_status_INITIAL;
                break;
            case ip_status_OK:
                break;
            default:
                /*60秒未成功连接重连*/
                if (TSEC_IS_OVER(tsConnect, 60)) {
                    ConnectShut();
                    GPRS_Param.IP_Status = ip_status_CLOSED;
                }
                if (TSEC_IS_OVER(tsStatus, 3)) {
                    TSEC_INIT(tsStatus);
                    GPRS_Param.IP_Status = GetIPStatus();
                }
                break;
        }
    }
    /*GSM网络离线时复位ip连接状态*/
    else {
        GPRS_Param.IP_Status = ip_status_INITIAL;
    }
}

/**
 * GPRS 串口数据监听处理
 */
static void GPRS_Intercept_Proc(void) {
    char* p = NULL;
    uint32_t ts = 0, tick = 0;
    static uint32_t tsold = 0;
    if (RspBufIndex < (sizeof(RspBuf) - 1)) {
        if (user_uart_ReadByte(&RspBuf[RspBufIndex])) {
#if GPRS_DEBUG > 0
            DBG_SEND(&RspBuf[RspBufIndex], 1);
#endif
            RspBufIndex++;
            app_timer_cnt_get(&tsold);
        }
        app_timer_cnt_get(&ts);
        app_timer_cnt_diff_compute(ts, tsold, &tick);
        if ((RspBufIndex > 3 && RspBuf[RspBufIndex - 1] == '\n' && RspBuf[RspBufIndex - 2] == '\r')
                || tick > 3500) {
            RspBuf[RspBufIndex] = 0;
            p = (char*)RspBuf;
            /*连接状态更新*/
            if (strstr(p, "CLOSED") || strstr(p, "CONNECT OK")) {
                GPRS_Param.IP_Status = GetIPStatus();
            }
            /*网络状态更新*/
            if (strstr(p, "+CREG:")) {
                GPRS_Param.status = ((GetNetWorkStatus() == TRUE) ? gprs_status_online : gprs_status_nonet);
                GPRS_Param.rssi =  GetRSSI();
            }
            GPRS_TCPIP_ReceiveProc(p);
            RspBufIndex = 0;
        }
    } else {
        RspBufIndex = 0;
    }
}


/**
 * TCPIP数据接收收处理
 * @param pReceive 接收到的数据的指针
 */
static void GPRS_TCPIP_ReceiveProc(char* pReceive) {
    uint16_t len = 0;
    char* p = NULL;
    p = strstr(pReceive, "IPD");
    while (p != NULL) {
        len = uatoi(p + 3);
        while (*p && *p++ != ':');
#if GPRS_DEBUG > 1
        DBG_LOG("GPRS Socket Receive:%d\r", len);
        DBG_SEND((uint8_t *)p, len);
#endif
        FIFO_Write(&SockRecFIFO, (uint8_t*)p, len);
        p = p + len + 1;
        p =  strstr(p, "IPD");
    }
}

/**
 * GPRS调试命令
 * @param argc 参数项数量
 * @param argv 参数列表
 */
static BOOL GPRS_ModuleInit(void) {
    char* p = NULL;
    BOOL r = FALSE;
    GPRS_SEND_DATA("ATE1\r", 5);
    GPRS_WAIT_ACK("OK", 100);
    GPRS_SEND_DATA("ATV1\r", 5);
    GPRS_WAIT_ACK("OK", 100);
    GPRS_WAIT_TOKEN("+CPIN:", 5000);
    GPRS_SEND_AT("CSDH=0");
    GPRS_WAIT_ACK("OK", 100);
    GPRS_SEND_AT("QSCLK=0");
    GPRS_WAIT_ACK("OK", 100);
    GPRS_SEND_AT("IFC=0,0");
    GPRS_WAIT_ACK("OK", 100);
    GPRS_SEND_AT("CLVL=10");
    GPRS_WAIT_ACK("OK", 100);
    GPRS_SEND_AT("CMGF=1");
    GPRS_WAIT_ACK("OK", 100);
    GPRS_SEND_AT("CSCS=\"GSM\"");
    GPRS_WAIT_ACK("OK", 100);
    GPRS_SEND_AT("CREG=2");
    GPRS_WAIT_ACK("OK", 100);
    GPRS_SEND_AT("CLIP=1");
    GPRS_WAIT_ACK("OK", 100);
    GPRS_SEND_AT("QIMUX=0");
    GPRS_WAIT_ACK("OK", 100);
    GPRS_SEND_AT("QIHEAD=1");
    GPRS_WAIT_ACK("OK", 100);
    GPRS_SEND_AT("CNMI=2,2,0,0,0");
    GPRS_WAIT_ACK("OK", 100);
    GPRS_SEND_AT("CPIN?");
    p = GPRS_WAIT_TOKEN("+CPIN:", 1000);
    if (p != NULL) {
        if (strstr(p, "READY")) {
            r = TRUE;
        }
    }
    return r;
}

/**
 * GPRS上电开机
 */
static void GPRS_ModulePowerOn(void) {
    /*掉电延时确保模块开机成功*/
    GPRS_KEY_OFF();
    GPRS_PWR_OFF();
    nrf_delay_ms(200);
    GPRS_PWR_ON();
    GPRS_KEY_ON();
}

/**
 * GPRS关机关电
 */
static BOOL GPRS_ModulePowerOff(void) {
    BOOL r = FALSE;
    GPRS_KEY_ON();
    if (GPRS_WAIT_ACK("POWER DOWN", 3000)) {
        r = TRUE;
    }
    GPRS_KEY_OFF();
    GPRS_PWR_OFF();
    return r;
}

/**
 * GPRS关闭连接与PDP场景
 * @return 返回关闭结果
 */
static BOOL ConnectShut(void) {
    GPRS_SEND_AT("CGACT=1");
    return GPRS_WAIT_ACK("OK", 10000) ? TRUE : FALSE;
}

/**
 * GPRS关闭连接
 * @return 返回关闭结果
 */
static BOOL ConnectClose(void) {
    GPRS_SEND_AT("QICLOSE");
    return GPRS_WAIT_ACK("CLOSE OK", 1000) ? TRUE : FALSE;
}

/**
 * GPRS建立连接
 * @param addr 连接的IP地址或者域名
 * @param port 连接的端口号
 * @return 返回连接结果
 */
static BOOL ConnectStart(char* addr, uint16_t port) {
    BOOL r = FALSE;
    GPRS_AT_PRINTF("QIOPEN=\"TCP\",\"%s\",%d", addr, port);
    if (GPRS_WAIT_ACK("CONNECT OK", 3000) || GPRS_WAIT_ACK("ALREADY CONNECT", 3000)) {
        r = TRUE;
    }
    return  r;
}

/**
 * 获取RSII的值
 * @return 返回RSSI的值
 */
static uint8_t GetRSSI(void) {
    uint8_t rssi = 0;
    char* p = NULL;
    GPRS_SEND_AT("CSQ");
    p = GPRS_WAIT_TOKEN("+CSQ:", 1000);
    if (p != NULL) {
        while (*p && !(isdigit(*p))) {
            p++;
        }
        rssi = uatoi(p);
    }
    return rssi;
}
/**
 * 获取RFID的ID值
 * i 通道
 * @return 返回ID的值,无RFID返回0
 */
static uint8_t GetRFID(uint32_t * rfid1, uint32_t *rfid2) {
    uint8_t ret = 0;
    char* p = NULL;
    GPRS_SEND_AT("RFID");
    p = GPRS_WAIT_TOKEN("+RFID:", 500);
    if (p != NULL) {
        while (*p && !(isdigit(*p))) {
            p++;
        }
        *rfid1 = uatoi(p);
        p++;
        while (*p && !(isdigit(*p))) {
            p++;
        }
        *rfid2 = uatoi(p);
        ret = 1;
    }
    return ret;
}
/**
 * 获取RSII的值
 * @return 返回RSSI的值
 */
static IP_Status_t GetIPStatus(void) {
    uint8_t i = 0;
    char* p = NULL, *ps = NULL;
    GPRS_SEND_AT("QISTAT");
    p = GPRS_WAIT_TOKEN("STATE:", 3000);
    if (p != NULL) {
        while (*p && *p++ != ':');
        while (*p == ' ')
            p++;
        do {
            ps = (char*)ipRetArray[i];
            if (strncmp(p, ps, strlen_t(ps)) == 0) {
                break;
            }
            i++;
        } while (ps != NULL);
    }
    return (IP_Status_t)i;
}

/**
 * 获取网络状态
 * @return 返回网络注册状态
 */
static BOOL GetNetWorkStatus(void) {
    uint8_t net = 0;
    char* p = NULL;
    GPRS_SEND_AT("CREG?");
    p = GPRS_WAIT_TOKEN("+CREG:", 1000);
    if (p != NULL) {
        while (*p && *p++ != ',');
        net = uatoi(p);
    }
    return (net == 1 || net == 5) ? TRUE : FALSE;
}

/**
 * TCPIP发送数据
 * @param data 数据指针
 * @param len  数据长度
 * @return 返回发送结果
 */
static uint16_t TCPIP_Send(uint8_t* data, uint16_t len) {
    uint16_t sent = 0;
    char* p = NULL;
    if (data != NULL && len > 0) {
        GPRS_AT_PRINTF("QISEND=%d", len);
        if (GPRS_WAIT_TOKEN(">", 1000)) {
            GPRS_SEND_DATA(data, len);
            p = GPRS_WAIT_TOKEN("SEND OK", 3000);
            if (p != NULL) {
                sent = len;
            }
        }
    }
    return sent;
}

/**
 * 播报TTS
 *
 * @param text   待播报的文本
 */
static void  TTS_Play(char* text) {
    GPRS_AT_PRINTF("QAUDPLAY=\"%s\",0,50,0", text);
    GPRS_WAIT_ACK("OK", 3000);
}

/**
 * GPRS等待AT命令返回
 * @param token 等待的token
 * @param time  等待的最长时间
 * @return 返回等待的token,超时返回NULL
 */
static char* WaitATRsp(char* token, uint16_t time) {
    char* psearch = NULL;
    RspTimeout = FALSE;
    app_timer_stop(TimerId_GPRS);
    app_timer_start(TimerId_GPRS, APP_TIMER_TICKS(time, APP_TIMER_PRESCALER), NULL);
    RspBufIndex = 0;
    while (RspTimeout == FALSE) {
        if (RspBufIndex < (sizeof(RspBuf) - 1) && user_uart_ReadByte(&RspBuf[RspBufIndex])) {
#if GPRS_DEBUG > 0
            DBG_SEND(&RspBuf[RspBufIndex], 1);
#endif
            RspBufIndex++;
            if ((RspBufIndex > 3 && RspBuf[RspBufIndex - 1] == '\n' && RspBuf[RspBufIndex - 2] == '\r')
                    || *token == '>') {
                RspBuf[RspBufIndex] = 0;
                psearch = (char*)SearchMemData(RspBuf, (uint8_t*)token, RspBufIndex, strlen(token));
                if (psearch != NULL || strstr((char*)RspBuf, "ERROR")) {
                    break;
                }
            }
        } else {
            WatchDogClear();
            sd_app_evt_wait();
        }
    }
    RspBufIndex = 0;
    return psearch;
}

/**
 * 超时回调
 *
 * @param p_context
 */
static void TimerCB_GPRS(void* p_context) {
    RspTimeout = TRUE;
}

/**
 * GPRS调试命令
 * @param argc 参数项数量
 * @param argv 参数列表
 */
static void GPRS_Console(int argc, char* argv[]) {
    argv++;
    argc--;
    if (strcmp(argv[0], "power") == 0) {
        if (strcmp(argv[1], "on") == 0) {
            GPRS_SetOnOff(TRUE);
        } else if (strcmp(argv[1], "off") == 0) {
            GPRS_SetOnOff(FALSE);
        }
        DBG_LOG("GPRS power:%s", argv[1]);
    }  else if (strcmp(*argv, "status") == 0) {
        DBG_LOG("GPRS status:%d, IP status:%s.", (int)GPRS_Param.status, ipRetArray[(int)GPRS_Param.IP_Status]);
    }  else if (strcmp(*argv, "key") == 0) {
        if (strcmp(argv[1], "on") == 0) {
            GPRS_KEY_ON();
        } else if (strcmp(argv[1], "off") == 0) {
            GPRS_KEY_OFF();
        }
        DBG_LOG("GPRS power key:%s", argv[1]);
    }  else if (strcmp(*argv, "test") == 0) {
        argv++;
        argc--;
        if (strcmp(argv[0], "poweron") == 0) {
            DBG_LOG("GPRS test power on.");
            GPRS_ModulePowerOn();
        } else if (strcmp(argv[0], "poweroff") == 0) {
            DBG_LOG("GPRS test power off.");
            GPRS_ModulePowerOff();
        } else if (strcmp(argv[0], "csq") == 0) {
            uint8_t rssi = GetRSSI();
            DBG_LOG("GPRS test CSQ:%d.", rssi);
        } else if (strcmp(argv[0], "at") == 0) {
            GPRS_SEND_AT(argv[1]);
            DBG_LOG("gprs test send AT :%s.", argv[1]);
        } else if (strcmp(argv[0], "send") == 0) {
            GPRS_SEND_DATA((uint8_t*)argv[1], strlen(argv[1]));
            GPRS_SEND_DATA("\r\n", 2);
            DBG_LOG("gprs test send data OK.");
        } else  if (strcmp(argv[0], "power") == 0) {
            if (strcmp(argv[1], "on") == 0) {
                GPRS_PWR_ON();
            } else if (strcmp(argv[1], "off") == 0) {
                GPRS_PWR_OFF();
            }
            DBG_LOG("GPRS test power:%s", argv[1]);
        } else  if (strcmp(argv[0], "tts") == 0) {
            TTS_Play(argv[1]);
            DBG_LOG("TTS_Play done.");
        }
    }
}


