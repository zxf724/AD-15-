/*
  **********************************  STM8S  ***********************************
  * @文件名     ： app.c
  * @作者       ： Huang Fugui
  * @库版本     ： V2.2.0
  * @文件版本   ： V1.0.0
  * @日期       ： 2016年09月02日
  * @摘要       ： 应用程序源文件
  ******************************************************************************/
#include "user_comm.h"
#include "uart.h"

uint8_t DevAddress = 0;

static uint32_t tsUART = 0;
static FIFO_t   UART_RecFIFO;
static uint8_t  UART_RecBuffer[UART_FIFO_BUF_SIZE], IC_ReadBuf[20], IC_Read = 0,error = 0x00,sucess = 0xff;

/**
 * APP初始化
 */
void App_Init(void)
{
    FIFO_Init(&UART_RecFIFO, UART_RecBuffer, sizeof(UART_RecBuffer));
}

/**
 * 刷卡轮循
 */
void RFID_Polling(void)
{
    if (RfidReadData(&IC_ReadBuf[4], CARD_BLOCK, 1, 0)) {
        LED_ON(RED);
        memcpy(IC_ReadBuf, MLastSelectedSnr, 4);
        IC_Read = 1;
    } else {
        LED_OFF(RED);
        IC_Read = 0;
    }
}

/**
 * 发送命令
 *
 * @param cmd
 * @param arg    发送参数
 */
void SendCmd(uint8_t cmd, uint8_t *data, uint8_t datalen)
{
    uint8_t buf[32], *p = buf, i = 0, len = 5;

    *p++ = 0x7E;
    *p++ = 0x1B;
    *p++ = DevAddress;
    *p++ = cmd;
    *p++ = datalen;
    if (data != NULL && datalen > 0) {
        memcpy(p, data, datalen);
        p += datalen;
        len = len + datalen;
    }
    *p = AddCheck(buf, len);
    len += 1;
    p = buf;

    EN485_Recevie_OFF;

    for (i = 0; i < len; i++) {
        while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET);
        UART1_SendData8(*p++);
    }
    while (UART1_GetFlagStatus(UART1_FLAG_TC) == RESET);
    EN485_Recevie_ON;
}

void SendID_Data(void)
{
  if (IC_Read)
  {
      SendCmd(CMD_READ_IC_RSP, IC_ReadBuf, 4);
  }
  
}

/**
 * 接收处理命令
 */
void ReadCmdDeal(void)
{

    if (TS_IS_OVER(tsUART, 50)) {
        while (FIFO_Length(&UART_RecFIFO) >= 6 && UART_RecFIFO.pBuffer[0] != 0x7E);
        if (FIFO_Length(&UART_RecFIFO) >= 5 &&UART_RecFIFO.pBuffer[1] == 0x1B) {
          
          if(Check(UART_RecFIFO.pBuffer,UART_RecFIFO.wpos-1) != UART_RecFIFO.pBuffer[UART_RecFIFO.wpos-1])
          {
            UART1_SendByte((uint8_t)error);
          }
            if ((DevAddress == UART_RecFIFO.pBuffer[2] || UART_RecFIFO.pBuffer[2] == 0) 
                && Check(UART_RecFIFO.pBuffer,UART_RecFIFO.wpos-1) == UART_RecFIFO.pBuffer[UART_RecFIFO.wpos-1]) {
                if (UART_RecFIFO.pBuffer[3] == CMD_DEVICE_RST) {
                    LED_ON(RED);
                    WWDG_SWReset();
                    while (1);
                } else if (UART_RecFIFO.pBuffer[3] == CMD_DEVICE_CHK) {
                    LED_ON(RED);
                    SendCmd(CMD_DEVICE_CHK_RSP, NULL, 0);
                } else if (UART_RecFIFO.pBuffer[3] == CMD_READ_IC) {
                    if (IC_Read) {
                        LED_ON(RED);
                        SendCmd(CMD_READ_IC_RSP, IC_ReadBuf, 4);
                    } else {
                        LED_ON(RED);
                        TS_DELAY(100);
                        LED_OFF(RED);
                    }
                } else if (UART_RecFIFO.pBuffer[3] == CMD_READ_IC_FRIM) {
                    if (IC_Read) {
                        LED_ON(RED);
                        SendCmd(CMD_READ_IC_FRIM_RSP, IC_ReadBuf, 20);
                    } else {
                        LED_ON(RED);
                        TS_DELAY(100);
                        LED_OFF(RED);
                    }
                }
                else if (UART_RecFIFO.pBuffer[3] == CMD_WRITE_FRIM && UART_RecFIFO.pBuffer[4] == 0x10) {
                    LED_ON(RED);
                    /*往IC卡中写入厂商信息*/
                    if (RfidWriteData(CARD_BLOCK, 1, &UART_RecFIFO.pBuffer[5])) {
                      UART1_SendByte((uint8_t)sucess);
                        LED_ON(RED);
                        TS_DELAY(200);
                        LED_OFF(RED);
                        TS_DELAY(200);
                        IWDG_ReloadCounter();
                        LED_ON(RED);
                        TS_DELAY(200);
                        LED_OFF(RED);
                        TS_DELAY(200);
                        IWDG_ReloadCounter();
                    }
                }
            }
        }
    }

    if (TS_IS_OVER(tsUART, 1000)) {
        LED_OFF(GREEN);
        if (FIFO_Length(&UART_RecFIFO) > 0 && TS_IS_OVER(tsUART, 3000)) {
            FIFO_Flush(&UART_RecFIFO);
        }
    }
}

/**
 * 串口接收到新的数据
 *
 * @param data   新接收到的数据
 */
void UART_NewData(uint8_t data)
{
    FIFO_Put(&UART_RecFIFO, data);
//    UART1_SendByte((uint8_t)data);
    TS_INIT(tsUART);
}

/************************************************
函数名称 ： GET_DeviceAddress
功    能 ： 得到设置地址
参    数 ： 无

返 回 值 ： 无
作    者 ： Huang Fugui
*************************************************/
void GET_DeviceAddress(void)
{
    DevAddress |= KEY1;
    DevAddress |= KEY2 << 1;
    DevAddress |= KEY3 << 2;
    DevAddress |= KEY4 << 3;
    DevAddress += 0xA0;
}





/***** Copyright (C)2016 HuangFugui. All Rights Reserved ***** END OF FILE *****/
