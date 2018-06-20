#ifndef USER_UART_H
#define USER_UART_H


#include <stdint.h>
#include <stdio.h>
#include "prjlib.h"


#define UART_TX_USE_INT                 0

#define UART_TX_BUF_SIZE                256        /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256        /**< UART RX buffer size. */

#define UART_TX_DEFAULT_PIN             GSM_TXD_PIN
#define UART_RX_DEFAULT_PIN             GSM_RXD_PIN

#define UART_RESUME_DELAY               200

#if DEBUG == 1
#define DBG_LOG(format, ...)            do {UART_TX_PIN_SELECT(TX_PIN_NUMBER); \
                                          printf("> "format"\r\n", ##__VA_ARGS__); \
                                          UART_TX_PIN_SELECT(UART_TX_DEFAULT_PIN);} while(0)
#define DBG_SEND(data, len)             do {UART_TX_PIN_SELECT(TX_PIN_NUMBER); \
                                          user_uart_SendData(data, len);\
                                          UART_TX_PIN_SELECT(UART_TX_DEFAULT_PIN);} while(0)
#endif

#define RFID_SEND(data, len)            do {UART_TX_PIN_SELECT(RFID_TX_PIN); \
                                          user_uart_SendData(data, len); \
                                          UART_TX_PIN_SELECT(UART_TX_DEFAULT_PIN);} while(0)



void user_uart_init(void);

BOOL user_uart_SendByte(uint8_t sentbyte);
BOOL user_uart_ReadByte(uint8_t* readbyte);
uint16_t user_uart_RecLength(void);
uint16_t user_uart_SendData(uint8_t* data, uint16_t len);
uint16_t user_uart_ReadData(uint8_t* data, uint16_t len);

#endif
