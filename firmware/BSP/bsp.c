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
void BSP_Init(void)
{
    int i = 0;

    /*空闲引脚初始化为输出省电*/
    for (i = 0; i <= 30; i++) 
    {
	if (i != 26 && i != 27) 
        {
            nrf_gpio_cfg_input(i, NRF_GPIO_PIN_PULLDOWN);
        }
    }
    
    
    	nrf_gpio_cfg_output(W25_CS);
	nrf_gpio_cfg_output(W25_WP);

	nrf_gpio_pin_set(W25_CS);

        nrf_gpio_cfg_output(M_CTR_R1);
        nrf_gpio_cfg_output(M_CTR_L1);
        nrf_gpio_cfg_output(LED1);
        nrf_gpio_cfg_output(LED2);
        nrf_gpio_cfg_output(TM8211_EN);
        nrf_gpio_cfg_output(LAMP_EN);
        
        nrf_gpio_cfg_input(M_SEN_1,GPIO_PIN_CNF_PULL_Pullup);
        
        
        MOTOR_STOP();

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


