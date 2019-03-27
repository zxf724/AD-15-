/**
  ******************************************************************************
  * @file    protocol.h
  * @author  ����
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
#define CMD_TIME_RALIB		      0x35
#define CMD_BORROW_UMBRELLA	    0x1A

/*Э����մ������ջ���ջ��Ŀ�������������Ϊ2��ڤ�η�ֵ */
#define PROTOCOL_REC_STACK_MAX  4

/*�ְ����ݻ������󳤶�*/
#define MULITI_PACKET_LEN_MAX		512

/*�������Ӻ������ݳ�ʱ�Ͽ�ʱ�䣬��λ��*/
#define BLE_CONNECT_TIMEOUT			60
    
typedef struct 
{
  uint8_t lock_num;
  uint8_t lock_state;
}lock_t;
    
extern BOOL isAuthOK;
extern uint32_t AuthOK_TS;
extern uint8_t Random_Running[];
extern uint8_t Key_Running[];

/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

void Protocol_Init(void);

void Protocol_NewDate(uint8_t *dat, uint8_t len);
void Protocol_DateProcPoll(void);
void Protocol_Report_Umbrella_Borrow(uint32_t rfid, motor_status_t status);

void Send_Cmd(uint8_t cmd, uint8_t *arg, uint8_t len);

uint8_t* Create_Random(void);

void AesData_decrypt(uint8_t* data, uint8_t* key, uint16_t len);




#endif /* _protocol_H */

/************************ (C)  *****END OF FILE****/

