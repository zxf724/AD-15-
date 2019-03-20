/*
  **********************************  STM8S  ***********************************
  * @文件名     ： timer.h
  * @作者       ： Huang Fugui
  * @库版本     ： V2.2.0
  * @文件版本   ： V1.0.0
  * @日期       ： 2016年09月02日
  * @摘要       ： TIMER头文件
  ******************************************************************************/

/* 定义防止递归包含 ----------------------------------------------------------*/
#ifndef _TIMER_BSP_H
#define _TIMER_BSP_H

/* 包含的头文件 --------------------------------------------------------------*/
#include "stm8s.h"


/* 宏定义 --------------------------------------------------------------------*/

/* 函数申明 ------------------------------------------------------------------*/
void TIMER_Initializes(void);

void TIM2_Timing_Nms(uint16_t Times);
void TIM2_Timing_Off(void);

void TIMDelay_N10us(uint16_t Times);
void TIMDelay_Nms(uint16_t Times);
void TIMDelay_Ns(uint16_t Times);


#endif /* _TIMER_BSP_H */

/***** Copyright (C)2016 HuangFugui. All Rights Reserved ***** END OF FILE *****/
