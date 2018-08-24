/**
  **********************************  stm8l15x  ***********************************
  * @文件名     ： uart.c
  * @作者       ： Huang Fugui
  * @库版本     ： V2.2.0
  * @文件版本   ： V1.0.0
  * @日期       ： 2016年09月02日
  * @摘要       ： UART源文件
  ******************************************************************************/
/*----------------------------------------------------------------------------
  更新日志:
  2016-09-02 V1.0.0:初始版本
  ----------------------------------------------------------------------------*/
/* 包含的头文件 --------------------------------------------------------------*/
#include "uart.h"


/************************************************
函数名称 ： UART_Initializes
功    能 ： UART初始化
参    数 ： 无
返 回 值 ： 无
作    者 ： Huang Fugui
*************************************************/
void UART_Initializes(void)
{
#if 0
  UART1_Init((uint32_t)9600, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO, UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);
  UART1_Cmd(ENABLE);
  UART1_ITConfig(UART1_IT_RXNE_OR, ENABLE);

#else
  UART1->BRR2 = 0x02;
  UART1->BRR1 = 0x68;
//UART1->CR1  = 0x00;
  UART1->CR2  = 0x2C;
//UART1->SR   = 0xC0;
#endif
}

/************************************************
函数名称 ： UART1_SendByte
功    能 ： UART1发送一个字符
参    数 ： Data --- 数据
返 回 值 ： 无
作    者 ： Huang Fugui
*************************************************/
void UART1_SendByte(uint8_t Data)
{
  while((USART_GetFlagStatus(USART1, UART1_FLAG_TXE)==RESET));
  USART_SendData8(USART1, Data);
}


/************************************************
函数名称 ： UART1_SendNByte
功    能 ： 串口1发送N个字符
参    数 ： pData ----- 字符串
            Length --- 长度
返 回 值 ： 无
作    者 ： Huang Fugui
*************************************************/
void UART1_SendNByte(uint8_t *pData, uint16_t Length)
{
  while(Length--)
  {
    UART1_SendByte(*pData);
    pData++;
  }
}

/************************************************
函数名称 ： UART1_Printf
功    能 ： 串口1打印输出
参    数 ： string --- 字符串
返 回 值 ： 无
作    者 ： Huang Fugui
*************************************************/
void UART1_Printf(uint8_t *String)
{
  while((*String) != '\0')
  {
    UART1_SendByte(*String);
    String++;
  }
}

#if 1
/******************************************* 串口重定义 *******************************************/
/************************************************
函数名称 ： fputc
功    能 ： 重定义putc函数
参    数 ： ch --- 字符
            *f --- 文件指针
返 回 值 ： 字符
作    者 ： Huang Fugui
*************************************************/
int fputc(int ch, FILE *f)
{
  UART1_SendByte((uint8_t)ch);

  return ch;
}

/************************************************
函数名称 ： fgetc
功    能 ： 重定义getc函数
参    数 ： *f --- 文件指针
返 回 值 ： 字符
作    者 ： Huang Fugui
*************************************************/
int fgetc(FILE *f)
{
  while(USART_GetFlagStatus(USART1, UART1_FLAG_RXNE) == RESET);

  return (int)UART1_ReceiveData8();
}
#endif


/***** Copyright (C)2016 HuangFugui. All Rights Reserved ***** END OF FILE *****/
