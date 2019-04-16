/**
 * *********************************************************************
 *             Copyright (c) 2017 AFU  All Rights Reserved.
 * @file moetor.c
 * @version V1.0
 * @date 2017.7.12
 * @brief 一秦共享雨伞电机控制函数.
 *
 * *********************************************************************
 * @note
 *
 * *********************************************************************
 * @author 宋阳
 */



/* Includes ------------------------------------------------------------------*/
#include "ble_nus.h"
#include "includes.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static FIFO_t UART_RecFIFO;
static uint8_t UART_Rec_Buffer[UART_RX_BUF_SIZE], rxPIN = 0;

#if UART_TX_USE_INT == 1
static FIFO_t UART_SendFIFO;
static uint8_t UART_Send_Buffer[UART_TX_BUF_SIZE], TxDoing = 0;
#endif

APP_TIMER_DEF(TimerId_UARTresume);
uint16_t UART_MutexCount = 0;

/* Private function prototypes -----------------------------------------------*/
static void uart_incoming_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
static void UART_Resume_TimerCB(void* p_context);


/* Exported functions --------------------------------------------------------*/


/**
 *
 * 串口初始化函数
 *
 */
void UserUartInit(uint32_t rx_pin_no, uint32_t tx_pin_no, uint32_t baud_rate) {
    FIFO_Init(&UART_RecFIFO, UART_Rec_Buffer, UART_RX_BUF_SIZE);
#if UART_TX_USE_INT == 1
    FIFO_Init(&UART_SendFIFO, UART_Send_Buffer, UART_TX_BUF_SIZE);
#endif

    app_timer_create(&TimerId_UARTresume, APP_TIMER_MODE_SINGLE_SHOT, UART_Resume_TimerCB);
    
    NRF_UART0->EVENTS_RXDRDY = 0;
    NRF_UART0->EVENTS_TXDRDY = 0;
    NRF_UART0->EVENTS_ERROR = 0;
    NRF_UART0->EVENTS_RXTO = 0;
    NRF_UART0->PSELRXD = rx_pin_no;
    NRF_UART0->PSELTXD = tx_pin_no;
    NRF_UART0->CONFIG = UART_CONFIG_HWFC_Disabled | UART_CONFIG_PARITY_Excluded;
    NRF_UART0->BAUDRATE = baud_rate;
    NRF_UART0->TASKS_STARTRX = 1;
#if UART_TX_USE_INT == 1
    NRF_UART0->INTENSET = NRF_UART_INT_MASK_RXDRDY | NRF_UART_INT_MASK_TXDRDY | NRF_UART_INT_MASK_ERROR;
#else
    NRF_UART0->INTENSET = NRF_UART_INT_MASK_RXDRDY | NRF_UART_INT_MASK_ERROR;
    NRF_UART0->TASKS_STARTTX = 1;
#endif
    nrf_drv_common_irq_enable(UART0_IRQn, APP_IRQ_PRIORITY_LOW);
    NRF_UART0->ENABLE = UART_ENABLE_ENABLE_Enabled;

//start serial port 
if (!nrf_drv_gpiote_is_init()) {
    nrf_drv_gpiote_init();
  }

  nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
  config.pull = NRF_GPIO_PIN_PULLUP;

  nrf_drv_gpiote_in_init(RX_PIN_NUMBER, &config, uart_incoming_handler);

  nrf_drv_gpiote_in_event_enable(RX_PIN_NUMBER, false);
//end serial port 

#if (DEBUG == 1)
    DBG_LOG("UART init...");
#endif
}


void Switch_Uart_Init(uint8_t uart) {
    switch(uart) {
        case 0:
            NRF_UART0->PSELRXD = UART_TX_DEFAULT_PIN;
            NRF_UART0->PSELTXD = UART_RX_DEFAULT_PIN;
            break;
        case 1:
            NRF_UART0->PSELRXD = RX_PIN_NUMBER;//RFID_RX_PIN;
            NRF_UART0->PSELTXD = TX_PIN_NUMBER;//RFID_TX_PIN;
            break;
    }
}
/**
 * UART发送一字节
 *
 * @param sentbyte 待发送的字节
 * @return 发送成功返回TRUE
 */
BOOL user_uart_SendByte(uint8_t sentbyte) {
#if UART_TX_USE_INT == 1
    BOOL ret = FALSE;
    if (FIFO_Length(&UART_SendFIFO) < (UART_TX_BUF_SIZE - 1)) {
        FIFO_Put(&UART_SendFIFO, sentbyte);
        ret = TRUE;
    }
    if (FIFO_Length(&UART_SendFIFO) == 1 && TxDoing == 0) {
        TxDoing = 1;
        NRF_UART0->TXD = FIFO_Get(&UART_SendFIFO);
        NRF_UART0->TASKS_STARTTX = 1;
    }
    return ret;
#else
    NRF_UART0->EVENTS_TXDRDY = 0;
    NRF_UART0->TXD = sentbyte;
    while (NRF_UART0->EVENTS_TXDRDY == 0);
    return TRUE;
#endif
}

/**
 * UART读出一字节
 *
 * @param readbyte 读出的指针
 * @return 读出成功返回TRUE
 */
BOOL user_uart_ReadByte(uint8_t* readbyte) {
    BOOL ret = FALSE;
    if (FIFO_Length(&UART_RecFIFO) > 0) {
        *readbyte = FIFO_Get(&UART_RecFIFO);
        ret = TRUE;
    }
    return ret;
}

/**
 * 返回接收的数据长度
 *
 * @return 返回串口接收的数据长度
 */
uint16_t UserUartRecLength(void) {
    return FIFO_Length(&UART_RecFIFO);
}

/**
 * UART发送数据
 *
 * @param data   待发送的数据
 * @param len    待发送的数据长度
 * @return 发送成功返回TRUE
 */
uint16_t user_uart_SendData(uint8_t* data, uint16_t len) {
#if UART_TX_USE_INT == 1
    uint16_t ret = 0, needtx = 0;
    if (FIFO_Length(&UART_SendFIFO) == 0) {
        needtx = 1;
    }
    while (FIFO_Length(&UART_SendFIFO) < (UART_TX_BUF_SIZE - 1) && len > 0) {
        FIFO_Put(&UART_SendFIFO, *data++);
        ret++;
    }
    if (needtx && TxDoing == 0) {
        TxDoing = 1;
        NRF_UART0->TXD = FIFO_Get(&UART_SendFIFO);
        NRF_UART0->TASKS_STARTTX = 1;
    }
    return ret;
#else
    while (len > 0) {
        NRF_UART0->EVENTS_TXDRDY = 0;
        NRF_UART0->TXD = *data++;
        len--;
        while (NRF_UART0->EVENTS_TXDRDY == 0);
    }
    return TRUE;
#endif
}

/**
 * UART读出一字节
 *
 * @param readbyte 读出的指针
 * @return 读出成功返回TRUE
 */
uint16_t user_uart_ReadData(uint8_t* data, uint16_t len) {
    int16_t ret = 0;
    while (FIFO_Length(&UART_RecFIFO) > 0 && ret < len) {
        *(data + ret) = FIFO_Get(&UART_RecFIFO);
        ret++;
    }
    return ret;
}

/**
 * UART中断回调
 */
void UART0_IRQHandler(void) {
    if (NRF_UART0->EVENTS_RXDRDY) {
        NRF_UART0->EVENTS_RXDRDY = 0;
        if (FIFO_Length(&UART_RecFIFO) < (UART_RX_BUF_SIZE - 1)) {
            FIFO_Put(&UART_RecFIFO, NRF_UART0->RXD);
        }
    }
#if UART_TX_USE_INT == 1
    if (NRF_UART0->EVENTS_TXDRDY) {
        NRF_UART0->EVENTS_TXDRDY = 0;
        if (FIFO_Length(&UART_SendFIFO) > 0) {
            NRF_UART0->TXD = FIFO_Get(&UART_SendFIFO);
        } else {
            NRF_UART0->TASKS_STOPTX = 1;
            TxDoing = 0;
        }
    }
#endif
    NRF_UART0->EVENTS_ERROR = 0;
    NRF_UART0->EVENTS_RXTO = 0;
}

/**
 * UART接收检测，用于切换UART通道
 *
 */
static void uart_incoming_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
    if (UART_MutexCount == 0) {
        NRF_UART0->TASKS_STOPRX = 1;
        rxPIN = pin;
        nrf_drv_gpiote_in_event_disable(RX_PIN_NUMBER);
        app_timer_start(TimerId_UARTresume, APP_TIMER_TICKS(20, APP_TIMER_PRESCALER), NULL);
    }
}


/**
 * UART超时回调，用于恢复至默认通道
 *
 */
static void UART_Resume_TimerCB(void* p_context) {
    static uint16_t step = 0;
    if (step == 0) {
        app_timer_start(TimerId_UARTresume, APP_TIMER_TICKS(UART_RESUME_DELAY, APP_TIMER_PRESCALER), NULL);
        FIFO_Flush(&UART_RecFIFO);
        UART_RX_PIN_SELECT(rxPIN);
        NRF_UART0->TASKS_STARTRX = 1;
        step = 1;
    } else {
        step = 0;
        UART_RX_PIN_SELECT(M26RX);
        nrf_drv_gpiote_in_event_enable(RX_PIN_NUMBER, false);
    }
}
