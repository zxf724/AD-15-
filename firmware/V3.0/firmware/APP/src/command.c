/**
  ******************************************************************************
  * @file    command.c
  * @author  宋阳
  * @version V1.0
  * @date    2015.11.31
  * @brief   命令处理相关函数.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "includes.h"
#include "ble_gap.h"

/** @addtogroup firmwave_F2_BLE
  * @{
  */



/** @defgroup COMMAND
  * @{
  */


/* Private typedef -----------------------------------------------------------*/

/** @defgroup COMMAND_Private_Typedef COMMAND Private Typedef
  * @{
  */

/**
  * @}
  */

/* Private define ------------------------------------------------------------*/
/** @defgroup COMMAND_Private_Constants COMMAND Private Constants
  * @{
  */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup COMMAND_Private_Macros COMMAND Private Macros
  * @{
  */


/**
  * @}
  */
/* Private variables ---------------------------------------------------------*/

/** @defgroup COMMAND_Private_Variables Private Variables
  * @{
  */

/**
  * @brief 定义串口收发缓存.
  */
static uint8_t  CmdRecBuf[COMMAND_MAX];



/**
  * @}
  */

/* Private function prototypes -----------------------------------------------*/
/** @defgroup command_Private_Functions command Private Functions
  * @{
  */
static void funSystem(int argc, char* argv[]);
static void funTime(int argc, char* argv[]);
static void funFICR(int argc, char* argv[]);
static void funUICR(int argc, char* argv[]);

/**
  * @}
  */

/* Exported functions ---------------------------------------------------------*/

/** @defgroup command_Exported_Functions command Exported Functions
  *  @brief   command 外部接口函数
  * @{
  */

/**
  * @brief  初始化命令处理.
  * @param  none.
  * @retval none
  */
void CommandInit(void) {
    CMD_ENT_DEF(system, funSystem);
    Cmd_AddEntrance(CMD_ENT(system));
    CMD_ENT_DEF(FICR, funFICR);
    Cmd_AddEntrance(CMD_ENT(FICR));
    CMD_ENT_DEF(UICR, funUICR);
    Cmd_AddEntrance(CMD_ENT(UICR));
    CMD_ENT_DEF(time, funTime);
    Cmd_AddEntrance(CMD_ENT(time));
    DBG_LOG("Command Init.");
}

/**
  * @brief  命令处理轮询.
  */
void CommandReceivePoll(void) {
    char* p = NULL;
    static uint16_t index = 0;
    if (NRF_UART0->PSELRXD == RX_PIN_NUMBER) {
        UART_MutexCount++;
        if (user_uart_ReadByte(&CmdRecBuf[index])) {
            if (CmdRecBuf[index] == '\n') {
                DBG_SEND(CmdRecBuf, index + 1);
                UART_RX_PIN_SELECT(UART_RX_DEFAULT_PIN);
                CmdRecBuf[index] = '\0';
                p = (char*)&CmdRecBuf[0];
                while (!isgraph(*p)) {
                    p++;
                }
                Cmd_Handle(p);
                index = 0;
            } else if (index++ >= COMMAND_MAX - 1) {
                index = 0;
                UART_RX_PIN_SELECT(UART_RX_DEFAULT_PIN);
            }
        }
        UART_MutexCount--;
    }
}

/**
  * @}
  */

/** @addtogroup COMMAND_Private_Functions
  * @{
  */

/**
  * @brief  复位命令处理.
  */
static void funSystem(int argc, char* argv[]) {
    uint32_t d = 0, i = 0, j = 0;
    uint8_t buf[22];
    argv++;
    argc--;
    if (ARGV_EQUAL("reset")) {
        d = uatoi(argv[i + 1]);
        DBG_LOG("System Will Reset %d ms latter.", d);
        user_BLE_Disconnected();
        nrf_delay_ms(d + 10);
        NVIC_SystemReset();
    } else if (ARGV_EQUAL("testble")) {
        d = uatoi(argv[i + 1]);
        for (j = 0; j < d; j++) {
            sprintf((char*)buf, "test send:%d\r\n", j);
            user_BLE_Send(buf, strlen((char*)buf));
            DBG_LOG("test BLE send: %s", buf);
        };
    } else if (ARGV_EQUAL("mac")) {
        ble_gap_addr_t mac_addr;
        sd_ble_gap_address_get(&mac_addr);
        DBG_LOG("MAC:%02X:%02X:%02X:%02X:%02X:%02X",
                mac_addr.addr[5], mac_addr.addr[4], mac_addr.addr[3],
                mac_addr.addr[2], mac_addr.addr[1], mac_addr.addr[0]);
    } else if (ARGV_EQUAL("version")) {
        DBG_LOG("system version:%u", VERSION);
    }
    /*
      else if (ARGV_EQUAL("batvol")) {
        DBG_LOG("System battery vol:%umv.", BSP_BatteryVol_Read());

      } else if (ARGV_EQUAL("batpercent")) {
        DBG_LOG("System battery percent:%u%%.", BSP_BatteryPercent_Read());

      }
      */
}


/**
  * @brief  时间命令处理.
  */
static void funTime(int argc, char* argv[]) {
    int i = 0;
    timeRTC_t time;
    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "set") == 0) {
            time.year = uatoi(argv[i + 1]);
            time.month = uatoi(argv[i + 2]);
            time.date = uatoi(argv[i + 3]);
            time.hours = uatoi(argv[i + 4]);
            time.minutes = uatoi(argv[i + 5]);
            time.seconds = uatoi(argv[i + 6]);
            time.day = uatoi(argv[i + 7]);
            RTC_SetTime(&time);
        } else if (strcmp(argv[i], "year") == 0) {
            RTC_ReadTime(&time);
            time.year = uatoi(argv[i + 1]);
            RTC_SetTime(&time);
        } else if (strcmp(argv[i], "month") == 0) {
            RTC_ReadTime(&time);
            time.month = uatoi(argv[i + 1]);
            RTC_SetTime(&time);
        } else if (strcmp(argv[i], "date") == 0) {
            RTC_ReadTime(&time);
            time.date = uatoi(argv[i + 1]);
            RTC_SetTime(&time);
        } else if (strcmp(argv[i], "hour") == 0) {
            RTC_ReadTime(&time);
            time.hours = uatoi(argv[i + 1]);
            RTC_SetTime(&time);
        } else if (strcmp(argv[i], "minute") == 0) {
            RTC_ReadTime(&time);
            time.minutes = uatoi(argv[i + 1]);
            RTC_SetTime(&time);
        } else if (strcmp(argv[i], "seconds") == 0) {
            RTC_ReadTime(&time);
            time.seconds = uatoi(argv[i + 1]);
            RTC_SetTime(&time);
        }
    }
    RTC_ReadTime(&time);
    DBG_LOG("time: %04d-%d-%d %02d:%02d:%02d day-%d ", time.year,
            time.month, time.date, time.hours, time.minutes,
            time.seconds, time.day);
}

/**
  * @brief  FCIR命令处理.
  */
static void funFICR(int argc, char* argv[]) {
    int i = 0;
    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "codesize") == 0) {
            DBG_LOG("code size: %d", NRF_FICR->CODESIZE);
        } else if (strcmp(argv[i], "pagesize") == 0) {
            DBG_LOG("code page size: %d", NRF_FICR->CODEPAGESIZE);
        } else if (strcmp(argv[i], "PPFC") == 0) {
            DBG_LOG("PPFC: 0x%04X", NRF_FICR->PPFC);
        }
    }
}

/**
  * @brief  UCIR命令处理.
  */
static void funUICR(int argc, char* argv[]) {
    int i = 0;
    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "FWID") == 0) {
            DBG_LOG("FWID: 0x%04X", NRF_UICR->FWID);
        } else if (strcmp(argv[i], "BOOTADDR") == 0) {
            DBG_LOG("Bootloader Address: 0x%04X", NRF_UICR->BOOTLOADERADDR);
        }
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

/************************ (C) COPYRIGHT 深圳逗爱创新科技有限公司 *****END OF FILE****/
