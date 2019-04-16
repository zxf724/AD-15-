/**
    ******************************************************************************
    * @file    profession.c
    * @author  ����
    * @version V1.0
    * @date    2017.12.15
    * @brief   ҵ���߼���غ���.
    *
    ******************************************************************************
    */


/* Includes ------------------------------------------------------------------*/
#include "includes.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static void button_handler(uint8_t pin_no, uint8_t button_action);
static const app_button_cfg_t app_buttons[2] =
{
  { FPG_INT_PIN, APP_BUTTON_ACTIVE_HIGH, NRF_GPIO_PIN_PULLDOWN, button_handler },
  { TP_INT_PIN, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, button_handler },
};

static uint8_t pwdBuf[PWD_SIZE_MAX + 1], pwdIndex = 0, fpgNew = 0, pwdNew = 0;
static uint8_t volatile tpInt = 0, m1Int = 0, fpgInt = 0;
static uint32_t pwdTS = 0, setTS = 0;
static uint32_t m1New = 0;
static uint8_t  LockopenRecord = 0;

MStatus_t mStatus;

APP_TIMER_DEF(TimerId_M1Wakeup);
APP_TIMER_DEF(TimerId_FPGWakeup);

/* Private function prototypes -----------------------------------------------*/
static void M1_TimerCB(void* p_context);
static void FPG_TimerCB(void* p_context);
static void M1_DelayCB(void);
static void fpg_polling(void);
static void button_polling(void);
static void M1_polling(void);
static void prof_Console(int argc, char* argv[]);

/* Exported functions --------------------------------------------------------*/

/**
 * ҵ���߼���ʼ��
 */
void Profession_init(void) {


  CMD_ENT_DEF(prof, prof_Console);
  Cmd_AddEntrance(CMD_ENT(prof));

  app_button_init((app_button_cfg_t*)app_buttons, 2,
                  APP_TIMER_TICKS(5, APP_TIMER_PRESCALER));
  app_button_enable();

  app_timer_create(&TimerId_M1Wakeup, APP_TIMER_MODE_REPEATED, M1_TimerCB);
  app_timer_start(TimerId_M1Wakeup, APP_TIMER_TICKS(M1_CARD_WAKEUP_PERIOD, APP_TIMER_PRESCALER), NULL);

  app_timer_create(&TimerId_FPGWakeup, APP_TIMER_MODE_SINGLE_SHOT, FPG_TimerCB);

  FPG_EN();
  nrf_delay_ms(FPG_POWERON_DELAY);
  if (FPG_PS_ValidTempleteNum(NULL) > 0) {
    LockopenRecord |= OPEN_FUN_FPG;
  }
  FPG_DIS();
  if (Search_PasswordIndex((uint8_t*)1, 0) != 1) {
    LockopenRecord |= OPEN_FUN_PWD;
  }
  if (Search_M1Card(1) != 1) {
    LockopenRecord |= OPEN_FUN_M1;
  }

  DBG_LOG("Procession init.");
}


/**
 * ҵ���߼���ѯ����
 */
void Profession_Polling(void) {
  int ret = -1;
  uint32_t rfid = 0;
  uint16_t pageid = 0, score = 0;
  LockLog_t log;

  /*M1��¼��*/ /*ָ��¼��*/
  if (mStatus == m_set_FPG || mStatus == m_set_M1) {
    if (mStatus == m_set_M1 && m1Int > 0) {
      m1Int = 0;
      ret = Search_M1Card(0);
      if (ret >= 0) {
        if (StartLinkRc522()) {
          rfid = *((uint32_t*)&MLastSelectedSnr[0]);

          /*�����ظ�¼��*/
          if (Search_M1Card(rfid) == -1) {
            WDATA_M1_UID_T = rfid;
            WDATA_M1_UID_SAVE(ret);
            DBG_LOG("Add New M1:%u, uid:%#x", ret, rfid);
          } else {
            DBG_LOG("Already Add M1:%u, uid:%#x", ret, rfid);
          }
          Report_SetM1(TRUE, rfid);
          setTS = RTC_ReadCount() - 6;
        }
      }
      /*��������*/
      else {
        BUZ_BEEP_DOOR_DIS_OPEN();
        Report_SetM1(FALSE, 0);
        setTS = RTC_ReadCount() - 6;
        DBG_LOG("Add New M1 failed.");
      }
    } else if (mStatus == m_set_FPG && IS_FPG_INT()) {
      ret = FPG_PS_Identify(&pageid, &score);
      if (ret == 0x09) {
        ret = FPG_PS_Enroll(&pageid);
        if (ret == 0) {
          Report_SetFPG(TRUE, pageid);
          setTS = RTC_ReadCount() - 6;
          DBG_LOG("Add New FPG:%u.", pageid);
        } else {
          BUZ_BEEP_DOOR_DIS_OPEN();
          Report_SetFPG(FALSE, ret);
          setTS = RTC_ReadCount() - 6;
          DBG_LOG("Add New FPG failed.");
        }
      }
      /*�����ظ�¼��*/
      else if (ret == 0) {
        Report_SetFPG(TRUE, pageid);
        setTS = RTC_ReadCount() - 6;
        DBG_LOG("Already Add FPG:%u.", pageid);
      }
      /*ָ��ģ�����*/
      else if (ret == 0xFF) {
        BUZ_BEEP_DOOR_DIS_OPEN();
        Report_SetFPG(FALSE, ret);
      }
    }
    /*��ʱ�˳�*/
    if (RTC_ReadCount() - setTS >= 5) {
      mStatus = m_idle;
      MFRC_DIS();
      FPG_DIS();
      LED_OFF(R);
      LED_OFF(G);
    }
  } else {
    setTS = RTC_ReadCount();
  }

  /*��¼������־*/
  if (mStatus > m_idle && mStatus <= m_open_Empty) {
    if (mStatus == m_open_Empty && LockopenRecord == 0) {
      BUZ_BEEP_DOOR_OPEN();
      LockOpen_Action();
    } else if (mStatus == m_open_FPG && (WDATA_FUNCTION & OPEN_FUN_FPG)) {
      BUZ_BEEP_DOOR_OPEN();
      LockOpen_Action();
      log.time = RTC_ReadCount();
      log.opentype = OPEN_FUN_FPG;
      log.arg =  fpgNew;
      Save_LockLog(&log);
    } else if (mStatus == m_open_BLE && (WDATA_FUNCTION & OPEN_FUN_BLE)) {
      BUZ_BEEP_DOOR_OPEN();
      LockOpen_Action();
      log.time = RTC_ReadCount();
      log.opentype = OPEN_FUN_BLE;
      log.arg = 0;
      Save_LockLog(&log);
    } else if (mStatus == m_open_M1 && (WDATA_FUNCTION & OPEN_FUN_M1)) {
      BUZ_BEEP_DOOR_OPEN();
      LockOpen_Action();
      log.time = RTC_ReadCount();
      log.opentype = OPEN_FUN_M1;
      log.arg = m1New;
      Save_LockLog(&log);
    } else if (mStatus == m_open_PWD && (WDATA_FUNCTION & OPEN_FUN_PWD)) {
      BUZ_BEEP_DOOR_OPEN();
      LockOpen_Action();
      log.time = RTC_ReadCount();
      log.opentype = OPEN_FUN_PWD;
      log.arg = pwdNew;
      Save_LockLog(&log);
    } else {
      DBG_LOG("Open door not allow, request:%u", mStatus);
      BUZ_BEEP_DOOR_NOTALLOW_OPEN();
    }
    mStatus = m_idle;
  }

  if (mStatus == m_idle) {
    /*������������*/
    button_polling();

    /*ָ�ƿ�������*/
    if (fpgInt > 0) {
      fpgInt = 0;
      if (IS_FPG_INT()) {
        fpg_polling();
      }
      FPG_DIS();
    }
    /*M1������*/
    if (m1Int > 0) {
      m1Int = 0;
      M1_polling();
      MFRC_DIS();
    }
  }
}

/**
 * ������¼������
 * 
 * @param pwd    �����ַ�,����ΪNULLʱ���ҿ��еı��
 * @param len    ���볤��
 * @return �ҵ����ر�ţ�ʧ�ܷ���-1
 */
int Search_PasswordIndex(uint8_t* pwd, uint8_t len) {
  int i;
  char tbuf[PWD_SIZE_MAX + 1];

  if (pwd == NULL) {
    for (i = 0; i < PWD_STORE_MAX; i++) {
      if (!isdigit(WDATA_PWD(i)[0])) {
        return i;
      }
    }
  } else if ((uint32_t)pwd == 1) {
    i = 0;
    while (i < PWD_STORE_MAX) {
      if (isdigit(WDATA_PWD(i)[0]) && isdigit(WDATA_PWD(i)[PWD_SIZE_LIM - 1])) {
        break;
      }
      i++;
    }
    if (i == PWD_STORE_MAX) {
      return 1;
    }
  } else if (len >= PWD_SIZE_LIM && len <= PWD_SIZE_MAX) {
    memcpy(tbuf, pwd, len);
    tbuf[len] = '\0';
    for (i = 0; i < PWD_STORE_MAX; i++) {
      if (isdigit(WDATA_PWD(i)[0])) {
        if (SearchMemData(pwd, (uint8_t*)WDATA_PWD(i), len, strlen(WDATA_PWD(i)))) {
          return i;
        }
      }
    }
  }
  return  -1;
}

/**
 * ������¼��M1����
 * 
 * @param uid    M1����,����Ϊ0ʱ���ҿ��еı��,
 *               ����Ϊ1ʱ��ѯ�Ƿ��¼Ϊ��
 * @return �ҵ����ر�ţ�ʧ�ܷ���-1
 */
int Search_M1Card(uint32_t uid) {
  int i;

  if (uid == 0) {
    for (i = 0; i < M1_CARD_STORE_MAX; i++) {
      if (WDATA_M1_UID(i) == 0 || WDATA_M1_UID(i) == BIT32_MAX) {
        return i;
      }
    }
  } else if (uid == 1) {
    i = 0;
    while (i < M1_CARD_STORE_MAX) {
      if (WDATA_M1_UID(i) > 0 && WDATA_M1_UID(i) < BIT32_MAX) {
        break;
      }
      i++;
    }
    if (i == M1_CARD_STORE_MAX) {
      return 1;
    }
  } else {
    for (i = 0; i < M1_CARD_STORE_MAX; i++) {
      if (WDATA_M1_UID(i) == uid) {
        return i;
      }
    }
  }
  return  -1;
}

/* Private function prototypes -----------------------------------------------*/

/**
 * M1����ʱ���ص�����
 */
static void M1_TimerCB(void* p_context) {
  MFRC_EN();
  Delay_Us(MFRC_POWERON_DELAY, M1_DelayCB);
}

/**
 * FPG������ʱ�ص�����
 */
static void FPG_TimerCB(void* p_context) {
  fpgInt = 1;
}

/**
 * M1����ʱ�ص�����
 */
static void M1_DelayCB(void) {
  m1Int = 1;
}

/**
 * ָ��ģ�鴦����ѯ����
 */
static void fpg_polling(void) {
  uint16_t pageid = 0, score = 0, ret = 0;

  ret = FPG_PS_Identify(&pageid, &score);
  if (ret == 0) {
    FPG_DIS();
    LockopenRecord |= OPEN_FUN_FPG;
    /*ָ��ƥ��ͬ��ʱ���������Կ���*/
    if (RTC_ReadCount() < WDATA_FPG_TS(pageid)) {
      fpgNew = ret;
      mStatus = m_open_FPG;
      DBG_LOG("FPG Match open:%d", pageid);
    } else {
      DBG_LOG("FPG TS not allow.");
      BUZ_BEEP_DOOR_NOTALLOW_OPEN();
    }
  } else if (ret == 0x09) {
    if (FPG_PS_ValidTempleteNum(&pageid) == 0) {
      FPG_DIS();
      LockopenRecord &= (~OPEN_FUN_FPG);

      /*��ָ�Ƽ�¼ʱ�κ�ָ�ƿ��Կ���*/
      if (pageid == 0) {
        mStatus = m_open_Empty;
        DBG_LOG("FPG Empty open.");
      }
      /*δ������ָ����ʾ����ʧ��*/
      else {
        BUZ_BEEP_DOOR_DIS_OPEN();
        DBG_LOG("FPG match failed.");
      }
    } else {
      LockopenRecord |= OPEN_FUN_FPG;
    }
  }
}

/**
 * ����������ѯ
 */
static void button_polling(void) {
  int ret = -1;
  uint8_t tp = 0;
  static uint8_t tpTemp = 0;

  if (tpInt > 0) {
    tpInt = 0;
    /*��������*/
    tp = TouchPad_ReadKey();
    if (tp != tpTemp) {
      tpTemp = tp;
      if (tpTemp > 0) {
        LED_TP_Flash_Start(300, 0, 1);
        BUZ_BEEP_TP();

        pwdTS = RTC_ReadCount();
        if (pwdIndex < PWD_SIZE_MAX) {
          pwdBuf[pwdIndex++] = tp;
        } else {
          memmove(&pwdBuf[0], &pwdBuf[1], PWD_SIZE_MAX - 1);
          pwdBuf[PWD_SIZE_MAX - 1] = tp;
        }
        /* *������������� */
        if (tp == '*') {
          pwdIndex = 0;
        }
        DBG_LOG("New Type:%.*s", pwdIndex, pwdBuf);
      }
    }
  }

  /*�ӳ�1���ѯ�Ƚ����룬��ʡ��*/
  if (pwdIndex >= PWD_SIZE_LIM && RTC_ReadCount() - pwdTS > 1) {
    ret = Search_PasswordIndex(pwdBuf, pwdIndex);
    /*����ƥ�估ʱ������ʱ����*/
    if (ret >= 0) {
      LockopenRecord |= OPEN_FUN_PWD;
      if (RTC_ReadCount() < WDATA_PWD_TS(ret)) {
        pwdNew = ret;
        mStatus = m_open_PWD;
        pwdIndex = 0;
        pwdTS = RTC_ReadCount();
        DBG_LOG("Password Match open:%d, password:%.*s", ret, PWD_SIZE_MAX, WDATA_PWD(ret));
      } else {
        pwdIndex = 0;
        pwdTS = RTC_ReadCount();
        DBG_LOG("Password TS not allow.");
        BUZ_BEEP_DOOR_NOTALLOW_OPEN();
      }
    }
    /*�������¼ʱ�κ�������Կ���*/
    if (ret == -1) {
      if (Search_PasswordIndex((uint8_t*)1, 0) == 1) {
        LockopenRecord &= (~OPEN_FUN_PWD); 
        mStatus = m_open_Empty;
        pwdIndex = 0;
        pwdTS = RTC_ReadCount();
        DBG_LOG("Password Empty open.");
      }
      /*��ʾ����ʧ��*/
      else if (RTC_ReadCount() - pwdTS >= 3) {
        LockopenRecord |= OPEN_FUN_PWD;
        pwdIndex = 0;
        pwdTS = RTC_ReadCount();
        BUZ_BEEP_DOOR_DIS_OPEN();
        DBG_LOG("Password match failed.");
      }
    }
  }
  /*��ʱ�����������*/
  if (pwdIndex > 0 && RTC_ReadCount() - pwdTS >= 3) {
    pwdIndex = 0;
    pwdTS = RTC_ReadCount();
  }
}

/**
 * ˢ��������ѯ
 */
static void M1_polling(void) {
  int ret = -1;
  uint32_t rfid = 0;

  if (StartLinkRc522()) {
    MFRC_DIS();
    rfid = *((uint32_t*)&MLastSelectedSnr[0]);
    if (m1New != rfid) {
      m1New = rfid;
      DBG_LOG("Read RFID:%#x", rfid);
      ret = Search_M1Card(rfid);
      /*������ͬ��ʱ���������Կ���*/
      if (ret >= 0) {
        LockopenRecord |= OPEN_FUN_M1;
        if (RTC_ReadCount() < WDATA_M1_TS(ret)) {
          mStatus = m_open_M1;
          DBG_LOG("RFID Match open:%d, uid:%#x", ret, rfid);
        } else {
          DBG_LOG("M1 TS not allow.");
          BUZ_BEEP_DOOR_NOTALLOW_OPEN();
        }
      }
      /*�޿���¼ʱ�κο����Կ���*/
      if (ret == -1) {
        if (Search_M1Card(1) == 1) {
          LockopenRecord &= (~OPEN_FUN_M1);
          mStatus = m_open_Empty;
          DBG_LOG("RFID Empty open.");
        }
        /*��ʾ����ʧ��*/
        else {
          LockopenRecord |= OPEN_FUN_M1;
          BUZ_BEEP_DOOR_DIS_OPEN();
          DBG_LOG("RFID match failed.");
        }
      }
    }
  } else {
    m1New = 0;
  }
}

/**
 * �����жϻص�����
 * 
 * @param pin_no �жϵ�����
 * @param button_action
 *               ���������ͣ������ػ����½���
 */
static void button_handler(uint8_t pin_no, uint8_t button_action) {
  if (pin_no == FPG_INT_PIN && IS_FPG_INT() && fpgInt == 0) {
    FPG_EN();
    app_timer_start(TimerId_FPGWakeup, APP_TIMER_TICKS(FPG_POWERON_DELAY, APP_TIMER_PRESCALER), NULL);
  }
  if (pin_no == TP_INT_PIN && IS_TP_INT()) {
    tpInt = 1;
  }
}
/**
 *  ָ�ƴ��������Խӿ�
 */
static void prof_Console(int argc, char* argv[]) {

  argv++;
  argc--;

  if (ARGV_EQUAL("setm1")) {
    MFRC_EN();
    nrf_delay_us(MFRC_POWERON_DELAY);
    mStatus = m_set_M1;
    LED_SETTING_FLASH();
  }	else if (ARGV_EQUAL("setfpg")) {
    FPG_EN();
    nrf_delay_ms(FPG_POWERON_DELAY);
    mStatus = m_set_FPG;
    LED_SETTING_FLASH();
  }
}

/************************ (C) COPYRIGHT  *****END OF FILE****/
