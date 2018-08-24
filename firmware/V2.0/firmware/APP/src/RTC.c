/**
  ******************************************************************************
  * @file    RTC.c
  * @author  ����
  * @version V1.0
  * @date    2015.12.29
  * @brief   RTC��������.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "includes.h"

/** @addtogroup firmwave_WTD_LED
  * @{
  */



/** @defgroup RTC
  * @brief �ⲿRTC����
  * @{
  */


/* Private typedef -----------------------------------------------------------*/

/** @defgroup RTC_Private_Typedef RTC Private Typedef
  * @{
  */


/**
  * @}
  */

/* Private define ------------------------------------------------------------*/
/** @defgroup RTCPrivate_Constants RTC Private Constants
  * @{
  */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup RTC_Private_Macros RTC Private Macros
  * @{
  */


/**
  * @}
  */
/* Private variables ---------------------------------------------------------*/
/** @defgroup RTC_Private_Variables Private Variables
  * @{
  */
APP_TIMER_DEF(TimerId_RTC);


static uint32_t RTC_Count = 0;

const uint8_t week_table[12] = { 0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5 };
const uint8_t mon_table[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/**
  * @}
  */

/* Private function prototypes -----------------------------------------------*/
/** @defgroup RTC_Private_Functions RTC Private Functions
  * @{
  */
static void RTC_CountTick(void *p_context);
static uint8_t RTC_GetWeek(uint16_t Year, uint8_t Month, uint8_t Date);

/**
  * @}
  */

/* Exported functions ---------------------------------------------------------*/

/** @defgroup RTC_Exported_Functions RTC Exported Functions
  *  @brief   RTC �����ⲿ�ӿں���
  * @{
  */

/**
  * @brief  ��ʼ��RTC ����.
  * @param  none.
  * @retval none.
  */
void RTC_Init(void)
{
    app_timer_create(&TimerId_RTC, APP_TIMER_MODE_REPEATED, RTC_CountTick);
    app_timer_start(TimerId_RTC, APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER), NULL);

    timeRTC_t time;

    time.year = 2018;
    time.month = 4;
    time.date = 20;
    time.hours = 17;
    time.minutes = 37;
    time.seconds = 0;
    RTC_SetTime(&time);
#if (DEBUG == 1)
  DBG_LOG("RTC Init.");
#endif
    
}

/**
  * @brief  ��RTC���������ֵ.
  */
uint32_t RTC_ReadCount(void)
{
    return RTC_Count;
}

/**
  * @brief  ����RTC���������ֵ.
  */
void RTC_SetCount(uint32_t cnt)
{
    RTC_Count = cnt;
}

/**
  * @brief  ��RTC.
  * @param  rdat: ���ݶ�����ָ��.
  * @param  len:  д��ĳ���.
  * @retval �����ɹ�����TRUE.
  */
void RTC_ReadTime(timeRTC_t *time)
{
    RTC_TickToTime(RTC_Count, time);
}

/**
  * @brief  дRTC.
  * @param  time: �����õ�ʱ��.
  * @retval none.
  */
void RTC_SetTime(timeRTC_t *time)
{
    RTC_Count = RTC_TimeToTick(time);
}

/**
  * @brief  RTC��ת����ʱ��.
  * @param  tick: ��ת����.
  * @param  time: ת����ʱ��.
  * @retval none.
  */
void RTC_TickToTime(uint32_t tick, timeRTC_t *time)
{
    uint8_t tmpMonth = 0, tmpHour = 0, tmpMinute = 0;
    uint16_t tmpYear = 0, temp = 0;
    uint32_t tmpresSecond = 0, TotalSeconds = 0, TotalDays = 0;

    TotalSeconds = tick;
    TotalDays = TotalSeconds / 86400;
    tmpresSecond = TotalSeconds % 86400;

    /*������*/
    tmpYear = RTC_YEAR_BASE;
    temp = (IS_LEAP(tmpYear)) ? 366 : 365;
    while (TotalDays >= temp) {
        TotalDays -= temp;
        tmpYear++;
        temp = (IS_LEAP(tmpYear)) ? 366 : 365;
    }
    /*������*/
    tmpMonth = 1;
    temp = mon_table[0];
    while (TotalDays >= temp && tmpMonth < 12) {
        TotalDays -= temp;
        tmpMonth++;
        temp = mon_table[tmpMonth - 1];
        if (tmpMonth == 2) {
            temp = (IS_LEAP(tmpYear)) ? 29 : 28;
        }
    }
    /*����ʱ���֣���*/
    tmpHour = tmpresSecond / 3600;
    tmpMinute = tmpresSecond % 3600 / 60;
    tmpresSecond = tmpresSecond % 60;

    time->seconds = tmpresSecond;
    time->minutes = tmpMinute;
    time->hours = tmpHour;
    time->date = TotalDays + 1;
    time->month = tmpMonth;
    time->year = tmpYear;
    time->day = RTC_GetWeek(time->year, time->month, time->date);
}

/**
  * @brief  RTCʱ��ת������.
  * @param  time: ��ת����ʱ��.
  * @retval none.
  */
uint32_t RTC_TimeToTick(timeRTC_t *time)
{
    uint32_t temp = 0, i = 0;

    if (time->year < RTC_YEAR_BASE) return 0;

    temp = (time->year - RTC_YEAR_BASE) * 365 * 86400;
    for (i = RTC_YEAR_BASE; i < time->year; i++) {
        if (IS_LEAP(i)) temp += 86400;
    }
    for (i = 0; i < (time->month - 1); i++) {
        temp += mon_table[i] * 86400;
        if (i == 1 && (IS_LEAP(time->year))) temp += 86400;
    }
    temp += (time->date - 1) * 86400;
    temp += time->hours * 3600;
    temp += time->minutes * 60;
    temp += time->seconds;

    return temp;
}


/**
  * @}
  */


/** @addtogroup RTC_Private_Functions
  * @{
  */

/**
  * @brief  RTC�����������.
  * @retval none.
  */
static void RTC_CountTick(void *p_context)
{
    RTC_Count++;
}

/**
  * @brief  RTC��������ֵ.
  */
static uint8_t RTC_GetWeek(uint16_t Year, uint8_t Month, uint8_t Date)
{
    uint16_t  temp2;
    uint8_t yearH, yearL;

    yearH = Year / 100;
    yearL = Year % 100;
    // ���Ϊ21����,�������100
    if (yearH > 19) yearL += 100;
    // ����������ֻ��1900��֮���
    temp2 = yearL + yearL / 4;
    temp2 = temp2 % 7;
    temp2 = temp2 + Date + week_table[Month - 1];
    if (yearL % 4 == 0 && Month < 3) temp2--;
    temp2 = temp2 % 7;
    if (temp2 == 0) return 7;
    else return temp2;
}



/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT  *****END OF FILE****/
