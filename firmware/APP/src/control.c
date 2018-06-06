/**
        ******************************************************************************
        * @file    Control.c
        * @author  宋阳
        * @version V1.0
        * @date    2015.12.4
        * @brief   控制操作相关函数
        ******************************************************************************
        */

/* Includes ------------------------------------------------------------------*/
#include "control.h"
#include "nrf_drv_wdt.h"
#include "user_uart.h"
#include "protocol.h"
/** @addtogroup firmwaveF2_BLE
        * @{
        */



/** @defgroup Control
        * @brief 控制操作相关函数
        * @{
        */


/* Private typedef -----------------------------------------------------------*/
/** @defgroup Control_Private_Types Control Private Types
        * @{
        */

/**
        * @}
        */

/* Private define ------------------------------------------------------------*/

/** @defgroup Control_Private_Constants Control Private Constants
        * @{
        */
#define TIME1_PATCH_REG           (*(uint32_t *)0x40009C0C)
#define TIME2_PATCH_REG           (*(uint32_t *)0x4000AC0C)



/**
        * @}
        */

/* Private macros ------------------------------------------------------------*/
/** @defgroup Control_Private_Macros Control Private Macros
        * @{
        */

/**
        * @}
        */
/* Private variables ---------------------------------------------------------*/
/** @defgroup Control_Private_Variables Private Variables
        * @{
        */
APP_TIMER_DEF(TimerId_Lock);
APP_TIMER_DEF(TimerId_RfidCmd);

static USDelay_CB timer2CB = NULL;
uint8_t  *data;
uint8_t RFID_SendCmd_Flag = 0,RFID_ReceiveData_Flag = 1;
uint16_t BatVol = 0;

/**
        * @}
        */
/* Private function prototypes -----------------------------------------------*/
/** @defgroup Control_Private_Functions Control Private Functions
        * @{
        */

static void Timer_Driver_init(void);
static void Motor_TimerCB(void* p_context);
static void RfidCmd_TimerCB(void *p_context);
static void funControl(int argc, char* argv[]);

/**
        * @}
        */

/* Exported functions ---------------------------------------------------------*/

/** @defgroup Control_Exported_Functions Control Exported Functions
        * @{
        */

/**
* @brief  控制轮循函数.
* @param  none.
* @retval none.
*/
void Control_Init(void) {


  nrf_drv_ppi_init();
  if (!nrf_drv_gpiote_is_init()) {
    nrf_drv_gpiote_init();
  }
  Timer_Driver_init();

  app_timer_create(&TimerId_Lock, APP_TIMER_MODE_SINGLE_SHOT, Motor_TimerCB);
  app_timer_create(&TimerId_RfidCmd, APP_TIMER_MODE_SINGLE_SHOT, RfidCmd_TimerCB);
//  app_timer_create(&TimerId_RfidCheck, APP_TIMER_MODE_SINGLE_SHOT, RFIDCheck_TimerCB);


  CMD_ENT_DEF(control, funControl);
  Cmd_AddEntrance(CMD_ENT(control));

  DBG_LOG("Device control init.");
}



/**
 * 用定时器实现us延时
 * 
 * @param us     延时的时间
 * @param cb     回调函数
 */
void Delay_Us(uint16_t us, USDelay_CB cb) {

  DELAY_TIMER->TASKS_STOP = 1;
  DELAY_TIMER->TASKS_CLEAR = 1;
  DELAY_TIMER->CC[0] = us;

  timer2CB = cb;
  DELAY_TIMER->TASKS_START = 1;
}

/**
 * 利用定时器等待接收RFID数据并发送APP数据
 */
void RfidSendCmd(void)
{
  app_timer_stop(TimerId_RfidCmd);
  RFID_SendCmd_Flag = 0;
  app_timer_start(TimerId_RfidCmd, APP_TIMER_TICKS(CMD_ACTION_TIME, APP_TIMER_PRESCALER), NULL);
}

/**
 * 借伞操作函数
 */
void Borrow_Action(uint8_t *dat) {
  app_timer_stop(TimerId_Lock);
  data = dat;
  RFID_ReceiveData_Flag = 0;
  MOTOR_FORWARD();
  (CommParam.UmbrellaCount)--;
  RfidSendCmd();
  app_timer_start(TimerId_Lock, APP_TIMER_TICKS(MOTOR_ACTION_TIME, APP_TIMER_PRESCALER), NULL);
}
/**
 * 还伞操作函数
 */
void Repay_Action(void)
{
  app_timer_stop(TimerId_Lock);

  MOTOR_BACK();
  (CommParam.UmbrellaCount)++;
  app_timer_start(TimerId_Lock, APP_TIMER_TICKS(MOTOR_ACTION_TIME, APP_TIMER_PRESCALER), NULL);
}
/**
 * 停止设备操作函数
 */
void Stop_Action(void)
{
  MOTOR_STOP();
}

/**
 * 蜂鸣器驱动定时器初始化，用于产生PWM波形
 */
static void Timer_Driver_init(void) {
  static nrf_ppi_channel_t ppi_buz1, ppi_buz2;

  BUZ_TIMER->TASKS_STOP = 1;
  TIME1_PATCH_REG = 0;
  BUZ_TIMER->SHORTS     = TIMER_SHORTS_COMPARE0_CLEAR_Msk;
  BUZ_TIMER->MODE       = TIMER_MODE_MODE_Timer;
  BUZ_TIMER->BITMODE    = (TIMER_BITMODE_BITMODE_16Bit << TIMER_BITMODE_BITMODE_Pos);
  BUZ_TIMER->PRESCALER  = (uint32_t)4; /*1M*/
  BUZ_TIMER->INTENSET   = 0;


//  nrf_gpiote_task_configure(0, BUZ_PIN, NRF_GPIOTE_POLARITY_TOGGLE, NRF_GPIOTE_INITIAL_VALUE_LOW);

  nrf_drv_ppi_channel_alloc(&ppi_buz1);
  nrf_drv_ppi_channel_alloc(&ppi_buz2);

  nrf_drv_ppi_channel_assign(
      ppi_buz1,
      (uint32_t)&(BUZ_TIMER->EVENTS_COMPARE[1]),
      (uint32_t)&(NRF_GPIOTE->TASKS_OUT[0]));
  nrf_drv_ppi_channel_assign(
      ppi_buz2,
      (uint32_t)&(BUZ_TIMER->EVENTS_COMPARE[0]),
      (uint32_t)&(NRF_GPIOTE->TASKS_OUT[0]));

  nrf_drv_ppi_channel_enable(ppi_buz1);
  nrf_drv_ppi_channel_enable(ppi_buz2);


  DELAY_TIMER->TASKS_STOP = 1;
  TIME2_PATCH_REG = 0;
  DELAY_TIMER->SHORTS     = TIMER_SHORTS_COMPARE0_CLEAR_Msk | TIMER_SHORTS_COMPARE0_STOP_Msk;
  DELAY_TIMER->MODE       = TIMER_MODE_MODE_Timer;
  DELAY_TIMER->BITMODE    = (TIMER_BITMODE_BITMODE_16Bit << TIMER_BITMODE_BITMODE_Pos);
  DELAY_TIMER->PRESCALER  = (uint32_t)4; /*1M*/
  DELAY_TIMER->INTENSET   = TIMER_INTENSET_COMPARE0_Msk;
  nrf_drv_common_irq_enable(TIMER2_IRQn, NRF_APP_PRIORITY_HIGH);
}

/**
 * Timer2的中断服务函数
 */
void TIMER2_IRQHandler(void) {
  DELAY_TIMER->EVENTS_COMPARE[0] = 0;
  if (timer2CB != NULL) {
    timer2CB();
    timer2CB = NULL;
  }
}

/**
        * @brief  借伞操作结束函数.
        * @retval none.
        */
static void Motor_TimerCB(void* p_context) {
  Stop_Action();
}

/**
 * 等待识别到RFID数据并返送APP雨伞ID数据
 */
static void RfidCmd_TimerCB(void *p_context){
  uint8_t param[9] = {0};
  
  if(RFID_SendCmd_Flag == 0){
    
    app_timer_start(TimerId_RfidCmd, APP_TIMER_TICKS(CMD_ACTION_TIME, APP_TIMER_PRESCALER), NULL);
  }
  else if(RFID_SendCmd_Flag == 1){
    app_timer_stop(TimerId_RfidCmd);
    param[0] = RFID_DATA[5];
    param[1] = RFID_DATA[6];
    param[2] = RFID_DATA[7];
    param[3] = RFID_DATA[8];
    param[4] =0x55;
    RFID_SendCmd_Flag = 0;
    RFID_ReceiveData_Flag = 1;
    ReBack(data,0x3B,param,5);
  }
}

/**
        * @brief  设备控制调试命令.
        */
static void funControl(int argc, char* argv[]) {
  
  	argv++;
	argc--;

	if (ARGV_EQUAL("borrow")) 
        {
          MOTOR_FORWARD();
          nrf_delay_ms(1000);
          nrf_delay_ms(1000);
          nrf_delay_ms(1000);
          MOTOR_STOP();
          DBG_LOG("MOTOR CONTROL.");
	}

}


/**
        * @}
        */



/**
        * @}
        */

/**
        * @}
        */

/************************ (C) COPYRIGHT  *****END OF FILE****/

