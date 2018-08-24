#include "MFRC522_Dev.h"
//#include "Fun.h"

/*****************************************************
** @函数名：PcdRequest
** @描述  ：寻卡
** @参数  ：req_code[IN]:寻卡方式
**                0x52 = 寻感应区内所有符合14443A标准的卡
**                0x26 = 寻未进入休眠状态的卡
**          pTagType[OUT]：卡片类型代码
**                0x4400 = Mifare_UltraLight
**                0x0400 = Mifare_One(S50)
**                0x0200 = Mifare_One(S70)
**                0x0800 = Mifare_Pro(X)
**                0x4403 = Mifare_DESFire
** @返回值：成功返回MI_OK
*****************************************************/
int8_t PcdRequest(uint8_t req_code, uint8_t *pTagType)
{
    int8_t   status;
    uint16_t unLen;
    static uint8_t  ucComMF522Buf[MAXRLEN]; 
    ClearBitMask(Status2Reg,0x08);
    WriteRawRC(BitFramingReg,0x07); // 定义接收LSB放在bit0,发送的最后一个字节位数  7 bit
    SetBitMask(TxControlReg,0x03);  // 设置TX1、TX2发送经13.56MHz调制载波
    ucComMF522Buf[0] = req_code;
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);
    if ((status == MI_OK) && (unLen == 0x10)){	 // 接收卡发回来的数据字节数16Byte  
            *pTagType     = ucComMF522Buf[0];
            *(pTagType+1) = ucComMF522Buf[1];
    }
    else
    {   
       status = MI_ERR;   
    }   
    return status;
}

/*****************************************************
** @函数名：PcdAnticoll
** @描述  ：防冲撞
** @参数  ：pSnr[OUT]:卡片序列号，4字节
** @返回值：成功返回MI_OK
*****************************************************/
int8_t PcdAnticoll (uint8_t *pSnr)
{
    int8_t   status;
    uint8_t  i,snr_check=0;
    uint16_t unLen;
    static uint8_t  ucComMF522Buf[MAXRLEN]; 
    

    ClearBitMask(Status2Reg,0x08);	 
    WriteRawRC(BitFramingReg,0x00);    // 暂停数据发送
	                                   // 接收LSB放在bit0位上
									   // 最后一个字节所有位都发送
    ClearBitMask(CollReg,0x80);		   // 所以接收的位在位冲突后都将清零
 
    ucComMF522Buf[0] = PICC_ANTICOLL1; // 防冲撞指令代码
    ucComMF522Buf[1] = 0x20;		   // ?

	// 发送并接收数据指令
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen);

    if (status == MI_OK)
    {
    	 for (i=0; i<4; i++)
         {   
             *(pSnr+i)  = ucComMF522Buf[i];	 // 0扇0区 0--3 Byte 卡片序列号
             snr_check ^= ucComMF522Buf[i];	 // 将卡片序列号按拉异或进行校验
         }
         if (snr_check != ucComMF522Buf[i])	 // 卡片序列号校验与Byte4相同，则正确
         {   status = MI_ERR;    }
    }
    
    SetBitMask(CollReg,0x80);
    return status;
}

/*****************************************************
** @函数名：PcdSelect
** @描述  ：选定卡片
** @参数  ：pSnr[IN]:卡片序列号，4字节
** @返回值：成功返回MI_OK
*****************************************************/
int8_t PcdSelect(uint8_t *pSnr)
{
    int8_t   status;
    uint8_t  i;
    uint16_t unLen;
    uint8_t  ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL1;  // 防冲撞  0x93
    ucComMF522Buf[1] = 0x70;			// ?
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);	// 选择卡片的序列号
    	ucComMF522Buf[6]  ^= *(pSnr+i);	// 选择卡片的序列号校验码
    }
	// CRC校验 	ucComMF522Buf[7] ucComMF522Buf[8] 分别存储校验结果LSB MSB数值
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);
  
    ClearBitMask(Status2Reg,0x08);

	// 发送并接收数据
	// 发送防冲撞指令 卡片序列号  序列号校验码 序列号CRC计算结果值 等值 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);

    if ((status == MI_OK) && (unLen == 0x18)) // 返回数据是否 24 bit == 3 Byte
    {   
        status = MI_OK;  
    }
    else
    {   
        status = MI_ERR;    
    }

    return status;
}

/*****************************************************
** @函数名：PcdAuthState
** @描述  ：验证卡片密码
** @参数  ：auth_mode[IN]: 密码验证模式
**                 0x60 = 验证A密钥
**                 0x61 = 验证B密钥 
**          addr[IN]：块地址
**          pKey[IN]：密码
**          pSnr[IN]：卡片序列号，4字节
** @返回值：成功返回MI_OK
*****************************************************/
int8_t PcdAuthState (uint8_t auth_mode,uint8_t addr,uint8_t *pKey,uint8_t *pSnr)
{
    int8_t   status;
    uint16_t unLen;
    uint8_t  i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for (i=0; i<6; i++)
    {    
	ucComMF522Buf[i+2] = *(pKey+i);
    }
    for (i=0; i<6; i++)	 // i < 4 因为序列号只有4个字节
    {    
	ucComMF522Buf[i+8] = *(pSnr+i);
    }
 //   memcpy(&ucComMF522Buf[2], pKey, 6); 
 //   memcpy(&ucComMF522Buf[8], pSnr, 4); 

	// 验证密钥
    status = PcdComMF522(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);
    if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {   
	status = MI_ERR;   
    }
    return status;
}

/*****************************************************
** @函数名：PcdRead
** @描述  ：读取M1卡一块数据
** @参数  ：addr[IN]：块地址
**          pData[OUT]：读出的数据，16字节
** @返回值：成功返回MI_OK
*****************************************************/
int8_t PcdRead(uint8_t addr,uint8_t *pData)
{
    int8_t   status;
    uint16_t unLen;
    uint8_t  i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_READ; //   读块   0x30
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
   
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    if ((status == MI_OK) && (unLen == 0x90)) // 读取回来共 144 bits == 18 Byte
 //   {   memcpy(pData, ucComMF522Buf, 16);   }
    {
        for (i=0; i<16; i++)
        {    *(pData+i) = ucComMF522Buf[i];   }
    }
    else
    {   status = MI_ERR;   }
    
    return status;
}

/*****************************************************
** @函数名：PcdWrite
** @描述  ：写数据到M1卡一块
** @参数  ：addr[IN]：块地址
**          pData[IN]：写入的数据，16字节
** @返回值：成功返回MI_OK
*****************************************************/
int8_t PcdWrite(uint8_t addr,uint8_t *pData)
{
    int8_t   status;
    uint16_t unLen;
    uint8_t  i,ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_WRITE;  // 0xA0 写块
    ucComMF522Buf[1] = addr;		// 块地址
	// CRC校验实际数值 LSB MSB分别存储在 ucComMF522Buf[2] ucComMF522Buf[3]
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);	
    
	// 发送并接收数据
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }
        
    if (status == MI_OK)
    {
        //memcpy(ucComMF522Buf, pData, 16);
        for (i=0; i<16; i++)
        {    
        	ucComMF522Buf[i] = *(pData+i);   
        }
        
        CalulateCRC(ucComMF522Buf,16,&ucComMF522Buf[16]);
        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   status = MI_ERR;   }
    }
    
    return status;
}

/*****************************************************
** @函数名：PcdHalt
** @描述  ：命令卡片进入休眠状态
** @参数  ：无
** @返回值：成功返回MI_OK
*****************************************************/
int8_t PcdHalt(void)
{
    int8_t   status;
    uint16_t unLen;
    uint8_t  ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_HALT;  // 0x50 休眠
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    return status;
}

/*****************************************************
** @函数名：CalulateCRC
** @描述  ：用MF522计算CRC16函数
** @参数  ：无
** @返回值：无
*****************************************************/
void CalulateCRC(uint8_t *pIndata,uint8_t len,uint8_t *pOutData)
{
    uint8_t  i,n; 
    
    ClearBitMask(DivIrqReg,0x04);   // 清零 当CRC数据位有效且所有数据被处理时置位
    WriteRawRC(CommandReg,PCD_IDLE);// 发送空闲命令
    SetBitMask(FIFOLevelReg,0x80);	// FIFO读写指令指针清0，错误与溢出标志位清0
    
	for (i=0; i<len; i++)
    {   
		WriteRawRC(FIFODataReg, *(pIndata+i)); // 将传入数据放入FIFO中   
	}
    WriteRawRC(CommandReg, PCD_CALCCRC);       // 发送CRC计算

    i = 0xFF;  // 0xFF;
    do 
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04)); // 退出条件： 
	                             // 1、自变量 i 自减到0
								 // 2、CRC所有数据位有效且所有数据被处理时置位
    // 读取CRC校验结果 CRC计算的实际 MSB LSB 数值
    delay_us(10); // 2000  100 适合读写
    pOutData[0] = ReadRawRC(CRCResultRegL);
    pOutData[1] = ReadRawRC(CRCResultRegM);
}

/*****************************************************
** @函数名：PcdReset
** @描述  ：复位RC522
**          复位RC522、设置定时器预分频3390，
**          定时器重载数值30 定时时间为 15ms
** @参数  ：无
** @返回值：成功返回MI_OK
*****************************************************/
int8_t PcdReset(void)
{

	SET_SPI_RST;
  delay_us(10);

	CLR_SPI_RST;
  delay_us(10);

	SET_SPI_RST;
	delay_us(10);
	WriteRawRC(CommandReg,PCD_RESETPHASE);
	delay_us(10);
	
	WriteRawRC(ModeReg,0x3D);            //和Mifare卡通讯，CRC初始值0x6363
	WriteRawRC(TReloadRegL,90);           
	WriteRawRC(TReloadRegH,1);		     // Timer Reload Value 30
	WriteRawRC(TModeReg,0x8D);
	WriteRawRC(TPrescalerReg,0x3E);	     // Timer F Div is 3390
	
	WriteRawRC(TxAutoReg,0x40);//必须要
     
  return MI_OK;
}

/*****************************************************
** @函数名：M500PcdConfigISOType
** @描述  ：设置RC632的工作方式 
** @参数  ：卡工作类型
** @返回值：成功返回MI_OK
*****************************************************/
int8_t M500PcdConfigISOType(uint8_t type)
{
	 if (type == 'A')                     //ISO14443_A
	 { 
		 ClearBitMask(Status2Reg,0x08);
		 WriteRawRC(ModeReg,0x3D);//3F
		 WriteRawRC(RxSelReg,0x86);//84
		 WriteRawRC(RFCfgReg,0x7F);   //4F
		 WriteRawRC(TReloadRegL,0x30);//tmoLength);// TReloadVal = 'h6a =tmoLength(dec) 
		 WriteRawRC(TReloadRegH,0x00);
		 WriteRawRC(TModeReg,0x8D);
         //WriteRawRC(RegRxThreshold, 0xff);
		 WriteRawRC(TPrescalerReg,0x3E);
		 delay_us(1000);
		 PcdAntennaOn();
	 } 
	 else 
	 { 
		 return -1; 
	 }
   
   return MI_OK;
}

/*****************************************************
** @函数名：ReadRawRC
** @描述  ：读RC632寄存器
** @参数  ：Address[IN]:寄存器地址
** @返回值：读出的值
*****************************************************/
uint8_t ReadRawRC(uint8_t Address)
{
  uint8_t ucAddr;
  uint8_t ucResult=0;
    
	CLR_SPI_SDA;
  ucAddr = ((Address<<1)&0x7E)|0x80;
	
	SPIWriteByte(ucAddr);
	ucResult=SPIReadByte();
	SET_SPI_SDA;
  return ucResult;
}

/*****************************************************
** @函数名：WriteRawRC
** @描述  ：写RC632寄存器
** @参数  ：Address[IN]:寄存器地址
**          value[IN]:写入的值
** @返回值：无
*****************************************************/
void WriteRawRC(uint8_t Address, uint8_t value)
{  
  uint8_t ucAddr;
	
	CLR_SPI_SDA;
  ucAddr = ((Address<<1)&0x7E);

	SPIWriteByte(ucAddr);
	SPIWriteByte(value);
	SET_SPI_SDA;
}

/*****************************************************
** @函数名：SetBitMask
** @描述  ：置RC522寄存器位
** @参数  ：reg[IN]:寄存器地址
**          mask[IN]:置位值
** @返回值：无
*****************************************************/
void SetBitMask(uint8_t reg,uint8_t mask)  
{
	int8_t tmp = 0x0;
	tmp = ReadRawRC(reg);
	WriteRawRC(reg,tmp | mask);  // set bit mask
}

/*****************************************************
** @函数名：ClearBitMask
** @描述  ：清RC522寄存器位
** @参数  ：reg[IN]:寄存器地址
**          mask[IN]:清位值
** @返回值：无
*****************************************************/
void ClearBitMask(uint8_t reg,uint8_t mask)  
{
	int8_t tmp = 0x00;
	tmp = ReadRawRC(reg);
	WriteRawRC(reg, tmp & ~mask);  // clear bit mask
} 

//*************************************************************************
// 函数名	：PcdComMF522  
// 描述		：通过RC522和ISO14443卡通讯
// 入口		：Command[IN]:RC522命令字
// 			  pDataIn[IN]:通过RC522发送到卡片的数据
// 			  InLenByte[IN]:发送数据的字节长度
// 			  *pOutLenBit[OUT]:返回数据的位长度
// 出口		：pDataOut[OUT]:接收到的卡片返回数据
// 返回		：无
//*************************************************************************
int8_t PcdComMF522(uint8_t Command, uint8_t *pDataIn, uint8_t InLenByte, uint8_t *pDataOut, uint16_t  *pOutLenBit)
{
    int8_t status = MI_ERR;
    uint8_t irqEn   = 0x00;
    uint8_t waitFor = 0x00;
    uint8_t lastBits;
    static uint8_t n;
    static uint16_t i;
    switch(Command)
    {
       case PCD_AUTHENT:
          irqEn   = 0x12;
          waitFor = 0x10;
          break;
       case PCD_TRANSCEIVE:
          irqEn   = 0x77;
          waitFor = 0x30;							// 接受到数据及命令执行完毕
          break;
       default:
         break;
    }
    WriteRawRC(ComIEnReg,irqEn|0x80);			    // 容许除定时器中断请求以为得所有中断请求
    ClearBitMask(ComIrqReg,0x80);					// 屏蔽位清除
    WriteRawRC(CommandReg,PCD_IDLE);				// 取消当前命令
    SetBitMask(FIFOLevelReg,0x80);					// 清除FIFO中的读写指针

    for (i=0; i<InLenByte; i++)
    {   
		WriteRawRC(FIFODataReg, pDataIn[i]);    		//数据写入FIFO
	}
    WriteRawRC(CommandReg, Command);					//写入命令,将缓冲区中的数据发送到天线,并激活自动接收器
   
    if (Command == PCD_TRANSCEIVE)						//如果命令为0C
    {    
		SetBitMask(BitFramingReg,0x80);  				//相当于启动发送STARTSENG
	}
    i = 10000;											//根据时钟频率调整，操作M1卡最大等待时间=600,操作CPU卡最大等待时间=1200
	do 
    {
         n = ReadRawRC(ComIrqReg);						//读取中断标志,检查数据返回
         i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitFor));		// 定时器未超时,没有错误,0x01,0x30
    ClearBitMask(BitFramingReg,0x80);					// 相当于清除发送STARTSENG
    if (i!=0)											// 定时时间到，i，没有递减到0
    {    
         if(!(ReadRawRC(ErrorReg)&0x1B))				// 判断有无出现错误标志	 Buffer溢出,位冲突,接收CRC错误,奇偶校验错误,
         {	
             status = MI_OK;							// 初始化状态 
		 	 if (n & irqEn & 0x01)						// 若是PCD_TRANSCEIVE, irq = 0x77,  定时器为0中断产生,定时器为0时为错误
			 {   
				status = MI_NOTAGERR;   				// 搜索不到卡
			 }
             if (Command == PCD_TRANSCEIVE)				// 如果是发送接收指令
             { 
                n = ReadRawRC(FIFOLevelReg);			// 读取接收到的数据的字节数
              	lastBits = ReadRawRC(ControlReg) & 0x07;// 2-0:RxLastBits,显示接收到最后一个字节的位数
                if (lastBits)							// 若该位为0，最后整个字节有效
                {   
					*pOutLenBit = (n-1)*8 + lastBits;   //pOutLenBit记录总共收到的位数
				}
                else
                {   
					*pOutLenBit = n*8;   				//接收完整位数
				}
                if (n == 0)								//假如没有中断产生
                {   
					n = 1;   							//n置1
				}
                if (n > MAXRLEN)						// 一次最大能接受到的字节数
                {   
					n = MAXRLEN;   						//超出最大长度,只接受最大长度的值
				}
                for (i=0; i<n; i++)
                {   
					pDataOut[i] = ReadRawRC(FIFODataReg); //从FIFO读取数据   
				}
            }
         }
         else
         {   
		 	status = MI_ERR;   							//有错误
		 }
   }
   SetBitMask(ControlReg,0x80);           	//停止定时器
   WriteRawRC(CommandReg,PCD_IDLE); 		//清空指令
   return status;								//返回状态
} 

/*****************************************************
** @函数名：PcdAntennaOn
** @描述  ：开启天线
**          每次启动或关闭天线发射之间应至少有1ms的间隔
** @参数  ：无
** @返回值：无
*****************************************************/
void PcdAntennaOn(void)
{
	uint8_t i;
	
	i = ReadRawRC(TxControlReg);
	if (!(i & 0x03))
	{
		SetBitMask(TxControlReg, 0x03);
	}
}

/*****************************************************
** @函数名：PcdAntennaOff
** @描述  ：关闭天线
**          清除屏蔽位为0 该代码中清除 TxControlReg 
**          寄存器的0x03位 即低两位	
** @参数  ：无
** @返回值：无
*****************************************************/
void PcdAntennaOff(void)
{
	ClearBitMask(TxControlReg, 0x03);
}

