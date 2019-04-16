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
void BSPInit(void) {
    //主电机1，推伞电机2，故障电机3，开关门电机4
    nrf_gpio_cfg_output(M_CTR_R1);
    nrf_gpio_cfg_output(M_CTR_L1);
    nrf_gpio_cfg_output(M_CTR_R2);
    nrf_gpio_cfg_output(M_CTR_L2);
    nrf_gpio_cfg_output(M_CTR_R3);
    nrf_gpio_cfg_output(M_CTR_L3);
    nrf_gpio_cfg_output(M_CTR_R4);
    nrf_gpio_cfg_output(M_CTR_L4);
    //灯
    nrf_gpio_cfg_output(LED_NET);
    nrf_gpio_cfg_output(LED_STATUS);
    nrf_gpio_pin_set(LED_NET);
    nrf_gpio_pin_set(LED_STATUS);
    //触发开关
    nrf_gpio_cfg_input(SQ_SW1, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(SQ_SW2, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(SQ_SW3, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(SQ_SW4, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(SQ_SW5, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(SQ_SW6, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(SQ_SW7, NRF_GPIO_PIN_PULLUP);
    //音频
    nrf_gpio_cfg_output(PA_EN);
    //红外
    nrf_gpio_cfg_input(IR_EN, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_output(IR_SW);
    //RFID
    nrf_gpio_cfg_input(M26TX, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_output(M26RX);
    //串口
    nrf_gpio_cfg_input(RX_PIN_NUMBER, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_output(TX_PIN_NUMBER);
    //
    nrf_gpio_cfg_input(M_SEN_1, NRF_GPIO_PIN_PULLDOWN);
    nrf_gpio_cfg_output(M26_EN);
    nrf_gpio_pin_clear(M26_EN);
    //
    nrf_gpio_cfg_input(GSM_RXD_PIN, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(GSM_EN_PIN, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(GSM_PWRKEY_PIN, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_pin_set(GSM_TXD_PIN);
    nrf_gpio_pin_set(RFID_TX_PIN);
    //old io config
    nrf_gpio_cfg_input(M_SEN_1, NRF_GPIO_PIN_PULLDOWN);
    nrf_gpio_cfg_input(M_SEN_2, NRF_GPIO_PIN_PULLDOWN);
    nrf_gpio_pin_set(TX_PIN_NUMBER);
    nrf_gpio_cfg_output(GSM_TXD_PIN);
    nrf_gpio_cfg_output(RFID_TX_PIN);
    nrf_gpio_cfg_output(TX_PIN_NUMBER);
    nrf_gpio_cfg_input(RFID_RX_PIN, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(RX_PIN_NUMBER, NRF_GPIO_PIN_PULLUP);
}

void MOTOR_STOP(uint8_t num) {
    if(num == 1) {      //1为主电机
        do {
            nrf_gpio_pin_clear(M_CTR_R1);
            nrf_gpio_pin_clear(M_CTR_L1);
        } while(0);
    } else if(num == 2) { //2为推伞电机
        do {
            nrf_gpio_pin_clear(M_CTR_R2);
            nrf_gpio_pin_clear(M_CTR_L2);
        } while(0);
    } else if(num == 3) {     //3为故障伞电机
        do {
            nrf_gpio_pin_clear(M_CTR_R3);
            nrf_gpio_pin_clear(M_CTR_L3);
        } while(0);
    } else if(num == 4) {     //4为开关门电机
        do {
            nrf_gpio_pin_clear(M_CTR_R4);
            nrf_gpio_pin_clear(M_CTR_L4);
        } while(0);
    }
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


