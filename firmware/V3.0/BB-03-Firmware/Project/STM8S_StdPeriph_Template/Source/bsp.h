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
#ifndef __BSP_H
#define __BSP_H

/* Includes ------------------------------------------------------------------*/

/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param expr: If expr is false, it calls assert_failed function
  *   which reports the name of the source file and the source
  *   line number of the call that failed.
  *   If expr is true, it returns no value.
  * @retval : None
  */

#define GPIO_MODE                   GPIOC,GPIO_PIN_7
#define GPIO_ON_OFF                 GPIOC,GPIO_PIN_6

//#define GPIO_LED1                   GPIOD,GPIO_PIN_2
//#define GPIO_ON_OFF                 GPIOD,GPIO_PIN_3

#define GPIO_SPEEK_CHECK            GPIOD,GPIO_PIN_6
//#define GPIO_ELE_CHECK              GPIOA,GPIO_PIN_1


#define GPIO_ELE_OUT                GPIOC,GPIO_PIN_3
#define GPIO_BEEP_OUT               GPIOA,GPIO_PIN_3
#define GPIO_MOTOR_OUT              GPIOC,GPIO_PIN_5


#define GPIO_LED1                   GPIOD,GPIO_PIN_1
#define GPIO_LED2                   GPIOD,GPIO_PIN_2
#define GPIO_LED3                   GPIOD,GPIO_PIN_3
#define GPIO_LED4                   GPIOD,GPIO_PIN_4
#define GPIO_LED5                   GPIOD,GPIO_PIN_5

#define GPIO_LED_BLE                GPIOB,GPIO_PIN_4
#define GPIO_LED_RED                GPIOB,GPIO_PIN_5

#define GPIO_POWER_EN               GPIOA,GPIO_PIN_2

#define GPIO_ADC                    GPIOC,GPIO_PIN_4
/*
#define SW1_IN_UP			GPIO_Init(GPIOD, GPIO_PIN_3, GPIO_MODE_IN_PU_IT)//����IO ���ж�����
#define SW3_IN_UP			GPIO_Init(GPIOC, GPIO_PIN_7, GPIO_MODE_IN_PU_IT)
#define SW2_IN_UP			GPIO_Init(GPIOC, GPIO_PIN_6, GPIO_MODE_IN_PU_IT)
#define SW4_IN_UP			GPIO_Init(GPIOD, GPIO_PIN_2, GPIO_MODE_IN_PU_IT)
#define SW1_OUT_LOW			GPIO_Init(GPIOD, GPIO_PIN_3, GPIO_MODE_OUT_OD_LOW_SLOW)//����IO ���ٿ�©������
#define SW2_OUT_LOW			GPIO_Init(GPIOC, GPIO_PIN_6, GPIO_MODE_OUT_OD_LOW_SLOW)
#define SW3_OUT_LOW			GPIO_Init(GPIOC, GPIO_PIN_7, GPIO_MODE_OUT_OD_LOW_SLOW)
#define SW4_OUT_LOW			GPIO_Init(GPIOD, GPIO_PIN_2, GPIO_MODE_OUT_OD_LOW_SLOW)
*/
#define GPIO_LED_BLE_ON()		         GPIO_WriteHigh(GPIO_LED_BLE)
#define GPIO_LED_BLE_OFF()	             GPIO_WriteLow(GPIO_LED_BLE)
#define GPIO_LED_READ1_ON()	             GPIO_WriteHigh(GPIO_LED_READ1)
#define GPIO_LED_READ1_OFF()	         GPIO_WriteLow(GPIO_LED_READ1)
#define GPIO_LED_READ2_ON()	             GPIO_WriteHigh(GPIO_LED_READ2)
#define GPIO_LED_READ2_OFF()	         GPIO_WriteLow(GPIO_LED_READ2)
#define GPIO_LED_READ3_ON()	             GPIO_WriteHigh(GPIO_LED_READ3)
#define GPIO_LED_READ3_OFF()	         GPIO_WriteLow(GPIO_LED_READ3)

#define GPIO_BEEP_OUT_HIGH()	         GPIO_WriteHigh(GPIO_BEEP_OUT)
#define GPIO_BEEP_OUT_LOW()	             GPIO_WriteLow(GPIO_BEEP_OUT)

#define GPIO_ELE_OUT_HIGH()	             GPIO_WriteHigh(GPIO_ELE_OUT)
#define GPIO_ELE_OUT_LOW()	             GPIO_WriteLow(GPIO_ELE_OUT)



#endif /* __STM8S_CONF_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
