/**
  **********************************  STM8S  ***********************************
  * @文件名     ： app.h
  * @作者       ： Huang Fugui
  * @库版本     ： V2.2.0
  * @文件版本   ： V1.0.0
  * @日期       ： 2016年09月02日
  * @摘要       ： 应用程序头文件
  ******************************************************************************/

/* 定义防止递归包含 ----------------------------------------------------------*/
#ifndef _APP_H
#define _APP_H

/* 包含的头文件 --------------------------------------------------------------*/
#include "stm8s.h"


/* 宏定义 --------------------------------------------------------------------*/

#define UART_FIFO_BUF_SIZE      128


/*协议命令*/
#define CMD_DEVICE_RST          0xFF
#define CMD_DEVICE_CHK          0x01
#define CMD_DEVICE_CHK_RSP      0x11
#define CMD_READ_IC             0x02
#define CMD_READ_IC_RSP         0x12
#define CMD_READ_IC_FRIM        0x03
#define CMD_READ_IC_FRIM_RSP    0x13
#define CMD_WRITE_ID            0x04
#define CMD_WRITE_FRIM          0x14
#define CMD_TEST                0x55

#define CARD_TYPE_USER          0x01                     //卡类型 - 普通卡

#define CARD_BLOCK              1                        //操作卡的那一块数据
#define START_BLOCK             1
#define CARD_BLOCK_COUN         4                        //卡次数保存在卡所在块-起始块(读取普通卡剩余次数)



/* 函数申明 ------------------------------------------------------------------*/
void App_Init(void);

void RFID_Polling(void);
void SendCmd(uint8_t cmd, uint8_t *data, uint8_t datalen);
void ReadCmdDeal(void);
void UART_NewData(uint8_t data);
void GET_DeviceAddress(void);

#endif /* _APP_H */

/***** Copyright (C)2016 HuangFugui. All Rights Reserved ***** END OF FILE *****/
