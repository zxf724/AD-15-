
/**
 * *********************************************************************
 *             Copyright (c) 2016 temp. All Rights Reserved.
 * @file protocols.c
 * @version V1.0
 * @date 2016.4.1
 * @brief 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷通讯协锟介函锟斤拷锟侥硷拷.
 *
 * *********************************************************************
 * @note
 *
 * *********************************************************************
 * @author 锟斤拷锟斤拷
 * 2016.12.30 锟斤拷锟接讹拷锟斤拷失锟斤拷锟斤拷锟斤拷
 */



/* Includes ------------------------------------------------------------------*/
#include "includes.h"
#include "mqtt_conn.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  Qos qos;
  char const* topic;
  Arrived_t callback;
} SubScribe_t;

/* Private define ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

static uint8_t txBuffer[MQTT_TX_BUFF_SIZE], rxBuffer[MQTT_RX_BUFF_SIZE];

static uint16_t Publish_Fail = 0, Connect_Fail = 0;
static uint32_t tsPingOut = 0;

static MQTTClient mClient;
static Network mNetwork;
static MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

static SubScribe_t Subs[MAX_MESSAGE_HANDLERS];


uint32_t DataFlowCnt = 0;
/* Private function prototypes -----------------------------------------------*/
void messageArrived(MessageData* data);
static void Manager_MQTT(void);
static void MQTT_ReSubscribe(void);
static BOOL Connect_MQTT(void);
static BOOL Disconnect_MQTT(void);
static void mqtt_Console(int argc, char* argv[]);


/* Exported functions --------------------------------------------------------*/

/**
 * 协议处理初始化
 */
void MQTT_Conn_Init(void) {
  NetworkInit(&mNetwork);
  MQTTClientInit(&mClient, &mNetwork, MQTT_TIMEOUT_DEF, txBuffer, sizeof(txBuffer), rxBuffer, sizeof(rxBuffer));

  CMD_ENT_DEF(mqtt, mqtt_Console);
  Cmd_AddEntrance(CMD_ENT(mqtt));

  GPRS_SetOnOff(TRUE);

  DBG_LOG("MQTT init OK.");
}

/**
 * 协议处理的任务
 * @param argument
 */
void MQTT_Conn_Polling(void) {
  Manager_MQTT();

  MQTTYield(&mClient, 0);
}

/**
 * MQTT连接状态查询,无连接返回0，已连接返回MQTT使用的网络路径
 */
BOOL MQTT_IsConnected(void) {
  return (BOOL)mClient.isconnected;
}


/**
 * 处理订阅的消息
 * @param data 消息数据
 */
void messageArrived(MessageData* data) {
  int i = 0;

#if MQTT_DEBUG > 0

  DBG_LOG("Receive New message%.*s, payload:",
          data->topicName->lenstring.len,
          data->topicName->lenstring.data);
#endif

  for (i = 0; i < MAX_MESSAGE_HANDLERS; i++) {
    if (Subs[i].topic != NULL) {
      if (strncmp(Subs[i].topic, data->topicName->lenstring.data, data->topicName->lenstring.len) == 0
          && Subs[i].callback != NULL) {
        Subs[i].callback(data->message->payload, data->message->payloadlen);
      }
    }
  }
}

/**
 * 协锟介处锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
 * @param dat  锟斤拷锟斤拷指锟斤拷
 * @param len  锟斤拷锟捷筹拷锟斤拷
 */
int16_t MQTT_SendData(uint8_t* dat, uint16_t len) {
  int16_t rc = -1;

  if (GPRS_IsSocketConnect()) {
    /*用于网络指示灯*/
    DataFlowCnt += 1;

    rc = GPRS_SocketSendData(dat, len);

#if MQTT_DEBUG > 1
    if (rc != 0) {
      DBG_LOG("MQTT send data, rc:%d", rc);
    }
#if MQTT_DEBUG > 2
    CMD_HEX_Print(dat, len);
#endif
#endif
  }
  return rc;
}

/**
 * 协议处理读出数据
 * @param dat  数据读出的指针
 * @param len  待读出的长度
 * @return 返回实际读出的长度
 */
int16_t MQTT_ReadData(uint8_t* dat, uint16_t len) {
  int16_t rc = 0;

  rc = GPRS_SocketReadData(dat, len);

  if (rc > 0) {
#if MQTT_DEBUG > 1
    DBG_LOG("MQTT read data, len:%d", rc);
#if MQTT_DEBUG > 2
    CMD_HEX_Print(dat, len);
#endif
#endif
    DataFlowCnt += 1;
  }
  return rc;
}

/**
 * 发布MQTT消息
 * @param topic   发布的主题名
 * @param qos     消息服务质量等级
 * @param payload 消息体
 * @param len     消息体长度
 * @return 返回发布结果
 */
BOOL Publish_MQTT(char const* topic, Qos qos, uint8_t* payload, uint16_t len) {
  int rc;
  MQTTMessage msg;

  msg.qos = qos;
  msg.retained = 0;
  msg.payload = payload;
  msg.payloadlen = len;

  if (MQTT_IsConnected()) {
    if ((rc = MQTTPublish(&mClient, topic, &msg)) != SUCESS) {
      Publish_Fail++;
    } else {
      Publish_Fail = 0;
    }
#if MQTT_DEBUG > 0
    DBG_LOG("MQTT publish top:%s, return:%d", topic, rc);
#endif
  }

  return (rc > 0) ? TRUE : FALSE;
}

/**
 * 订阅MQTT消息，须在MQTT连接之前调用此函数
 * @param topic 订阅的主题名
 * @param qos   消息服务质量等级
 * @param fun   订阅消息回调的指针
 * @return 返回订阅结果
 */
BOOL Subscribe_MQTT(char const* topic, Qos qos, Arrived_t fun) {
  uint8_t i = 0;

  if (topic != NULL && fun != NULL) {
    for (i = 0; i < MAX_MESSAGE_HANDLERS; i++) {
      if (Subs[i].topic == NULL) {
        Subs[i].topic = topic;
        Subs[i].qos = qos;
        Subs[i].callback = fun;
        return TRUE;
      }
    }
  }
  return FALSE;
}

/* Private function prototypes -----------------------------------------------*/

/**
 * MQTT连接管理
 */
static void Manager_MQTT(void) {
  BOOL pingout = FALSE;


  /*网络类型变化或者掉线时应断开MQTT，否则在线状态是假的*/
  if (mClient.isconnected && !GPRS_IsSocketConnect()) {
    mClient.isconnected = 0;
  }

  /*推送失败或心跳无应答时断开连接*/
  if (mClient.ping_outstanding == 0) {
    TSEC_INIT(tsPingOut);
  } else if (TSEC_IS_OVER(tsPingOut, 6)) {
    pingout = TRUE;
    TSEC_INIT(tsPingOut);
  }
  /*发送失败或心跳无应答时重连*/
  if (Publish_Fail > 1 || pingout > 0) {
    Publish_Fail = 0;
    mClient.ping_outstanding = 0;
    if (mClient.isconnected) {
      Disconnect_MQTT();
    }
  }
  /*多次连接失败后重新鉴权*/
  if (Connect_Fail >= CONNECT_FAIL_REAUTH) {
    if (mClient.isconnected) {
      Disconnect_MQTT();
    }
    Connect_Fail = 0;
    DBG_LOG("CONNECT_FAIL_REAUTH.");
  }

  if (mClient.isconnected == 0) {
    /*socket未占用时连接到服务器*/
    if (GPRS_IsSocketConnect() == FALSE) {
      GPRS_SetSocketParam(WorkData.MQTT_Server, WorkData.MQTT_Port);
    }
    /*建立连接*/
    if (GPRS_IsSocketConnect() == 1) {
      /*参数初始化*/
      connectData.MQTTVersion = 4;
      connectData.keepAliveInterval = MQTT_PING_INVT_DEF;

      static char clientid[32];
      sprintf(clientid, "%u", WorkData.DeviceID);
      connectData.clientID.cstring = clientid;
      connectData.username.cstring = "AD-15";
      connectData.password.cstring = "123456";
      connectData.cleansession = 1;
      connectData.willFlag = 0;
      /*连接MQTT*/
      if (Connect_MQTT()) {
        DBG_LOG("MQTT-clientId:%s", clientid);
        DBG_LOG("MQTT server:%s, port:%u", WorkData.MQTT_Server, WorkData.MQTT_Port);
        DBG_LOG("MQTT Ping invter %d s", MQTT_PING_INVT_DEF / 2);
        Connect_Fail = 0;
        MQTT_ReSubscribe();
      } else {
        Connect_Fail++;
      }
    }
  }
}

/**
 * MQTT锟斤拷锟铰讹拷锟斤拷锟斤拷锟斤拷
 */
static void MQTT_ReSubscribe(void) {
  int i, rc;

  for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
    mClient.messageHandlers[i].topicFilter = 0;
  }

  /*已订阅的主题重新订阅*/
  for (i = 0; i < MAX_MESSAGE_HANDLERS; i++) {
    if (Subs[i].topic != NULL) {
      rc = MQTTSubscribe(&mClient, Subs[i].topic, Subs[i].qos, messageArrived);
#if MQTT_DEBUG > 0
      DBG_LOG("MQTT subscribe %s, return:%d", Subs[i].topic, rc);
#endif
      if (rc != 0) {
        Publish_Fail++;
        break;
      }
    }
  }
}

/**
 * 连接至MQTT服务
 * @param id   MQTT的ID
 * @return 返回连接结果
 */
static BOOL Connect_MQTT(void) {
  int rc = 0;

  /*连接失败时重试一次*/
  rc = MQTTConnect(&mClient, &connectData);
  if (rc != SUCESS) {
    rc = MQTTConnect(&mClient, &connectData);
  }
  if (rc != SUCESS) {
    DBG_LOG("Return code from MQTT connect is %d", rc);
  } else {
    DBG_LOG("MQTT Connected");
  }
  return (BOOL)!rc;
}

/**
 * 断开MQTT的连接
 * @return 返回断开结果
 */
static BOOL Disconnect_MQTT(void) {
  int rc;

  if ((rc = MQTTDisconnect(&mClient)) != SUCESS) {
    DBG_LOG("Return code from MQTT disconnect is %d", rc);
  } else {
    DBG_LOG("MQTT Disonnected");
  }

  return (BOOL)!rc;
}

/**
 * 调试命令
 * @param argc 参数项数量
 * @param argv 参数列表
 */
static void mqtt_Console(int argc, char* argv[]) {
  Qos qos;

  argv++;
  argc--;
  if (strcmp(argv[0], "publish") == 0) {
    qos = (Qos)uatoi(argv[2]);
    if (Publish_MQTT(argv[1], qos, (uint8_t*)argv[3], strlen(argv[3]))) {
      DBG_LOG("MQTT publish %s OK.", argv[1]);
    }
  } else if (strcmp(argv[0], "status") == 0) {
    DBG_LOG("MQTT client connect:%d, ping out:%d",
            mClient.isconnected,
            mClient.ping_outstanding);
  } else if (strcmp(argv[0], "disconnect") == 0) {
    Disconnect_MQTT();
    DBG_LOG("test MQTT disconnect.");
  } else if (strcmp(argv[0], "connect") == 0) {
    Connect_MQTT();
    DBG_LOG("test MQTT connect.");
  }
}
