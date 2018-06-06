#include "ble_nus.h"
#include "includes.h"
#include "user_comm.h"

static void uart_event_handle(app_uart_evt_t * p_event);
uint8_t Umbrella_ID[4] = {0x04,0xc4,0xb4,0x01};



/*内存分配*/

/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to
 *          a string. The string will be be sent over BLE when the last character received was a
 *          'new line' i.e '\n' (hex 0x0D) or if the string has reached a length of
 *          @ref NUS_MAX_DATA_LENGTH.
 */
/**@snippet [Handling the data received over UART] */
static void uart_event_handle(app_uart_evt_t * p_event)
{
    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
            break;
        case APP_UART_COMMUNICATION_ERROR:
        case APP_UART_FIFO_ERROR:
            break;
        case APP_UART_TX_EMPTY:
            break;
        default:
            break;
    }
}
/*
* 命令打印函数，由UART_SET设置命令打印的串口引脚            
*
*/
uint32_t Print(uint8_t data)
{
  uint32_t err_code;
  
  UART_SET(CMD);
  err_code = app_uart_put(data);
  
  return err_code;
}
/**
 * 接收命令函数
 * 
 * 由UART_SET设置串口引脚
 */
uint32_t Get(uint8_t * p_byte)
{
  UART_SET(CMD);
  return app_uart_get(p_byte);
}
/**
 * 接收RFID数据函数
 * 
 * 由UART_SET设置串口引脚
 */
uint32_t RFID_Get(uint8_t *p_byte)
{
   UART_SET(RFID);
  return app_uart_get(p_byte);
}
/**
 * 发送命令函数
 * 
 * 由UART_SET设置串口引脚
 */
void CMD_uartSendData(uint8_t *data, uint16_t len)
{
  UART_SET(CMD);
    while (len--)
    {
        app_uart_put(*data++);
    }
}
/**
 * 发送数据函数
 * 
 * 由UART_SET设置串口引脚
 */
void RFID_uartSendData(uint8_t *data, uint16_t len)
{
  UART_SET(RFID);
    while (len--)
    {
        app_uart_put(*data++);
    }
}

/**
 * 
 * 串口初始化函数
 * 
 */
void user_uart_init(void)
{
    uint32_t                     err_code;
    const app_uart_comm_params_t comm_params =
    {
        CMD_RX_PIN_NUMBER,
        CMD_TX_PIN_NUMBER,
        RTS_PIN_NUMBER,
        CTS_PIN_NUMBER,
        APP_UART_FLOW_CONTROL_DISABLED,
        false,
        UART_BAUDRATE_BAUDRATE_Baud19200
    };

    APP_UART_FIFO_INIT( &comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handle,
                       APP_IRQ_PRIORITY_LOW,
                       err_code);
    /*使能输入引脚上拉*/
    nrf_gpio_cfg_input(CMD_RX_PIN_NUMBER, NRF_GPIO_PIN_PULLUP);
    APP_ERROR_CHECK(err_code);
#if (DEBUG == 1)
    DBG_LOG("UART init...");
#endif
}

/**
 * 
 * 串口初始化函数
 * 
 */
void RFID_uart_init(void)
{
    uint32_t                     err_code;
    const app_uart_comm_params_t comm_params =
    {
        RFID_RX_PIN_NUMBER,
        RFID_TX_PIN_NUMBER,
        RTS_PIN_NUMBER,
        CTS_PIN_NUMBER,
        APP_UART_FLOW_CONTROL_DISABLED,
        false,
        UART_BAUDRATE_BAUDRATE_Baud19200
    };

    APP_UART_FIFO_INIT( &comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handle,
                       APP_IRQ_PRIORITY_LOW,
                       err_code);
    /*使能输入引脚上拉*/
    nrf_gpio_cfg_input(RFID_RX_PIN_NUMBER, NRF_GPIO_PIN_PULLUP);
    APP_ERROR_CHECK(err_code);
#if (DEBUG == 1)
    DBG_LOG("UART init...");
#endif
}

/**
 * 
 * 对数据buf进行校验
 * 
 */

uint8_t Check(uint8_t *buf,uint16_t len)
{
  uint8_t  temp = 0,i;
  if(buf != NULL)
  {
    for(i=0;i<len;i++)
    {
      temp += buf[i];
    }
    
  }
  return temp;
}

