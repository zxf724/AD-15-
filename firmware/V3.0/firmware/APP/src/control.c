/* Includes ------------------------------------------------------------------*/
#include "includes.h"



/* Private typedef -----------------------------------------------------------*/
typedef struct {
    uint16_t activetime;
    uint16_t idletime;
    uint16_t times;
    uint16_t index;
    BOOL active;
} Period_Block_t;


/* Private define ------------------------------------------------------------*/
/*协议命令*/
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
APP_TIMER_DEF(TimerId_Rap);
APP_TIMER_DEF(TimerId_Move);
APP_TIMER_DEF(TimerId_LED_NET);
APP_TIMER_DEF(TimerId_LED_STATUS);
APP_TIMER_DEF(TimerId_TTS);
APP_TIMER_DEF(TimerId_Borrow);
APP_TIMER_DEF(TimerId_Repay);
APP_TIMER_DEF(TimerId_In_Repay);
APP_TIMER_DEF(TimerId_BreakDown);
APP_TIMER_DEF(TimerId_RFID);

static uint8_t TTS_Step = 0, Direction = 0, move_status = 0, RFID_flag = 0,
               move_step = 0, timeout_status = 0, IR_Status = 0, borrow_flag = 0;
// a series switch flag, at funtion InitFlag() would be initialize all of the flag
static uint8_t flag_motor4 = 0, flag_if_is_have_unber = 0, flag_if_is_have_unb = 0,
               flag_RFID_GPRS_Read = 0, flag_IR_CHECK = 0, flag_rfid = 0, flag_is_have = 0, flag_motor2 = 0, flag_solve_motor = 0,
               flag_if_is_touch = 0, flag_IR_SW = 0;
static char* TTS_Text = NULL;
static uint16_t motorTick = 0;
static uint32_t  RFID_Read = 0;
static Period_Block_t  LED_NET_Period, LED_STATUS_Period;

/* Private function prototypes -----------------------------------------------*/
static void Motor_TimerCB(void* p_context);
static void LED_NET_TimerCB(void* p_context);
static void LED_STATUS_TimerCB(void* p_context);
static void TTS_TimerCB(void* p_context);
static void Move_TimerCB(void * p_context);
static void RFID_SendCmd(uint8_t addr, uint8_t cmd, uint8_t* data, uint8_t datalen);
static void funControl(int argc, char* argv[]);
static void Reback_Action(void);
static void Reforward_Action(void);
static void RepayInAction(void*);
static void BreakdownInRepay(void* p_context);
static void TimerIdInRepay(void* p_context);
static void TimerIdRFID(void* p_context);


/* Exported functions ---------------------------------------------------------*/

/**
* @brief  控制轮循函数.
* @param  none.
* @retval none.
*/
void Control_Init(void) {
    //borrow
    app_timer_create(&TimerId_Lock, APP_TIMER_MODE_REPEATED, Motor_TimerCB);
    app_timer_create(&TimerId_RFID, APP_TIMER_MODE_REPEATED, TimerIdRFID);
    app_timer_create(&TimerId_Move, APP_TIMER_MODE_REPEATED, Move_TimerCB);
    //repay
    app_timer_create(&TimerId_Repay, APP_TIMER_MODE_REPEATED, RepayInAction);
    app_timer_create(&TimerId_In_Repay, APP_TIMER_MODE_REPEATED, TimerIdInRepay);
    //repay browndown
    app_timer_create(&TimerId_BreakDown, APP_TIMER_MODE_REPEATED, BreakdownInRepay);
    //others
    app_timer_create(&TimerId_LED_NET, APP_TIMER_MODE_SINGLE_SHOT, LED_NET_TimerCB);
    app_timer_create(&TimerId_LED_STATUS, APP_TIMER_MODE_SINGLE_SHOT, LED_STATUS_TimerCB);
    app_timer_create(&TimerId_TTS, APP_TIMER_MODE_SINGLE_SHOT, TTS_TimerCB);
    CMD_ENT_DEF(control, funControl);
    Cmd_AddEntrance(CMD_ENT(control));
    DBG_LOG("Device control init.");
}

/**
 * 控制轮询函数
 */
void Control_Polling(void) {
    uint8_t led_n;
    static uint8_t led_net;
    static motor_status_t status;
    /*循环读卡*/
    uint8_t* pdata = NULL;
    if(flag_IR_SW == 1) {
        IO_H(IR_SW);  //enable the sensor
    } else if(flag_IR_SW == 0) {
        IO_L(IR_SW);
    }
    if(flag_RFID_GPRS_Read == 1) {
        RFID_Read = GPRS_ReadRFID(2);
        DBG_LOG("RFID_Read = %u", RFID_Read);
    }
    /*TTS播报*/
    if (TTS_Step == 1) {
        TTS_Step = 2;
        app_timer_start(TimerId_TTS, APP_TIMER_TICKS(1000 + strlen(TTS_Text) * 200, APP_TIMER_PRESCALER), NULL);
        GPRS_TTS(TTS_Text);
    }
    /*LED闪烁*/
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
    /*取还伞管理*/
    if (Motor_staus != status) {
        status = Motor_staus;
        switch (Motor_staus) {
            //output unbrella
            case status_start_output_unbrella:
                DBG_LOG("RAM: StartOutputUnbrella.mp3");
                TTS_Play("RAM:StartOutputUnbrella.mp3");
                Motor_staus = status_idle;
                break;
            case status_output_unbrella_success:
                Motor_staus = status_idle;
                if (0 == RFID_Read) {
                    TTS_Play("RAM:OutputUnbrellaSuccess.mp3");
                    WorkData.StockCount--;
                    WorkData_Update();
                    Protocol_Report_Umbrella_Borrow(RFID_Read, status);
                    Report_Umbrella_Borrow_Status(RFID_Read, status);
                    borrow_flag = 0;
                }
                break;
            case status_take_the_unbrella_soon:
                TTS_Play("RAM:TakeTheUnbrellaSoon.mp3");
                Motor_staus = status_idle;
                break;
            case status_have_no_unbrella:
                TTS_Play("RAM:HaveNoUnbrella.mp3");
                Motor_staus = status_idle;
                break;
            //input unbrella
            case status_start_input_unbrella:
                TTS_Play("RAM:StartInputUnbrella.mp3");
                Motor_staus = status_idle;
                break;
            case status_input_unbrella_success:
                TTS_Play("RAM:InputUnbrellaSuccess.mp3");
                Motor_staus = status_idle;
                if (0 != RFID_Read) {
                    WorkData.StockCount++;
                    DBG_LOG("cJSON repay umbrella status");
                    WorkData_Update();
                    if (Report_Umbrella_Repy_Status(RFID_Read, status, RTC_ReadCount()) == FALSE) {
                        /*还伞上报失败，存储记录*/
                        Write_StoreLog(RFID_Read);
                    }
                }
                break;
            case status_do_not_occlusion_door:
                TTS_Play("RAM:DoNotOcclusionDoor.mp3");
                Motor_staus = status_idle;
                break;
            case status_input_unbrella_soon:
                TTS_Play("RAM:InputUnbrellaSoon.mp3");
                Motor_staus = status_idle;
                break;
            case status_full_unbrella:
                TTS_Play("RAM:FullUnbrella.mp3");
                Motor_staus = status_idle;
                break;
            //input breakdown unbrella
            case status_input_breakdown_unbrella:
                TTS_Play("RAM:InputBreakDownUnbrella.mp3");
                Motor_staus = status_idle;
                break;
            case status_restart_ouput:
                TTS_Play("RAM:RestartOuput.mp3");
                Motor_staus = status_idle;
                break;
            case status_report_breakdown:
                TTS_Play("RAM:ReportBreakDown.mp3");
                Motor_staus = status_idle;
                break;
            //others
            // case status_motor_stuck:
            //     TTS_Play("RAM:stuck.mp3");
            //     Motor_staus = status_idle;
            //     if (Direction == 1) {
            //         Protocol_Report_Umbrella_Borrow(0, status);
            //         Report_Umbrella_Borrow_Status(0, status);
            //     } else if (Direction == 2) {
            //         Report_Umbrella_Repy_Status(0, status, RTC_ReadCount());
            //     }
            //     break;
            // case status_ir_stuck:
            //     Motor_staus = status_idle;
            //     if (Direction == 1) {
            //         if(IR_Status == 1) {
            //             TTS_Play("RAM:exitfaultrestart.mp3");
            //         } else if(IR_Status == 2) {
            //             nrf_delay_ms(1000);
            //             TTS_Play("RAM:fullexchange.mp3");//缺少
            //             nrf_delay_ms(2000);
            //             Reback_Action();
            //         }
            //         IR_Status = 0;
            //         Protocol_Report_Umbrella_Borrow(0, status);
            //         Report_Umbrella_Borrow_Status(0, status);
            //     } else if (Direction == 2) {
            //         if(IR_Status == 1) {
            //             TTS_Play("RAM:rein.mp3");
            //         } else if(IR_Status == 2) {
            //             nrf_delay_ms(1000);
            //             TTS_Play("RAM:rein.mp3");
            //             nrf_delay_ms(2000);
            //             Reforward_Action();
            //         }
            //         IR_Status = 0;
            //         Report_Umbrella_Repy_Status(0, status, RTC_ReadCount());
            //     }
            //     break;
            // case status_timeout:
            //     TTS_Play("RAM:fault.mp3");
            //     Motor_staus = status_idle;
            //     if (Direction == 1) {
            //         Protocol_Report_Umbrella_Borrow(0, status);
            //         Report_Umbrella_Borrow_Status(0, status);
            //     } else if (Direction == 2) {
            //         Report_Umbrella_Repy_Status(0, status, RTC_ReadCount());
            //     }
            //     break;
            // case status_empty:
            //     TTS_Play("RAM:noneexchange.mp3");
            //     Motor_staus = status_idle;
            //     Protocol_Report_Umbrella_Borrow(0, status);
            //     Report_Umbrella_Borrow_Status(0, status);
            //     break;
            // case status_full:
            //     TTS_Play("RAM:fullexchange.mp3");
            //     Motor_staus = status_idle;
            //     Report_Umbrella_Repy_Status(0, status, RTC_ReadCount());
            //     break;
            // case status_repay_breakdown_complite:
            //     TTS_Play("RAM:fullexchange.mp3"); //缺少
            //     Motor_staus = status_idle;
            //     //Report_Umbrella_Borrow_Status(0, status, RTC_ReadCount());
            //     break;
            default:
                break;
        }
    }
    /*补传还伞记录*/
    if (WorkData.StoreLog_Report != WorkData.StoreLog_In && MQTT_IsConnected()) {
        motor_status_t sta;
        uint16_t index = WorkData.StoreLog_Report + 1;
        if (index >= LOG_STORE_MAX) {
            index = 0;
        }
        DBG_LOG("Report Histroy:%u.", index);
        sta = status_input_unbrella_success;
        if (Report_Umbrella_Repy_Status(STROE_LOG_POINT(index)->rfid, sta, STROE_LOG_POINT(index)->time)) {
            WorkData.StoreLog_Report = index;
            WorkData_Update();
        }
    }
}

/**
 * 借伞操作函数
 */
void Borrow_Action(void) {
    app_timer_stop(TimerId_Lock);
    flag_IR_SW = 1;
    motorTick = 0;
    nrf_delay_ms(80);
    Direction = 1;
    flag_RFID_GPRS_Read = 1;
    WorkData.StockCount = 3;        //单独测试
    if (WorkData.StockCount == 0) {
        LED_MOTOR_NG();
        Motor_staus = status_have_no_unbrella;
        DBG_LOG("Borrow_Action Empty.");
    }
    /*红外检测*/
    else if (IR_CHECK() == 0) {
        LED_IR_OVER_FLASH();
        IR_Status = 1;
        Motor_staus = status_ir_stuck;
        DBG_LOG("Borrow_Action IR Stuck.");
    } else {
        DBG_LOG("Borrow_Action NOT stuck!");
        // Motor_staus = status_borrow;
        borrow_flag = 2;
        LED_ON(STATUS);
        app_timer_start(TimerId_Lock, APP_TIMER_TICKS(MOTOR_ACTION_TIME, APP_TIMER_PRESCALER), NULL);
    }
}

static void Reback_Action(void) {
    Direction = 2;
    MOTOR_BACK(1);
    app_timer_start(TimerId_Lock, APP_TIMER_TICKS(MOTOR_ACTION_TIME, APP_TIMER_PRESCALER), NULL);
}
static void Reforward_Action(void) {
    Direction = 1;
    MOTOR_FORWARD(1);
    app_timer_start(TimerId_Lock, APP_TIMER_TICKS(MOTOR_ACTION_TIME, APP_TIMER_PRESCALER), NULL);
}

/**
 * 还伞操作函数
 */
void Repay_Action(void) {
    flag_RFID_GPRS_Read = 1; // open RFID read!
    app_timer_stop(TimerId_Lock);
    flag_IR_SW = 1;
    motorTick = 0;
    Direction = 2;
    nrf_delay_ms(80);
    WorkData.StockCount = 9; //test
    if (WorkData.StockCount >= WorkData.StockMax) {
        LED_MOTOR_NG();
        Motor_staus = status_full_unbrella;
        DBG_LOG("Repay_Action Full.");
    }
    /*检查是否卡住*/
    else if (IR_CHECK() == 0) {
        LED_IR_OVER_FLASH();
        IR_Status = 1;
        Motor_staus = status_ir_stuck;
        DBG_LOG("Repay_Action IR Stuck.");
    } else {
        Motor_staus = status_start_input_unbrella;
        LED_ON(STATUS);
        DBG_LOG("Repay_Action");
        app_timer_start(TimerId_Repay, APP_TIMER_TICKS(MOVE_ACTION_TIME, APP_TIMER_PRESCALER), NULL);
    }
}

void Move_Forward_Action(void) {
    DBG_LOG("Move forward action");
    app_timer_start(TimerId_Move, APP_TIMER_TICKS(MOVE_ACTION_TIME, APP_TIMER_PRESCALER), NULL);
}
void Move_Back_Action(void) {
    DBG_LOG("Move back action");
    app_timer_start(TimerId_In_Repay, APP_TIMER_TICKS(MOVE_ACTION_TIME, APP_TIMER_PRESCALER), NULL);
}


/**
 * 停止设备操作函数
 */
void Stop_Action(uint8_t num) {
    LED_OFF(STATUS);
    motorTick = 0;
    //app_timer_stop(TimerId_Lock);
    MOTOR_STOP(num);
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
 * 播报TS
 *
 * @param text   待播报的文本
 */
void TTS_Play(char* text) {
    app_timer_stop(TimerId_TTS);
    PA_ENABLE();
    TTS_Step = 0;
    TTS_Text = text;
    app_timer_start(TimerId_TTS, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), NULL);
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

static void Move_TimerCB(void * p_context) {
    static uint8_t i = 0, j = 0, step = 0;
    static uint8_t  flag_motor5 = 0;
    if(1) {   //bluetooth condition add  todo
        if(flag_motor2 == 0) {
            MOTOR_FORWARD(2);
            flag_motor2 = 1;
            flag_if_is_touch = 1;
        }
        if((IF_IS_TOUCH(1) == 0) && (flag_if_is_touch == 1)) {
            MOTOR_STOP(2);
            flag_IR_CHECK = 1;
            //检测借伞成功
            app_timer_start(TimerId_RFID, APP_TIMER_TICKS(MOVE_ACTION_TIME, APP_TIMER_PRESCALER), NULL);
        }
        step++;
        // DBG_LOG("step = %d,i = %d,j = %d", step, i, j);
        if(step >= 100) {
            i++;
            step = 0;
        }
        if(i >= 1) {
            i = 0;
            WatchDog_Clear();
            DBG_LOG("请尽快取伞");
            j++;
            Motor_staus = status_take_the_unbrella_soon;
        }
        if(j >= 3) {
            DBG_LOG("出伞失败，没有及时取伞");
            MOTOR_BACK(2);
            flag_if_is_touch = 0;
            flag_motor5 = 1;
            flag_RFID_GPRS_Read = 0;
            //关闭开关门
            if(flag_motor5 == 1) {
                MOTOR_FORWARD(4);
                flag_motor5 = 0;
            }
            if(IF_IS_TOUCH(5) == 0) {
                MOTOR_STOP(4);
            }
            if((IF_IS_TOUCH(2) == 0) && (flag_if_is_touch == 0)) {
                MOTOR_STOP(2);
                app_timer_stop(TimerId_Move);
                app_timer_stop(TimerId_RFID);
            }
        }
    }
}
/**
 * @brief 借伞--定时器回调函数
 * @retval none.
 * @param p_context
 */
static void Motor_TimerCB(void* p_context) {
    /*打开开关门电机*/
    if(flag_motor4 == 0) {
        MOTOR_BACK(4);
        flag_motor4 = 1;
    }
    if(IF_IS_TOUCH(3) == 0) {
        MOTOR_STOP(4);
    }
    //判断是否有伞
    if((RFID_Read == 0) && (flag_if_is_have_unber == 0)) {
        MOTOR_FORWARD(1);
        flag_if_is_have_unber = 1;
    }
    if ((flag_if_is_have_unber == 1) && (RFID_flag > 0)) {
        MOTOR_STOP(1);
    }
    /*延时检查是否过流*/
    if (motorTick > 10 && MOTOR_IS_STUCK()) {
        LED_MOTOR_OVER_FLASH();
        app_timer_stop(TimerId_Lock);
        Stop_Action(1);
        Motor_staus = status_motor_stuck;
        DBG_LOG("Motor is Stuck.");
    }
    /*检查红外*/
    if (IR_CHECK() == 0) {
        LED_IR_OVER_FLASH();
        Stop_Action(1);
        IR_Status = 2;
        Motor_staus = status_ir_stuck;
        DBG_LOG("Motor IR Stuck.");
    }
    /*延时检查到位*/
    if ((motorTick > 100) && (IF_IS_TOUCH(7) == 0) && (RFID_Read > 0)) {
        motorTick = 0;
        Stop_Action(1);
        // if (Motor_staus == status_start_output_unbrella) {
        if (RFID_Read > 0) {
            Move_Forward_Action();
            DBG_LOG("In Move_Forward_Action()");
        }
        // }
    }
    /*超时停止*/
    if (motorTick++ >= MOTOR_OVERFLOW_TIMES) {
        motorTick = 0;
        Stop_Action(1);
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
 * TTS定时器回调函数
 */
static void TTS_TimerCB(void* p_context) {
    if (TTS_Step == 0) {
        TTS_Step = 1;
    } else if (TTS_Step == 2) {
        TTS_Step = 0;
        PA_DISABLE();
    }
}
/**
 * 发送命令
 *
 * @param addr    RFID从机地址
 * @param cmd     命令
 * @param data    附加数据
 * @param datalen 附加数据长度
 */
static void RFID_SendCmd(uint8_t addr, uint8_t cmd, uint8_t* data, uint8_t datalen) {
    uint8_t buf[32], len = 5;
    buf[0] = 0x7E;
    buf[1] = 0x1B;
    buf[2] = addr;
    buf[3] = cmd;
    buf[4] = datalen;
    if (data != NULL && datalen > 0) {
        memcpy(&buf[5], data, datalen);
        len = len + datalen;
    }
    buf[5 + datalen] = AddCheck(buf, len);
    len += 1;
    RFID_SEND(buf, len);
}
/**
 * RFID读设备应答
 *
 * @param addr   读的设备地址
 * @param data   读出的附加数据内容
 * @return 返回反馈的命令
 */
uint8_t RFID_ReadPoll(uint8_t addr, uint8_t** data) {
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
        for(uint8_t i = 0; i < len; i++) {
            DBG_LOG("data[%d]:0x%02x", i, buf[i]);
        }
        while (*p != 0x7E) {
            p++;
            len--;
        }
        if (*p == 0x7E && *(p + 1) == 0x1B
                && (*(p + 2) == 0xA7 || addr == 0)) {  //&& AddCheck(p, len - 1) == p[len - 1]
            ret = *(p + 3);
            *data = p + 5;
        }
    }
    return ret;
}

/**
 * 定时器回调函数--还伞
 *
 * @param addr   读的设备地址
 * @param data   读出的附加数据内容
 * @return 返回反馈的命令
 */
static void RepayInAction(void *a) {
    static uint8_t i, flag_is_have = 0;
    static uint8_t flag_motor41 = 0;
    static uint16_t motorTick1 = 0;
    Direction = 1;
    app_timer_stop(TimerId_RFID);
    app_timer_stop(TimerId_Lock);
    WorkData.StockCount = 3;        //单独测试
    if (WorkData.StockCount == 0) {
        LED_MOTOR_NG();
        Motor_staus = status_have_no_unbrella;
        DBG_LOG("Borrow_Action Empty.");
    }
    /*检查伞桶内是否有伞*/
    if((RFID_Read > 0) && (flag_rfid == 0)) {
        MOTOR_BACK(1);
        flag_rfid = 1;
        flag_is_have = 1;
    }
    if((motorTick1 > 10) && (IF_IS_TOUCH(7) == 0) && (RFID_Read == 0) && (flag_is_have == 1)) {
        MOTOR_STOP(1);
        flag_rfid = 1;
        flag_if_is_have_unb = 1;
        flag_solve_motor = 1;
    }
    if(RFID_Read == 0) {
        flag_rfid = 1;
        flag_if_is_have_unb = 1;
    }
    /*打开开关门电机*/
    if(flag_if_is_have_unb == 1) {
        Motor_staus = status_start_input_unbrella;
        if(flag_motor4 == 0) {
            MOTOR_BACK(4);
            flag_motor4 = 1;
        }
        if((IF_IS_TOUCH(3) == 0) && (flag_motor41 == 0)) {
            DBG_LOG("关闭开关门电机");
            MOTOR_STOP(4);
            flag_motor41 = 1;
        }
        /*延时检查是否过流*/
        if (motorTick1 > 10 && MOTOR_IS_STUCK()) {
            LED_MOTOR_OVER_FLASH();
            app_timer_stop(TimerId_Lock);
            Stop_Action(1);
            Motor_staus = status_motor_stuck;
            DBG_LOG("Motor is Stuck.");
        }
        if ((motorTick1 > 50) && (IF_IS_TOUCH(7) == 0)) {
            motorTick1 = 0;
            app_timer_stop(TimerId_Lock);
            if (Motor_staus == status_start_input_unbrella) {
                DBG_LOG("Motor Running forware.");
                Move_Back_Action();
                flag_motor41 = 0;
            }
        }
    }
    /*超时停止*/
    if (motorTick1++ >= MOTOR_OVERFLOW_TIMES) {
        motorTick1 = 0;
        Stop_Action(1);
        Motor_staus = status_timeout;
        DBG_LOG("Motor Running Timeout.");
    }
}

void Breakdown_Repay(void) {
    //检测系列
    MOTOR_FORWARD(3);
    flag_IR_SW = 1;
    flag_RFID_GPRS_Read = 1;
    //TTS_Play("请将伞折叠好后伞头向里放入还伞口");
    DBG_LOG("请将伞折叠好后伞头向里放入还伞口");
    app_timer_start(TimerId_BreakDown, APP_TIMER_TICKS(MOVE_ACTION_TIME, APP_TIMER_PRESCALER), NULL);
}

/**
 * @brief 回收伞回调函数.
 * @param argc
 * @param argv
 */
static void TimerIdInRepay(void* p_context) {
    static uint8_t i = 0, j = 0, step = 0, flag_already = 0;
    static uint8_t flag_motor5 = 0, step1 = 0;
    if(1) {   //后面要添加条件
        step++;
        step1++;
        DBG_LOG("step1 = %d", step1);
        DBG_LOG("step = %d,i = %d,j = %d", step, i, j);
        if(step >= 100) {
            i++;
            step = 0;
        }
        if(i >= 1) {
            i = 0;
            WatchDog_Clear();
            DBG_LOG("请尽快还伞");
            j++;
        }
        if(j >= 3) {
            DBG_LOG("还伞失败，没有及时还伞，关闭电机");
            j = 0;
            //关闭开关门电机
            if(flag_already == 0) {
                MOTOR_FORWARD(4);
                flag_already = 1;
            }
            if(IF_IS_TOUCH(5) == 0) {
                MOTOR_STOP(4);
                InitFlag();
                app_timer_stop(TimerId_Repay);
                app_timer_stop(TimerId_In_Repay);
            }
        }
        if((RFID_Read > 0) && (IR_CHECK() == 1) && (step1 >= 50)) {
            if(flag_already == 0) {
                MOTOR_FORWARD(4);
                flag_already = 1;
            }
        }
        if((IF_IS_TOUCH(5) == 0) && (flag_already == 1)) {
            MOTOR_STOP(4);
            Motor_staus = status_input_unbrella_success;
            DBG_LOG("close all the timeid");
            //init all the flag
            InitFlag();
            step = 0;
            i = 0;
            j = 0;
            flag_already = 0;
            step1 = 0;
            //stop timer
            app_timer_stop(TimerId_Repay);
            app_timer_stop(TimerId_In_Repay);
        }
    }
}

/**
 * @brief 初始化flag
 * @param argc
 * @param argv
 */
void InitFlag(void) {
    DBG_LOG("at the InitFlag()");
    flag_RFID_GPRS_Read = 0;
    flag_motor4 = 0;
    flag_rfid = 0;
    flag_is_have = 0;
    flag_if_is_have_unb = 0;
    flag_if_is_have_unber = 0;
    flag_solve_motor = 0;
    flag_motor2 = 0;
    flag_if_is_touch = 0;
    flag_IR_SW = 0;
}

/**
 * @brief 故障伞回收回调函数.
 * @param argc
 * @param argv
 */
static void BreakdownInRepay(void* p_context) {
    static uint8_t flag_motor3 = 0, i = 0, j = 0, flag_time = 0,
                   flag_switch = 0;
    if(IF_IS_TOUCH(4) == 0) {
        MOTOR_STOP(3);
        flag_motor3 = 1;
    }
    i++;
    DBG_LOG("i = %d", i);
    if(RFID_Read > 0) {
        flag_switch = 1;
        i = 0;
    }
    if(i >= 100) {
        DBG_LOG("请尽快还伞");
        i = 0;
        j++;
        DBG_LOG("j = %d", j);
    }
    if((j >= 3) && (flag_time == 0)) {
        DBG_LOG("十五秒到，故障，没有还伞");
        MOTOR_BACK(3);
        app_timer_stop(TimerId_BreakDown);
        flag_motor3 = 0;
        flag_time = 0;
        flag_switch = 0;
        flag_RFID_GPRS_Read = 0;
    }
    if((flag_switch == 1) && (i >= 20)) {
        flag_time = 1;
        DBG_LOG("还故障伞成功");
        MOTOR_BACK(3);
    }
    if((IF_IS_TOUCH(6) == 0) && (flag_motor3 == 1)) {
        MOTOR_STOP(3);
        Motor_staus = status_restart_ouput;
        flag_RFID_GPRS_Read = 0;
        flag_motor3 = 0;
        flag_time = 0;
        flag_switch = 0;
        app_timer_stop(TimerId_BreakDown);
    }
}

/**
 * @brief 检测还伞成功
 * @param argc
 * @param argv
 */
static void TimerIdRFID(void* p_context) {
    //红外检测
    if((IR_CHECK() == 1) && (flag_IR_CHECK == 1) && (RFID_Read == 0)) {
        // if(0) {
        // nrf_delay_ms(2000);
        flag_RFID_GPRS_Read = 0;
        app_timer_stop(TimerId_Move);
        app_timer_stop(TimerId_Lock);
        DBG_LOG("取伞成功");
        Motor_staus = status_output_unbrella_success;
        //关闭开关门
        MOTOR_FORWARD(4);
        if(IF_IS_TOUCH(5) == 0) {
            MOTOR_STOP(4);
            flag_if_is_touch = 1;
            //关闭推伞电机
            MOTOR_BACK(2);
        }
        if((IF_IS_TOUCH(2) == 0) && (flag_if_is_touch == 1)) {
            MOTOR_STOP(2);
            InitFlag();
            app_timer_stop(TimerId_RFID);
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
        MOTOR_FORWARD(1);
        DBG_LOG("Motor forward.");
    } else if (ARGV_EQUAL("back")) {
        MOTOR_BACK(1);
        DBG_LOG("Motor back.");
    } else if (ARGV_EQUAL("stop")) {
        MOTOR_STOP(1);
        DBG_LOG("Motor stop.");
    } else if (ARGV_EQUAL("forward_action")) {
        Move_Forward_Action();
        DBG_LOG("forward action.");
    } else if (ARGV_EQUAL("back_action")) {
        Move_Back_Action();
        DBG_LOG("back action.");
    } else if (ARGV_EQUAL("rfid_cmd")) {
        RFID_SendCmd(0, 0x02, NULL, 0);
        DBG_LOG("rfid send cmd.");
    } else if (ARGV_EQUAL("test_forware_action")) {
        Repay_Action();
        DBG_LOG("test forware action.");
    } else if (ARGV_EQUAL("test_back_actionn")) {
        Borrow_Action();
        DBG_LOG("test back action.");
    } else if (ARGV_EQUAL("motor2_forware")) {
        MOTOR_FORWARD(1);
        DBG_LOG("motor2 forware action.");
    } else if (ARGV_EQUAL("motor2_back")) {
        MOTOR_BACK(1);
        DBG_LOG("motor2 back action.");
    } else if (ARGV_EQUAL("motor2_stop")) {
        MOTOR_STOP(1);
        DBG_LOG("motor2 back action.");
    }
}

/************************ (C) COPYRIGHT  *****END OF FILE****/