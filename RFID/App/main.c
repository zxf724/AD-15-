
/*----------------------------------------------------------------------------
  更新日志:
  2016-09-02 V1.0.0:初始版本
  ----------------------------------------------------------------------------*/
/* 包含的头文件 --------------------------------------------------------------*/
#include "user_comm.h"


static void IWDG_Config(void);

/************************************************
函数名称 ： System_Initializes
功    能 ： 系统初始化
参    数 ： 无
返 回 值 ： 无
作    者 ： Huang Fugui
*************************************************/
void System_Initializes(void)
{
    BSP_Initializes();

    TIM1_SetCounter(0);
    TIM1_TimeBaseInit(0, TIM1_COUNTERMODE_UP, 15999, 0);
    TIM1_ARRPreloadConfig(ENABLE);
    TIM1_UpdateDisableConfig(DISABLE);
    TIM1_ITConfig(TIM1_IT_UPDATE, ENABLE);
    TIM1_Cmd(ENABLE);

    UART1_Init((uint32_t)19200, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO,
               UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);
    UART1_ITConfig(UART1_IT_RXNE_OR, ENABLE);
    UART1_Cmd(ENABLE);

    RC522SPI_Init();
}

/************************************************
函数名称 ： main
功    能 ： 主函数入口
参    数 ： 无
返 回 值 ： 无
作    者 ： Huang Fugui
*************************************************/
void main(void)
{
    IWDG_Config();
    IWDG_ReloadCounter();
    UART1_DeInit();
    TIM1_DeInit();

    System_Initializes();
    LED_ON(RED);
    LED_ON(GREEN);
    enableInterrupts();

    /*开启flash读保护*/
#if RWP_EN > 0
    if (FLASH_ReadOptionByte(0x4800) != 0xAA) {
        DBG_LOG("System ROP enable.");
        FLASH_Unlock(FLASH_MEMTYPE_DATA);
        FLASH_ProgramOptionByte(0x4800, 0xAA);
        FLASH_Lock(FLASH_MEMTYPE_DATA);
        while (1);
    }
#endif

    EN485_Recevie_ON;

    GET_DeviceAddress();
    App_Init();
    while (1) {
        IWDG_ReloadCounter();
        RFID_Polling();
        SendID_Data();
//        ReadCmdDeal();
    }
}

/**
  * 控制台打印可变参数字符串.
  * @param  fomat: 参数列表.
  * @param  ...:可变参数
  */
void CMD_Print(char *str, ...)
{
    char *p = str;

    EN485_Recevie_OFF;

    while (p && *p) {
        while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET);
        UART1_SendData8(*p++);
    }
    while (UART1_GetFlagStatus(UART1_FLAG_TC) == RESET);
    EN485_Recevie_ON;
}

/**
  * @brief  Configures the IWDG to generate a Reset if it is not refreshed at the
  *         correct time.
  * @param  None
  * @retval None
  */
static void IWDG_Config(void)
{
    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();

    /* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency
       dispersion) */
    /* Enable write access to IWDG_PR and IWDG_RLR registers */
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

    /* IWDG counter clock: LSI/128 */
    IWDG_SetPrescaler(IWDG_Prescaler_256);

    /* Set counter reload value to obtain 250ms IWDG Timeout.
      Counter Reload Value = 250ms/IWDG counter clock period
                           = 250ms / (LSI/128)
                           = 0.25s / (LsiFreq/128)
                           = LsiFreq/(128 * 4)
                           = LsiFreq/512
     */
    IWDG_SetReload(255);

    /* Reload IWDG counter */
    IWDG_ReloadCounter();
}

#ifdef USE_FULL_ASSERT                           //这里参考官方

void assert_failed(u8 *file, u32 line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1) {
    }
}
#endif


/***** Copyright (C)2016 HuangFugui. All Rights Reserved ***** END OF FILE *****/
