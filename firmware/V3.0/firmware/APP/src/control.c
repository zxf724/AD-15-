/* Includes ------------------------------------------------------------------*/
#include "includes.h"
#include "gprs.h"



/* Private typedef -----------------------------------------------------------*/
typedef struct {
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

motor_status_Enum Motor_staus;

APP_TIMER_DEF(TimerId_Lock);
APP_TIMER_DEF(TimerId_Move);
APP_TIMER_DEF(TimerId_LED_NET);
APP_TIMER_DEF(TimerId_LED_STATUS);
APP_TIMER_DEF(TimerId_TTS);
APP_TIMER_DEF(TimerId_Repay);
APP_TIMER_DEF(TimerId_In_Repay);
APP_TIMER_DEF(TimerId_BreakDown);
APP_TIMER_DEF(TimerId_RFID);

// a series switch flag, at funtion InitFlag() would be initialize all of the flag
static uint8_t gs_flag_motor4 = 0, gs_flag_if_is_have_unber = 0, gs_flag_if_is_have_unb = 0,
               gs_flag_RFID_GPRS_Read = 0, gs_flag_IR_CHECK = 0, gs_flag_rfid = 0,
               gs_flag_motor2 = 0, gs_flag_if_is_touch = 0, gs_flag_IR_SW = 0,gs_TTS_Step = 0, 
               gs_flag_is_have = 0;
static char* gs_TTS_Text = NULL;
static uint16_t gs_motorTick = 0;
static uint32_t  gs_RFID_Read = 0;
static Period_Block_t  LED_NET_Period_Def, LED_STATUS_Period_Def;

/* Private function prototypes -----------------------------------------------*/
static void MotorTimerCB(void* p_context);
static void LED_NET_TimerCB(void* p_context);
static void LED_STATUS_TimerCB(void* p_context);
static void TTS_TimerCB(void* p_context);
static void Move_TimerCB(void * p_context);
static void funControl(int argc, char* argv[]);
static void RepayInAction(void*);
static void BreakdownInRepay(void* p_context);
static void TimerIdInRepay(void* p_context);
static void TimerIdRFID(void* p_context);


/* Exported functions ---------------------------------------------------------*/

/**
* @brief  ������ѭ����.
* @param  none.
* @retval none.
*/
void ControlInit(void) {
    //borrow
    app_timer_create(&TimerId_Lock, APP_TIMER_MODE_REPEATED, MotorTimerCB);
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
 * ������ѯ����
 */
void ControlPolling(void) {
    uint8_t led_n;
    static uint8_t led_net;
    static motor_status_Enum status;
    /*ѭ������*/
    if(gs_flag_IR_SW == 1) {
        IO_H(IR_SW);  //enable the sensor
    } else if(gs_flag_IR_SW == 0) {
        IO_L(IR_SW);
    }
    //borrow,repay
    if(gs_flag_RFID_GPRS_Read == 1) {
        gs_RFID_Read = GPRS_ReadRFID(2);
        DBG_LOG("gs_RFID_Read = %u", gs_RFID_Read);
    }
    //reapy browndown
    if(gs_flag_RFID_GPRS_Read == 2) {
        gs_RFID_Read = GPRS_ReadRFID(1);
        DBG_LOG("gs_RFID_Read = %u", gs_RFID_Read);
    }
    /*TTS����*/
    if (gs_TTS_Step == 1) {
        gs_TTS_Step = 2;
        app_timer_start(TimerId_TTS, APP_TIMER_TICKS(1000 + strlen(gs_TTS_Text) * 200, APP_TIMER_PRESCALER), NULL);
        GPRS_TTS(gs_TTS_Text);
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
            //output unbrella
            case k_status_start_output_unbrella:
                TTS_Play("RAM:StartOutputUnbrella.mp3");
                Motor_staus = k_status_idle;
                break;
            case k_status_output_unbrella_success:
                Motor_staus = k_status_idle;
                if (0 == gs_RFID_Read) {
                    TTS_Play("RAM:OutputUnbrellaSuccess.mp3");
                    WorkData.StockCount--;
                    WorkData_Update();
                    Protocol_Report_Umbrella(gs_RFID_Read, status);
                    Report_Umbrella_Status(gs_RFID_Read, status);
                }
                break;
            case k_status_take_the_unbrella_soon:
                TTS_Play("RAM:TakeTheUnbrellaSoon.mp3");
                Motor_staus = k_status_idle;
                break;
            case k_status_have_no_unbrella:
                TTS_Play("RAM:HaveNoUnbrella.mp3");
                Motor_staus = k_status_idle;
                break;
            //input unbrella
            case k_status_start_input_unbrella:
                TTS_Play("RAM:StartInputUnbrella.mp3");
                Motor_staus = k_status_idle;
                break;
            case k_status_input_unbrella_success:
                TTS_Play("RAM:InputUnbrellaSuccess.mp3");
                Protocol_Report_Umbrella(gs_RFID_Read, status);
                Report_Umbrella_Status(gs_RFID_Read, status);
                Motor_staus = k_status_idle;
                if (0 != gs_RFID_Read) {
                    WorkData.StockCount++;
                    DBG_LOG("cJSON repay umbrella status");
                    WorkData_Update();
                    if (Report_Umbrella_Repy_Status(gs_RFID_Read, status, RTC_ReadCount()) == FALSE) {
                        /*��ɡ�ϱ�ʧ�ܣ��洢��¼*/
                        Write_StoreLog(gs_RFID_Read);
                    }
                }
                break;
            case k_status_do_not_occlusion_door:
                TTS_Play("RAM:DoNotOcclusionDoor.mp3");
                Motor_staus = k_status_idle;
                break;
            case k_status_input_unbrella_soon:
                TTS_Play("RAM:InputUnbrellaSoon.mp3");
                Motor_staus = k_status_idle;
                break;
            case k_status_full_unbrella:
                TTS_Play("RAM:FullUnbrella.mp3");
                Motor_staus = k_status_idle;
                break;
            //input breakdown unbrella
            case k_status_input_breakdown_unbrella:
                TTS_Play("RAM:InputBreakDownUnbrella.mp3");
                Motor_staus = k_status_idle;
                break;
            case k_status_restart_ouput:
                TTS_Play("RAM:RestartOuput.mp3");
                Motor_staus = k_status_idle;
                break;
            case k_status_report_breakdown:
                TTS_Play("RAM:ReportBreakDown.mp3");
                Protocol_Report_Umbrella(gs_RFID_Read, status);
                Report_Umbrella_Status(gs_RFID_Read, status);
                Motor_staus = k_status_idle;
                break;
            default:
                break;
        }
    }
    /*������ɡ��¼*/
    if (WorkData.StoreLog_Report != WorkData.StoreLog_In && MQTT_IsConnected()) {
        motor_status_Enum sta;
        uint16_t index = WorkData.StoreLog_Report + 1;
        if (index >= LOG_STORE_MAX) {
            index = 0;
        }
        DBG_LOG("Report Histroy:%u.", index);
        sta = k_status_input_unbrella_success;
        if (Report_Umbrella_Repy_Status(STROE_LOG_POINT(index)->rfid, sta, STROE_LOG_POINT(index)->time)) {
            WorkData.StoreLog_Report = index;
            WorkData_Update();
        }
    }
}

/**
 * ��ɡ��������
 */
void BorrowAction(void) {
    app_timer_stop(TimerId_Lock);
    gs_flag_IR_SW = 1;
    gs_motorTick = 0;
    nrf_delay_ms(80);
    gs_flag_RFID_GPRS_Read = 1;
    Motor_staus = k_status_start_output_unbrella;
    // WorkData.StockCount = 1; //test!
    WorkData.StockCount = 0;
    if (WorkData.StockCount == 0) {
        LED_MOTOR_NG();
        Motor_staus = k_status_have_no_unbrella;
        DBG_LOG("BorrowAction Empty.");
        app_timer_stop(TimerId_Lock);
        app_timer_stop(TimerId_RFID);
        app_timer_stop(TimerId_Move);
        InitFlag();
    }
    /*������*/
    else if (IR_CHECK() == 0) {
        LED_IR_OVER_FLASH();
        Motor_staus = k_status_ir_stuck;
        DBG_LOG("BorrowAction IR Stuck.");
    } else {
        DBG_LOG("BorrowAction NOT stuck!");
        LED_ON(STATUS);
        app_timer_start(TimerId_Lock, APP_TIMER_TICKS(MOTOR_ACTION_TIME, APP_TIMER_PRESCALER), NULL);
    }
}

/**
 * ��ɡ��������
 */
void RepayAction(void) {
    gs_flag_RFID_GPRS_Read = 1; // open RFID read!
    app_timer_stop(TimerId_Lock);
    gs_flag_IR_SW = 1;
    gs_motorTick = 0;
    nrf_delay_ms(80);
    WorkData.StockCount = 9; //test
    if (WorkData.StockCount >= WorkData.StockMax) {
        LED_MOTOR_NG();
        Motor_staus = k_status_full_unbrella;
        DBG_LOG("RepayAction Full.");
    }
    /*����Ƿ�ס*/
    else if (IR_CHECK() == 0) {
        LED_IR_OVER_FLASH();
        Motor_staus = k_status_ir_stuck;
        DBG_LOG("RepayAction IR Stuck.");
    } else {
        Motor_staus = k_status_start_input_unbrella;
        LED_ON(STATUS);
        DBG_LOG("RepayAction");
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
 * ֹͣ�豸��������
 */
void Stop_Action(uint8_t num) {
    LED_OFF(STATUS);
    gs_motorTick = 0;
    //app_timer_stop(TimerId_Lock);
    MOTOR_STOP(num);
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
    LED_NET_Period_Def.index = 0;
    LED_NET_Period_Def.active = FALSE;
    LED_NET_Period_Def.activetime = activetime;
    LED_NET_Period_Def.idletime = idletime;
    LED_NET_Period_Def.times = times;
    LED_ON(NET);
    LED_NET_Period_Def.active = TRUE;
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
    LED_STATUS_Period_Def.index = 0;
    LED_STATUS_Period_Def.active = FALSE;
    LED_STATUS_Period_Def.activetime = activetime;
    LED_STATUS_Period_Def.idletime = idletime;
    LED_STATUS_Period_Def.times = times;
    LED_ON(STATUS);
    LED_STATUS_Period_Def.active = TRUE;
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
    gs_TTS_Step = 0;
    gs_TTS_Text = text;
    app_timer_start(TimerId_TTS, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), NULL);
}

/**
* �忴�Ź�.
*/
void WatchDogClear(void) {
#if WTD_EN == 1
    nrf_drv_wdt_feed();
#endif
}

/**
 * @brief ��ɡ������������.
 * @retval none.
 * @param p_context
 */

static void Move_TimerCB(void * p_context) {
    static uint8_t i = 0, j = 0, step = 0;
    static uint8_t  flag_motor5 = 0, gs_flag_motor_tmp = 0;
    if(1) {   //bluetooth condition add  todo
        if(gs_flag_motor2 == 0) {
            MOTOR_FORWARD(2);
            gs_flag_motor2 = 1;
            gs_flag_if_is_touch = 1;
        }
        if((IF_IS_TOUCH(1) == 0) && (gs_flag_if_is_touch == 1)) {
            MOTOR_STOP(2);
            gs_flag_IR_CHECK = 1;
            //if successed
            app_timer_start(TimerId_RFID, APP_TIMER_TICKS(MOVE_ACTION_TIME, APP_TIMER_PRESCALER), NULL);
        }
        step++;
        // DBG_LOG("step = %d,i = %d,j = %d", step, i, j);
        if(step >= 80) {
            i++;
            step = 0;
        }
        if(i >= 1) {
            i = 0;
            WatchDogClear();
            DBG_LOG("�뾡��ȡɡ");
            j++;
            Motor_staus = k_status_take_the_unbrella_soon;
        }
        if(j >= 5) {
            DBG_LOG("��ɡʧ�ܣ�û�м�ʱȡɡ");
            MOTOR_BACK(2);
            gs_flag_if_is_touch = 0;
            flag_motor5 = 1;
            gs_flag_RFID_GPRS_Read = 0;
            //�رտ�����
            if(flag_motor5 == 1) {
                MOTOR_FORWARD(4);
                flag_motor5 = 0;
            }
            if(IF_IS_TOUCH(5) == 0) {
                MOTOR_STOP(4);
                gs_flag_motor_tmp = 1;
                DBG_LOG("have stop it!!");
            }
            if((IF_IS_TOUCH(2) == 0) && (gs_flag_if_is_touch == 0) && (gs_flag_motor_tmp == 1)) {
                MOTOR_STOP(2);
                j = 0;
                gs_flag_motor_tmp = 0;
                InitFlag();
                DBG_LOG("stop all the timer!");
                app_timer_stop(TimerId_Move);
                app_timer_stop(TimerId_RFID);
                app_timer_stop(TimerId_Lock);
            }
        }
    }
}
/**
 * @brief ��ɡ--��ʱ���ص�����
 * @retval none.
 * @param p_context
 */
static void MotorTimerCB(void* p_context) {
    /*�򿪿����ŵ��*/
    if(gs_flag_motor4 == 0) {
        MOTOR_BACK(4);
        gs_flag_motor4 = 1;
    }
    if(IF_IS_TOUCH(3) == 0) {
        MOTOR_STOP(4);
    }
    //�ж��Ƿ���ɡ
    if((gs_RFID_Read == 0) && (gs_flag_if_is_have_unber == 0)) {
        MOTOR_FORWARD(1);
        gs_flag_if_is_have_unber = 1;
    }
    if ((gs_RFID_Read > 0) && (IF_IS_TOUCH(7) == 0)) {
        MOTOR_STOP(1);
        Motor_staus = k_status_start_output_unbrella;
    }
    // /*��ʱ����Ƿ����*/
    if (gs_motorTick > 10 && MOTOR_IS_STUCK()) {
        LED_MOTOR_OVER_FLASH();
        app_timer_stop(TimerId_Lock);
        Stop_Action(1);
        Motor_staus = k_status_motor_stuck;
        DBG_LOG("Motor is Stuck.");
    }
    /*��ʱ��鵽λ*/
    if ((IF_IS_TOUCH(7) == 0) && (gs_RFID_Read > 0)) {
        gs_motorTick = 0;
        Stop_Action(1);
        if (Motor_staus == k_status_start_output_unbrella) {
        Move_Forward_Action();
        DBG_LOG("In Move_Forward_Action()");
        }
    }
    /*��ʱֹͣ*/
    if (gs_motorTick++ >= MOTOR_OVERFLOW_TIMES) {
        gs_motorTick = 0;
        Stop_Action(1);
        Motor_staus = k_status_timeout;
        DBG_LOG("Motor Running Timeout.");
    }
}
/**
 * ����ָʾ�ƶ�ʱ���ص�
 */
static void LED_NET_TimerCB(void* p_context) {
    if (LED_NET_Period_Def.index == LED_FLASH_CONTINUE) {
        LED_NET_Period_Def.index = 0;
    }
    if (LED_NET_Period_Def.index < LED_NET_Period_Def.times) {
        if (LED_NET_Period_Def.active) {
            LED_OFF(NET);
            LED_NET_Period_Def.active = FALSE;
            LED_NET_Period_Def.index++;
            if (LED_NET_Period_Def.index < LED_NET_Period_Def.times) {
                app_timer_start(TimerId_LED_NET, APP_TIMER_TICKS(LED_NET_Period_Def.idletime, APP_TIMER_PRESCALER), NULL);
            }
        } else {
            LED_ON(NET);
            LED_NET_Period_Def.active = TRUE;
            app_timer_start(TimerId_LED_NET, APP_TIMER_TICKS(LED_NET_Period_Def.activetime, APP_TIMER_PRESCALER), NULL);
        }
    }
}
/**
 * ״ָ̬ʾ�ƶ�ʱ���ص�
 */
static void LED_STATUS_TimerCB(void* p_context) {
    if (LED_STATUS_Period_Def.index == LED_FLASH_CONTINUE) {
        LED_STATUS_Period_Def.index = 0;
    }
    if (LED_STATUS_Period_Def.index < LED_STATUS_Period_Def.times) {
        if (LED_STATUS_Period_Def.active) {
            LED_OFF(STATUS);
            LED_STATUS_Period_Def.active = FALSE;
            LED_STATUS_Period_Def.index++;
            if (LED_STATUS_Period_Def.index < LED_STATUS_Period_Def.times) {
                app_timer_start(TimerId_LED_STATUS, APP_TIMER_TICKS(LED_STATUS_Period_Def.idletime, APP_TIMER_PRESCALER), NULL);
            }
        } else {
            LED_ON(STATUS);
            LED_STATUS_Period_Def.active = TRUE;
            app_timer_start(TimerId_LED_STATUS, APP_TIMER_TICKS(LED_STATUS_Period_Def.activetime, APP_TIMER_PRESCALER), NULL);
        }
    }
}
/**
 * TTS��ʱ���ص�����
 */
static void TTS_TimerCB(void* p_context) {
    if (gs_TTS_Step == 0) {
        gs_TTS_Step = 1;
    } else if (gs_TTS_Step == 2) {
        gs_TTS_Step = 0;
        PA_DISABLE();
    }
}

/**
 * RFID���豸Ӧ��
 *
 * @param addr   �����豸��ַ
 * @param data   �����ĸ�����������
 * @return ���ط���������
 */
uint8_t gs_RFID_ReadPoll(uint8_t addr, uint8_t** data) {
    static uint8_t buf[32];
    uint8_t len = 0, *p = buf, ret = 0;
    len = UserUartRecLength();
    if (len >= 6 && NRF_UART0->PSELRXD == RFID_RX_PIN) {
        nrf_delay_ms(5); //
        len = UserUartRecLength();
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
 * ��ʱ���ص�����--��ɡ
 *
 * @param addr   �����豸��ַ
 * @param data   �����ĸ�����������
 * @return ���ط���������
 */
static void RepayInAction(void *a) {
    static uint8_t gs_flag_motor41 = 0;
    static uint16_t gs_motorTick1 = 0;
    app_timer_stop(TimerId_RFID);
    app_timer_stop(TimerId_Lock);
    WorkData.StockCount = 3;        //��������
    // WorkData.StockCount = 0;
    if (WorkData.StockCount == 0) {
        LED_MOTOR_NG();
        Motor_staus = k_status_have_no_unbrella;
        app_timer_stop(TimerId_Repay);
        app_timer_stop(TimerId_In_Repay);
        DBG_LOG("BorrowAction Empty.");
    }
    /*���ɡͰ���Ƿ���ɡ*/
    if(WorkData.StockCount > 0) {
        if((gs_RFID_Read > 0) && (gs_flag_rfid == 0)) {
            MOTOR_BACK(1);
            gs_flag_rfid = 1;
            gs_flag_is_have = 1;
        }
        if((gs_motorTick1 > 10) && (IF_IS_TOUCH(7) == 0) && (gs_RFID_Read == 0) && (gs_flag_is_have == 1)) {
            MOTOR_STOP(1);
            gs_flag_rfid = 1;
            gs_flag_if_is_have_unb = 1;
        }
        if(gs_RFID_Read == 0) {
            gs_flag_rfid = 1;
            gs_flag_if_is_have_unb = 1;
        }
    }
    
    /*�򿪿����ŵ��*/
    if(gs_flag_if_is_have_unb == 1) {
        if(gs_flag_motor4 == 0) {
            MOTOR_BACK(4);
            gs_flag_motor4 = 1;
        }
        if((IF_IS_TOUCH(3) == 0) && (gs_flag_motor41 == 0)) {
            DBG_LOG("�رտ����ŵ��");
            MOTOR_STOP(4);
            gs_flag_motor41 = 1;
        }
        /*��ʱ����Ƿ����*/
        if (gs_motorTick1 > 10 && MOTOR_IS_STUCK()) {
            LED_MOTOR_OVER_FLASH();
            app_timer_stop(TimerId_Lock);
            Stop_Action(1);
            Motor_staus = k_status_motor_stuck;
            DBG_LOG("Motor is Stuck.");
        }   
        if ((gs_motorTick1 > 50) && (IF_IS_TOUCH(7) == 0)) {
            gs_motorTick1 = 0;
            app_timer_stop(TimerId_Lock);
            DBG_LOG("Motor Running forware.");
            Move_Back_Action();
            gs_flag_motor41 = 0;
        }
    }
    /*��ʱֹͣ*/
    if (gs_motorTick1++ >= MOTOR_OVERFLOW_TIMES) {
        gs_motorTick1 = 0;
        Stop_Action(1);
        Motor_staus = k_status_timeout;
        DBG_LOG("Motor Running Timeout.");
    }
}

void BreakdownRepay(void) {
    //���ϵ��
    MOTOR_FORWARD(3);
    gs_flag_IR_SW = 1;
    gs_flag_RFID_GPRS_Read = 2;
    DBG_LOG("�뽫ɡ�۵��ú�ɡͷ������뻹ɡ��");
    app_timer_start(TimerId_BreakDown, APP_TIMER_TICKS(MOVE_ACTION_TIME, APP_TIMER_PRESCALER), NULL);
}

/**
 * @brief ����ɡ�ص�����.
 * @param argc
 * @param argv
 */
static void TimerIdInRepay(void* p_context) {
    static uint8_t i = 0, j = 0, step = 0, flag_already = 0;
    static uint8_t step1 = 0, gs_flag_situation2 = 0;
    if(1) {   //add app protocal
        step++;
        step1++;
        DBG_LOG("step1 = %d", step1);
        DBG_LOG("step = %d,i = %d,j = %d", step, i, j);
        if(step >= 50) {
            i++;
            step = 0;
        }
        if(i >= 1) {
            i = 0;
            WatchDogClear();
            Motor_staus = k_status_input_unbrella_soon;
            DBG_LOG("�뾡�컹ɡ");
            j++;
        }
        if(j >= 4) {
            DBG_LOG("��ɡʧ�ܣ�û�м�ʱ��ɡ���رյ��");
            gs_flag_situation2 = 1;
            //�رտ����ŵ��
            if(flag_already == 0) {
                MOTOR_FORWARD(4);
                flag_already = 1;
            }
            if(IF_IS_TOUCH(5) == 0) {
                j = 0;
                MOTOR_STOP(4);
                InitFlag();
                app_timer_stop(TimerId_Repay);
                app_timer_stop(TimerId_In_Repay);
            }
        }
        if((gs_RFID_Read > 0) && (IR_CHECK() == 1) && (step1 >= 25)) {
            if(flag_already == 0) {
                MOTOR_FORWARD(4);
                gs_flag_situation2 = 2;
                flag_already = 1;
            }
        }
        if((IF_IS_TOUCH(5) == 0) && (flag_already == 1) && (gs_flag_situation2 == 2)) {
            MOTOR_STOP(4);
            Motor_staus = k_status_input_unbrella_success;
            DBG_LOG("close all the timeid");
            //init all the flag
            InitFlag();
            step = 0;
            i = 0;
            j = 0;
            flag_already = 0;
            step1 = 0;
            step = 0;
            gs_flag_situation2 = 0;
            //stop timer
            app_timer_stop(TimerId_Repay);
            app_timer_stop(TimerId_In_Repay);
        }
    }
}

/**
 * @brief ��ʼ��flag
 * @param argc
 * @param argv
 */
void InitFlag(void) {
    DBG_LOG("at the InitFlag()");
    gs_flag_RFID_GPRS_Read = 0;
    gs_flag_motor4 = 0;
    gs_flag_rfid = 0;
    gs_flag_is_have = 0;
    gs_flag_if_is_have_unb = 0;
    gs_flag_if_is_have_unber = 0;
    gs_flag_motor2 = 0;
    gs_flag_if_is_touch = 0;
    gs_flag_IR_SW = 0;
}

/**
 * @brief ����ɡ���ջص�����.
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
    if(gs_RFID_Read > 0) {
        flag_switch = 1;
    }
    if(i >= 50) {
        DBG_LOG("�뾡�컹ɡ");
        Motor_staus = k_status_input_unbrella_soon;
        i = 0;
        j++;
        DBG_LOG("j = %d", j);
    }
    if((j >= 3) && (flag_time == 0)) {
        DBG_LOG("ʮ���뵽�����ϣ�û�л�ɡ");
        MOTOR_BACK(3);
        Motor_staus = k_status_report_breakdown;
        flag_time = 0;
        flag_switch = 0;
        gs_flag_RFID_GPRS_Read = 0;
    }
    if((flag_switch == 1) && (i >= 20)) {
        flag_time = 1;
        DBG_LOG("������ɡ�ɹ�");
        MOTOR_BACK(3);
    }
    if((IF_IS_TOUCH(6) == 0) && (flag_motor3 == 1)) {
        MOTOR_STOP(3);
        Motor_staus = k_status_restart_ouput;
        gs_flag_RFID_GPRS_Read = 0;
        flag_motor3 = 0;
        flag_time = 0;
        flag_switch = 0;
        app_timer_stop(TimerId_BreakDown);
    }
}

/**
 * @brief ��⻹ɡ�ɹ�
 * @param argc
 * @param argv
 */
static void TimerIdRFID(void* p_context) {
    //������
    static uint8_t step = 0;
    step++;
    if((IR_CHECK() == 1) && (gs_flag_IR_CHECK == 1) && (gs_RFID_Read == 0) && (step >= 40)) {
        app_timer_stop(TimerId_Move);
        app_timer_stop(TimerId_Lock);
        Motor_staus = k_status_output_unbrella_success;
        //�رտ�����
        MOTOR_FORWARD(4);
        if(IF_IS_TOUCH(5) == 0) {
            MOTOR_STOP(4);
            gs_flag_if_is_touch = 1;
            //�ر���ɡ���
            MOTOR_BACK(2);
        }
        if((IF_IS_TOUCH(2) == 0) && (gs_flag_if_is_touch == 1)) {
            MOTOR_STOP(2);
            InitFlag();
            app_timer_stop(TimerId_RFID);
        }
    }
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
    } else if (ARGV_EQUAL("borrow")) {     //borrow unbreally
        DBG_LOG("Test BorrowAction().");
        BorrowAction();
    } else if (ARGV_EQUAL("repay")) {      //repay unbreally
        DBG_LOG("Test RepayAction().");
        RepayAction();
    } else if (ARGV_EQUAL("BreakDown")) {   //repay breakdown unbreally
        BreakdownRepay();
        DBG_LOG("Test BreakDown().");
    } else if (ARGV_EQUAL("status")) {      //Motor_staus
        DBG_LOG("Motor_staus:%u.", Motor_staus);
    } else if (ARGV_EQUAL("MainMotorForward")) {  //main motor control 
        MOTOR_FORWARD(1);
        DBG_LOG("Motor Forward.");
    } else if (ARGV_EQUAL("MainMotorBack")) {    
        MOTOR_BACK(1);
        DBG_LOG("Main Motor Back.");
    } else if (ARGV_EQUAL("MainMotorStop")) {
        MOTOR_STOP(1);
        DBG_LOG("Main Motor Stop.");
    } else if(ARGV_EQUAL("Reset")) {        // reset
        Reset();
        DBG_LOG("reset all the Motol");
    } else if(ARGV_EQUAL("TestMainMotor")) {   //test function begin
        TestMainMotor();
        DBG_LOG("Test Main Motor");
    } else if(ARGV_EQUAL("TestSwitchMotor")) {
        TestSwitchMotor();
        DBG_LOG("Test Switch Motor");
    } else if(ARGV_EQUAL("TestPushMotor")) {
        TestPushMotor();
        DBG_LOG("Test Push Motor");        
    } else if(ARGV_EQUAL("TestBreakDownMotor")) {
        TestBreakDownMotor();
        DBG_LOG("Test Push Motor");        
    } else if(ARGV_EQUAL("TestInfraredSensor")) {
        TestInfraredSensor();
        DBG_LOG("Test Infrared Sensor");
    } else if(ARGV_EQUAL("TestRFID")){          //test function end
        TestRFID();
        DBG_LOG("Test RFID");
    } else if(ARGV_EQUAL("BorrowAction")) {      //test there kinds of modes
        DBG_LOG("Borrow Action");
        BorrowAction();
    } else if(ARGV_EQUAL("RepayAction")) {
        DBG_LOG("Repay Action");
        RepayAction();
    } else if(ARGV_EQUAL("BreakdownRepay")) {
        DBG_LOG("Breakdown Repay"); 
        BreakdownRepay();
    } else if(ARGV_EQUAL("IfIsTouch")) {
        DBG_LOG("test If Is Touch");
        IfIsTouch();
    } else if(ARGV_EQUAL("TmpTest")) {
        DBG_LOG("Tmp Test");
        TmpTest();

    }
}

/** test TmpTest();
  * @}
  */
void TmpTest(void) {
    // Motor_staus = k_status_input_unbrella_soon;
    // static uint8_t test = 0;
    // while(1){
    //     if(test == 0) {
    //         MOTOR_FORWARD(4);
    //         test = 1;
    //     }
    //     if(IF_IS_TOUCH(5) == 0) {
    //         MOTOR_STOP(5);
    //         DBG_LOG("hello,world!");
    //     }
    // }
    while(1) {
        if(IR_CHECK() == 1) {
            DBG_LOG("hello,world!");
        }
    }
}


/************************ (C) COPYRIGHT  *****END OF FILE****/