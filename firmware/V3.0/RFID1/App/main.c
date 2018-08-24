
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
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM3, ENABLE);
    //TIM1_SetCounter(0);
    TIM3_TimeBaseInit(TIM3_Prescaler_1, TIM3_CounterMode_Up, 15999);
    TIM3_ARRPreloadConfig(ENABLE);
    //TIM1_UpdateDisableConfig(DISABLE);
    TIM3_ITConfig(TIM3_IT_Update, ENABLE);
    TIM3_Cmd(ENABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);
    /* Configure USART Tx as alternate function push-pull  (software pull up)*/
    GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_5, ENABLE);

  /* Configure USART Rx as alternate function push-pull  (software pull up)*/
    GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_6, ENABLE);
    //GPIO_ExternalPullUpConfig(COM_PORT[COM], COM_RX_PIN[COM], ENABLE);
    USART_Init(USART1, (uint32_t)38400, 
               USART_WordLength_8b, 
               USART_StopBits_1, 
               USART_Parity_No, 
               USART_Mode_Tx | USART_Mode_Rx);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    
    USART_Cmd(USART1, ENABLE);
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
    //IWDG_Config();
    IWDG_ReloadCounter();
    USART_DeInit(USART1);
    TIM1_DeInit();

    System_Initializes();
    LED_ON(RED);
    //LED_ON(GREEN);
    enableInterrupts();
    MFRC522_Init();
    /*开启flash读保护*/
#if RWP_EN > 0
    if (FLASH_GetReadOutProtectionStatus() == ENABLE) {
        DBG_LOG("System ROP enable.");
        FLASH_Unlock(FLASH_MemType_Program);
        FLASH_EraseOptionByte(0x4800);
        FLASH_ProgramOptionByte(0x4800, 0x00);
        FLASH_Lock(FLASH_MemType_Program);
        while (1);
    }
#endif

    EN485_Recevie_ON;
    GET_DeviceAddress();
    App_Init();
    enableInterrupts();
    while (1) {
        IWDG_ReloadCounter();
        RFID_Polling();
        ReadCmdDeal();
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
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData8(USART1, *p++);
    }
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
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

void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1) {
    }
}
#endif


/***** Copyright (C)2016 HuangFugui. All Rights Reserved ***** END OF FILE *****/
