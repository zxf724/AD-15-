/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#ifndef BOARD_H
#define BOARD_H
   
#define  UART_C      1

#define RX_PIN_NUMBER       15
#define TX_PIN_NUMBER       16
#define RTS_PIN_NUMBER      0
#define CTS_PIN_NUMBER      0

#define CMD_RX_PIN_NUMBER       15
#define CMD_TX_PIN_NUMBER       16   
   
#define GPRS_RX_PIN_NUMBER      5
#define GPRS_TX_PIN_NUMBER      4

   
#define RFID_RX_PIN_NUMBER             24
#define RFID_TX_PIN_NUMBER             25

   
#define UART_SET_RX_PIN(x)   x##_RX_PIN_NUMBER 
#define UART_SET_TX_PIN(x)   x##_TX_PIN_NUMBER
/**
 * 直接操作串口寄存器切换串口引脚
 */
#if  (UART_C == 1)
#define UART_SET(x)          do{NRF_UART0->PSELRXD =UART_SET_RX_PIN(x);NRF_UART0->PSELTXD = UART_SET_TX_PIN(x); }while(0)
#else
#define UART_SET(x) 
#endif

#endif // PCA10028_H
