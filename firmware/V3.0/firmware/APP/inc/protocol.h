/**
  ******************************************************************************
  * @file    protocol.h
  * @author  宋阳
  * @version V1.0
  * @date    2015.12.31
  * @brief   Header file of protocol
  ******************************************************************************
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _protocol_H
#define _protocol_H



/* Includes ------------------------------------------------------------------*/
#include "prjlib.h"


/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
#define MSG_TYPE_CMD			      1
#define MSG_TYPE_DATA			      3
#define CMD_TIME_RALIB		      0x35    //school time
#define CMD_BORROW_UMBRELLA	    0x1A    //borrow unbrally 
#define CMD_RETURN_UMBRELLA     0xA1    //repay unbrally
#define CMD_RETURN_BREAKDOWN_UMBRELLA     0x1C  //repay browndown unbrally


/*协议接收处理，接收缓存栈条目的最大数量，需为2的冥次方值 */
#define PROTOCOL_REC_STACK_MAX  4

/*分包数据缓存的最大长度*/
#define MULITI_PACKET_LEN_MAX		512

/*蓝牙连接后无数据超时断开时间，单位秒*/
#define BLE_CONNECT_TIMEOUT			300

typedef struct {
    uint8_t lock_num;
    uint8_t lock_state;
} lock_t;

extern BOOL isAuthOK;
extern uint32_t AuthOK_TS;
extern uint8_t Random_Running[];
extern uint8_t Key_Running[];

/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

void Protocol_Init(void);

void Protocol_NewDate(uint8_t *dat, uint8_t len);
void Protocol_DateProcPoll(void);
void Protocol_Report_Umbrella(uint32_t rfid, motor_status_t status);

void Send_Cmd(uint8_t cmd, uint8_t *arg, uint8_t len);

uint8_t* Create_Random(void);

void AesData_decrypt(uint8_t* data, uint8_t* key, uint16_t len);




#endif /* _protocol_H */

/************************ (C)  *****END OF FILE****/

