/**
	******************************************************************************
	* @file    BSP.h
	* @author  ËÎÑô
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

#define LED_NET                 13
#define LED_STATUS              14

#define PA_EN_PIN               8

#define GSM_TXD_PIN             5
#define GSM_RXD_PIN             4
#define GSM_PWRKEY_PIN          6
#define GSM_EN_PIN              7

#define M_SEN_1                 21
#define M_CTR_R1                22
#define M_CTR_L1                23

#define IR_SENSOR_PIN           17

#define OVER_SENSOR_PIN         20
   
#define RFID_RX_PIN             24
#define RFID_TX_PIN             25

/**
	* @}
	*/

/* Exported macro ------------------------------------------------------------*/
/** @addtogroup BSP_Exported_Macros
	* @{
	*/


#define LED_ON(x)     					(nrf_gpio_pin_clear(LED##_##x))
#define LED_OFF(x)    					(nrf_gpio_pin_set(LED##_##x))
#define LED_TOGGLE(x)					  (nrf_gpio_pin_toggle(LED##_##x))                           

#define MOTOR_STOP()            do{nrf_gpio_pin_clear(M_CTR_R1);nrf_gpio_pin_clear(M_CTR_L1);} while(0)
#define MOTOR_FORWARD()         do{nrf_gpio_pin_set(M_CTR_R1);nrf_gpio_pin_clear(M_CTR_L1);} while(0)
#define MOTOR_BACK()            do{nrf_gpio_pin_set(M_CTR_L1);nrf_gpio_pin_clear(M_CTR_R1);} while(0)
#define MOTOR_IS_STUCK()        (nrf_gpio_pin_read(M_SEN_1))

#define IO_H(x)               	(nrf_gpio_pin_set(x))
#define IO_L(x)                 (nrf_gpio_pin_clear(x))
#define IO_TOGGLE(x)            (nrf_gpio_pin_toggle(x))
#define IO_READ(x)              (nrf_gpio_pin_read(x))

#define IR_CHECK()              (nrf_gpio_pin_read(IR_SENSOR_PIN) == 0)
  
#define UM_OVER_CHECK()         (nrf_gpio_pin_read(OVER_SENSOR_PIN) != 0)

#define UART_SET_CMD()          nrf_uart_txrx_pins_set(NRF_UART0, TX_PIN_NUMBER, RX_PIN_NUMBER)
#define UART_SET_RFID()         nrf_uart_txrx_pins_set(NRF_UART0, RFID_TX_PIN, RFID_RX_PIN)
#define UART_SET_GPRS()         nrf_uart_txrx_pins_set(NRF_UART0, GSM_TXD_PIN, GSM_RXD_PIN)

#define UART_RX_PIN_SELECT(x)   (NRF_UART0->PSELRXD = x)
#define UART_TX_PIN_SELECT(x)   (NRF_UART0->PSELTXD = x)

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




