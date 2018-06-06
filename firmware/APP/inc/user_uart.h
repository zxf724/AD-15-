#ifndef USER_UART_H
#define USER_UART_H


#include <stdint.h>
#include <stdio.h>
#include "prjlib.h"

#if DEBUG == 1
#define DBG_LOG(format, ...)    printf("> "format"\r\n", ##__VA_ARGS__)
#define DBG_SEND(data, len)     CMD_uartSendData(data, len)

/*使能微信蓝牙的LOG输出*/
#define CATCH_LOG


#else
#define DBG_LOG(format, ...)
#define DBG_SEND(data, len)
#endif


extern uint8_t Umbrella_ID[4];

#define UART_REFRESH_TICKS          5000
#define UART_PORT_MAX               4


#define MMEMORY_ALLOC            pvPortMalloc
#define MMEMORY_FREE             vPortFree


#define UART_TX_BUF_SIZE                512        /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                512        /**< UART RX buffer size. */

void CMD_uartSendData(uint8_t *data, uint16_t len);
void RFID_uartSendData(uint8_t *data, uint16_t len);
uint32_t Print(uint8_t data);
uint32_t Get(uint8_t *data);
uint32_t RFID_Get(uint8_t *p_byte);
void user_uart_init(void);
void GPRS_uart_init(void);
void RFID_uart_init(void);
uint32_t UART_GetDataIdleTicks(uint8_t num);
uint16_t UART_DataSize(uint8_t num);
void RFID_SendCmd(uint8_t cmd);
void RFID_Write_Umbrella_ID(uint8_t cmd,uint8_t *dat,uint8_t len);

#endif
