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

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

uint16_t BatVol = 0;
motor_status_t Motor_staus;

APP_TIMER_DEF(TimerId_Lock);
APP_TIMER_DEF(TimerId_LED_NET);
APP_TIMER_DEF(TimerId_LED_STATUS);

static uint16_t motorTick = 0;
static Period_Block_t  LED_NET_Period, LED_STATUS_Period;

/* Private function prototypes -----------------------------------------------*/
static void Motor_TimerCB(void* p_context);
static void LED_NET_TimerCB(void* p_context);
static void LED_STATUS_TimerCB(void* p_context);
static void funControl(int argc, char* argv[]);

/* Exported functions ---------------------------------------------------------*/

/**
* @brief  控制轮循函数.
* @param  none.
* @retval none.
*/
void Control_Init(void) {

  app_timer_create(&TimerId_Lock, APP_TIMER_MODE_REPEATED, Motor_TimerCB);
  app_timer_create(&TimerId_LED_NET, APP_TIMER_MODE_SINGLE_SHOT, LED_NET_TimerCB);
  app_timer_create(&TimerId_LED_STATUS, APP_TIMER_MODE_SINGLE_SHOT, LED_STATUS_TimerCB);

  CMD_ENT_DEF(control, funControl);
  Cmd_AddEntrance(CMD_ENT(control));

  DBG_LOG("Device control init.");
}

/**
 * 借伞操作函数
 */
void Borrow_Action(uint8_t* dat) {

  app_timer_stop(TimerId_Lock);
  motorTick = 0;

  /*检查是否卡住*/
  if (IR_CHECK()) {
    Motor_staus = status_ir_stuck;
    DBG_LOG("Borrow_Action IR Stuck.");
  } else {
    LED_ON(STATUS);
    MOTOR_FORWARD();
    app_timer_start(TimerId_Lock, APP_TIMER_TICKS(MOTOR_ACTION_TIME, APP_TIMER_PRESCALER), NULL);
  }
}

/**
 * 还伞操作函数
 */
void Repay_Action(void) {

  app_timer_stop(TimerId_Lock);
  motorTick = 0;

  /*检查是否卡住*/
  if (IR_CHECK()) {
    Motor_staus = status_ir_stuck;
    DBG_LOG("Repay_Action IR Stuck.");
  } else {
    LED_ON(STATUS);
    MOTOR_BACK();
    app_timer_start(TimerId_Lock, APP_TIMER_TICKS(MOTOR_ACTION_TIME, APP_TIMER_PRESCALER), NULL);
  }
}

/**
 * 停止设备操作函数
 */
void Stop_Action(void) {
  LED_OFF(STATUS);
  motorTick = 0;
  app_timer_stop(TimerId_Lock);
  MOTOR_STOP();
}

/**
 * 网络指示灯开始闪烁
 * 
 * @param activetime 一个周期亮的时间
 * @param idletime   一个周期灭的时间
 * @param times      周期数
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
 * 状态指示灯开始闪烁
 * 
 * @param activetime 一个周期亮的时间
 * @param idletime   一个周期灭的时间
 * @param times      周期数
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
* 清看门狗.
*/
void WatchDog_Clear(void) {
#if WTD_EN == 1
  nrf_drv_wdt_feed();
#endif
}

/**
 * @brief 借伞操作结束函数.
 * @retval none.
 * @param p_context
 */
static void Motor_TimerCB(void* p_context) {
  /*延时检查是否过流*/
  if (motorTick > 10 && MOTOR_IS_STUCK()) {
    app_timer_stop(TimerId_Lock);
    Stop_Action();
    Motor_staus = status_motor_stuck;
    DBG_LOG("Motor is Stuck.");
  }
  /*检查红外*/
  if (IR_CHECK()) {
    app_timer_stop(TimerId_Lock);
    Stop_Action();
    Motor_staus = status_ir_stuck;
    DBG_LOG("Motor IR Stuck.");
  }
  /*延时检查到位*/
  if (motorTick > 10 && UM_OVER_CHECK()) {
    app_timer_stop(TimerId_Lock);
    Stop_Action();
    if (Motor_staus == status_borrow) {
      Motor_staus = status_borrow_complite;
    } else if (Motor_staus == status_repay) {
      Motor_staus = status_repay_complite;
    }
    DBG_LOG("Motor Running over.");
  }
  /*超时停止*/
  if (motorTick++ >= MOTOR_OVERFLOW_TIMES) {
    motorTick = 0;
    Stop_Action();
    Motor_staus = status_timeout;
    DBG_LOG("Motor Running Timeout.");
  }
}

/**
 * 网络指示灯定时器回调
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
 * 状态指示灯定时器回调
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
 * @brief 设备控制调试命令.
 * @param argc
 * @param argv
 */
static void funControl(int argc, char* argv[]) {

  argv++;
  argc--;

  if (ARGV_EQUAL("borrow")) {}

}


/************************ (C) COPYRIGHT  *****END OF FILE****/

