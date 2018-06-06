
/**
 * *********************************************************************
 *             Copyright (c) 2017 AFU  All Rights Reserved.
 * @file moetor.c
 * @version V1.0
 * @date 2017.7.12
 * @brief 一秦共享雨伞电机控制函数.
 *
 * *********************************************************************
 * @note
 *
 * *********************************************************************
 * @author 宋阳
 */



/* Includes ------------------------------------------------------------------*/
#include "user_comm.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void motor_Console(int argc, char *argv[]);

/* Exported functions --------------------------------------------------------*/

/**
 * DFU初始化
 */
void Motor_Init(void)
{
    CMD_ENT_DEF(motor, motor_Console);
    Cmd_AddEntrance(CMD_ENT(motor));

    DBG_LOG("Motor Driver Init.");
}

/**
 * 电机前向运行
 *
 * @param path   动作的分路
 */
void Motor_Forward(uint8_t path)
{
    if (path == 1) {
        MOTOR_FORWARD(11);
    } else if (path == 2) {
        MOTOR_FORWARD(21);
    } else if (path == 3) {
        MOTOR_FORWARD(12);
    } else if (path == 4) {
        MOTOR_FORWARD(22);
    } else if (path == 5) {
        MOTOR_FORWARD(13);
    } else if (path == 6) {
        MOTOR_FORWARD(23);
    } else if (path == 7) {
        MOTOR_FORWARD(14);
    } else if (path == 8) {
        MOTOR_FORWARD(24);
    } else if (path == 9) {
        MOTOR_FORWARD(15);
    } else if (path == 10) {
        MOTOR_FORWARD(25);
    } else if (path == 11) {
        MOTOR_FORWARD(16);
    } else if (path == 12) {
        MOTOR_FORWARD(26);
    }
}

/**
 * 电机反向运行
 *
 * @param path   动作的分路
 */
void Motor_Back(uint8_t path)
{
    if (path == 1) {
        MOTOR_BACK(11);
    } else if (path == 2) {
        MOTOR_BACK(21);
    } else if (path == 3) {
        MOTOR_BACK(12);
    } else if (path == 4) {
        MOTOR_BACK(22);
    } else if (path == 5) {
        MOTOR_BACK(13);
    } else if (path == 6) {
        MOTOR_BACK(23);
    } else if (path == 7) {
        MOTOR_BACK(14);
    } else if (path == 8) {
        MOTOR_FORWARD(24);
    } else if (path == 9) {
        MOTOR_BACK(15);
    } else if (path == 10) {
        MOTOR_BACK(25);
    } else if (path == 11) {
        MOTOR_BACK(16);
    } else if (path == 12) {
        MOTOR_BACK(26);
    }
}

/**
 * 电机停止运行
 *
 * @param path   停止的分路
 */
void Motor_Stop(uint8_t path)
{
    if (path == 1) {
        MOTOR_STOP(11);
    } else if (path == 2) {
        MOTOR_STOP(21);
    } else if (path == 3) {
        MOTOR_STOP(12);
    } else if (path == 4) {
        MOTOR_STOP(22);
    } else if (path == 5) {
        MOTOR_STOP(13);
    } else if (path == 6) {
        MOTOR_STOP(23);
    } else if (path == 7) {
        MOTOR_STOP(14);
    } else if (path == 8) {
        MOTOR_STOP(24);
    } else if (path == 9) {
        MOTOR_STOP(15);
    } else if (path == 10) {
        MOTOR_STOP(25);
    } else if (path == 11) {
        MOTOR_STOP(16);
    } else if (path == 12) {
        MOTOR_STOP(26);
    }
}

/**
 * 查询电机是否被卡住
 *
 * @param path   查询的分路
 * @return 电机卡住返回1
 */
uint8_t Motor_IsStuck(uint8_t path)
{
    uint8_t ret = 0;

    if (path == 1) {
        ret = MOTOR_IS_STUCK(11);
    } else if (path == 2) {
        ret = MOTOR_IS_STUCK(21);
    } else if (path == 3) {
        ret = MOTOR_IS_STUCK(12);
    } else if (path == 4) {
        ret = MOTOR_IS_STUCK(22);
    } else if (path == 5) {
        ret = MOTOR_IS_STUCK(13);
    } else if (path == 6) {
        ret = MOTOR_IS_STUCK(23);
    } else if (path == 7) {
        ret = MOTOR_IS_STUCK(14);
    } else if (path == 8) {
        ret = MOTOR_IS_STUCK(24);
    } else if (path == 9) {
        ret = MOTOR_IS_STUCK(15);
    } else if (path == 10) {
        ret = MOTOR_IS_STUCK(25);
    } else if (path == 11) {
        ret = MOTOR_IS_STUCK(16);
    } else if (path == 12) {
        ret = MOTOR_IS_STUCK(26);
    }
    return ret;
}

/* Private function prototypes -----------------------------------------------*/

/**
 * 锁控调试命令
 *
 * @param argc   参数个数
 * @param argv   参数列表
 */
static void motor_Console(int argc, char *argv[])
{
    uint8_t path = 0;

    argv++;
    argc--;
    if (ARGV_EQUAL("test")) {
        path = uatoi(argv[2]);
        argv += 1;
        argc -= 1;
        if (ARGV_EQUAL("foward")) {
            Motor_Forward(path);
        } else if (ARGV_EQUAL("back")) {
            Motor_Back(path);
        } else if (ARGV_EQUAL("stop")) {
            Motor_Stop(path);
        } else if (ARGV_EQUAL("isstuck")) {
            DBG_LOG("Motor path%u stuck status:%u", path, Motor_IsStuck(path));
        }
    }
}

