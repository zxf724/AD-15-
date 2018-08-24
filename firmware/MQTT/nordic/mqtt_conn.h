/**
 * **********************************************************************
 *             Copyright (c) 2016 temp. All Rights Reserved.
 * @file mqtt_conn.h
 * @author 宋阳
 * @version V1.0
 * @date 2016.12.20
 * @brief MQTT连接管理函数文件.
 *
 * **********************************************************************
 * @note
 *
 * **********************************************************************
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _MQTT_CONNECT_H
#define _MQTT_CONNECT_H


/* Includes ------------------------------------------------------------------*/
#include "prjlib.h"
#include "MQTTPacket.h"
#include "MQTTClient.h"

/* Exported define -----------------------------------------------------------*/
/*MQTT调试使能*/
#define MQTT_DEBUG                      1

/*协议处理缓存长度*/
#define MQTT_TX_BUFF_SIZE               280
#define MQTT_RX_BUFF_SIZE               200

/*MQTT数据发送的超时时间，单位毫秒*/
#define MQTT_TIMEOUT_DEF                5000

/*MQTT的心跳包间隔,实际发送间隔为设置值的一半，单位秒*/
#define MQTT_PING_INVT_DEF              110

/*连接失败重新鉴权的失败次数*/
#define CONNECT_FAIL_REAUTH             5
/*连接失败超时时间,单位秒*/
#define CONNECT_FAIL_TIMEOUT            30


/* Exported types ------------------------------------------------------------*/

/*MQTT订阅回调函数类型*/
typedef void (*Arrived_t)(uint8_t *data, uint16_t len);

/* Exported constants --------------------------------------------------------*/
extern  uint32_t DataFlowCnt;

/* Exported macro ------------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
void MQTT_Conn_Init(void);

void MQTT_Conn_Polling(void);

BOOL MQTT_IsConnected(void);

int16_t MQTT_SendData(uint8_t *dat, uint16_t len);
int16_t MQTT_ReadData(uint8_t *dat, uint16_t len);

BOOL Publish_MQTT(char const *topic, Qos qos, uint8_t *payload, uint16_t len);
BOOL Subscribe_MQTT(char const *topic, Qos qos, Arrived_t fun);

#endif
