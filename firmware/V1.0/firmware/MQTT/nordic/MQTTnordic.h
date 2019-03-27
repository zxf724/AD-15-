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
 *******************************************************************************/

#ifndef MQTTNORDIC_H
#define MQTTNORDIC_H

#include "prjlib.h"

typedef struct Timer
{
    uint32_t xTicks;
    uint32_t xTimeOut;
} Timer;

typedef struct Network Network;

struct Network
{
    int my_socket;
    int (*mqttread) (Network*, unsigned char*, int, int);
    int (*mqttwrite) (Network*, unsigned char*, int, int);
    void (*disconnect) (Network*);
};

void TimerInit(Timer*);
char TimerIsExpired(Timer*);
void TimerCountdownMS(Timer*, unsigned int);
void TimerCountdown(Timer*, unsigned int);
int TimerLeftMS(Timer*);


int _mqtt_read(Network*, unsigned char*, int, int);
int _mqtt_write(Network*, unsigned char*, int, int);
void _mqtt_disconnect(Network*);

void NetworkInit(Network*);


#endif
