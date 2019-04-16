/*******************************************************************************
 * Copyright (c) 2014, 2015 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *    Ian Craggs - convert to FreeRTOS
 *******************************************************************************/

#include "MQTTnordic.h"
#include "includes.h"

void TimerCountdownMS(Timer* timer, unsigned int timeout_ms) {
  app_timer_cnt_get(&(timer->xTicks));
  timer->xTimeOut = timeout_ms;
}

void TimerCountdown(Timer* timer, unsigned int timeout) {
  TimerCountdownMS(timer, timeout * 1000);
}

int TimerLeftMS(Timer* timer) {
  uint32_t rc = 0, tick = 0;

  app_timer_cnt_get(&tick);
  app_timer_cnt_diff_compute(tick, timer->xTicks, &rc);
  rc /= 32;

  if (rc < timer->xTimeOut) {
    rc = timer->xTimeOut - rc;
  } else {
    rc = 0;
  }

  return rc;
}

char TimerIsExpired(Timer* timer) {

  uint32_t rc = 0, tick = 0;

  app_timer_cnt_get(&tick);
  app_timer_cnt_diff_compute(tick, timer->xTicks, &rc);
  rc /= 32;

  if (rc > timer->xTimeOut) {
    return 1;
  } else {
    return 0;
  }
}

void TimerInit(Timer* timer) {
  timer->xTicks = 0;
  timer->xTimeOut = 0;
}

int _mqtt_read(Network* n, unsigned char* buffer, int len, int timeout_ms) {

  int recvLen = 0, rc = -1;
  uint32_t tsold, ts, tsdiff;

  if (timeout_ms > 0) {
    app_timer_cnt_get(&tsold);
    do {

      /*GPRS轮循接收数据*/
      GPRS_Polling();

      rc = MQTT_ReadData(buffer + recvLen, len - recvLen);
      if (rc > 0) recvLen += rc;
      else if (rc < 0) {
        recvLen = rc;
        break;
      }
      app_timer_cnt_get(&ts);
      app_timer_cnt_diff_compute(ts, tsold, &tsdiff);

      WatchDog_Clear();
      if (user_uart_RecLength() == 0 && recvLen < len && tsdiff <= timeout_ms * 32) {
        sd_app_evt_wait();
      }
    } while (recvLen < len && tsdiff <= timeout_ms * 32);
  } else {
    recvLen = MQTT_ReadData(buffer, len);
  }
  return recvLen;
}

int _mqtt_write(Network* n, unsigned char* buffer, int len, int timeout_ms) {

  return  MQTT_SendData(buffer, len);
}

void _mqtt_disconnect(Network* n) {
  //待添加
}

void NetworkInit(Network* n) {
  n->my_socket = 0;
  n->mqttread = _mqtt_read;
  n->mqttwrite = _mqtt_write;
  n->disconnect = _mqtt_disconnect;
}
