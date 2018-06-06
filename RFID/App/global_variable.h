/**
  **********************************  STM8S  ***********************************
  * @文件名     ： global_variable.h
  * @作者       ： Huang Fugui
  * @库版本     ： V2.2.0
  * @文件版本   ： V1.0.0
  * @日期       ： 2016年09月02日
  * @摘要       ： 全局变量头文件
  ******************************************************************************/

/* 定义防止递归包含 ----------------------------------------------------------*/
#ifndef _GLOBAL_VERIABLE_H
#define _GLOBAL_VERIABLE_H

/* 包含的头文件 --------------------------------------------------------------*/
#include "global_define.h"


/* 全局变量 ------------------------------------------------------------------*/
/* 串口 */
extern uint8_t  gRx_Buf[];                                 //串口接收数据BUF
extern uint8_t  gRx_Ok;                                    //数据包接收完成(有效)

/* 关于刷卡 */
extern uint8_t  gEeprom_Buf[];                             //数据BUF
extern uint16_t gCount;                                    //刷卡总次数
extern uint16_t gRecord_Offset;                            //记录偏移

extern uint8_t  gDeviceID;                                //刷卡机(设备)编号

/* TIM定时器 */
extern uint8_t  gTIMTiming2_FlagOver;                      //定时2时间溢出标志
extern uint16_t gTIMTiming2_Num;                           //定时2计数

/* 卡数据块 */
extern uint8_t  gCardBlock_Buf[];                          //卡一块数据BUF

/* 卡ID号 */
extern uint8_t  gCardID[];                                 //最后一次刷卡ID号

/* 定时器延时 */
extern uint16_t gTiming_Card;                              //刷卡定时（防止两种相同卡重复操作）
extern uint16_t gTiming_Out;                               //输出定时（继电器输出）

/* 设备地址 */
extern  uint8_t  gDeviceAddress;



#endif /* _GLOBAL_VERIABLE_H */

/***** Copyright (C)2016 HuangFugui. All Rights Reserved ***** END OF FILE *****/
