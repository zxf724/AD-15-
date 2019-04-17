/**
  ******************************************************************************
  * @file    main.c
  * @author  宋阳
  * @version V1.0
  * @date    2015.12.2
  * @brief   智能狗碗main函数.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "includes.h"
#include "control.h"

/** @addtogroup firmwave_F2_BLE
  * @{
  */



/** @defgroup MAIN
  * @brief 主函数文件
  * @{
  */

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
/** @defgroup main_Private_Variables Private Variables
  * @{
  */

/**
  * @}
  */

/* Private function prototypes -----------------------------------------------*/
/** @defgroup main_Private_Functions main Private Functions
  * @{
  */
static void WdtInit(uint32_t value);
static void FlashWRPProcess(void);



#if WTD_EN == 1
static void wdt_event_handler(void);
#endif

/**
  * @}
  */

/* Exported functions ---------------------------------------------------------*/

/** @defgroup main_Exported_Functions main Exported Functions
  *  @brief   main 外部函数
  * @{
  */

#include "nrf_uart.h"



/**
  * @brief  main函数.
  * @param  none.
  * @retval none.
  */
int main(void) {
    WdtInit(5000);
    FlashWRPProcess();
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
    BSPInit();
    UserUartInit(RX_PIN_NUMBER, TX_PIN_NUMBER, UART_BAUDRATE_BAUDRATE_Baud115200);
    CommandInit();
    RTCInit();
    UserBLEStart();
    WorkDataInit();
    UserBLEConnected();
    nrf_drv_rng_init(NULL);
    ProtocolInit();
    GPRSInit();
    ControlInit();
    MQTTConnInit();
    ProcessInit();
    DBG_LOG("System Start.");
    LED_OFF(STATUS);
    LED_OFF(NET);
    for (;;) {
        WatchDogClear();
        ProtocolDateProcPoll();
        CommandReceivePoll();
        ControlPolling();
        // GPRSPolling();
        // MQTTConnPolling();
        /* 进入休眠 */
        if (UserUartRecLength() == 0) {
            sd_app_evt_wait();
        }
    }
}

/**
  * @brief  错误抛出函数.
  * @param  error_codr: 错误代码.
  * @param  line_num: 错误发生的行数.
  * @param  p_file_name: 错误发生的文件名.
  * @retval none.
  */
uint32_t m_error_code;
uint32_t m_line_num;
const uint8_t* m_p_file_name;

void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info) {
    // On assert, the system can only recover with a reset.
    m_error_code = ((error_info_t*)info)->err_code;
    m_line_num = ((error_info_t*)info)->line_num;
    m_p_file_name = ((error_info_t*)info)->p_file_name;
    UNUSED_VARIABLE(m_error_code);
    UNUSED_VARIABLE(m_line_num);
    UNUSED_VARIABLE(m_p_file_name);
    DBG_LOG("APP Error handler: Code: 0x%X  line: %d file: %s", m_error_code, m_line_num, m_p_file_name);
    nrf_delay_ms(100);
    DBG_LOG("APP Error handler: Code: 0x%X  line: %d file: %s", m_error_code, m_line_num, m_p_file_name);
    nrf_delay_ms(100);
    NVIC_SystemReset();
    __disable_irq();
    while (1);
}

/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t* p_file_name) {
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**
  * @}
  */

/** @addtogroup main_Private_Functions
  * @{
  */
/**
  * @brief  看门狗初始化.
  * @param  value: 看门狗溢出时间.
  * @retval none.
  */
static void WdtInit(uint32_t value) {
#if WTD_EN == 1
    static nrf_drv_wdt_channel_id m_channel_id;
    nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
    if (value > 0) config.reload_value = value;
    nrf_drv_WdtInit(&config, wdt_event_handler);
    nrf_drv_wdt_channel_alloc(&m_channel_id);
    nrf_drv_wdt_enable();
    nrf_drv_wdt_feed();
#endif
}

/**
  * @brief  FLASH读写保护初始化.
  * @param  none.
  * @retval none.
  */
static void FlashWRPProcess(void) {
#if FLASH_WRP == 1
    if (NRF_UICR->RBPCONF != 0) {
        nrf_nvmc_write_word((uint32_t)(&NRF_UICR->RBPCONF), 0);
        user_BLE_Disconnected();
        NVIC_SystemReset();
    }
#endif
}

/**
  * @brief  看门狗溢出中断处理.
  * @param  none.
  * @retval none.
  */
#if WTD_EN == 1

static void wdt_event_handler(void) {
}
#endif

/**test main motor
  * @}
  */
void TestMainMotor(void) {
   static uint8_t gs_step = 0, flag_motor1 = 0;
   while(1) {
     if(flag_motor1 == 0) {
          MOTOR_FORWARD(1);
          flag_motor1 = 1;
        }
        gs_step++;
        DBG_LOG("gs_step = %d", gs_step);
        DBG_LOG("flag_motor1 = %d", flag_motor1);
        if((IF_IS_TOUCH(7) == 0) && (gs_step >= 100)) {
          DBG_LOG("hello,world!");
          MOTOR_STOP(1);
        }
        if(IF_IS_TOUCH(7) == 0){
          DBG_LOG("here!!");
        }
    }
}
      

/**test switch motor
  * @}
  */
void TestSwitchMotor(void) {
    static uint8_t gs_flag_motor4 = 0, gs_flag_if_is_touch=0;
    while(1) {
      if(gs_flag_motor4 == 0) {
        MOTOR_BACK(4);
        gs_flag_motor4 = 1;
        gs_flag_if_is_touch = 1;
        DBG_LOG("test!");
      }
      if(IF_IS_TOUCH(3) == 0) {
        MOTOR_STOP(4);
        MOTOR_FORWARD(4);
        gs_flag_if_is_touch = 0;
        DBG_LOG("test in if_is_touch(3)");
      }
      if((IF_IS_TOUCH(5) == 0) && (gs_flag_if_is_touch == 0)) {
        MOTOR_STOP(4);
        gs_flag_motor4 = 0;
      }
    }
}

/**test push motor
  * @}
  */
void TestPushMotor(void) {
    static uint8_t gs_flag_motor2 = 0, gs_flag_if_is_touch = 0;
    while(1) {
      if(gs_flag_motor2 == 0) {
        MOTOR_FORWARD(2);
        gs_flag_motor2 = 1;
        gs_flag_if_is_touch = 1;
      }
      if(IF_IS_TOUCH(1) == 0) {
        MOTOR_BACK(2);
        gs_flag_if_is_touch = 0;
      }
      if((IF_IS_TOUCH(2) == 0) && (gs_flag_if_is_touch == 0)) {
        MOTOR_STOP(2);
        gs_flag_motor2 = 0;
      }
    }
}

/**test infrared sensor
  * @}
  */
void TestInfraredSensor(void) {
  while(1){
    IO_H(IR_SW);
    if(IR_CHECK() == 0) {   // output 1 when it cover
        DBG_LOG("here is in the infrared sensor");
        DBG_LOG("here is the ");
    }
    if(IR_CHECK() == 1){
      DBG_LOG("done!!");
    }
  }
}

/**test breakdown motor
  * @}
  */
void TestBreakDownMotor(void) {
    static uint8_t flag_motor3 = 0, gs_flag_if_is_touch = 0;
    while(1) {
      if(flag_motor3 == 0) {
        MOTOR_FORWARD(3);
        flag_motor3 = 1;
        gs_flag_if_is_touch = 1;
      }
      if(IF_IS_TOUCH(4) == 0) {
        MOTOR_STOP(3);
        MOTOR_BACK(3);
        gs_flag_if_is_touch = 0;
      }
      if((IF_IS_TOUCH(6) == 0) && (gs_flag_if_is_touch == 0)) {
        MOTOR_STOP(3);
        flag_motor3 = 0;
      } 
    }
}

/** test rfid
  * @}
  */
void TestRFID(void) {
    static uint32_t  gs_RFID_Read = 0;
    while(1) {
      gs_RFID_Read = GPRS_ReadRFID(2);
      DBG_LOG("gs_RFID_Read = %u", gs_RFID_Read);
    }
}

/** test if_is_touch();
  * @}
  */
void IfIsTouch(void){
  while(1) {
    if(IF_IS_TOUCH(7) == 0) {
      DBG_LOG("touch!!");
    }
  }
}

/**reset function
  * @}
  */
void Reset(void) {
    /*reset push unberally motor -- done*/
    static uint8_t gs_flag_motor2 = 0, flag_motor5 = 0;
    static uint8_t flag_motor1 = 0;
    if(gs_flag_motor2 == 0) {
        MOTOR_BACK(2);
        gs_flag_motor2 = 1;
    }
    if((IF_IS_TOUCH(2) == 0 ) && (gs_flag_motor2 == 1)) {
        MOTOR_STOP(2);
    }
    /*reset switch door motor -- done*/
    if(flag_motor5 == 0) {
        MOTOR_FORWARD(4);
        flag_motor5 = 1;
    }
    if(IF_IS_TOUCH(5) == 0) {
        MOTOR_STOP(4);
    }
    /*reset fault umberlla motor*/
    static uint8_t flag_motor3 = 0;
    if(flag_motor3 == 0) {
        MOTOR_BACK(3);
        flag_motor3 = 1;
    }
    if(IF_IS_TOUCH(6) == 0) {
        MOTOR_STOP(3);
    }
    /*reset main motor*/
    if(flag_motor1 == 0) {
        MOTOR_FORWARD(1);
        flag_motor1 = 1;
    }
    if((IF_IS_TOUCH(7) == 0) && (flag_motor1 == 1)) {
        MOTOR_STOP(1);
    }
}

/**
  * @}
  */

/************************ (C) COPYRIGHT  *****END OF FILE****/


