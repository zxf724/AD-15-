/**
  ******************************************************************************
  * @file    BSP.c
  * @author  宋阳
  * @version V1.0
  * @date    2017.12.1
  * @brief   智能锁固件BSP函数.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "bsp.h"



/* Exported functions ---------------------------------------------------------*/

/** @defgroup BSP_Exported_Functions BSP Exported Functions
  *  @brief   BSP 外部接口函数
  * @{
  */

/**
  * @brief  BSP初始化.
  * @param  none.
  * @retval none
  */
void BSP_Init(void) {
  int i = 0;

  /*空闲引脚初始化为输出省电*/
  for (i = 0; i <= 30; i++) {
    if (i != 26 && i != 27) {
      nrf_gpio_cfg_input(i, NRF_GPIO_PIN_PULLDOWN);
    }
  }

  nrf_gpio_cfg_output(M_CTR_R1);
  nrf_gpio_cfg_output(M_CTR_L1);
  MOTOR_STOP();

  nrf_gpio_cfg_output(LED_NET);
  nrf_gpio_cfg_output(LED_STATUS);
  nrf_gpio_pin_set(LED_NET);
  nrf_gpio_pin_set(LED_STATUS);

  nrf_gpio_cfg_output(PA_EN_PIN);
  nrf_gpio_pin_clear(PA_EN_PIN); 

  nrf_gpio_cfg_input(IR_SENSOR_PIN, NRF_GPIO_PIN_PULLUP);
  nrf_gpio_cfg_input(OVER_SENSOR_PIN, NRF_GPIO_PIN_PULLUP);
  nrf_gpio_cfg_input(M_SEN_1, NRF_GPIO_PIN_PULLDOWN);

  nrf_gpio_cfg_input(GSM_EN_PIN, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_cfg_input(GSM_PWRKEY_PIN, NRF_GPIO_PIN_NOPULL);

  nrf_gpio_pin_set(GSM_TXD_PIN);
  nrf_gpio_pin_set(RFID_TX_PIN);
  nrf_gpio_pin_set(TX_PIN_NUMBER);
  nrf_gpio_cfg_output(GSM_TXD_PIN);
  nrf_gpio_cfg_output(RFID_TX_PIN);
  nrf_gpio_cfg_output(TX_PIN_NUMBER);

  nrf_gpio_cfg_input(GSM_RXD_PIN, NRF_GPIO_PIN_PULLUP);
  nrf_gpio_cfg_input(RFID_RX_PIN, NRF_GPIO_PIN_PULLUP);
  nrf_gpio_cfg_input(RX_PIN_NUMBER, NRF_GPIO_PIN_PULLUP);

}

/**
  * @}
  */



/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT  *****END OF FILE****/


