/**
  **********************************  stm8l15x  ***********************************
  * @鏂囦欢鍚�     锛� app.c
  * @浣滆€�       锛� Huang Fugui
  * @搴撶増鏈�     锛� V2.2.0
  * @鏂囦欢鐗堟湰   锛� V1.0.0
  * @鏃ユ湡       锛� 2016骞�09鏈�02鏃�
  * @鎽樿��       锛� 搴旂敤绋嬪簭婧愭枃浠�
  ******************************************************************************/
#include "user_comm.h"


uint8_t DevAddress = 0;

static uint32_t tsUART = 0;
static FIFO_t   UART_RecFIFO;
static uint8_t  UART_RecBuffer[UART_FIFO_BUF_SIZE], IC_ReadBuf[20], IC_Read = 0;

/**
 * APP鍒濆�嬪寲
 */
void App_Init(void) {
    FIFO_Init(&UART_RecFIFO, UART_RecBuffer, sizeof(UART_RecBuffer));
}

/**
 * 鍒峰崱杞�寰�
 */
void RFID_Polling(void) {
    uint8_t ret = 0;
    ret = RfidReadData(&IC_ReadBuf[4], CARD_BLOCK, 1, 0);
    if(ret != 0) {
        LED_ON(RED);
//        GPIO_ResetBits(GPIOD, GPIO_Pin_0);
//        delay(500);
//        memcpy(IC_ReadBuf, MLastSelectedSnr, 4);
        if(ret == 1) {
            if (memcmp(IC_ReadBuf, MLastSelectedSnr, 4) != 0) {
                memcpy(IC_ReadBuf, MLastSelectedSnr, 4);
                SendCmd(CMD_READ_IC_RSP, IC_ReadBuf, 4);
                TS_DELAY(50);
                SendCmd(CMD_READ_IC_RSP, IC_ReadBuf, 4);
                ret = 0;
            } else {
                ret = 0;
                SendCmd(CMD_READ_IC_RSP, IC_ReadBuf, 4);
                TS_DELAY(50);
                SendCmd(CMD_READ_IC_RSP, IC_ReadBuf, 4);
            }
        } else {
            SendCmd(CMD_READ_IC_FRIM_RSP, IC_ReadBuf, 4);
        }
        IC_Read = 1;
    } else {
        LED_OFF(RED);
        GPIO_SetBits(GPIOD, GPIO_Pin_0);
        IC_Read = 0;
    }
}

/**
 * 鍙戦€佸懡浠�
 *
 * @param cmd
 * @param arg    鍙戦€佸弬鏁�
 */
void SendCmd(uint8_t cmd, uint8_t *data, uint8_t datalen) {
    uint8_t buf[32], *p = buf, i = 0, len = 5;
    *p++ = 0x7E;
    *p++ = 0x1B;
    *p++ = DevAddress;
    *p++ = cmd;
    *p++ = datalen;
    if (data != NULL && datalen > 0) {
        memcpy(p, data, datalen);
        p += datalen;
        len = len + datalen;
    }
    *p = AddCheck(buf, len);
    len += 1;
    p = buf;
    EN485_Recevie_OFF;
    for (i = 0; i < len; i++) {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData8(USART1, *p++);
    }
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    EN485_Recevie_ON;
}

/**
 * 鎺ユ敹澶勭悊鍛戒护
 */
void ReadCmdDeal(void) {
    uint8_t buf[32], len = 0;
    if (TS_IS_OVER(tsUART, 50)) {
        while (FIFO_Length(&UART_RecFIFO) >= 6 && FIFO_Get(&UART_RecFIFO) != 0x7E);
        if (FIFO_Length(&UART_RecFIFO) >= 5 && FIFO_Get(&UART_RecFIFO) == 0x1B) {
            buf[0] = 0x7E;
            buf[1] = 0x1B;
            FIFO_Read(&UART_RecFIFO, &buf[2], 3);
            if (buf[4] > 0) {
                FIFO_Read(&UART_RecFIFO, &buf[5], buf[4]);
            }
            len = buf[4] + 5;
            buf[len] = FIFO_Get(&UART_RecFIFO);
            if ((DevAddress == buf[2] || buf[2] == 0) && AddCheck(buf, len) == buf[len]) {
                if (buf[3] == CMD_DEVICE_RST) {
                    LED_ON(GREEN);
                    WWDG_SWReset();
                    while (1);
                } else if (buf[3] == CMD_DEVICE_CHK) {
                    LED_ON(GREEN);
                    SendCmd(CMD_DEVICE_CHK_RSP, NULL, 0);
                } else if (buf[3] == CMD_READ_IC) {
                    if (IC_Read) {
                        LED_ON(GREEN);
                        SendCmd(CMD_READ_IC_RSP, IC_ReadBuf, 4);
                    } else {
                        LED_ON(RED);
                        TS_DELAY(100);
                        LED_OFF(RED);
                    }
                } else if (buf[3] == CMD_READ_IC_FRIM) {
                    if (IC_Read) {
                        LED_ON(GREEN);
                        SendCmd(CMD_READ_IC_FRIM_RSP, IC_ReadBuf, 20);
                    } else {
                        LED_ON(RED);
                        TS_DELAY(100);
                        LED_OFF(RED);
                    }
                } else if (buf[3] == CMD_WRITE_FRIM && buf[4] == 16) {
                    LED_ON(GREEN);
                    /*寰€IC鍗′腑鍐欏叆鍘傚晢淇℃伅*/
                    if (RfidWriteData(CARD_BLOCK, 1, &buf[5])) {
                        LED_ON(RED);
                        TS_DELAY(200);
                        LED_OFF(RED);
                        TS_DELAY(200);
                        IWDG_ReloadCounter();
                        LED_ON(RED);
                        TS_DELAY(200);
                        LED_OFF(RED);
                        TS_DELAY(200);
                        IWDG_ReloadCounter();
                    }
                }
            }
        }
    }
    if (TS_IS_OVER(tsUART, 1000)) {
        LED_OFF(GREEN);
        TS_INIT(tsUART);
        if (FIFO_Length(&UART_RecFIFO) > 0 && TS_IS_OVER(tsUART, 3000)) {
            FIFO_Flush(&UART_RecFIFO);
        }
    }
}

/**
 * 涓插彛鎺ユ敹鍒版柊鐨勬暟鎹�
 *
 * @param data   鏂版帴鏀跺埌鐨勬暟鎹�
 */
void UART_NewData(uint8_t data) {
    FIFO_Put(&UART_RecFIFO, data);
    TS_INIT(tsUART);
}

/************************************************
鍑芥暟鍚嶇О 锛� GET_DeviceAddress
鍔�    鑳� 锛� 寰楀埌璁剧疆鍦板潃
鍙�    鏁� 锛� 鏃�

杩� 鍥� 鍊� 锛� 鏃�
浣�    鑰� 锛� Huang Fugui
*************************************************/
void GET_DeviceAddress(void) {
    DevAddress |= KEY1;
    DevAddress |= KEY2 << 1;
    DevAddress |= KEY3 << 2;
    DevAddress |= KEY4 << 3;
    DevAddress += 0xA0;
}





/***** Copyright (C)2016 HuangFugui. All Rights Reserved ***** END OF FILE *****/
