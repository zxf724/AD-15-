/**
 * **********************************************************************
 *             Copyright (c) 2016 temp. All Rights Reserved.
 * @file uaer_comm.h
 * @author 宋阳
 * @version V1.0
 * @date 2016.4.1
 * @brief 用户公用头文件.
 *
 * **********************************************************************
 * @note
 *
 * **********************************************************************
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _USER_COMM_H
#define _USER_COMM_H

/* Includes ------------------------------------------------------------------*/


#include "prjlib.h"
#include "user_uart.h"
#include "spi_flash.h"

#include "datasave.h"

   
   
   
   


/* Exported define -----------------------------------------------------------*/
/*调试串口号，为0时关闭打印输出*/


/*DEBUG 信息等级动态可设置使能*/
#define LOG_LEVEL_DYNAMIC   1

/*DEBUG
  信息默认等级,须设置为常量数字宏才能展开*/
#define LOG_LEVEL_DEF       4

/*GPRS的串口号*/
#define GPRS_UART_PORT      2

/*RFID总线串口号*/
#define RFID_UART_PORT      3

/*定义DFU标记BKP寄存器*/
#define DFU_BKP             (BKP->DR10)

/*定义复位标识记数BKP*/
#define NRST_BKP            (BKP->DR9)
#define IWDG_BKP            (BKP->DR8)
#define SWRST_BKP           (BKP->DR7)
#define PORRST_BKP          (BKP->DR6)

/*使能flash读写保护*/
#define FLASH_WRP_EN        1

/*使能硬件看门狗*/
#define IWDG_HW_EN          1

/*版本号定义*/
//#define PROJECT             "AD-02"
//#define VERSION             "AD-02_FM_V1.01"
//#define VERSION_HARDWARE    "AD-02_HD_V1.0"
//#define VERSION_GSM         "SIM800C"

/*UART接收缓存的大小，必须为2的幂次方值*/
#define UART1_RECEVIE_BUFFER_SIZE   2048
#define UART2_RECEVIE_BUFFER_SIZE   2048
#define UART3_RECEVIE_BUFFER_SIZE   1024

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
#define DBG_LEVEL_ERR           1
#define DBG_LEVEL_WAR           2
#define DBG_LEVEL_LOG           3
#define DBG_LEVEL_INFO          4
#define DBG_LEVEL_DEBUG         5
#define DBG_LEVEL_TEMP          6

/* Exported macro ------------------------------------------------------------*/

/*启用DEBUG信息*/


/*DEBUG 信息等级静态*/
#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF <= DBG_LEVEL_ERR))
#define DBG_ERR(format, ...)    CMD_Printf("error> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_ERR(format, ...)
#endif

#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF <= DBG_LEVEL_WAR))
#define DBG_WAR(format, ...)    CMD_Printf("warring> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_WAR(format, ...)
#endif

#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF <= DBG_LEVEL_LOG))
#define DBG_LOG(format, ...)    CMD_Printf("log> "format"\r\n", ##__VA_ARGS__)

#endif

#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF <= DBG_LEVEL_INFO))
#define DBG_INFO(format, ...)   CMD_Printf("inf> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_INFO(format, ...)
#endif

#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF <= DBG_LEVEL_DEBUG))
#define DBG_DBG(format, ...)    CMD_Printf("dbg> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_DBG(format, ...)
#endif

#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF <= DBG_LEVEL_TEMP))
#define DBG_TEMP(format, ...)   CMD_Printf("temp> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_TEMP(format, ...)
#endif



#define THROW(str)
#define DBG_HEX(dat, len)
#define DBG_PRINT(level, format, ...)
#define DBG_PRINTBUF(level, format, buf, len)

/* Exported variables --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/



#endif
