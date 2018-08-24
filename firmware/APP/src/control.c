/* Includes ------------------------------------------------------------------*/
#include "includes.h"



/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  uint16_t activetime;
  uint16_t idletime;
  uint16_t times;
  uint16_t index;
  BOOL active;
} Period_Block_t;


/* Private define ------------------------------------------------------------*/
/*Э������*/
#define CMD_DEVICE_RST          0xFF
#define CMD_DEVICE_CHK          0x01
#define CMD_DEVICE_CHK_RSP      0x11
#define CMD_READ_IC             0x02
#define CMD_READ_IC_RSP         0x12
#define CMD_READ_IC_FRIM        0x03
#define CMD_READ_IC_FRIM_RSP    0x13
#define CMD_WRITE_FRIM          0x14


/* Private macros ------------------------------------------------------------*/
#define SEND_READ_RFID()        RFID_SendCmd(0, CMD_READ_IC, NULL, 0);

/* Private variables ---------------------------------------------------------*/

uint16_t BatVol = 0;
motor_status_t Motor_staus;

APP_TIMER_DEF(TimerId_Lock);
APP_TIMER_DEF(TimerId_LED_NET);
APP_TIMER_DEF(TimerId_LED_STATUS);
APP_TIMER_DEF(TimerId_TTS);

static uint8_t TTS_Step = 0, Direction = 0, RFID_OK = 0;
static char* TTS_Text = NULL;
static uint16_t motorTick = 0;
static uint32_t  RFID_Read = 0, RFID_Repay = 0, RFID_Borrow = 0;
static Period_Block_t  LED_NET_Period, LED_STATUS_Period;

/* Private function prototypes -----------------------------------------------*/
static void Motor_TimerCB(void* p_context);
static void LED_NET_TimerCB(void* p_context);
static void LED_STATUS_TimerCB(void* p_context);
static void TTS_TimerCB(void* p_context);
static void RFID_SendCmd(uint8_t addr, uint8_t cmd, uint8_t* data, uint8_t datalen);
static uint8_t RFID_ReadPoll(uint8_t addr, uint8_t** data);
static void funControl(int argc, char* argv[]);

/* Exported functions ---------------------------------------------------------*/

/**
* @brief  ������ѭ����.
* @param  none.
* @retval none.
*/
void Control_Init(void) {

  app_timer_create(&TimerId_Lock, APP_TIMER_MODE_REPEATED, Motor_TimerCB);
  app_timer_create(&TimerId_LED_NET, APP_TIMER_MODE_SINGLE_SHOT, LED_NET_TimerCB);
  app_timer_create(&TimerId_LED_STATUS, APP_TIMER_MODE_SINGLE_SHOT, LED_STATUS_TimerCB);
  app_timer_create(&TimerId_TTS, APP_TIMER_MODE_SINGLE_SHOT, TTS_TimerCB);

  CMD_ENT_DEF(control, funControl);
  Cmd_AddEntrance(CMD_ENT(control));

  DBG_LOG("Device control init.");
}

/**
 * ������ѯ����
 */
void Control_Polling(void) {
  uint8_t led_n;
  static uint8_t led_net;
  static motor_status_t status;
  /*TTS����*/
  if (TTS_Step == 1) {
    TTS_Step = 2;
    app_timer_start(TimerId_TTS, APP_TIMER_TICKS(1000 + strlen(TTS_Text) * 200, APP_TIMER_PRESCALER), NULL);
    GPRS_TTS(TTS_Text);
  }
  /*LED��˸*/
  if (DataFlowCnt > 0) {
    DataFlowCnt--;
    led_n = 1;
  } else if (GPRS_ReadStatus() == gprs_status_nocard) {
    led_n = 2;
  } else if (GPRS_ReadStatus() == gprs_status_nonet) {
    led_n = 3;
  } else if (!MQTT_IsConnected()) {
    led_n = 4;
  } else if (led_net != 1) {
    led_n = 0;
  }
  if (led_n != led_net) {
    led_net = led_n;
    switch (led_n) {
      case 1:
        LED_NET_FLASH();
        break;
      case 2:
        LED_NOCARD_FLASH();
        break;
      case 3:
        LED_NONET_FLASH();
        break;
      case 4:
        LED_NOMQTT_FLASH();
        break;
    }
  }
  /*ȡ��ɡ����*/
  if (Motor_staus != status) {
    status = Motor_staus;
    switch (Motor_staus) {
      case status_borrow:
        TTS_Play("��ʼ��ɡ");
        break;
      case status_repay:
        TTS_Play("��ʼ��ɡ");
        break;
      case status_borrow_complite:
        Motor_staus = status_idle;
        if (RFID_Borrow != RFID_Read) {
          RFID_Borrow = RFID_Read;
          TTS_Play("��ɡ�ɹ���лл");
          WorkData.StockCount--;
          WorkData_Update();
          Protocol_Report_Umbrella_Borrow(RFID_Read, status);
          Report_Umbrella_Borrow_Status(RFID_Read, status);
        }
        break;
      case status_repay_complite:
        Motor_staus = status_idle;
        if (RFID_Repay != RFID_Read) {
          RFID_Repay = RFID_Read;
          TTS_Play("��ɡ�ɹ���лл");
          WorkData.StockCount++;
          WorkData_Update();
          if (Report_Umbrella_Repy_Status(RFID_Read, status, RTC_ReadCount()) == FALSE) {
            /*��ɡ�ϱ�ʧ�ܣ��洢��¼*/
            Write_StoreLog(RFID_Read);
          }
        }
        break;
      case status_motor_stuck:
        TTS_Play("�����ס");
        Motor_staus = status_idle;
        if (Direction == 1) {
          Protocol_Report_Umbrella_Borrow(0, status);
          Report_Umbrella_Borrow_Status(0, status);
        } else if (Direction == 2) {
          Report_Umbrella_Repy_Status(0, status, RTC_ReadCount());
        }
        break;
      case status_ir_stuck:
        TTS_Play("����ȷ�۵�������ɡ���ٴι黹");
        Motor_staus = status_idle;
        if (Direction == 1) {
          Protocol_Report_Umbrella_Borrow(0, status);
          Report_Umbrella_Borrow_Status(0, status);
        } else if (Direction == 2) {
          Report_Umbrella_Repy_Status(0, status, RTC_ReadCount());
        }
        break;
      case status_timeout:
        TTS_Play("�豸����");
        Motor_staus = status_idle;
        if (Direction == 1) {
          Protocol_Report_Umbrella_Borrow(0, status);
          Report_Umbrella_Borrow_Status(0, status);
        } else if (Direction == 2) {
          Report_Umbrella_Repy_Status(0, status, RTC_ReadCount());
        }
        break;
      case status_empty:
        TTS_Play("��������ɡ��ȡ���뻻һ̨");
        Motor_staus = status_idle;
        Protocol_Report_Umbrella_Borrow(0, status);
        Report_Umbrella_Borrow_Status(0, status);
        break;
      case status_full:
        TTS_Play("������ɡ�������뻻һ̨");
        Motor_staus = status_idle;
        Report_Umbrella_Repy_Status(0, status, RTC_ReadCount());
        break;
      default:
        break;
    }
  }
  /*������ɡ��¼*/
  if (WorkData.StoreLog_Report != WorkData.StoreLog_In && MQTT_IsConnected()) {
    motor_status_t sta;
    uint16_t index = WorkData.StoreLog_Report + 1;
    if (index >= LOG_STORE_MAX) {
      index = 0;
    }
    DBG_LOG("Report Histroy:%u.", index);
    sta = status_repay_complite;
    if (Report_Umbrella_Repy_Status(STROE_LOG_POINT(index)->rfid, sta, STROE_LOG_POINT(index)->time)) {
      WorkData.StoreLog_Report = index;
      WorkData_Update();
    }
  }
  /*ѭ������*/
  uint8_t* pdata = NULL;
  if (RFID_ReadPoll(0, &pdata) == CMD_READ_IC_RSP) {

    RFID_OK = 1;
    RFID_Read = (pdata[3] << 24) | (pdata[2] << 16) | (pdata[1] << 8) | pdata[0];
    DBG_LOG("Read New ID:%#x", RFID_Read);
    if (Motor_staus == status_idle) {
      Repay_Action();
    }
  } else {
    RFID_OK = 0;
  }
}

/**
 * ��ɡ��������
 */
void Borrow_Action(void) {

  app_timer_stop(TimerId_Lock);
  motorTick = 0;

  Direction = 1;
  if (WorkData.StockCount == 0) {
    LED_MOTOR_NG();
    Motor_staus = status_empty;
    DBG_LOG("Borrow_Action Empty.");
  }
  /*����Ƿ�ס*/
  else if (IR_CHECK()) {
    LED_IR_OVER_FLASH();
    Motor_staus = status_ir_stuck;
    DBG_LOG("Borrow_Action IR Stuck.");
  } else {
    Motor_staus = status_borrow;
    LED_ON(STATUS);
    MOTOR_FORWARD();
    app_timer_start(TimerId_Lock, APP_TIMER_TICKS(MOTOR_ACTION_TIME, APP_TIMER_PRESCALER), NULL);
  }
}

/**
 * ��ɡ��������
 */
void Repay_Action(void) {

  app_timer_stop(TimerId_Lock);
  motorTick = 0;

  Direction = 2;
  if (WorkData.StockCount >= WorkData.StockMax) {
    LED_MOTOR_NG();
    Motor_staus = status_full;
    DBG_LOG("Borrow_Action Full.");
  }
  /*����Ƿ�ס*/
  else if (IR_CHECK()) {
    LED_IR_OVER_FLASH();
    Motor_staus = status_ir_stuck;
    DBG_LOG("Repay_Action IR Stuck.");
  } else {
    Motor_staus = status_repay;
    LED_ON(STATUS);
    MOTOR_BACK();
    app_timer_start(TimerId_Lock, APP_TIMER_TICKS(MOTOR_ACTION_TIME, APP_TIMER_PRESCALER), NULL);
  }
}

/**
 * ֹͣ�豸��������
 */
void Stop_Action(void) {
  LED_OFF(STATUS);
  motorTick = 0;
  app_timer_stop(TimerId_Lock);
  MOTOR_STOP();
}

/**
 * ����ָʾ�ƿ�ʼ��˸
 * 
 * @param activetime һ����������ʱ��
 * @param idletime   һ���������ʱ��
 * @param times      ������
 */
void LED_NET_Flash_Start(uint16_t activetime, uint16_t idletime, uint16_t times) {
  app_timer_stop(TimerId_LED_NET);
  LED_OFF(NET);

  LED_NET_Period.index = 0;
  LED_NET_Period.active = FALSE;
  LED_NET_Period.activetime = activetime;
  LED_NET_Period.idletime = idletime;
  LED_NET_Period.times = times;

  LED_ON(NET);
  LED_NET_Period.active = TRUE;
  app_timer_start(TimerId_LED_NET, APP_TIMER_TICKS(activetime, APP_TIMER_PRESCALER), NULL);
}

/**
 * ״ָ̬ʾ�ƿ�ʼ��˸
 * 
 * @param activetime һ����������ʱ��
 * @param idletime   һ���������ʱ��
 * @param times      ������
 */
void LED_STATUS_Flash_Start(uint16_t activetime, uint16_t idletime, uint16_t times) {
  app_timer_stop(TimerId_LED_STATUS);
  LED_OFF(STATUS);

  LED_STATUS_Period.index = 0;
  LED_STATUS_Period.active = FALSE;
  LED_STATUS_Period.activetime = activetime;
  LED_STATUS_Period.idletime = idletime;
  LED_STATUS_Period.times = times;

  LED_ON(STATUS);
  LED_STATUS_Period.active = TRUE;
  app_timer_start(TimerId_LED_STATUS, APP_TIMER_TICKS(activetime, APP_TIMER_PRESCALER), NULL);
}

/**
 * ����TS
 * 
 * @param text   ���������ı�
 */
void TTS_Play(char* text) {
  app_timer_stop(TimerId_TTS);
  PA_ENABLE();

  TTS_Step = 0;
  TTS_Text = text;
  app_timer_start(TimerId_TTS, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), NULL);
}

/**
* �忴�Ź�.
*/
void WatchDog_Clear(void) {
#if WTD_EN == 1
  nrf_drv_wdt_feed();
#endif
}

/**
 * @brief ��ɡ������������.
 * @retval none.
 * @param p_context
 */
static void Motor_TimerCB(void* p_context) {
  /*��ʱ����Ƿ����*/
  if (motorTick > 10 && MOTOR_IS_STUCK()) {
    LED_MOTOR_OVER_FLASH();
    app_timer_stop(TimerId_Lock);
    Stop_Action();
    Motor_staus = status_motor_stuck;
    DBG_LOG("Motor is Stuck.");
  }
  /*������*/
  if (IR_CHECK()) {
    LED_IR_OVER_FLASH();
    app_timer_stop(TimerId_Lock);
    Stop_Action();
    Motor_staus = status_ir_stuck;
    DBG_LOG("Motor IR Stuck.");
  }
  /*��ʱ��鵽λ*/
  if (motorTick > 100 && UM_OVER_CHECK()) {
    app_timer_stop(TimerId_Lock);
    Stop_Action();
    if (Motor_staus == status_borrow) {
      Motor_staus = status_borrow_complite;
    } else if (Motor_staus == status_repay) {
      Motor_staus = status_repay_complite;
    }
    DBG_LOG("Motor Running over.");
  }
  /*��ʱֹͣ*/
  if (motorTick++ >= MOTOR_OVERFLOW_TIMES) {
    motorTick = 0;
    Stop_Action();
    Motor_staus = status_timeout;
    DBG_LOG("Motor Running Timeout.");
  }
}

/**
 * ����ָʾ�ƶ�ʱ���ص�
 */
static void LED_NET_TimerCB(void* p_context) {
  if (LED_NET_Period.index == LED_FLASH_CONTINUE) {
    LED_NET_Period.index = 0;
  }

  if (LED_NET_Period.index < LED_NET_Period.times) {
    if (LED_NET_Period.active) {
      LED_OFF(NET);
      LED_NET_Period.active = FALSE;
      LED_NET_Period.index++;
      if (LED_NET_Period.index < LED_NET_Period.times) {
        app_timer_start(TimerId_LED_NET, APP_TIMER_TICKS(LED_NET_Period.idletime, APP_TIMER_PRESCALER), NULL);
      }
    } else {
      LED_ON(NET);
      LED_NET_Period.active = TRUE;
      app_timer_start(TimerId_LED_NET, APP_TIMER_TICKS(LED_NET_Period.activetime, APP_TIMER_PRESCALER), NULL);
    }
  }
}

/**
 * ״ָ̬ʾ�ƶ�ʱ���ص�
 */
static void LED_STATUS_TimerCB(void* p_context) {
  if (LED_STATUS_Period.index == LED_FLASH_CONTINUE) {
    LED_STATUS_Period.index = 0;
  }
  if (LED_STATUS_Period.index < LED_STATUS_Period.times) {
    if (LED_STATUS_Period.active) {
      LED_OFF(STATUS);
      LED_STATUS_Period.active = FALSE;
      LED_STATUS_Period.index++;
      if (LED_STATUS_Period.index < LED_STATUS_Period.times) {
        app_timer_start(TimerId_LED_STATUS, APP_TIMER_TICKS(LED_STATUS_Period.idletime, APP_TIMER_PRESCALER), NULL);
      }
    } else {
      LED_ON(STATUS);
      LED_STATUS_Period.active = TRUE;
      app_timer_start(TimerId_LED_STATUS, APP_TIMER_TICKS(LED_STATUS_Period.activetime, APP_TIMER_PRESCALER), NULL);
    }
  }
}

/**
 * TTS��ʱ���ص�����
 */
static void TTS_TimerCB(void* p_context) {

  if (TTS_Step == 0) {
    TTS_Step = 1;
  } else if (TTS_Step == 3) {
    TTS_Step = 0;
    PA_DISABLE();
  }
}


/**
 * ��������
 *
 * @param addr    RFID�ӻ���ַ
 * @param cmd     ����
 * @param data    ��������
 * @param datalen �������ݳ���
 */
static void RFID_SendCmd(uint8_t addr, uint8_t cmd, uint8_t* data, uint8_t datalen) {
  uint8_t buf[32], *p = buf, len = 5;

  *p++ = 0x7E;
  *p++ = 0x1B;
  *p++ = addr;
  *p++ = cmd;
  *p++ = datalen;
  if (data != NULL && datalen > 0) {
    memcpy(p, data, datalen);
    p += datalen;
    len = len + datalen;
  }
  *p = AddCheck(buf, len);
  len += 1;

  RFID_SEND(buf, len);
}

/**
 * RFID���豸Ӧ��
 *
 * @param addr   �����豸��ַ
 * @param data   �����ĸ�����������
 * @return ���ط���������
 */
static uint8_t RFID_ReadPoll(uint8_t addr, uint8_t** data) {
  static uint8_t buf[32];
  uint8_t len = 0, *p = buf, ret = 0;

  len = user_uart_RecLength();
  if (len >= 6 && NRF_UART0->PSELRXD == RFID_RX_PIN) {
    nrf_delay_ms(5); //
    len = user_uart_RecLength();
    if (len > 32) {
      len = 32;
    }
    user_uart_ReadData(buf, len);
    UART_RX_PIN_SELECT(UART_RX_DEFAULT_PIN);
    while (*p != 0x7E) {
      p++;
      len--;
    }
    if (*p == 0x7E && *(p + 1) == 0x1B
        && (*(p + 2) == addr || addr == 0) && AddCheck(p, len - 1) == p[len - 1]) {
      ret = *(p + 3);
      *data = p + 5;
    }
  }
  return ret;
}

/**
 * @brief �豸���Ƶ�������.
 * @param argc
 * @param argv
 */
static void funControl(int argc, char* argv[]) {

  argv++;
  argc--;

  if (ARGV_EQUAL("tts")) {
    DBG_LOG("TTS play start.");
    TTS_Play(argv[1]);
  } else if (ARGV_EQUAL("borrow")) {
    DBG_LOG("Borrow_Action.");
    Borrow_Action();
  } else if (ARGV_EQUAL("repay")) {
    DBG_LOG("Repay_Action.");
    Repay_Action();
  } else if (ARGV_EQUAL("read")) {
    DBG_LOG("SEND_READ_RFID() .");
    SEND_READ_RFID();
  } else if (ARGV_EQUAL("status")) {
    DBG_LOG("Motor_staus:%u.", Motor_staus);
  } else if (ARGV_EQUAL("forward")) {
    MOTOR_FORWARD();
    DBG_LOG("Motor forward.");
  } else if (ARGV_EQUAL("back")) {
    MOTOR_BACK();
    DBG_LOG("Motor back.");
  } else if (ARGV_EQUAL("stop")) {
    MOTOR_STOP();
    DBG_LOG("Motor stop.");
  }
}

/************************ (C) COPYRIGHT  *****END OF FILE****/

