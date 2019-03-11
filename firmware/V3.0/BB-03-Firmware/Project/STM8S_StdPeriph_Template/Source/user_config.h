/*******************************************************************************
  * @file    stm8s_conf.h
  * @author  MCD Application Team
  * @version V2.2.0
  * @date    30-September-2014
  * @brief   This file is used to configure the Library.
   ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USER_CONFIG_H
#define __USER_CONFIG_H

/* Includes ------------------------------------------------------------------*/
#include "stm8s_conf.h"
#define TICK_AWU                        AWU_TIMEBASE_12S    //HALT


#define LED_FLASH_ON_INTER              2 //10S


#define ELE_FREQUENCY                   1000 //
#define BEEP_FREQUENCY                  2000 //

#define MOTOR_DUTY                      90 //
#define SLEEP_TIME                      10//10tick
#define PRO_CYC_NEW                     2 //
#define PRO_CYC_END                     5 //

#define RWP_EN                          0

#define SPEEK_SENSITIVITY               45

#define KEY_ON_OFF                      0x01
#define KEY_MODE                        0x02



#define LEVEL1_DUTY                      16
#define LEVEL2_DUTY                      20
#define LEVEL3_DUTY                      24
#define LEVEL4_DUTY                      32
#define LEVEL5_DUTY                      36

#define CHECK_DUTY                       1

#define LEVEL_ELE_ONLY_ADDR              0x4000

#define EEROM_MODE_ADD                   0x4010

#define IS_POWER_OF_TWO(A) ( ((A) != 0) && ((((A) - 1) & (A)) == 0) )
#define IS_WORD(A)         (A % 4 == 0)


#define BIT_SET(reg, bit)       (reg |= (1 << bit))
#define BIT_CLEAR(reg, bit)     (reg &= ~(1 << bit))
#define BIT_READ(reg, bit)      ((reg & (1 << bit)) >> bit)

#define MASK_SET(reg, mask)      (reg |= (mask))
#define MASK_CLEAR(reg, mask)    (reg &= (~(mask)))
#define IS_MASK_SET(reg, mask)   (reg & mask)



typedef enum{
    WORK_OFF = 0,
    WORK_ON = 1
}WorkStatus_em;
typedef enum{
    SHAKE = 0,//
    ELE_SHAKE = 1 //
}WorkMode_em;
typedef struct
{
    WorkStatus_em       status; //
    WorkMode_em         mode;   //
    uint8_t             step;   //
    uint8_t             batterStatus;
    uint16_t            sensitivity; //
    uint32_t            sleepTick; //

}WorkData_st;
typedef struct
{
    //uint32_t     tick_1ms; //
    //uint32_t     tick_10ms;

    __IO uint32_t     tick_1s;
}Tick_st;
/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param expr: If expr is false, it calls assert_failed function
  *   which reports the name of the source file and the source
  *   line number of the call that failed.
  *   If expr is true, it returns no value.
  * @retval : None
  */

#endif /* __STM8S_CONF_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
