/**
	******************************************************************************
	* @file    BSP.h
	* @author  锟斤拷锟斤拷
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

#define PA_EN		            17

#define M_SEN_1                 21		//main motor feedback
#define M_SEN_2					15		//push umberalla motor feedback

#define M_CTR_R1                22		//main motor right 
#define M_CTR_L1                23
#define M_CTR_L4				30		//switch door 
#define M_CTR_R4				29
#define M_CTR_L3				20		//fault umbrella
#define M_CTR_R3				12
#define M_CTR_L2				28		//push umberalla motor
#define M_CTR_R2				25

#define SQ_SW1					6		//push motor front trigger switch
#define SQ_SW2					7		//push motor behind trigger switch 
#define SQ_SW3					8		//switch door front trigger switch
#define SQ_SW4					9		//fault umberlla motor front trigger switch 
#define SQ_SW5					10		//switch door behind trigger switch
#define SQ_SW6					11		//fault umberlla motor front trigger switch
#define SQ_SW7					24		//photoelectric trigger switch   // low votage when it is cover

#define M26_EN					0
#define M26_DTR					1
#define M26TX					4
#define M26RX					5

#define RFID_RX_PIN				M26TX
#define RFID_TX_PIN				M26RX

#define RX_PIN_NUMBER			15
#define TX_PIN_NUMBER			16

//switch door photoelectric check
#define IR_SW					19
#define IR_EN					18

//OLD IO
#define GSM_TXD_PIN             M26RX//M26TX
#define GSM_RXD_PIN             M26TX//M26RX
#define GSM_PWRKEY_PIN          M26_DTR
#define GSM_EN_PIN              M26_EN


/**
	* @}
	*/

/* Exported macro ------------------------------------------------------------*/
/** @addtogroup BSP_Exported_Macros
	* @{
	*/


#define LED_ON(x)     			(nrf_gpio_pin_set(LED##_##x))
#define LED_OFF(x)    			(nrf_gpio_pin_clear(LED##_##x))
#define LED_TOGGLE(x)			(nrf_gpio_pin_toggle(LED##_##x))

//main motor--1, push motor--2, fault umberlla motor--3, switch door--4
#define MOTOR_FORWARD(x)		do{nrf_gpio_pin_set(M_CTR_L##x);nrf_gpio_pin_clear(M_CTR_R##x);} while(0)
#define MOTOR_BACK(x)			do{nrf_gpio_pin_set(M_CTR_R##x);nrf_gpio_pin_clear(M_CTR_L##x);} while(0)
#define MOTOR_IS_STUCK()		(nrf_gpio_pin_read(M_SEN_1))

//trigger switch  -- push motor,front-behind--12,fault umberlla motor,front-behind--46,switch door front-behind--35,photoelectric-7
#define IF_IS_TOUCH(x)			(nrf_gpio_pin_read(SQ_SW##x))

//voice enable
#define PA_ENABLE()				(nrf_gpio_pin_set(PA_EN))
#define PA_DISABLE()			(nrf_gpio_pin_clear(PA_EN))

//io configation
#define IO_H(x)					(nrf_gpio_pin_set(x))
#define IO_L(x)					(nrf_gpio_pin_clear(x))
#define IO_TOGGLE(x)			(nrf_gpio_pin_toggle(x))
#define IO_READ(x)				(nrf_gpio_pin_read(x))

//photoelectric check
#define IR_CHECK()				nrf_gpio_pin_read(IR_EN)

//RFID CHECK
#define RFID_M26_EN()			(nrf_gpio_pin_clear(M26_EN))
#define RFID_M26_DIS()			(nrf_gpio_pin_set(M26_EN))
#define RFID_M26_NRF()			(nrf_gpio_pin_read(M26TX))

//M26
#define UART_SET_CMD()			nrf_uart_txrx_pins_set(NRF_UART0, TX_PIN_NUMBER, M26RX)
#define UART_SET_RFID()			nrf_uart_txrx_pins_set(NRF_UART0, TX, RX)

#define UART_SET_GPRS()			nrf_uart_txrx_pins_set(NRF_UART0, M26RX, M26TX)

#define UART_RX_PIN_SELECT(x)	(NRF_UART0->PSELRXD = x)
#define UART_TX_PIN_SELECT(x)	(NRF_UART0->PSELTXD = x)

//time calcatulation
#define TSEC_INIT(ts)             do {ts = RTC_ReadCount();}while(0)
#define TSEC_IS_OVER(ts, over)    (RTC_ReadCount() - ts >= over)
#define TSEC_COUNT(ts)            (RTC_ReadCount() - ts)

/**
	* @}
	*/

/* Exported functions --------------------------------------------------------*/
/** @addtogroup BSP_Exported_Functions
	* @{
	*/
void BSP_Init(void);
void MOTOR_STOP(uint8_t num);

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




