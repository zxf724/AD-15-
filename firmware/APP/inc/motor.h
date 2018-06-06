/**
 * **********************************************************************
 *             Copyright (c) 2017 AFU All Rights Reserved.
 * @file expresslock.h
 * @author 宋阳
 * @version V1.0
 * @date 2017.7.12
 * @brief 快递框锁控处理函数头文件.
 *
 * **********************************************************************
 * @note
 *
 * **********************************************************************
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _EXPRESSLOCK_H
#define _EXPRESSLOCK_H


/* Includes ------------------------------------------------------------------*/
#include "prjlib.h"
#include "cjson.h"

/* Exported define -----------------------------------------------------------*/
/*外部锁控板的波特率*/
#define LOCK_UART_BDR           9600

/*锁控的最大数路*/
#define LOCK_INT_PATH_MAX       10
#define LOCK_EXT_PATH_MAX       16

/* Exported types ------------------------------------------------------------*/
typedef struct eLOCK {
    uint8_t     header1;
    uint8_t     header2;
    uint8_t     setAdd;
    uint8_t     lock_switch;            //0x01是开锁  0x00是反馈
    uint32_t    address;                //后24位为锁地址。 最高8位预留，最低8位为第1组锁地址
    uint32_t    lock_status;
} eLOCK;


/* Exported constants --------------------------------------------------------*/
#define CMD_OPEN                0x01
#define CMD_STATUS_REQ          0x02
#define CMD_STATUS_RSP          0x03
#define CMD_PATH_REQ            0x04
#define CMD_PATH_RSP            0x05

/* Exported macro ------------------------------------------------------------*/
#define MOTOR_FORWARD(i)            do{IO_H(MOTOR_##i##A); IO_L(MOTOR_##i##B);}while(0)
#define MOTOR_BACK(i)               do{IO_H(MOTOR_##i##B); IO_L(MOTOR_##i##A);}while(0)
#define MOTOR_STOP(i)               do{IO_L(MOTOR_##i##B); IO_L(MOTOR_##i##A);}while(0)
#define MOTOR_IS_STUCK(i)           (IO_READ(SENSOR##i) == 0)


/* Exported variables --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
void Motor_Init(void);

void Motor_Forward(uint8_t path);
void Motor_Back(uint8_t path);
void Motor_Stop(uint8_t path);
uint8_t Motor_IsStuck(uint8_t path);

#endif
