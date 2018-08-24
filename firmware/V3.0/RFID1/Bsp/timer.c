/**
  **********************************  stm8l15x  ***********************************
  * @文件名     ： timer.c
  * @作者       ： Huang Fugui
  * @库版本     ： V2.2.0
  * @文件版本   ： V1.0.0
  * @日期       ： 2016年09月02日
  * @摘要       ： TIMER源文件
  ******************************************************************************/
/*----------------------------------------------------------------------------
  更新日志:
  2016-09-02 V1.0.0:初始版本
  ----------------------------------------------------------------------------*/
/* 包含的头文件 --------------------------------------------------------------*/
#include "timer.h"
#include "global_variable.h"


/************************************************
函数名称 ： TIMER_Initializes
功    能 ： 定时器初始化
参    数 ： 无
返 回 值 ： 无
作    者 ： Huang Fugui
*************************************************/
void TIMER_Initializes(void)
{
#if 0
                                                  //(定时100毫秒)
  TIM1_TimeBaseInit(16000, TIM1_COUNTERMODE_UP, 100, 0);
  TIM1->IER |= (uint8_t)TIM1_IT_UPDATE;          //中断配置

  TIM2_TimeBaseInit(TIM2_PRESCALER_16, 1000);    //16M/16 / 1000 = 1000(定时1毫秒)
  TIM2->IER |= (uint8_t)TIM2_IT_UPDATE;          //中断配置

                                                 //TIM4延时10us (2分频=8M  计数80个脉冲)
  TIM4_TimeBaseInit(TIM4_PRESCALER_2, 79);
  TIM4_ClearFlag(TIM4_FLAG_UPDATE);

#else
  TIM1->ARRH  = 0x00;
  TIM1->ARRL  = 0x64;
  TIM1->PSCRH = 0x3E;
  TIM1->PSCRL = 0x80;
  TIM1->IER   = 0x01;

  TIM2->PSCR  = 0x04;
  TIM2->ARRH  = 0x03;
  TIM2->ARRL  = 0xE8;
  TIM2->IER   = 0x01;

  TIM4->PSCR  = 0x01;
  TIM4->ARR   = 0x4F;
#endif

  TIM1->CR1 |= (uint8_t)TIM1_CR1_CEN;            //启动定时器1（间隔1秒中断一次）
}

/************************************************
函数名称 ： TIM2_Timing_Nms
功    能 ： 定时NS
参    数 ： Times --- N值
返 回 值 ： 无
作    者 ： Huang Fugui
*************************************************/
void TIM2_Timing_Nms(uint16_t Times)
{
  gTIMTiming2_Num = Times;
  gTIMTiming2_FlagOver = FLAG_N_VALID;
  TIM2->CNTRH  = 0x00;
  TIM2->CNTRL  = 0x00;
  TIM2->CR1 |= (uint8_t)TIM2_CR1_CEN;            //启动定时器
}

/************************************************
函数名称 ： TIM2_Timing_Off
功    能 ： 关闭定时
参    数 ： 无
返 回 值 ： 无
作    者 ： Huang Fugui
*************************************************/
void TIM2_Timing_Off(void)
{
  gTIMTiming2_Num = 0;
  gTIMTiming2_FlagOver = FLAG_N_VALID;
  TIM2->CR1 &= (uint8_t)(~TIM2_CR1_CEN);         //关闭定时器
}

/************************************************
函数名称 ： TIMDelay_N10us
功    能 ： 定时器延时N个10us
参    数 ： Times --- N值
返 回 值 ： 无
作    者 ： Huang Fugui
*************************************************/
void TIMDelay_N10us(uint16_t Times)
{
  TIM4->CR1 |= (uint8_t)TIM4_CR1_CEN;            //启动定时器
  while(Times--)
  {
    while((TIM4->SR1 & TIM4_FLAG_UPDATE) == RESET);
    TIM4->SR1 &= ~TIM4_FLAG_UPDATE;
  }
  TIM4->CR1 &= (uint8_t)(~TIM4_CR1_CEN);         //关闭定时器
}

/************************************************
函数名称 ： TIMDelay_Nms
功    能 ： 定时器延时Nms
参    数 ： Times --- N值
返 回 值 ： 无
作    者 ： Huang Fugui
*************************************************/
void TIMDelay_Nms(uint16_t Times)
{
  while(Times--)
  {
    TIMDelay_N10us(100);
  }
}

/************************************************
函数名称 ： TIMDelay_Ns
功    能 ： 定时器延时Ns
参    数 ： Times --- N值
返 回 值 ： 无
作    者 ： Huang Fugui
*************************************************/
void TIMDelay_Ns(uint16_t Times)
{
  while(Times--)
  {
    TIMDelay_Nms(1000);
  }
}


/***** Copyright (C)2016 HuangFugui. All Rights Reserved ***** END OF FILE *****/
