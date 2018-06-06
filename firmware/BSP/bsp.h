/**
	******************************************************************************
	* @file    BSP.h
	* @author  宋阳
	* @version V1.0
	* @date    2017.12.1
	* @brief   Header file of BSP
	******************************************************************************
	*
	******************************************************************************
	*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _BSP_H
#define _BSP_H

/* Includes ------------------------------------------------------------------*/
#include "boards.h"
#include "nordic_common.h"
#include "nrf_gpio.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_error.h"
#include "nrf_uart.h"
#include "nrf_drv_common.h"
#include "nrf_adc.h"
#include "app_button.h"
#include "app_util.h"
#include "app_util_platform.h"
#include "app_timer.h"

/** @addtogroup firmwave_F2_BLE
	* @{
	*/


/** @defgroup BSP
	* @{
	*/


/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/** @defgroup BSP_Exported_Constants BSP Exported Constants
	* @{
	*/

/* 按键引脚定义 */
#define         LED1                                            13
#define         LED2                                            14

#define         PA_EN                                           8
#define         RCH                                              9
#define         TM_BCK                                          10
#define         TM_WS                                           11
#define         TM_DIN                                          12
#define         TM8211_EN                                       18

#define         GSM_TXD                                         4
#define         GSM_RXD                                         5
#define         GSM_PWRKE                                       6
#define         GSM_EN                                          7

#define         W25_CS                                          30 
#define         W25_MISO                                        0
#define         W25_WP                                          1
#define         W25_MOSI                                        2
#define         W25_CLK                                         3

#define         TL485_RX                                        24
#define         TL485_TX                                        25
#define         TL485_DE                                        28
#define         TL485_RE                                        29

#define         M_SEN_1                                         21
#define         M_CTR_R1                                        22
#define         M_CTR_L1                                        23

#define         LAMP_EN                                         17


#define         SPI_MISO_PIN            W25_MISO
#define         SPI_MOSI_PIN            W25_MOSI
#define         SPI_SCK_PIN             W25_CLK
#define         W25_CS_PIN              W25_CS
#define         W25_WP_PIN              W25_WP

/**
	* @}
	*/

/* Exported macro ------------------------------------------------------------*/
/** @addtogroup BSP_Exported_Macros
	* @{
	*/


#define LED_ON(x)     					(nrf_gpio_pin_clear(LED##x))
#define LED_OFF(x)    					(nrf_gpio_pin_set(LED##x))
#define LED_TOGGLE(x)					(nrf_gpio_pin_toggle(LED##x))                           

#define MOTOR_STOP()            do{nrf_gpio_pin_clear(M_CTR_R1);nrf_gpio_pin_clear(M_CTR_L1);} while(0)
#define MOTOR_FORWARD()         do{nrf_gpio_pin_set(M_CTR_R1);nrf_gpio_pin_clear(M_CTR_L1);} while(0)
#define MOTOR_BACK()            do{nrf_gpio_pin_set(M_CTR_L1);nrf_gpio_pin_clear(M_CTR_R1);} while(0)
#define MOTOR_CHECK()           do{nrf_gpio_pin_read(M_SEN_1);}while(0)
/**
	* @}
	*/

/* Exported functions --------------------------------------------------------*/
/** @addtogroup BSP_Exported_Functions
	* @{
	*/
void BSP_Init(void);


/**
	* @}
	*/



/**
	* @}
	*/

/**
	* @}
	*/



#endif /* _BSP_H */

/************************ (C)  *****END OF FILE****/




