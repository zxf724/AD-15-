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

#if !defined(NRF_LOG_USES_RTT) || NRF_LOG_USES_RTT != 1

#include <stdio.h>
#include <stdint.h>
#include "app_uart.h"
#include "nordic_common.h"
#include "nrf_error.h"
#include "user_uart.h"

#if !defined(__ICCARM__)
struct __FILE {
  int handle;
};
#endif

FILE __stdout;
FILE __stdin;


#if defined(__CC_ARM) ||  defined(__ICCARM__)
int fgetc(FILE* p_file) {
  uint8_t input;
  while (!user_uart_ReadByte(&input));
  return input;
}


int fputc(int ch, FILE* p_file) {
  UNUSED_PARAMETER(p_file);

  UNUSED_VARIABLE(user_uart_SendByte(ch));
  return ch;
}

#elif defined(__GNUC__)


int _write(int file, const char* p_char, int len) {
  int i;

  UNUSED_PARAMETER(file);

  user_uart_SendData((uint8_t*)p_char, len);

  return len;
}


int _read(int file, char* p_char, int len) {
  UNUSED_PARAMETER(file);

  return user_uart_ReadData((uint8_t*)p_char, len);
}
#endif

#if defined(__ICCARM__)

__ATTRIBUTES size_t __write(int file, const unsigned char* p_char, size_t len) {

  UNUSED_PARAMETER(file);

  user_uart_SendData((uint8_t*)p_char, len); 
  return len;
}

#endif

#endif // NRF_LOG_USES_RTT != 1
