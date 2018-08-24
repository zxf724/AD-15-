
/**
 * *********************************************************************
 *             Copyright (c) 2016 temp. All Rights Reserved.
 * @file protocols.c
 * @version V1.0
 * @date 2016.4.1
 * @brief ��������������ͨѶЭ�麯���ļ�.
 *
 * *********************************************************************
 * @note
 *
 * *********************************************************************
 * @author ����
 * 2016.12.30 ���Ӷ���ʧ������
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
 * Э�鴦���ʼ��
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
 * Э�鴦�������
 * @param argument
 */
void MQTT_Conn_Polling(void) {
  UART_MutexCount++;
  Manager_MQTT();

  MQTTYield(&mClient, 0);
  UART_MutexCount--;
}

/**
 * MQTT����״̬��ѯ,�����ӷ���0�������ӷ���MQTTʹ�õ�����·��
 */
BOOL MQTT_IsConnected(void) {
  return (BOOL)mClient.isconnected;
}


/**
 * �����ĵ���Ϣ
 * @param data ��Ϣ����
 */
void messageArrived(MessageData* data) {
  int i = 0;

#if MQTT_DEBUG > 0
  DBG_LOG("Receive New message%.*s:",
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
 * Э�鴦����������
 * @param dat  ����ָ��
 * @param len  ���ݳ���
 */
int16_t MQTT_SendData(uint8_t* dat, uint16_t len) {
  int16_t rc = -1;

  if (GPRS_IsSocketConnect()) {
    /*��������ָʾ��*/
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
 * Э�鴦���������
 * @param dat  ���ݶ�����ָ��
 * @param len  �������ĳ���
 * @return ����ʵ�ʶ����ĳ���
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
 * ����MQTT��Ϣ
 * @param topic   ������������
 * @param qos     ��Ϣ���������ȼ�
 * @param payload ��Ϣ��
 * @param len     ��Ϣ�峤��
 * @return ���ط������
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

  return (rc == 0) ? TRUE : FALSE;
}

/**
 * ����MQTT��Ϣ������MQTT����֮ǰ���ô˺���
 * @param topic ���ĵ�������
 * @param qos   ��Ϣ���������ȼ�
 * @param fun   ������Ϣ�ص���ָ��
 * @return ���ض��Ľ��
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
 * MQTT���ӹ���
 */
static void Manager_MQTT(void) {
  BOOL pingout = FALSE;


  /*�������ͱ仯���ߵ���ʱӦ�Ͽ�MQTT����������״̬�Ǽٵ�*/
  if (mClient.isconnected && !GPRS_IsSocketConnect()) {
    mClient.isconnected = 0;
  }

  /*����ʧ�ܻ�������Ӧ��ʱ�Ͽ�����*/
  if (mClient.ping_outstanding == 0) {
    TSEC_INIT(tsPingOut);
  } else if (TSEC_IS_OVER(tsPingOut, 15)) {
    pingout = TRUE;
    TSEC_INIT(tsPingOut);
    DBG_LOG("MQTT ping timeout.");
  }
  /*����ʧ�ܻ�������Ӧ��ʱ����*/
  if (Publish_Fail > 1 || pingout > 0 || Connect_Fail > 1) {
    Publish_Fail = 0;
    mClient.ping_outstanding = 0;
    if (mClient.isconnected || Connect_Fail) {
      Disconnect_MQTT();
    }
  }

  if (mClient.isconnected == 0) {
    /*socketδռ��ʱ���ӵ�������*/
    if (GPRS_IsSocketConnect() == FALSE) {
      GPRS_SetSocketParam(WorkData.MQTT_Server, WorkData.MQTT_Port);
    }
    /*��������*/
    if (GPRS_IsSocketConnect() == 1) {
      /*������ʼ��*/
      connectData.MQTTVersion = 4;
      connectData.keepAliveInterval = MQTT_PING_INVT_DEF;

      static char clientid[32];
      sprintf(clientid, "%u", WorkData.DeviceID);
      connectData.clientID.cstring = clientid;
      connectData.username.cstring = "AD-15";
      connectData.password.cstring = "AD-15";
      connectData.cleansession = 1;
      connectData.willFlag = 0;
      /*����MQTT*/
      if (Connect_MQTT()) {
        DBG_LOG("MQTT-clientId:%s", clientid);
        DBG_LOG("MQTT server:%s, port:%u", WorkData.MQTT_Server, WorkData.MQTT_Port);
        DBG_LOG("MQTT Ping invter %d s", MQTT_PING_INVT_DEF / 2);
        Connect_Fail = 0;
        MQTT_ReSubscribe();
        /*MQTT���ӳɹ�ʱ�ϱ��豸����*/
        Status_Updata();
      } else {
        Connect_Fail++;
      }
    }
  }
}

/**
 * MQTT���¶�������
 */
static void MQTT_ReSubscribe(void) {
  int i, rc;

  for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
    mClient.messageHandlers[i].topicFilter = 0;
  }

  /*�Ѷ��ĵ��������¶���*/
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
 * ������MQTT����
 * @param id   MQTT��ID
 * @return �������ӽ��
 */
static BOOL Connect_MQTT(void) {
  int rc = 0;

  /*����ʧ��ʱ����һ��*/
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
 * �Ͽ�MQTT������
 * @return ���ضϿ����
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
 * ��������
 * @param argc ����������
 * @param argv �����б�
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
