#include "MFRC522_API.h"
#include "stm8l15x.h"

uint8_t fcnt;				//响应时间
uint8_t Pcb;				//CPU卡APDU指令分组号	
uint8_t Err[3];				//CPU卡APDU指令分组号
uint8_t PassWd[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};       //用于访问的密码（用户密码）
                    //{0x00,0x00,0x00,0x00,0x00,0x00};
                    //{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	   // 默认密码(密钥A) - 不能读出，能改写

uint8_t MLastSelectedSnr[4];      // 存储 卡片序列号


uint8_t NewKey[16] = {0x01,0xAB,0x23,0xCD,0x45,0xEF,
                      0xff, 0x07, 0x80, 0x69, 
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00
					           };	// 新密码  应该是每扇区最后一个权限
				                // 密钥A(6Byte) + 权限(4Byte) + 密钥B(6Byte)
uint8_t ERASE_ORDER[] =         {0x54, 0x02, 0x00, 0x00, 0x00};
uint8_t GET_CARD_ID[] =         {0x54, 0x06, 0x00, 0x00, 0x08};
uint8_t GET_CARD_VERSION[] =    {0x54, 0x06, 0x00, 0x01, 0x01};
uint8_t GET_CARD_LIFECYCLE[] =  {0x54, 0x06, 0x00, 0x02, 0x01};
uint8_t GET_CARD_SPACE[] =      {0x54, 0x06, 0x00, 0x03, 0x02};
uint8_t GET_CARD_PINCNT[] =     {0x54, 0x06, 0x00, 0x04, 0x01};
uint8_t GET_CARD_UID[] =        {0x54, 0x06, 0x00, 0x05, 0x10};
//const uint8_t PassWord_Default[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};           //默认的密码（卡原始密码）
//const uint8_t PassWord_user[6] = {0x01,0xAB,0x23,0xCD,0x45,0xEF};              //修改的密码（发卡后用户使用的密码）

/*****************************************************
** @函数名：InitRc522
** @描述  ：初始化 RC522
** @参数  ：无
** @返回值：无
*****************************************************/
void MFRC522_Init(void)
{
	RC522SPI_Init();					//初始化端口
	PcdReset();								//复位RC522
	PcdAntennaOff(); 					//关闭天线
	PcdAntennaOn();						//开启天线
	
	M500PcdConfigISOType('A');//配置为ISO14443_A模式
    //PcdReset();
}

/*****************************************************
** @函数名：StartLinkRc522
** @描述  ：开始连接启动 RC522
** @参数  ：无
** @返回值：成功返回1
*****************************************************/
uint8_t LinkMFRC522 (uint8_t *SN)
{
	uint8_t  status;
    uint8_t  ucRFIDCardSVN[6];
	
 	PcdReset(); // 复位RC522、设置定时器预分频3390，定时器重载数值30 定时时间为 15ms
	status = PcdRequest(PICC_REQIDL, &ucRFIDCardSVN[0]);
	if(status != MI_OK)
    {
		return 0;
	}
	status = PcdAnticoll(&ucRFIDCardSVN[2]);
	if(status != MI_OK)	
    {
		return 0;
	}
	memcpy(SN,&ucRFIDCardSVN[2],4);
	
	status = PcdSelect(SN); // 选卡
	if(status != MI_OK) 
	{
		return 0;
	}
	return 1;
}




/*****************************************************
** @函数名：StartLinkRc522
** @描述  ：开始连接启动 RC522
** @参数  ：无
** @返回值：成功返回1
*****************************************************/
uint8_t StartLinkRc522 (void)
{
    uint8_t ucNum;
    int8_t  status;
    uint8_t ucRFIDCardSVN[6];
	
    //PcdReset(); // 复位RC522、设置定时器预分频3390，定时器重载数值30 定时时间为 15ms
	
    // 寻天线区内未进入休眠状态的卡
    // 返回卡片类型 2字节 卡片类型位于 0扇区0块 Byte6 -- Byte7
    // 存储在	ucRFIDCardSVN[0] ucRFIDCardSVN[1] 中
    status = PcdRequest(PICC_REQIDL, &ucRFIDCardSVN[0]);
    if(status != MI_OK) {
        //status = PcdRequest(PICC_REQIDL, &ucRFIDCardSVN[0]);
        if(status != MI_OK) {
            return 0;
        }
    }
    
    // 防冲撞，返回卡的序列号 4字节 序列号为：0扇区0块的 Byte0 -- Byte3
    // 返回成功则卡片序列号存储在 RevBuffer[2] RevBuffer[3] RevBuffer[4] RevBuffer[5] 中
    status = PcdAnticoll(&ucRFIDCardSVN[2]);
    if(status != MI_OK)	{
        return 0;
    }
    // 将卡片序列号转存于 MLastSelectedSnr 全局量中
    for (ucNum = 0; ucNum < 4; ucNum++)
    {
        MLastSelectedSnr[ucNum] = ucRFIDCardSVN[2+ucNum];
    }
    
    status = PcdSelect(MLastSelectedSnr); // 选卡
    if(status != MI_OK) {
        return 0;
    }

    return 1;
}

/*****************************************************
** @函数名：RC522SendSnr
** @描述  ：从RFID卡中读取序列号
** @参数  ：None
** @返回值：成功返回1
*****************************************************/
uint8_t RC522SendSnr (void)
{
	uint8_t ucRecDat = 0;
	
 	ucRecDat = StartLinkRc522();
	if (1 != ucRecDat) 
  {
//	    OperateFailure(LINKCARD);		
		return 0;
	}

//	OperateSuccess(MCUSENDSNR, 4);
// 	for(ucRecDat = 0; ucRecDat < 4; ucRecDat++)
//  {
//  	UART1_TxChar(MLastSelectedSnr[ucRecDat]);
//  }
	PcdHalt();					 // 系统休眠模式

	return 1;
}

/*****************************************************
** @函数名：RC522SnrPassward
** @描述  ：从RFID卡中读取序列号，将序列号生成6位密码
** @参数  ：NewKey:新密码
** @返回值：成功返回1
*****************************************************/
uint8_t RC522SnrPassward (void)
{
  uint8_t ucNum    = 0;
  uint8_t DealData = 0;
  
 	if (StartLinkRc522())
 	{
   	for (ucNum = 0; ucNum < 4; ucNum++)
   	{
   	  NewKey[ucNum]    = MLastSelectedSnr[ucNum];
   	  NewKey[10+ucNum] = 0xFF - MLastSelectedSnr[ucNum];
   	}
   	
   	DealData = MLastSelectedSnr[0];
   	for (ucNum = 1; ucNum < 4; ucNum++)
   	{
     	DealData ^= MLastSelectedSnr[ucNum];
   	}
   	NewKey[4]  = DealData;
   	NewKey[14] = 0xFF - DealData;
   	
   	DealData = MLastSelectedSnr[0];
   	for (ucNum = 1; ucNum < 4; ucNum++)
   	{
     	DealData += MLastSelectedSnr[ucNum];
   	}
   	NewKey[5]  = 0xFF - DealData;
   	NewKey[15] = 0xFF - DealData;
   	
   	PcdHalt(); // 系统休眠模式
   	return 1;
 	}
 	PcdHalt();   // 系统休眠模式
 	return 0;
}


/*****************************************************
** @函数名：RC522ModifyPsw
** @描述  ：修改RFID卡指定块访问密码
** @参数  ：KuaiN  : 修改块
**          NewKey : 修改后的新密码
** @返回值：成功返回1
*****************************************************/
uint8_t RC522ModifyPsw (uint8_t KuaiN)
{
 	uint8_t ucRecDat = 0;

 	ucRecDat = StartLinkRc522();
	if (1 != ucRecDat) 
  {
//		OperateFailure(LINKCARD);
		return 0;
	}

	ucRecDat = PcdAuthState(PICC_AUTHENT1A,KuaiN,PassWd,MLastSelectedSnr);
  if (ucRecDat != MI_OK) 
  {
//        OperateFailure(CHECKPSW);
	  return 0;
  }

  ucRecDat = PcdWrite(KuaiN,&NewKey[0]); // 写入修改之后密码数据
  if (ucRecDat != MI_OK) 
  {
//    OperateFailure(MCUMODIFYPSW);
	  return 0;
  }
  PcdHalt();                             // 休眠
//	OperateSuccess(MCUMODIFYPSW, 0); 

	return 1;
}

/*****************************************************
** @函数名：RfidReadData
** @描述  ：从RFID卡中读取数据
** @参数  ：pucKuaiDat   读取块数据
**          ucKuaiNum    RFID 读取块开始地址
**          ucKuaiLen    RFID 读取块的长度
**          ucContiFlg   读取块数据是否连续 
**                        0 只读取数据块
**                        1 数据块与控制块一起读
** @返回值：成功返回1
*****************************************************/
uint8_t cardId[20];
uint8_t cardLen = 0;
uint8_t RfidReadData (uint8_t *pucKuaiDat,
                      uint8_t  ucKuaiNum, 
                      uint8_t  ucKuaiLen, 
                      uint8_t  ucContiFlg
                     )
{
    uint8_t ucStatus;
  	uint8_t ucStartKuai;
  	uint8_t ucKuaiLength;
 	uint8_t ucNum;
  	uint8_t ucDatNum;
  	uint8_t ucPswFlg;     // 验证密码标志
  	uint8_t ucReadDat[16];
	uint8_t ret = 0;
  	ucPswFlg     = 0;
  	ucStartKuai  = ucKuaiNum;
  	ucKuaiLength = ucKuaiLen;
    ucStatus = StartLinkRc522(); // 链接RDIF
    if (1 != ucStatus) {
		PcdHalt();
		return 0;
    }
    ret = 1;
   	for (ucNum = 0; ucNum < ucKuaiLength; ucNum++, ucStartKuai++) {
    	
    	// 判断是数据块还是控制块
    	if (ucStartKuai < MAXAREASTA) { // 小区数据
        	if (MINCTRLKUAI == ucStartKuai%MINAREALEN) 
        	{
          		ucPswFlg = 1;
        	}
      	} 
      	else{                  // 大区数据
        	if (MAXCTRLKUAI == ucStartKuai%MAXAREALEN) 
       	 	{
          		ucPswFlg = 1;
       	 	}
      	}	    
	    // 判断是否连续读取      
      	if ((!ucContiFlg) && (ucPswFlg)) { // 只读取数据块
        	ucStartKuai++;     	    
	    }	    
	    //KuaiN = ucStartKuai;
	    if ((!ucNum) || (ucPswFlg)) {
    	    ucPswFlg = 0;
	        ucStatus = PcdAuthState(PICC_AUTHENT1A,ucStartKuai,PassWd,MLastSelectedSnr);// 验证卡片密码
            if (ucStatus != MI_OK) {
        		PcdHalt();
        		return 1;
            }
	    }	    
	    ucStatus = PcdRead(ucStartKuai, ucReadDat);
        if (ucStatus != MI_OK) {
    		PcdHalt();
    		return 1;
        }
		else {
    	    for (ucDatNum = 0; ucDatNum < 16; ucDatNum++) {    	        
    	        pucKuaiDat[ucNum*16+ucDatNum] = ucReadDat[ucDatNum];
    	    }	
			if(pucKuaiDat[ucNum*16 + 0] == 0x13 && pucKuaiDat[ucNum*16 + 1] == 0x13){
				ret = 2;
			}     
	    } 
	}
    
	PcdHalt();	        
    return ret;
}

/*****************************************************
** @函数名：RfidWriteData
** @描述  ：从RFID卡中连续区中写入数据
** @参数  ：ucKuaiNum    RFID 写数据块开始地址
**          ucKuaiLen    RFID 写数据块的长度
**         *pucDat       写入的数据
** @返回值：成功返回1
*****************************************************/
uint8_t RfidWriteData (uint8_t ucKuaiNum, 
                       uint8_t ucKuaiLen, 
                       uint8_t *pucDat
                      )
{
    uint8_t ucStatus;
    uint8_t ucStartKuai;
    uint8_t ucKuaiLength;
    uint8_t ucNum;
    uint8_t ucDatNum;
    uint8_t ucPswFlg;     // 验证密码标志
    uint8_t ucWriteDat[16];    
    ucPswFlg     = 0;
    ucStartKuai  = ucKuaiNum;
    ucKuaiLength = ucKuaiLen;    
    ucStatus = StartLinkRc522(); // 链接RDIF
	if (1 != ucStatus) {
		PcdHalt();
		return 0;
	}	
	for (ucNum = 0; ucNum < ucKuaiLength; ucNum++, ucStartKuai++) {
	    // 判断控制块
    	if (ucStartKuai < MAXAREASTA) { // 小区数据
            if (MINCTRLKUAI == ucStartKuai%MINAREALEN) {
                ucPswFlg = 1;                
            }
        } else {                  // 大区数据
            if (MAXCTRLKUAI == ucStartKuai%MAXAREALEN) {
	            ucPswFlg = 1;
	        }
        }
        // 判断是否连续读取      
        if (ucPswFlg){ // 只读取数据块
	       ucStartKuai++;     	    
	    }
	    
	    //KuaiN = ucStartKuai;
	    if ((!ucNum) || (ucPswFlg)) {
    	    ucPswFlg = 0;
	        ucStatus = PcdAuthState(PICC_AUTHENT1A,ucStartKuai,PassWd,MLastSelectedSnr);// 验证卡片密码
            if (ucStatus != MI_OK) {
        		PcdHalt();
        		return 0;
            }
	    }
	    
	    for (ucDatNum = 0; ucDatNum < 16; ucDatNum++) {
		 	 ucWriteDat[ucDatNum] = *pucDat++; // 写入16位数据
		}

		ucStatus = PcdWrite(ucStartKuai,&ucWriteDat[0]);
		if (ucStatus != MI_OK) {
    		PcdHalt();
		    return 0;
        }     
	}
	PcdHalt();	        
	return 1;
}

//*************************************************************************
// 函数名	：PcdSwitchPCB(void)
// 描述		：切换分组号
// 入口		：
// 出口		：
// 返回		：成功返回MI_OK
//*************************************************************************
void PcdSwitchPCB(void)
{
    switch(Pcb)
    {
        case 0x00:
                Pcb=0x0A;
                break;
        case 0x0A:
                Pcb=0x0B;
                break;
        case 0x0B:
                Pcb=0x0A;
                break;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//*************************************************************************
// 函数名	：Pcd_Cmd
// 描述		：执行上位机发来的指令
// 入口		：pDataIn : 要执行的指令 In_Len  指令数据长度					
// 出口		：pDataOut：输出执行后的返回数据   Out_Len输出的数据长度
// 返回		：MI_OK
//*************************************************************************
 
int8_t Pcd_Cmd(uint8_t* pDataIn, uint8_t  In_Len, uint8_t* pDataOut,uint8_t * Out_Len)
{
	int8_t status =MI_ERR;  
	uint16_t unLen;
	uint8_t ucComMF522Buf[MAXRLEN]; 
	uint8_t i;

	ClearBitMask(Status2Reg,0x08);					// 清空校验成功标志,清除MFCrypto1On位
	memset(ucComMF522Buf, 0x00, MAXRLEN);

 	//PcdSwitchPCB();
	
	ucComMF522Buf[0] = Pcb;
	ucComMF522Buf[1] = 0x01;
	ucComMF522Buf[2] = pDataIn[0];				// CLA
	ucComMF522Buf[3] = pDataIn[1];				// INS 			
	ucComMF522Buf[4] = pDataIn[2];				// P1						 
	ucComMF522Buf[5] = pDataIn[3];				// P2					
	ucComMF522Buf[6] = pDataIn[4];				// LEN	
	for(i=0;i<In_Len;i++)				//DATA
	{
		ucComMF522Buf[7+i] = pDataIn[5+i];	  

	}
     //CalulateCRC(ucComMF522Buf, 7, &ucComMF522Buf[7]);
        //PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, &unLen);
    CalulateCRC(ucComMF522Buf,7, &ucComMF522Buf[In_Len + 7]);	// 生成发送内容的CRC校验,保存到最后两个字节
	status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,In_Len + 9,pDataOut,&unLen); 
	if (status == MI_OK){	
        *Out_Len = unLen/8-4;	 //接收到数据的长度,不要前面和后面的各两个字节，才是返回的有用数据						
        return MI_OK;
	}
	else{
	    Out_Len[0] = unLen/8-4;
            if((pDataOut[2]==0x90)&&(pDataOut[3]==0x00))
                return MI_OK;
            else
		return MI_ERR;
	}
}
/*************************************************************************
// 函数名	：CpuReset()
// 描述		：CPU卡专用复位
// 入口		：
// 出口		：
// 返回		：成功返回MI_OK
*************************************************************************/
int8_t Pcd_GetId(uint8_t* pdest,uint8_t * pdestLen){
    return (Pcd_Cmd(GET_CARD_ID, 0, pdest, pdestLen));
}

/*************************************************************************
// 函数名	：CpuReset()
// 描述		：CPU卡专用复位
// 入口		：
// 出口		：
// 返回		：成功返回MI_OK
*************************************************************************/
void CardReset(void)
{
	int8_t status = MI_OK;
	
	status = PcdRats();										//卡片复位
	if(status)
	{
		Err[0] = 0xFF;
		Err[1] = 0xFF;
		return;
	}

	Err[0] = 0x90;
	Err[1] = 0x00;
}

//*************************************************************************
// 函数名	：PcdRats
// 描述		：转入APDU命令格式
// 入口		：
// 出口		：
// 返回		：成功返回MI_OK
//*************************************************************************/
int8_t PcdRats(void)
{
	int8_t status = MI_ERR;  
	uint16_t unLen;
	uint8_t ucComMF522Buf[MAXRLEN]; 

	ClearBitMask(Status2Reg,0x08);	// 清空校验成功标志,清除MFCrypto1On位

	memset(ucComMF522Buf, 0x00, MAXRLEN);

	ucComMF522Buf[0] = 0xE0;		
	ucComMF522Buf[1] = 0x51;		

    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);	// 生成发送内容的CRC校验,保存到最后两个字节

	status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);// 将收到的卡片类型号保存
	return status;								//返回结果
}
//*************************************************************************
// 函数名	：PcdEraseDF
// 描述		：擦除目录
// 入口		：
// 出口		：
// 返回		：成功返回MI_OK
//*************************************************************************/
void PcdEraseDF(void)
{  
	
	uint16_t unLen;
	uint8_t ucComMF522Buf[MAXRLEN];
	//uint8_t i,ucComMF522Buf[MAXRLEN]; 

	ClearBitMask(Status2Reg, 0x08);												// 清空校验成功标志,清除MFCrypto1On位

	memset(ucComMF522Buf, 0x00, MAXRLEN);

 	PcdSwitchPCB();
	
	ucComMF522Buf[0] = Pcb;	 
	ucComMF522Buf[1] = 0x01;
	ucComMF522Buf[2] = 0x80;			//CLA
	ucComMF522Buf[3] = 0x0E;			//INS	
	ucComMF522Buf[4] = 0x00;			//P1
	ucComMF522Buf[5] = 0x00;			//P2
	ucComMF522Buf[6] = 0x00;			//Lc
	
        CalulateCRC(ucComMF522Buf, 7, &ucComMF522Buf[7]);								// 生成发送内容的CRC校验,保存到最后两个字节

	PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, &unLen);			// 将收到的卡片类型号保存			 
	
	Err[0] = ucComMF522Buf[2];
	Err[1] = ucComMF522Buf[3]; 
//	for(i = 0; i < 255; i++)
//	{		 
//		ReaderSend(ucComMF522Buf, MAXRLEN); 	
//		if(ucComMF522Buf[1] == 0x01)
//		{	  					 			
//			delay_ms(50);
//			PcdEraseCheck(Err);
//			Fw_Erasing();	 
//		}
//		else
//		{
//			Err[0] = ucComMF522Buf[0];
//			Err[1] = ucComMF522Buf[1]; 	
//			break;
//		}
//	}
}	
//*************************************************************************
// 函数名	：PcdEraseCheck
// 描述		：擦除检查
// 入口		：
// 出口		：
// 返回		：
//*************************************************************************/
void PcdEraseCheck(uint8_t* err)
{
	uint16_t unLen;
	uint8_t i,ucComMF522Buf[MAXRLEN]; 
	
	ClearBitMask(Status2Reg, 0x08); 											// 清空校验成功标志,清除MFCrypto1On位
	
	memset(ucComMF522Buf, 0x00, MAXRLEN);
					   
 	PcdSwitchPCB();
	
	ucComMF522Buf[0] = 0xFA;
	ucComMF522Buf[1] = 0x01;
	ucComMF522Buf[2] = 0x01;

    CalulateCRC(ucComMF522Buf, 3, &ucComMF522Buf[3]);						// 生成发送内容的CRC校验,保存到最后两个字节

	PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 5, ucComMF522Buf, &unLen);	// 将收到的卡片类型号保存		

	for(i = 0; i < MAXRLEN-5; i++)
	{
		err[i] = ucComMF522Buf[i];
	}
}
//*************************************************************************
// 函数名	：PcdCreateFile
// 描述		：创建文件
// 入口		：
// 出口		：
// 返回		：
//*************************************************************************/
void PcdCreateFile(uint8_t *fileid, uint8_t Len, uint8_t *pDataIn)
{
	int8_t status =MI_ERR;	
	uint16_t unLen;
	uint8_t i, ucComMF522Buf[MAXRLEN]; 
	
	ClearBitMask(Status2Reg,0x08);	// 清空校验成功标志,清除MFCrypto1On位
	
	memset(ucComMF522Buf, 0x00, MAXRLEN);
	
	PcdSwitchPCB();
		
	ucComMF522Buf[0] = Pcb;  
	ucComMF522Buf[1] = 0x01;
	ucComMF522Buf[2] = 0x80;			//CLA
	ucComMF522Buf[3] = 0xE0;			//INS	
	ucComMF522Buf[4] = fileid[0];		//P1
	ucComMF522Buf[5] = fileid[1];		//P2
	ucComMF522Buf[6] = Len;				//Lc
	for (i=0; i<Len; i++)
		ucComMF522Buf[7+i] = *(pDataIn+i);						//写入内容
	
	CalulateCRC(ucComMF522Buf,7+Len,&ucComMF522Buf[7+Len]); 	// 生成发送内容的CRC校验,保存到最后两个字节
										
	status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9+Len,ucComMF522Buf,&unLen);// 将收到的卡片类型号保存 	
					   				
	if(status == MI_OK)
	{
		Err[0] = ucComMF522Buf[2];
		Err[1] = ucComMF522Buf[3]; 
	}
	else
	{
		Err[0] = 0xFF;
		Err[1] = 0xFF;
	}
}

//*************************************************************************
// 函数名	：PcdGetChallenge
// 描述		：外部认证密钥
// 入口		：keyflag: 外部认证密钥标识号
// 			  		pRan: 4个或8个字节加密随机数
// 出口		：
// 返回		：成功返回MI_OK
//*************************************************************************/
void PcdGetChallenge(uint8_t* pRan)
{
	int8_t status =MI_ERR;  
	uint16_t unLen;
	uint8_t i,ucComMF522Buf[MAXRLEN]; 

	ClearBitMask(Status2Reg,0x08);		//清空校验成功标志,清除MFCrypto1On位
 
	memset(ucComMF522Buf, 0x00, MAXRLEN);
	
 	PcdSwitchPCB();
	
	ucComMF522Buf[0] = Pcb;
	ucComMF522Buf[1] = 0x01;
	ucComMF522Buf[2] = 0x00;		
	ucComMF522Buf[3] = 0x84;				
	ucComMF522Buf[4] = 0x00;
	ucComMF522Buf[5] = 0x00;			
	ucComMF522Buf[6] = 0x04;			//RanLe个字节的随机数	

    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);	// 生成发送内容的CRC校验,保存到最后两个字节
	
	status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);// 将收到的卡片类型号保存 	
	
	if (status == MI_OK)
	{
		Err[0] = ucComMF522Buf[6];
		Err[1] = ucComMF522Buf[7]; 
		if(Err[0] == 0x90 && Err[1] == 0x00)
		{
			for(i = 0;i<4;i++)
				*(pRan+i) = ucComMF522Buf[2+i];
		}
	}
	else
	{
		Err[0] = 0xFF;
		Err[1] = 0xFF; 
	}
}
//*************************************************************************
// 函数名	：PcdExAuth
// 描述		：外部认证密钥
// 入口		：keyflag: 外部认证密钥标识号
// 			 		pRan: 8个字节加密随机数.
// 出口		：
// 返回		：成功返回MI_OK
//*************************************************************************/
void PcdExAuth(uint8_t keysign, uint8_t *pRan)
{
	int8_t status =MI_ERR;  
	uint16_t unLen;
	uint8_t ucComMF522Buf[MAXRLEN]; 

	ClearBitMask(Status2Reg,0x08);			// 清空校验成功标志,清除MFCrypto1On位
	
	memset(ucComMF522Buf, 0x00, MAXRLEN);
	
 	PcdSwitchPCB();
	
	ucComMF522Buf[0] = Pcb;	
	ucComMF522Buf[1] = 0x01;
	ucComMF522Buf[2] = 0x00;	
	ucComMF522Buf[3] = 0x82;				
	ucComMF522Buf[4] = 0x00;
	ucComMF522Buf[5] = keysign;				//认证的密码标识号
	ucComMF522Buf[6] = 0x08;
	ucComMF522Buf[7] = pRan[0];					//8个字节的随机数
	ucComMF522Buf[8] = pRan[1];
	ucComMF522Buf[9] = pRan[2];
	ucComMF522Buf[10] = pRan[3];
	ucComMF522Buf[11] = pRan[4];
	ucComMF522Buf[12] = pRan[5];
	ucComMF522Buf[13] = pRan[6];
	ucComMF522Buf[14] = pRan[7];
        CalulateCRC(ucComMF522Buf,15,&ucComMF522Buf[15]);	// 生成发送内容的CRC校验,保存到最后两个字节										
	status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,17,ucComMF522Buf,&unLen);// 将收到的卡片类型号保存	
	if (status == MI_OK)
	{
		Err[0] = ucComMF522Buf[2];
		Err[1] = ucComMF522Buf[3]; 
	}
	else
	{ 
 		Err[0] = 0xFF;
		Err[1] = 0xFF; 
	}
}
//*************************************************************************
// 函数名	：PcdSelectFile
// 描述		：选择文件
// 入口		：ChooseType : 选择方式
//					Lc : 根据选择方式而定数据长度
//					pDataIn : 文件标识符或者DF 名称
// 出口		：
// 返回		：成功返回MI_OK
//*************************************************************************/
void PcdSelectFile(uint8_t* pDataIn)
{
	int8_t status =MI_ERR;  
	uint16_t unLen;
	uint8_t ucComMF522Buf[MAXRLEN];
	ClearBitMask(Status2Reg,0x08);					// 清空校验成功标志,清除MFCrypto1On位
	memset(ucComMF522Buf, 0x00, MAXRLEN);
 	PcdSwitchPCB();	
	ucComMF522Buf[0] = Pcb;
	ucComMF522Buf[1] = 0x01;
	ucComMF522Buf[2] = 0x00;
	ucComMF522Buf[3] = 0xA4;				
	ucComMF522Buf[4] = 0x00;							//计算种类
	ucComMF522Buf[5] = 0x00;							//认证的密码标识号
	ucComMF522Buf[6] = 0x02;									
	ucComMF522Buf[7] = pDataIn[0];						//写入内容
	ucComMF522Buf[8] = pDataIn[1];									
    CalulateCRC(ucComMF522Buf,9,&ucComMF522Buf[9]);	// 生成发送内容的CRC校验,保存到最后两个字节
	status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,11,ucComMF522Buf,&unLen);// 将收到的卡片类型号保存
	//ReaderSend(ucComMF522Buf, MAXRLEN);
					
	if (status == MI_OK)
	{
		Err[0] = ucComMF522Buf[unLen/8-4];
		Err[1] = ucComMF522Buf[unLen/8-3];
	}
	else
	{
		Err[0] = 0xFF;
		Err[1] = 0xFF; 
	}
}
//*************************************************************************
// 函数名	：PcdReadBinary
// 描述		：读取二进制文件
// 入口		：mode : 线路保护
//					soffset:最高位为1,P1 P2为欲读文件的偏移量,P1为偏移量高字节,P2为低字节,所读的文件为当前文件
//					doffset:最高位不为1,低5位为短的文件标识符,P2为读的偏移量
//					Lc : 长度
//					pDataIn : 输入数据
//					Le : 返回长度
// 出口		：
// 返回		：成功返回MI_OK
//*************************************************************************/
void PcdReadCardData(uint8_t offset, uint16_t Len, uint8_t* pDataOut)
{
	int8_t status =MI_ERR;  
	uint16_t unLen;
	uint8_t i;
    static uint8_t ucComMF522Buf[MAXRLEN]; 

	ClearBitMask(Status2Reg,0x08);	//清空校验成功标志,清除MFCrypto1On位
	
 	if(Len > sizeof(ucComMF522Buf) - 6) return;
	
	memset(ucComMF522Buf, 0x00, MAXRLEN);
	
 	PcdSwitchPCB();

	ucComMF522Buf[0] = Pcb;
	ucComMF522Buf[1] = 0x01;
	ucComMF522Buf[2] = 0x54;
	ucComMF522Buf[3] = 0x06;				
	ucComMF522Buf[4] = 0x00;								//当前文件
	ucComMF522Buf[5] = 0x00;								//起始位置
	ucComMF522Buf[6] = 0x08;									//返回数据的长度
	CalulateCRC(ucComMF522Buf, 7, &ucComMF522Buf[7]);	// 生成发送内容的CRC校验,保存到最后两个字节		
	status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, &unLen);// 将收到的卡片类型号保存
	if (status == MI_OK){
		Err[0] = ucComMF522Buf[Len+2];
		Err[1] = ucComMF522Buf[Len+3];
		if(Err[0] == 0x90 &&Err[1] == 0x00){
            for (i=0; i< Len; i++){
        		*(pDataOut+i) = ucComMF522Buf[i+2];				//保存接收到数据
            }
        }
	}
	else{
		Err[0] = 0xFF;
		Err[1] = 0xFF; 
	}
}
//*************************************************************************
// 函数名	：PcdReadBinary
// 描述		：读取二进制文件
// 入口		：mode : 线路保护
//					soffset:最高位为1,P1 P2为欲读文件的偏移量,P1为偏移量高字节,P2为低字节,所读的文件为当前文件
//					doffset:最高位不为1,低5位为短的文件标识符,P2为读的偏移量
//					Lc : 长度
//					pDataIn : 输入数据
//					Le : 返回长度
// 出口		：
// 返回		：成功返回MI_OK
//*************************************************************************/
void PcdReadBinary(uint8_t offset, uint16_t Len, uint8_t* pDataOut)
{
	int8_t status =MI_ERR;  
	uint16_t unLen;
	uint8_t i,ucComMF522Buf[MAXRLEN]; 

	ClearBitMask(Status2Reg,0x08);	//清空校验成功标志,清除MFCrypto1On位
	
 	if(Len > sizeof(ucComMF522Buf) - 6) return;
	
	memset(ucComMF522Buf, 0x00, MAXRLEN);
	
 	PcdSwitchPCB();

	ucComMF522Buf[0] = Pcb;
	ucComMF522Buf[1] = 0x01;
	ucComMF522Buf[2] = 0x00;
	ucComMF522Buf[3] = 0xB0;				
	ucComMF522Buf[4] = 0x00;								//当前文件
	ucComMF522Buf[5] = offset;								//起始位置
	ucComMF522Buf[6] = Len;									//返回数据的长度
	CalulateCRC(ucComMF522Buf, 7, &ucComMF522Buf[7]);	// 生成发送内容的CRC校验,保存到最后两个字节	
		
	status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, &unLen);// 将收到的卡片类型号保存

	if (status == MI_OK)
	{
		Err[0] = ucComMF522Buf[Len+2];
		Err[1] = ucComMF522Buf[Len+3];
		if(Err[0] == 0x90 &&Err[1] == 0x00)
			for (i=0; i< Len; i++)
        		*(pDataOut+i) = ucComMF522Buf[i+2];				//保存接收到数据
	}
	else
	{
		Err[0] = 0xFF;
		Err[1] = 0xFF; 
	}
}
//*************************************************************************
// 函数名	：PcdUpdateBinary
// 描述		：写入二进制文件
// 入口		：mode : 线路保护
//					//P1 : 最高位为1,低5位为短文件标识符,P2为欲写文件的偏移量
//					P1 : 最高位不为1,P1 P2为欲写文件的偏移量,所写文件为当前文件
//					offset : 起始位置
//					Lc : 写入的字节数
//					pDataIn : 写入数据
// 出口		：
// 返回		：成功返回MI_OK
//*************************************************************************/
void PcdUpdateBinary(uint8_t offset, uint16_t Len, uint8_t* pDataIn)
{
	int8_t status =MI_ERR;  
	uint16_t unLen;
	uint8_t i,ucComMF522Buf[MAXRLEN]; 

	ClearBitMask(Status2Reg,0x08);	// 清空校验成功标志,清除MFCrypto1On位

 	if(Len > sizeof(ucComMF522Buf) - 6) return;

	memset(ucComMF522Buf, 0x00, MAXRLEN);

 	PcdSwitchPCB();
	
	ucComMF522Buf[0] = Pcb;
	ucComMF522Buf[1] = 0x01;
	ucComMF522Buf[2] = 0x00;
	ucComMF522Buf[3] = 0xD6;				
	ucComMF522Buf[4] = 0x00;										//当前文件
	ucComMF522Buf[5] = offset;										//起始位置
	ucComMF522Buf[6] = Len;											//数据内容长度
	for (i=0; i<Len; i++)										
		ucComMF522Buf[7+i] = *(pDataIn+i);							//数据内容
		
	CalulateCRC(ucComMF522Buf, 7+Len, &ucComMF522Buf[7+Len]);
	
	status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 9+Len, ucComMF522Buf, &unLen);

	if (status == MI_OK)
	{
		Err[0] = ucComMF522Buf[2];
		Err[1] = ucComMF522Buf[3];
	}
	else
	{
		Err[0] = 0xFF;
		Err[1] = 0xFF; 
	}
}
//*************************************************************************
// 函数名	：PcdSetKey
// 描述		：修改密钥
// 入口		：Keytype:密钥类型
//					Keysign:密钥标识
//					Lc : 长度
//					pDataIn : 输入数据
//					Le : 返回长度
// 出口		：
// 返回		：成功返回MI_OK
//*************************************************************************/
void PcdSetKey(uint8_t Keytype, uint8_t Keysign, uint8_t Len, uint8_t* pDataIn)
{
	int8_t status = MI_ERR;  
	uint16_t unLen;
	uint8_t i,ucComMF522Buf[MAXRLEN]; 

	ClearBitMask(Status2Reg,0x08);							//清空校验成功标志,清除MFCrypto1On位
	
	memset(ucComMF522Buf, 0x00, MAXRLEN);
	
 	PcdSwitchPCB();

	ucComMF522Buf[0] = Pcb;
	ucComMF522Buf[1] = 0x01;
	ucComMF522Buf[2] = 0x80;
	ucComMF522Buf[3] = 0xD4;				
	ucComMF522Buf[4] = Keytype;								//密钥类型
	ucComMF522Buf[5] = Keysign;								//密钥标识
	ucComMF522Buf[6] = Len;									//返回数据的长度
	for (i=0; i < Len; i++)										
		ucComMF522Buf[7+i] = *(pDataIn+i);					//数据内容
		
	CalulateCRC(ucComMF522Buf, 7+Len, &ucComMF522Buf[7+Len]);	// 生成发送内容的CRC校验,保存到最后两个字节	  
	
	status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 9+Len, ucComMF522Buf, &unLen);// 将收到的卡片类型号保存
	
	//ReaderSend(ucComMF522Buf, MAXRLEN);
								  
	if (status == MI_OK)
	{
		Err[0] = ucComMF522Buf[2];
		Err[1] = ucComMF522Buf[3];
	}
	else
	{
		Err[0] = 0xFF;
		Err[1] = 0xFF; 
	}
}



