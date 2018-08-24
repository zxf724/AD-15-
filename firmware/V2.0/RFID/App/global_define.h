/**
  **********************************  STM8S  ***********************************
  * @文件名     ： global_define.h
  * @作者       ： Huang Fugui
  * @库版本     ： V2.2.0
  * @文件版本   ： V1.0.0
  * @日期       ： 2016年09月02日
  * @摘要       ： 全局定义
  ******************************************************************************/

/* 定义防止递归包含 ----------------------------------------------------------*/
#ifndef _GLOBAL_DEFINE_H
#define _GLOBAL_DEFINE_H

/* 包含的头文件 --------------------------------------------------------------*/
#include "stm8s.h"


/* 宏定义 --------------------------------------------------------------------*/
/* 标志位 */
#define FLAG_N_VALID              0xFF                     //标志无效
#define FLAG_VALID                0x80                     //标志有效

/* 串口 */
#define RXBUF_LEN                 0x04                     //串口接收BUF长度


#define RECORD_PRE_SIZE           3                        //每条记录长度
#define RECORD_COUNT_MAX          2600                     //记录最大条数（8000/3 = 2666）

//---------------------------- EEPROM映射 -----------------------------
/* EEPROM为AT24C64 共8K字节，最大地址为8192-1 */
#define EEADDR_DEVICE_ID          8000                     //刷卡机设备ID

#define EEADDR_CARD_COUNT         8010                     //刷卡总条数-普通卡扣减次数(共2字节)
#define EEADDR_RECORD_OFFSET      8012                     //记录偏移(共2字节)

#define EEADDR_CLEAR_CARD_ID      8020                     //清零卡ID(共4字节)


#endif /* _GLOBAL_DEFINE_H */

/***** Copyright (C)2016 HuangFugui. All Rights Reserved ***** END OF FILE *****/
