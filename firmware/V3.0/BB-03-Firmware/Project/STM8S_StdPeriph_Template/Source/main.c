/**
  ******************************************************************************
  * @file    Project/main.c
  * @author  MCD Application Team
  * @version V2.2.0
  * @date    30-September-2014
  * @brief   Main program body
   ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
#include  "stm8s_tim2.h"
#include  "stm8s_gpio.h"
#include  "stm8s_clk.h"
#include  "bsp.h"
#include  "user_config.h"

extern void SetHookTick(uint32_t * hookTick,uint16_t cnt);
extern uint32_t GetTick(void);
/* Private defines -----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

static void GPIO_Config(void/* arguments */);

static void CLK_Config(void);
static void AWU_Config(void);

static void ADC1_Config(void);
//static void IWDG_Config(void);
void SPEEK_CheckEnable(void/* arguments */);
void SPEEK_CheckDisable(void/* arguments */);
static bool KEY_Pro10ms(WorkData_st *data);
static uint8_t Key_GetValue(void);
static void LED_FlashPro(WorkData_st data);
static void PWM_ELEOutInit(uint32_t frequency,uint16_t duty);
static void PWM_ELEOutEnable(uint16_t frequency, uint16_t duty);
static void PWM_ELEOutDisable(void/* arguments */);
//static void Delay(uint16_t nCount);
static void BEEP_ProSingal10ms(uint8_t cnt);
static  uint16_t ADC1_GetValue(void);
static void PWM_BeepOutInit(uint16_t frequency,uint16_t duty);
static void PWM_BeepOutEnable(uint16_t frequency, uint16_t duty);
static void PWM_BeepOutDisable(void/* arguments */);
static void Delay10ms(uint16_t nCount);
static void LED_ModeMiddle(void);
//static void PWM_MotorOutEnable(uint16_t frequency, uint16_t duty);
//static void PWM_MotorOutDisable(void);
static void Delayms(uint32_t nCount);
static void Delay10ms(uint16_t nCount);
static void LED_BleEn(void);
static void LED_BleDis(void);
static void LED_RedEn(void);
static void LED_RedDis(void);
static void LED_AllON();
static void LED_AllOFF();
static void BEEP_ProTwo(uint8_t lon);
static void POWER_EN(void);
static void POWER_DIS(void);
static void SHAKE_EN(void);
static void SHAKE_DIS(void);
__IO uint16_t SpeakCnt = 0;

Tick_st TickSt;

//u32 linetemp = 0;
WorkData_st WorkData;

#if 0
/**
 * [GetTick_10ms description]
 * @return  [description]
 */
uint32_t GetTick_10ms(void)
{
    return TickSt.tick_10ms;
}
#endif

/**
 * [GetTick_1s description]
 * @method GetTick_1s
 * @return            [description]
 */
uint32_t GetTick_1s(void)
{
    return TickSt.tick_1s;
}
/**
 * [main description]
 * @method main
 */
void SetHooktick_1s(uint32_t * hookTick,int16_t cnt)
{
    *hookTick = TickSt.tick_1s + cnt;
}
static uint32_t tick_1sProLedTick = 0;
static uint8_t speakCheckStatus = 0;
static __IO uint16_t senCnt = 0; //
/**
 * [main description]
 * @method main
*/
void main(void){    
    CLK_Config();
    CFG->GCR |= 0x01;
    GPIO_Config();
    
    ADC1_Config();
    PWM_ELEOutInit(ELE_FREQUENCY,0);
    PWM_BeepOutInit(BEEP_FREQUENCY,0);
    #if RWP_EN > 0
    if (FLASH_ReadOptionByte(0x4800) == 0x00) {
        IWDG_Enable();
        FLASH_Unlock(FLASH_MEMTYPE_DATA);
        FLASH_ProgramOptionByte(0x4800, 0xAA);
        FLASH_Lock(FLASH_MEMTYPE_DATA);
        while (1);
    }
    #endif
    uint16_t val = FLASH_ReadByte(EEROM_MODE_ADD);
    WorkData.mode = (WorkMode_em)val;
    WorkData.batterStatus = 0;
    val = FLASH_ReadByte(EEROM_MODE_ADD + 1);
    WorkData.sensitivity = (val << 8) + FLASH_ReadByte(EEROM_MODE_ADD + 1);// +
    if(WorkData.sensitivity == 0){
        WorkData.sensitivity = SPEEK_SENSITIVITY;
    }

    AWU_Config();
    enableInterrupts();

    while (1){
        KEY_Pro10ms(&WorkData);
        if(WorkData.status == WORK_ON && TickSt.tick_1s < 500){ //
            if(speakCheckStatus == 0){  //开启检测
                SPEEK_CheckEnable();
                speakCheckStatus = 1;
                AWU_Cmd(ENABLE);
            }
            if(TickSt.tick_1s - tick_1sProLedTick >= LED_FLASH_ON_INTER){ // >= LED_FLASH_ON_INTER
                static uint8_t adcCnt = 0;
                uint16_t adcValue;
                tick_1sProLedTick = TickSt.tick_1s;
                adcValue = ADC1_GetValue();
                if(adcValue <= 700){//700
                    if(adcCnt > 1){                      
                        for(uint8_t i = 0; i < 5; i++){
                            LED_RedEn();
                            GPIO_WriteHigh(GPIO_LED1);
                            Delay10ms(50);
                            GPIO_WriteLow(GPIO_LED1);
                            Delay10ms(50);
                            LED_RedDis();
                        }                        
                        WorkData.batterStatus = 1; //低电压
                        SPEEK_CheckDisable(); //关闭检测

                    }
                    else{
                        adcCnt++;
                    }

                }
                else{
                    if(adcCnt == 0 && WorkData.batterStatus == 1){
                        WorkData.batterStatus = 0;
                        SPEEK_CheckEnable();
                    }
                    else{
                        adcCnt--;
                    }
                }
                if(WorkData.batterStatus == 0 && TickSt.tick_1s - WorkData.sleepTick <  SLEEP_TIME){
                    LED_FlashPro(WorkData); 
                }

            }

            static uint32_t tick_1sProCycTick = 0;
            if(WorkData.step < 7){
                if(TickSt.tick_1s - tick_1sProCycTick > PRO_CYC_NEW){
                    WorkData.step  = 0;
                }
            }
            else{
                if(TickSt.tick_1s - tick_1sProCycTick > PRO_CYC_END){
                    WorkData.step  = 0;
                }
            }
            if(SpeakCnt > WorkData.sensitivity){//
                SPEEK_CheckDisable();
                senCnt = 1000;
                WorkData.sleepTick = TickSt.tick_1s;
                if(WorkData.mode == SHAKE){
                    BEEP_ProTwo(10);
                    SHAKE_EN();
                    LED_BleEn();
                    LED_ModeMiddle();
                    LED_BleDis();
                }
                else{
                    if(WorkData.step  <= 6){
                        WorkData.step ++;
                        BEEP_ProTwo(10);
                        if(WorkData.step  == 2){
                            SHAKE_EN();
                        }

                        LED_RedEn();
                        LED_ModeMiddle();
                        LED_RedDis();
                        SHAKE_DIS();
                        switch(WorkData.step ){
                            case 1:
                            tick_1sProCycTick = TickSt.tick_1s;
                            break;
                            break;
                            case 3:
                                PWM_ELEOutEnable(ELE_FREQUENCY, LEVEL1_DUTY);
                            break;
                            case 4:
                                PWM_ELEOutEnable(ELE_FREQUENCY, LEVEL2_DUTY);
                            break;
                            case 5:
                                PWM_ELEOutEnable(ELE_FREQUENCY, LEVEL3_DUTY);
                            break;
                            case 6:
                                PWM_ELEOutEnable(ELE_FREQUENCY, LEVEL4_DUTY);
                            break;
                            case 7:
                                PWM_ELEOutEnable(ELE_FREQUENCY, LEVEL5_DUTY);
                            break;
                            default:
                            break;
                        }
                        Delay10ms(100);
                        PWM_ELEOutDisable();
                        if(WorkData.step  >= 7){
                            tick_1sProCycTick = TickSt.tick_1s;
                            LED_RedEn();
                            for(uint8_t i = 0; i < 5; i++){
                                LED_AllON();
                                Delay10ms(50);
                                LED_AllOFF();
                                Delay10ms(50);
                            }
                            LED_RedDis();
                        }
                    }
                }
                SPEEK_CheckEnable();
                SpeakCnt = 0;
            }
            else{
                if(SpeakCnt > 0){
                    if(senCnt == 0){
                        senCnt = 1000;
                    }                       
                }
                if(senCnt > 0){
                    senCnt --;
                }
                if(senCnt == 0){                            
                    SpeakCnt = 0;
                }                
            }
            if(TickSt.tick_1s - WorkData.sleepTick >=  SLEEP_TIME){//SLEEP_TIME){ //休眠
                //低电量模式不提示休眠
                if(TickSt.tick_1s - WorkData.sleepTick ==  SLEEP_TIME && WorkData.batterStatus == 0){
                    TickSt.tick_1s++;
                    LED_RedEn();
                    for(uint8_t i = 0; i < 5; i++){
                        LED_AllON();
                        Delay10ms(50);
                        LED_AllOFF();
                        Delay10ms(50);
                    }
                    LED_RedDis();
                }
            }            
        }
        else{
            if(speakCheckStatus == 1){
                SPEEK_CheckDisable();
                speakCheckStatus = 0;
                AWU_Cmd(DISABLE); 
            }            
            senCnt = 0;
                       
        }
        if(senCnt == 0){
            halt();
            
        }         
    }

}
/**
 * [CLK_Config description]
 *
 * @method CLK_Config
 */
static void CLK_Config(void)
{
  /* Initialization of the clock */
  /* Clock divider to HSI/1 */
    //CLK_SYSCLKConfig(CLK_PRESCALER_HSIDIV8);
    //CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV8);
    CLK->PCKENR1 = 0xb0;
    CLK->PCKENR2 = 0x0c;
    //CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV2);
    //CLK->PCKENR1 = 0x00;
    //CLK->PCKENR2 = 0x00;

}
/**
 * [LED_BleEn description]
 * @method LED_BleEn
 */
static void LED_BleEn(void){
    GPIO_WriteLow(GPIO_LED_BLE);
}
/**
 * [LED_BleDis description]
 * @method LED_BleDis
 */
static void LED_BleDis(void){
    GPIO_WriteHigh(GPIO_LED_BLE);
}
/**
 * [LED_RedEn description]
 * @method LED_RedEn
 */
static void LED_RedEn(void){
    GPIO_WriteLow(GPIO_LED_RED);
}
/**
 * [LED_RedDis description]
 * @method LED_RedDis
 */
static void LED_RedDis(void){
    GPIO_WriteHigh(GPIO_LED_RED);
}
/**
 * [LED_ALL_ON description]
 * @method LED_ALL_ON
 */
static void LED_AllON(void){
    GPIO_WriteHigh(GPIO_LED1);
    GPIO_WriteHigh(GPIO_LED2);
    GPIO_WriteHigh(GPIO_LED3);
    GPIO_WriteHigh(GPIO_LED4);
    GPIO_WriteHigh(GPIO_LED5);
}
/**
 * [LED_ALL_OFF description]
 * @method LED_ALL_OFF
 */
static void LED_AllOFF(void){
    GPIO_WriteLow(GPIO_LED1);
    GPIO_WriteLow(GPIO_LED2);
    GPIO_WriteLow(GPIO_LED3);
    GPIO_WriteLow(GPIO_LED4);
    GPIO_WriteLow(GPIO_LED5);
}
static void LED_ModeMiddle(void){
    uint8_t i = 0;
    uint8_t j = 0;
    GPIO_Pin_TypeDef pin = GPIO_PIN_3;
    for(j = 0; j < 2; j++){
        for(i = 0; i < 3; i++){
            GPIO_WriteHigh(GPIOD, (GPIO_Pin_TypeDef)(pin >> i));
            GPIO_WriteHigh(GPIOD, (GPIO_Pin_TypeDef)(pin << i));
            Delay10ms(20);
            //GPIO_WriteLow(GPIOD, (GPIO_Pin_TypeDef)(pin >> i));
            //GPIO_WriteLow(GPIOD, (GPIO_Pin_TypeDef)(pin << i));
            //Delay10ms(7);
        }

        SHAKE_DIS();
        LED_AllOFF();
        Delay10ms(20);
    }
    /*
    for(i = 0; i < 3; i++){
        GPIO_WriteHigh(GPIOD, (GPIO_Pin_TypeDef)(pin >> i));
        GPIO_WriteHigh(GPIOD, (GPIO_Pin_TypeDef)(pin << i));
        Delay10ms(10);
        GPIO_WriteLow(GPIOD, (GPIO_Pin_TypeDef)(pin >> i));
        GPIO_WriteLow(GPIOD, (GPIO_Pin_TypeDef)(pin << i));
        //Delay10ms(7);
    }
    */
}
static void LED_ModeSide(uint8_t motor_cnt){
    uint8_t i = 0;
    uint8_t j = 0;
    GPIO_Pin_TypeDef pin = GPIO_PIN_1;
    for(i = 0; i < 5; i++){
        j++;
        GPIO_WriteHigh(GPIOD, (GPIO_Pin_TypeDef)(pin << i));
        Delay10ms(20);
        //GPIO_WriteLow(GPIOD, (GPIO_Pin_TypeDef)(pin << i));
        if(j == motor_cnt){

        }
    }
    LED_AllOFF();
    Delay10ms(20);
    pin = GPIO_PIN_5;
    for(i = 0; i < 5; i++){
        j++;
        GPIO_WriteHigh(GPIOD, (GPIO_Pin_TypeDef)(pin >> i));
        Delay10ms(20);
        //GPIO_WriteLow(GPIOD, (GPIO_Pin_TypeDef)(pin >> i));
        if(j > motor_cnt){

        }

    }
    LED_AllOFF();
}
/**
 * [POWER_EN description]
 */
static void POWER_EN(void){
    GPIO_WriteHigh(GPIO_POWER_EN);
}
/**
 * [POWER_DIS description]
 */
static void POWER_DIS(void){
    GPIO_WriteLow(GPIO_POWER_EN);
}
/**
 * [LED_FlashPro description]
 * @method LED_FlashPro
 * @param  data         [description]
 */
static void LED_FlashPro(WorkData_st data)
{
    if(data.mode == SHAKE){
        LED_BleEn();
    }
    else if(data.mode == ELE_SHAKE){
        LED_RedEn();
    }
    LED_ModeSide(0);
    LED_RedDis();
    LED_BleDis();
}

/**
 * [SHAKE_EN description]
 * @method SHAKE_EN
 */
static void SHAKE_EN(void){
    GPIO_WriteHigh(GPIO_MOTOR_OUT);
}
/**
 * [SHAKE_DIS description]
 * @method SHAKE_DIS
 */
static void SHAKE_DIS(void){
    GPIO_WriteLow(GPIO_MOTOR_OUT);
}

/**
 * [Key_GetValue description]
 * @method Key_Pro
 */
static uint8_t Key_GetValue(void){
    uint8_t ret = 0;
    if(GPIO_ReadInputPin(GPIO_ON_OFF)){
        MASK_SET(ret, KEY_ON_OFF);
    }
    if(!GPIO_ReadInputPin(GPIO_MODE)){
        MASK_SET(ret, KEY_MODE);
    }
    return ret;
}
/**
 * [Key_Pro description]
 * @method Key_Pro
 */
static bool KEY_Pro10ms(WorkData_st *data){
    uint8_t keyValueOld = 0;
    uint16_t key10msCnt = 0;
    uint8_t keyValue = 0;
    keyValue = Key_GetValue();
    keyValueOld = keyValue;
    if(keyValue != 0){
        while(1){
            keyValue = Key_GetValue();
            if(keyValue == 0){
                break;
            }
            Delayms(10);
            key10msCnt++;
            if(WorkData.batterStatus == 1){ //低电压
                if(key10msCnt > 20){
                    for(uint8_t i = 0; i < 5; i++){
                        LED_RedEn();
                        GPIO_WriteHigh(GPIO_LED1);
                        Delay10ms(50);
                        GPIO_WriteLow(GPIO_LED1);
                        Delay10ms(50);
                        LED_RedDis();
                    }
                }
            }
            else if(key10msCnt >= 100){ //1.5s  && key10msCnt < 500
                WorkData.sleepTick = TickSt.tick_1s;
                if(key10msCnt == 100){
                    switch(keyValue){
                        case KEY_ON_OFF:{
                            if(data->status == WORK_ON){
                                data->status = WORK_OFF;
                                if(WorkData.mode == SHAKE){
                                    LED_BleEn();
                                    //LED_RedDis();
                                }
                                else{
                                    LED_RedEn();
                                    //LED_BleDis();
                                }
                                LED_AllON();
                                BEEP_ProSingal10ms(50);
                                Delay10ms(100);
                                POWER_DIS();
                                Delay10ms(10);
                                LED_AllOFF();
                                LED_BleDis();
                                LED_RedDis();
                            }else{
                                POWER_EN();
                                data->status = WORK_ON;
                                BEEP_ProTwo(20);
                                SHAKE_EN();
                                Delay10ms(20);
                                SHAKE_DIS();
                                if(WorkData.mode == SHAKE){
                                    LED_BleEn();
                                    //LED_RedDis();
                                }
                                else{
                                    LED_RedEn();
                                    //LED_BleDis();
                                }
                                LED_ModeSide(0);
                                LED_BleDis();
                                LED_RedDis();
                            }
                        }
                        break;
                        case KEY_MODE:{
                            if(data->status == WORK_ON){ //
                                WorkData.step  = 0;
                                BEEP_ProSingal10ms(50);
                                //FLASH_SetProgrammingTime(FLASH_PROGRAMTIME_STANDARD);
                                //PWM_BeepOutEnable(BEEP_FREQUENCY,50);
                                if(data->mode == SHAKE){
                                    data->mode = ELE_SHAKE;
                                    FLASH_Unlock(FLASH_MEMTYPE_DATA);
                                    FLASH_ProgramByte(EEROM_MODE_ADD, data->mode);
                                    FLASH_Lock(FLASH_MEMTYPE_DATA);

                                }else{
                                    data->mode = SHAKE;
                                    FLASH_Unlock(FLASH_MEMTYPE_DATA);
                                    FLASH_ProgramByte(EEROM_MODE_ADD, data->mode);
                                    FLASH_Lock(FLASH_MEMTYPE_DATA);
                                }
                                if(WorkData.mode == SHAKE){
                                    LED_BleEn();
                                }
                                else{
                                    LED_RedEn();
                                }
                                LED_ModeSide(0);
                                LED_BleDis();
                                LED_RedDis();
                            }
                            /*
                            else{
                                LED_BleEn();
                                LED_RedEn();
                                LED_AllON();
                                Delay10ms(10);
                                LED_BleDis();
                                LED_RedDis();
                            }
                            */
                        }
                        break;
                        default:
                        break;
                    }
                    
                }
                SpeakCnt = 0;
            }
        }
        //key10msCnt = 0;
    }
    if(key10msCnt < 20){
        //闇查湶闇茬倝
    }
    else if(key10msCnt < 100){
        WorkData.sleepTick = TickSt.tick_1s;
        switch(keyValueOld){
            case KEY_ON_OFF:
                if(data->status == WORK_ON){
                    WorkData.step  = 0;
                    if(WorkData.batterStatus == 0){
                        if(WorkData.mode == SHAKE){
                            LED_BleEn();
                            LED_RedDis();
                        }
                        else{
                            LED_RedEn();
                            LED_BleDis();
                        }
                        LED_ModeSide(0);
                        SHAKE_DIS();
                        LED_BleDis();
                        LED_RedDis();
                    }

                }
            break;
            default:
            break;
        }
        SpeakCnt = 0;
        keyValueOld = 0;

    }
    return TRUE;
}

#if 0
/**
  * @brief  Configure TIM4 to generate an update interrupt each 1ms
  * @param  None
  * @retval None
  */
static void TIM4_Config(void)
{
  /* TIM4 configuration:
   - TIM4CLK is set to 16 MHz, the TIM4 Prescaler is equal to 128 so the TIM1 counter
   clock used is 2 MHz / 128 = 125 000 Hz
  - With 125 000 Hz we can generate time base:
      max time base is 2.048 ms if TIM4_PERIOD = 255 --> (255 + 1) / 125000 = 2.048 ms
      min time base is 0.016 ms if TIM4_PERIOD = 1   --> (  1 + 1) / 125000 = 0.016 ms
  - In this example we need to generate a time base equal to 1 ms
   so TIM4_PERIOD = (0.001 * 125000 - 1) = 124 */

  /* Time base configuration */
  TIM4_TimeBaseInit(TIM4_PRESCALER_128, 39);//10ms
  /* Clear TIM4 update flag */
  TIM4_ClearFlag(TIM4_FLAG_UPDATE);
  /* Enable update interrupt */
 // TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);

  /* enable interrupts */
  ///* Enable TIM4 */
  //TIM4_Cmd(ENABLE);
}
/**
 * [TIM4_TickDisable description]
 */
/*
static void TIM4_TickDisable(void) {


    //TIM2_SetCompare1(0);
    TIM4_Cmd(DISABLE);
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER4, DISABLE);
    //GPIO_ELE_OUT_LOW();
}
*/
/**
 * [TIM4_TickDisable description]
 */

/*
static void TIM4_TickEnable(void) {


    //TIM2_SetCompare1(0);
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER4, ENABLE);
    TIM4_Cmd(ENABLE);

    //GPIO_ELE_OUT_LOW();
}
*/
#endif

/**
* @brief  Configure the AWU time base to 12s
* @param  None
* @retval None
*/
static void AWU_Config(void)
{
  /* Initialization of AWU */
   /* LSI calibration for accurate auto wake up time base*/
    //AWU_LSICalibrationConfig(LSIMeasurment());
    CLK->ICKR |= CLK_ICKR_FHWU;
    FLASH_SetLowPowerMode(FLASH_LPMODE_POWERDOWN);//
    CLK->ICKR |= CLK_ICKR_SWUAH;//
     /* The delay corresponds to the time we will stay in Halt mode */
    AWU_Init(TICK_AWU);
}
void AWU_GPIO_Dint(void/* arguments */) {
    /* code */
}
/**
 * [SPEEK_CheckEnable description]
 * @method SPEEK_CheckEnable
 */
void SPEEK_CheckEnable(void/* arguments */) {
   // disableInterrupts();

    GPIO_Init(GPIO_SPEEK_CHECK,      GPIO_MODE_IN_FL_IT);
    enableInterrupts();
    //EXTI_DeInit();
    //GPIO_Init(GPIO_SPEEK_CHECK,      GPIO_MODE_OUT_PP_LOW_SLOW);//GPIO_MODE_OUT_PP_LOW_SLOW
    //EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOC, EXTI_SENSITIVITY_FALL_ONLY);
    //EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOD, EXTI_SENSITIVITY_FALL_ONLY);
}
/**
 * [SPEEK_CheckDisable description]
 * @method SPEEK_CheckDisable
 */
void SPEEK_CheckDisable(void/* arguments */) {

    disableInterrupts();
    GPIO_Init(GPIO_SPEEK_CHECK,      GPIO_MODE_OUT_PP_LOW_SLOW);//GPIO_MODE_OUT_PP_LOW_SLOW
    //enableInterrupts();
}

static void GPIO_Config(void/* arguments */) {
    /* code */
    //
    GPIO_Init(GPIO_ADC, GPIO_MODE_IN_FL_NO_IT);
    GPIO_Init(GPIO_ELE_OUT, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_BEEP_OUT, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_MOTOR_OUT, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_BEEP_OUT,    GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_POWER_EN, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_ON_OFF,      GPIO_MODE_IN_FL_IT);
    GPIO_Init(GPIO_MODE,        GPIO_MODE_IN_PU_IT);
    GPIO_Init(GPIO_SPEEK_CHECK,      GPIO_MODE_IN_FL_NO_IT); //GPIO_MODE_IN_FL_IT
    //GPIO_Init(GPIO_ELE_CHECK,        GPIO_MODE_IN_PU_IT);
    GPIO_Init(GPIO_LED_BLE,    GPIO_MODE_OUT_OD_HIZ_SLOW);
    GPIO_Init(GPIO_LED_RED,    GPIO_MODE_OUT_OD_HIZ_SLOW);
    GPIO_Init(GPIO_LED1 ,    GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_LED2 ,    GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_LED3 ,    GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_LED4 ,    GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_LED5 ,    GPIO_MODE_OUT_PP_LOW_SLOW);
    //GPIO_WriteHigh(GPIO_POWER_EN);
    //GPIO_Init(GPIOD, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_SLOW); //GPIO_PIN_1 |

  /*
    GPIO_Init(GPIO_SW_ELE, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(GPIO_ADC, GPIO_MODE_IN_FL_NO_IT);
    GPIO_Init(GPIOA,GPIO_PIN_1, GPIO_MODE_IN_FL_NO_IT);
    GPIO_Init(GPIO_SW_ELE_BEEP, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_SW_BEEP, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_BUTTTON_LEVEL, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_ELE_CHECK, GPIO_MODE_IN_FL_NO_IT);
    GPIO_Init(GPIO_ELE_OUT, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_BEEP_OUT, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_VIB_CHECK, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIOD,GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_LED_BLE, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_LED_READ1, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_LED_READ2, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_LED_READ3, GPIO_MODE_OUT_PP_LOW_SLOW);
    */
    //EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOA, EXTI_SENSITIVITY_FALL_ONLY);
    EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOC, EXTI_SENSITIVITY_RISE_FALL);
    EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOD, EXTI_SENSITIVITY_FALL_ONLY);
}

/**
 * [ADC1_Config description]
 * @method ADC1_Config
 */
static void ADC1_Config(void)
{
   //CLK_PeripheralClockConfig(CLK_PERIPHERAL_ADC, ENABLE);
    ADC1_Init(ADC1_CONVERSIONMODE_SINGLE, ADC1_CHANNEL_2, ADC1_PRESSEL_FCPU_D2, \
			  ADC1_EXTTRIG_TIM, DISABLE, ADC1_ALIGN_RIGHT, ADC1_SCHMITTTRIG_CHANNEL0, DISABLE);//
    ADC1_Cmd(DISABLE);
//        ADC1_ITConfig(ADC1_IT_EOCIE, ENABLE);
}
/**
 * [ADC1_GetValue description]
 * @return  [description]
 */
static  uint16_t ADC1_GetValue(void){
    uint16_t adcValue = 0;
    //CLK_PeripheralClockConfig(CLK_PERIPHERAL_ADC, ENABLE);
    ADC1_Cmd(ENABLE);
    ADC1_StartConversion();
    Delayms(1);
    adcValue = ADC1_GetConversionValue();
    ADC1_Cmd(DISABLE);
    //CLK_PeripheralClockConfig(CLK_PERIPHERAL_ADC, DISABLE);
    return adcValue;
}
/**
 * [PWM_ELEOutInit description]
 * @param frequency [description]
 * @param duty      [description]
 */
static void PWM_ELEOutInit(uint32_t frequency,uint16_t duty)
{
    uint32_t ulTmp;
    //CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER1, ENABLE);
    ulTmp = 2000000;
    //ulTmp /= 32;
    ulTmp /= frequency;
    TIM1_TimeBaseInit(1,
                      TIM1_COUNTERMODE_UP,
                      ulTmp - 1,
                      0);
    //TIM1_OC1Init(TIM1_OCMODE_PWM1, TIM1_OUTPUTSTATE_ENABLE, 0X00, TIM1_OCPOLARITY_HIGH);

    TIM1_OC3Init(TIM1_OCMODE_PWM1,
                  TIM1_OUTPUTSTATE_ENABLE,
                  TIM1_OUTPUTNSTATE_DISABLE,
                  0X00, TIM1_OCPOLARITY_HIGH,
                  TIM1_OCNPOLARITY_HIGH,
                  TIM1_OCIDLESTATE_RESET,
                  TIM1_OCNIDLESTATE_RESET);
    TIM1_ClearFlag(TIM1_FLAG_UPDATE);
    TIM1_OC3PreloadConfig(ENABLE);
    TIM1_CtrlPWMOutputs(DISABLE);
    return;
}

/**
 * [PWM_ELEOutEnable description]
 * @method PWM_ELEOutEnable
 * @param  frequency        [description]
 * @param  duty             [description]
 */
static void PWM_ELEOutEnable(uint16_t frequency, uint16_t duty){
    /* code */
    uint32_t temp = 0;
    temp = 2000000;
    temp /= frequency;
    temp *= duty;
    temp /= 1000;
    //CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER1, ENABLE);

    TIM1_Cmd(ENABLE);
    TIM1_SetCompare3(temp);
    TIM1_CtrlPWMOutputs(ENABLE);
    //GPIO_WriteLow(GPIO_ELE_OUT);
    //GPIO_ELE_OUT_LOW();
    //TIM2_CtrlPWMOutputs(ENABLE);

}
/**
 * [PWM_ELEOutDisable description]
 * @method PWM_ELEOutDisable
 */
static void PWM_ELEOutDisable(void/* arguments */) {
    /* code */
    TIM1_SetCompare3(0);
    TIM1_CtrlPWMOutputs(DISABLE);
    TIM1_Cmd(DISABLE);
    //CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER1, DISABLE);
    //GPIO_WriteLow(GPIO_ELE_OUT);
    //WorkData.outFlag = FALSE;
    //GPIO_ELE_OUT_LOW();
}
/**
 * [PWM_BeepOutInit description]
 * @method PWM_BeepOutInit
 * @param  frequency       [description]
 * @param  duty            [description]
 */
static void PWM_BeepOutInit(uint16_t frequency,uint16_t duty)
{
    uint32_t ulTmp;
    //CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER2, ENABLE);
    ulTmp = 2000000;
    //ulTmp /= 32;
    ulTmp /= frequency;
    TIM2_TimeBaseInit(TIM2_PRESCALER_1, ulTmp - 1);
    TIM2_ClearFlag(TIM2_FLAG_UPDATE);

    TIM2_SetCompare3(0);
    //TIM2_SetCompare1(0);
    TIM2_OC3Init(TIM2_OCMODE_PWM2, TIM2_OUTPUTSTATE_ENABLE, 0X00, TIM2_OCPOLARITY_LOW);
    //TIM2_OC1Init(TIM2_OCMODE_PWM2, TIM2_OUTPUTSTATE_ENABLE, 0X00, TIM2_OCPOLARITY_LOW);
    //TIM2_OC1PreloadConfig(ENABLE);
    //TIM2_OC3PreloadConfig(ENABLE);
    //TIM2_ARRPreloadConfig(ENABLE);
    TIM2_Cmd(DISABLE);
    return;
}

/**
 * [PWM_BeepOutEnable description]
 */
static void PWM_BeepOutEnable(uint16_t frequency, uint16_t duty) {
    /* code */
    uint32_t ulTmp;
    //CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER2, ENABLE);
    ulTmp = 2000000;
    //ulTmp /= 32;
    ulTmp /= frequency;
    ulTmp *= duty;
    ulTmp /= 100;
    TIM2_Cmd(ENABLE);
    TIM2_SetCompare3(ulTmp);


    //GPIO_WriteLow(GPIO_BEEP_OUT);
    //GPIO_ELE_OUT_LOW();
    //GPIO_ELE_OUT_LOW();
    //TIM2_CtrlPWMOutputs(ENABLE);

}
/**
 * [PWM_ELEOutDisable description]
 * @method PWM_ELEOutDisable
 */
static void PWM_BeepOutDisable(void/* arguments */) {
    /* code */

    TIM2_SetCompare3(0);
    //GPIO_ELE_OUT_LOW();
    TIM2_Cmd(DISABLE);
   //CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER2, DISABLE);
    //GPIO_WriteLow(GPIO_BEEP_OUT);
    //WorkData.outFlag = FALSE;
    //GPIO_ELE_OUT_LOW();
}
#if 0
static void PWM_MotorOutEnable(uint16_t frequency, uint16_t duty) {
    /* code */
    uint32_t ulTmp;
    //CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER2, ENABLE);
    ulTmp = 2000000;
    //ulTmp /= 32;
    ulTmp /= frequency;
    ulTmp *= duty;
    ulTmp /= 100;
    TIM2_Cmd(ENABLE);
    TIM2_SetCompare1(ulTmp);


    //GPIO_WriteLow(GPIO_BEEP_OUT);
    //GPIO_ELE_OUT_LOW();
    //GPIO_ELE_OUT_LOW();
    //TIM2_CtrlPWMOutputs(ENABLE);

}
/**
 * [PWM_ELEOutDisable description]
 * @method PWM_ELEOutDisable
 */
static void PWM_MotorOutDisable(void/* arguments */) {
    /* code */

    TIM2_SetCompare1(0);
    //GPIO_ELE_OUT_LOW();
    TIM2_Cmd(DISABLE);
    //CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER2, DISABLE);
    //GPIO_WriteLow(GPIO_BEEP_OUT);
    //WorkData.outFlag = FALSE;
    //GPIO_ELE_OUT_LOW();
}
#endif
/**
 * [BEEP_ProSingal10ms description]
 * @method BEEP_ProSingal10ms
 * @param  cnt                [description]
 */
static void BEEP_ProSingal10ms(uint8_t cnt){
    PWM_BeepOutEnable(BEEP_FREQUENCY,50);
    Delay10ms(cnt);
    PWM_BeepOutDisable();

}
/**
 * [BEEP_ProTwo description]
 * @method BEEP_ProTwo
 * @param  lon         [description]
 */
static void BEEP_ProTwo(uint8_t lon){
    PWM_BeepOutEnable(BEEP_FREQUENCY,50);
    Delay10ms(lon);
    PWM_BeepOutDisable();
    Delay10ms(10);
    PWM_BeepOutEnable(BEEP_FREQUENCY,50);
    Delay10ms(lon);
    PWM_BeepOutDisable();
}
#if 0
/**
  * @brief  Configures the IWDG to generate a Reset if it is not refreshed at the
  *         correct time.
  * @param  None
  * @retval None
  */
static void IWDG_Config(void)
{
  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();

  /* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency
     dispersion) */
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  /* IWDG counter clock: LSI/128 */
  IWDG_SetPrescaler(IWDG_Prescaler_256);

  /* Set counter reload value to obtain 250ms IWDG Timeout.
    Counter Reload Value = 250ms/IWDG counter clock period
                         = 250ms / (LSI/128)
                         = 0.25s / (LsiFreq/128)
                         = LsiFreq/(128 * 4)
                         = LsiFreq/512
   */
  IWDG_SetReload(0xff);

  /* Reload IWDG counter */
  IWDG_Enable();
  IWDG_ReloadCounter();

}
#endif
/**
  * @brief Delay.
  * @param nCount
  * @retval None
  */
static void Delayms(uint32_t nCount){
    /* Decrement nCount value */
    for(uint32_t j = 0; j < nCount; j++){
        for(uint16_t i = 0; i < 350; i++){
            asm("nop");
        }
    }
}

/**
  * @brief Delay.
  * @param nCount
  * @retval None
  */
static void Delay10ms(uint16_t nCount){
    /* Decrement nCount value */
    /*
    uint32_t tick10ms = 0;
    while(nCount != 0){
        if(tick10ms != GetTick_10ms()){
            nCount--;
            IWDG_ReloadCounter();
            tick10ms = GetTick_10ms();
        }
    }*/
    Delayms(10*nCount);
}

#if 0
/**
  * @brief Delay.
  * @param nCount
  * @retval None
  */
static void Delay(uint16_t nCount)
{
    /* Decrement nCount value */
    while (nCount != 0)
    {
        nCount--;
    }
}
#endif
#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
  */
//__IO static uint16_t Line  = 0;
//__IO u8 File[100];
void assert_failed(u8* file, u32 line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */

  while (1)
  {
      //Line = line;
      //strcpy(File,file);
      //printf("Wrong parameters value: file %s on line %d\r\n", file, line);
  }
}
#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
