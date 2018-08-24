
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
#include "stm8l15x.h"
#include "stm8l15x_usart.h"
#include "stm8l15x_gpio.h"
#include "stm8l15x_spi.h"
#include "stm8l15x_tim1.h"
#include "stm8l15x_exti.h"
#include "stm8l15x_iwdg.h"
#include "stm8l15x_wwdg.h"
#include "stm8l15x_flash.h"

#include "prjlib.h"
#include "MFRC522_API.h"

#include "bsp.h"
#include "app.h"

/* Exported define -----------------------------------------------------------*/
/*信息默认等级,须设置为常量数字宏才能展开*/
#define DEBUG                   1
#define LOG_LEVEL_DEF           4

/*版本号定义*/
#define VERSION                 "ATPA-1707-AD-02.2_v1.0"

/*读保护使能*/
#define RWP_EN                  0

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
#if DEBUG > 0
#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF >= DBG_LEVEL_ERR))
#define DBG_ERR(format, ...)    CMD_Print("error> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_ERR(format, ...)
#endif

#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF >= DBG_LEVEL_WAR))
#define DBG_WAR(format, ...)    CMD_Print("warring> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_WAR(format, ...)
#endif

#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF >= DBG_LEVEL_LOG))
#define DBG_LOG(format, ...)    CMD_Print("log> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_LOG(format, ...)
#endif

#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF >= DBG_LEVEL_INFO))
#define DBG_INFO(format, ...)   CMD_Print("inf> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_INFO(format, ...)
#endif

#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF >= DBG_LEVEL_DEBUG))
#define DBG_DBG(format, ...)    CMD_Print("dbg> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_DBG(format, ...)
#endif

#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF >= DBG_LEVEL_TEMP))
#define DBG_TEMP(format, ...)   CMD_Print("temp> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_TEMP(format, ...)
#endif


/*禁用DEBUG信息*/
#else
#define DBG_ERR(format, ...)
#define DBG_WAR(format, ...)
#define DBG_LOG(format, ...)
#define DBG_INFO(format, ...)
#define DBG_DBG(format, ...)
#define DBG_TEMP(format, ...)
#define DBG_SEND(dat, len)
#define THROW(str)
#define DBG_HEX(dat, len)
#define DBG_PRINT(level, format, ...)
#define DBG_PRINTBUF(level, format, buf, len)

#endif


/* Exported variables --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
void CMD_Print(char *str, ...);


#endif

