/*----------------------------------------------------------------------------*/
/* PROGRAM NM : Iso8583toOasis.c                                              */
/* PROCESS NM : smartifd                                                      */
/* SUBJECT    : DP ISO8583 -> OASIS Conversion PROGRAM                        */
/* Programmer : ChangKoo Lee                                                  */
/* Target User: Shinhan Vietnam BANK                                          */
/* Description: Copyright (C) 2008 NAUTILUS HYOSUNG. Ltd                      */
/*              All rights Reserved                                           */
/*----------------------------------------------------------------------------*/
/* VERSION    DATE         NAME      COMMENT                                  */
/*----------------------------------------------------------------------------*/
/* V1.00      2008.07.13   CKLEE     FIRST CODING                             */
/*----------------------------------------------------------------------------*/
#include "defcom.h"
#include "dpifcom.h"
#include "trdef.h"
#include "isotrdef.h"
#include "nlogging.h"
// #include "hsmManagement.h"  
   
#define NUM_BUF             16
#define NUM_STRING_BUF      1024

/*----------------------------------------------------------------------------*/
/* FUNCTION : Iso8583toOasis                                                  */
/* SUBJECT  : Data Junction Main                                              */
/* ARGUMENT : gb : send/recv bit, data : send/recv data                       */
/* RETURN   : SUCCESS: NORMAL                                                 */
/*----------------------------------------------------------------------------*/
int Iso8583toOasis(gb, msgtype, pcode)
int gb;
char *msgtype;
char *pcode;
{
    SendLength = 0;                               /* Send Length */

    memset(SendBuffer, 0, sizeof(SendBuffer));    /* Send Buffer */
    memset(&HMERecv, 0, sizeof(HMERecv));         /* Host Message Edit Send */

    if (gb == 1)
    {   
        if( makeOasisAtmMsg(gb, msgtype, pcode) == ABNORMAL)
            return ABNORMAL; 
    }

    return NORMAL;
}

/*----------------------------------------------------------------------------*/
/* FUNCTION : makeOasisAtmMsg                                                 */
/* SUBJECT  : make ISO8583 Message                                            */
/* ARGUMENT : gb : send/recv bit, data : send/recv data                       */
/* RETURN   : SUCCESS: NORMAL                                                 */
/*----------------------------------------------------------------------------*/
int makeOasisAtmMsg(gb, msgtype, pcode)   
int  gb;          
char *msgtype;
char *pcode;
{
    int   Cnt = 0;
    int   Length1 = 0;     
    int   MaxBitMap = 0;      
    char  CardAccountNo[20]= {0x00, };
    char  track2[200]= {0x00, };
    char  ClearPin[20]= {0x00, };
    char  WorkClearKey[40]= {0x00, };     
    char  l_temp[1024] = {0x00, };

    char  szTmpBit[1024]; 
    char  szTmpBuff[1024];
    char  szTmpData[1024];

    char  tdate[16];
    char  seq[10];
    char  inAmount[30]={0x00, };
    char  outAmount[30]={0x00, };

    char  ic_temp[4000] = {0x00, };
    char  ic_temp_str[4000] = {0x00, };

    char  pinbuf[20] = {0x00, };
    char  pinpack[128] = {0x00, };
    int i = 0;

    memset(szTmpBit, 0, sizeof(szTmpBit));
    memset(szTmpBuff, 0, sizeof(szTmpBuff));
    memset(szTmpData, 0, sizeof(szTmpData));

    memset(&HMERecv, 0, sizeof(HMERecv));

    /* Recv Data Field Length */
    HMERecv.DataLength[0]  = 4;
    HMERecv.DataLength[1]  = MAXBITMAP/8;
    HMERecv.DataLength[2]  = 99;
    HMERecv.DataLength[3]  = 6;
    HMERecv.DataLength[4]  = 12;
    HMERecv.DataLength[5]  = 12;
    HMERecv.DataLength[6]  = 12;
    HMERecv.DataLength[7]  = 10;
    HMERecv.DataLength[9]  = 8;			// add kbh 20171019 Settlement Conversion Rate
	HMERecv.DataLength[10] = 8;			// add kbh 20171019 Cardholder Billing Conversion Rate
    HMERecv.DataLength[11] = 6;
    HMERecv.DataLength[12] = 6;
    HMERecv.DataLength[13] = 4;
    HMERecv.DataLength[14] = 4;
    HMERecv.DataLength[15] = 4;     
    HMERecv.DataLength[18] = 4;
    HMERecv.DataLength[19] = 3;
    HMERecv.DataLength[22] = 3;
    HMERecv.DataLength[23] = 3;			// add kbh 20170622 Card Sequence Number
    HMERecv.DataLength[25] = 2;
    HMERecv.DataLength[32] = 99;
    // HMERecv.DataLength[33] = 99;
    HMERecv.DataLength[35] = 99;
    HMERecv.DataLength[36] = 999;		// add kbh 20171019 Track-3 Data
    HMERecv.DataLength[37] = 12;
    HMERecv.DataLength[38] = 6;
    HMERecv.DataLength[39] = 2;
    HMERecv.DataLength[41] = 8;
    HMERecv.DataLength[42] = 15;
    HMERecv.DataLength[43] = 40;
    HMERecv.DataLength[44] = 25;
    HMERecv.DataLength[45] = 99;     
    HMERecv.DataLength[48] = 999;
    HMERecv.DataLength[49] = 3;
    HMERecv.DataLength[50] = 3;			// add kbh 20171016 Settlement Currency Code
	HMERecv.DataLength[51] = 3;			// add kbh 20171016 Cardholder Billing Currency Code
    HMERecv.DataLength[52] = 8;
    HMERecv.DataLength[53] = 16;     
    HMERecv.DataLength[54] = 999;
    HMERecv.DataLength[55] = 999;		// add kbh 20170622 Chip Data
    HMERecv.DataLength[60] = 999;		// chg kbh 20171019 Self - defined Field
	HMERecv.DataLength[62] = 99;		// add kbh 20171019 NAPAS Service Code
	HMERecv.DataLength[63] = 999;		// add kbh 20171019 Transaction Reference Number
    HMERecv.DataLength[90] = 42;
    HMERecv.DataLength[95] = 42;
	  HMERecv.DataLength[100] = 99; /* 2013.07.09 */
    HMERecv.DataLength[102] = 99;
    HMERecv.DataLength[103] = 99;     
	  HMERecv.DataLength[104] = 999; /* 2013.07.09 */       
    HMERecv.DataLength[105] = 999; 
    HMERecv.DataLength[120] = 999;
       
    getDate(tdate);       
     
    /* 0 Transaction Kind */
    memcpy(HMERecv.Data[0], &RecvBuffer[Cnt], HMERecv.DataLength[0]);
    memcpy(MsgData_INT.msgtype, &RecvBuffer[Cnt], HMERecv.DataLength[0]);
    Cnt += HMERecv.DataLength[0];

    /* 1 Bitmap */
    memcpy(HMERecv.Data[1], &RecvBuffer[Cnt], 1);

    if (HMERecv.Data[1][0] & 0x80 )    
    {    
        MaxBitMap = 128;
    }
    else
    {
        MaxBitMap = 64;
    }
    
    LOGGER(LOG_DEBUG, " MaxBitMap [%d]", MaxBitMap);

    HMERecv.DataLength[1] = MaxBitMap/8;
    
    /* 1 Bitmap */
    memcpy(HMERecv.Data[1], &RecvBuffer[Cnt], HMERecv.DataLength[1]);

    memset(szTmpBit, 0, sizeof(szTmpBit));
    memcpy(szTmpBit, HMERecv.Data[1], HMERecv.DataLength[1]);

    for (i = 1; i < MaxBitMap+1; i++)
    {
        char BT = 0x01;

        if ((szTmpBit[(i-1)/8] & (BT<<(7-(i-1)%8))) >> (7-(i-1)%8))
        {
            if (i == 1)  continue;

            if (99 == HMERecv.DataLength[i])
            {    
                Length1 = 0;
                memset(szTmpBuff, 0, sizeof(szTmpBuff));

                if (MaxBitMap == 128)  memcpy(szTmpBuff, RecvBuffer + Cnt + 16, 2);
                else                   memcpy(szTmpBuff, RecvBuffer + Cnt + 8, 2);

                Cnt += 2;
                memcpy(HMERecv.Data[i], szTmpBuff, 2);            /* LL */
                Length1 = atoi(szTmpBuff);

                if (MaxBitMap == 128)  memcpy(HMERecv.Data[i] + 2, &RecvBuffer[Cnt+16], Length1);
                else                   memcpy(HMERecv.Data[i] + 2, &RecvBuffer[Cnt+8], Length1);

                HMERecv.DataLength[i] = 2 + Length1;
                Cnt += Length1;
                LOGGER(LOG_DEBUG, " #%d: 99 [%d][%s]", i, HMERecv.DataLength[i], HMERecv.Data[i]);

                continue;
            }
            else
            if (999 == HMERecv.DataLength[i])
            {
                Length1 = 0;
                memset(szTmpBuff, 0, sizeof(szTmpBuff));
                if (MaxBitMap == 128)  memcpy(szTmpBuff, RecvBuffer + Cnt+16, 3);
                else                   memcpy(szTmpBuff, RecvBuffer + Cnt+8, 3);

                Cnt += 3;
                memcpy(HMERecv.Data[i], szTmpBuff, 3);            /* LLL */
                Length1 = atoi(szTmpBuff);

                
                if( i == 55 ) // IC Chip Card  
                {
                    memset( ic_temp , 0x00, sizeof(ic_temp));   
                    memset( ic_temp_str , 0x00, sizeof(ic_temp));

                    if (MaxBitMap == 128){
                        bit2str(&RecvBuffer[Cnt+16] , ic_temp, Length1);
                        fc_2bitstrTO16bitstr( ic_temp, ic_temp_str, strlen(ic_temp));
                        memcpy(HMERecv.Data[i] + 3, ic_temp_str, strlen(ic_temp_str));
                    }
                    else{
                          
                        bit2str(&RecvBuffer[Cnt+8] , ic_temp, Length1);
                        //memcpy(HMERecv.Data[i] + 3, ic_temp, Length1);
                        fc_2bitstrTO16bitstr( ic_temp, ic_temp_str, strlen(ic_temp));
                                memcpy(HMERecv.Data[i] + 3, ic_temp_str, strlen(ic_temp_str));
                    }	
                    LOGGER(LOG_DEBUG, "#%d: 999 ic_temp  [%d][%s",i, HMERecv.DataLength[i], ic_temp);
                }
                else
                {
                    if (MaxBitMap == 128)  memcpy(HMERecv.Data[i] + 3, &RecvBuffer[Cnt+16], Length1);
                    else                   memcpy(HMERecv.Data[i] + 3, &RecvBuffer[Cnt+8], Length1);
                    if (MaxBitMap == 128)
                        LOGGER(LOG_DEBUG, "#%d: 999 [%d][%s][%s",i, HMERecv.DataLength[i], HMERecv.Data[i] ,RecvBuffer+Cnt+16);
                    else 
                        LOGGER(LOG_DEBUG, "#%d: 999 [%d][%s][%s", i, HMERecv.DataLength[i], HMERecv.Data[i] ,RecvBuffer+Cnt+8);
                }

                HMERecv.DataLength[i] = 3 + Length1;
                Cnt += Length1;
                LOGGER(LOG_DEBUG, " #%d: 999 [%d][%s]", i, HMERecv.DataLength[i], HMERecv.Data[i]);
                continue;
            }

            if (MaxBitMap == 128)  memcpy(HMERecv.Data[i], &RecvBuffer[Cnt+16], HMERecv.DataLength[i]);
            else                   memcpy(HMERecv.Data[i], &RecvBuffer[Cnt+8], HMERecv.DataLength[i]);

            Cnt += HMERecv.DataLength[i];
            if (i == 52)
            {   /* PIN */
                LOGGER(LOG_DEBUG, " #%d: [%d][********]", i, HMERecv.DataLength[i]);
            }
            else
            {
                LOGGER(LOG_DEBUG, " #%d: [%d][%s]", i, HMERecv.DataLength[i], HMERecv.Data[i]);
            }

            if (i == 39)
            {
                memcpy(MsgData_INT.bit_39, HMERecv.Data[i], sizeof(MsgData_INT.bit_39));
            }
        }
    }
    LOGGER(LOG_DEBUG, " msgtype [%s]", msgtype);

    /* 2 PAN */
    memcpy(CardAccountNo, &HMERecv.Data[2][2], strlen(&HMERecv.Data[2][2]));
    if (strlen(HMERecv.Data[2]) > 0)
    {
        memcpy(MsgData_INT.bit_2, &HMERecv.Data[2][2], strlen(&HMERecv.Data[2][2]));
    }
    else
    {
        memcpy(track2, &HMERecv.Data[35][2], strlen(&HMERecv.Data[35][2]));
        trimString(track2);
        getPAN(track2);
        memcpy(MsgData_INT.bit_2, track2, strlen(track2));
    }

    if (memcmp(MsgData_INT.bit_2, "00000", 5) == 0)
    {
        LOGGER(LOG_DEBUG, " track2->pan start->HOST [%.*s]", sizeof(MsgData_INT.bit_2), MsgData_INT.bit_2);
        memset(track2, 0x00, sizeof(track2));
        memcpy(track2, &HMERecv.Data[35][2], strlen(&HMERecv.Data[35][2]));
        trimString(track2);

        if (strlen(track2) > 0)
        {
            getPAN2(track2);
            memset(MsgData_INT.bit_2, 0x20, sizeof(MsgData_INT.bit_2));
            memcpy(MsgData_INT.bit_2, track2, strlen(track2));
        }
    }
    
    LOGGER(LOG_DEBUG, " CardAccountNo [%s]", CardAccountNo);

    /* 3 Processing Code */
    memcpy(MsgData_INT.bit_3, HMERecv.Data[3], strlen(HMERecv.Data[3]));
    memcpy(pcode, HMERecv.Data[3], strlen(HMERecv.Data[3]));

    /* 4 Transaction Amount */
    memset(MsgData_INT.bit_4, 0x20, sizeof(MsgData_INT.bit_4));

    if (strlen(HMERecv.Data[4]) > 1)
    {
        memcpy(MsgData_INT.bit_4, HMERecv.Data[4], strlen(HMERecv.Data[4]));
    }

    /* 7 Transmission Date and and Time - MMDDhhmmss ??? */
    memcpy(MsgData_INT.bit_7, HMERecv.Data[7], strlen(HMERecv.Data[7]));

    /* 11 System Trace Audit Number */
    memcpy(MsgData_INT.bit_11, HMERecv.Data[11], strlen(HMERecv.Data[11]));
 
    /* 12 Local Transaction Time */
    memcpy(MsgData_INT.bit_12, HMERecv.Data[12], strlen(HMERecv.Data[12]));

    /* 13 Local Transaction Date */
    memcpy(MsgData_INT.bit_13, HMERecv.Data[13], strlen(HMERecv.Data[13]));

	/* 14 Card Expiry Date */ /* 2013.07.09 */
	if((!memcmp(msgtype, "0200", 4) || !memcmp(msgtype, "0420", 4)) &&
		(!memcmp(pcode, "43", 2) || !memcmp(pcode, "91", 2)))	
	{
		if (strlen(HMERecv.Data[14]) > 0)
		{
			memcpy(MsgData_INT.bit_14, HMERecv.Data[14], strlen(HMERecv.Data[14]));
		}
	}

    /* 15 Settlement Date */
   memcpy(MsgData_INT.bit_15, HMERecv.Data[15], strlen(HMERecv.Data[15]));
   
    /* 18 Merchant Type : "6011" */
    /*memcpy(OasisMsg.merchantype, "6011", 4);*/
    memcpy(MsgData_INT.bit_18, HMERecv.Data[18], strlen(HMERecv.Data[18]));

    /* 19 Acquiring Institution Country Code */ /* 2013.07.09 */
	if((!memcmp(msgtype, "0200", 4) || !memcmp(msgtype, "0420", 4)) &&
		(!memcmp(pcode, "43", 2) || !memcmp(pcode, "91", 2)))	
	{
		memcpy(MsgData_INT.bit_19, HMERecv.Data[19], strlen(HMERecv.Data[19]));
	}
	
    /* 22 Point of Service Code */ /* 2013.07.09 */
	if((!memcmp(msgtype, "0200", 4) || !memcmp(msgtype, "0420", 4)) &&
		(!memcmp(pcode, "43", 2) || !memcmp(pcode, "91", 2)))	
	{
		memcpy(MsgData_INT.bit_22, HMERecv.Data[22], strlen(HMERecv.Data[22]));
	}
    /* 23 Ic card Serial Number */ // add kbh. 20170622 
	memcpy(MsgData_INT.atm_card_ser, HMERecv.Data[23], strlen(HMERecv.Data[23]));
    /* 25 Point of Service Condition Code */ /* 2013.07.09 */
	if((!memcmp(msgtype, "0200", 4) || !memcmp(msgtype, "0420", 4)) &&
		(!memcmp(pcode, "43", 2) || !memcmp(pcode, "91", 2)))	
	{
		memcpy(MsgData_INT.bit_25, HMERecv.Data[25], strlen(HMERecv.Data[25]));
	}

    /* 32 Acquire Institution ID Code */
    memcpy(MsgData_INT.bit_32, &HMERecv.Data[32][2], strlen(&HMERecv.Data[32][2]));

    /* 33 Forwarding Institution ID */ /* 2013.07.09 */
	if((!memcmp(msgtype, "0200", 4) || !memcmp(msgtype, "0420", 4)) &&
		(!memcmp(pcode, "43", 2) || !memcmp(pcode, "91", 2)))	
	{
		if (strlen(&HMERecv.Data[33][2]) > 0)
		{
			memcpy(MsgData_INT.bit_33, &HMERecv.Data[33][2], strlen(&HMERecv.Data[33][2]));
		}
	}
	
    /* 35 Track2 Data */
    memcpy(MsgData_INT.bit_35, &HMERecv.Data[35][2], strlen(&HMERecv.Data[35][2]));

    /* 37 Retrieval Reference Number ??? */
    memcpy(MsgData_INT.bit_37, HMERecv.Data[37], strlen(HMERecv.Data[37]));
    /* 38: Authorization Identification Response ??? */  
    memcpy(MsgData_INT.bit_38, HMERecv.Data[38], strlen(HMERecv.Data[38]));
    /* 39 Response Code */
    memcpy(MsgData_INT.bit_39, HMERecv.Data[39], strlen(HMERecv.Data[39]));

    /* 41 Card Acceptor Terminal ID */
    memcpy(MsgData_INT.bit_41, HMERecv.Data[41], strlen(HMERecv.Data[41]));

    /* 42 Card Acceptor ID Code */
    memcpy(MsgData_INT.bit_42, HMERecv.Data[42], strlen(HMERecv.Data[42]));

    memcpy(MsgData_INT.bit_43, HMERecv.Data[43], strlen(HMERecv.Data[43]));

    /* 48 Additional Data */ /* 2013.07.09 */
	if((!memcmp(msgtype, "0200", 4) || !memcmp(msgtype, "0420", 4)) &&
		(!memcmp(pcode, "43", 2) || !memcmp(pcode, "91", 2)))
	{
		if (strlen(&HMERecv.Data[48][3]) > 0)
		{
			memcpy(MsgData_INT.bit_48, &HMERecv.Data[48][3], strlen(&HMERecv.Data[48][3]));
		}
	}

    /* 49 Transaction Currency Code */
    memcpy(MsgData_INT.bit_49, HMERecv.Data[49], strlen(HMERecv.Data[49]));
    if (memcmp(msgtype, "0200", 4) == 0)
    {
        memcpy(WorkClearKey, DesClearKey, strlen(DesClearKey));
        memset(MsgData_INT.bit_52, 0x20, sizeof(MsgData_INT.bit_52));
		 setSmartDecrypt(HMERecv.Data[52], CardAccountNo, WorkClearKey, ClearPin);

        memcpy(MsgData_INT.bit_52, ClearPin, strlen(ClearPin));
        LOGGER(LOG_DEBUG, " SHB [%.*s]", sizeof(MsgData_INT.bit_52), MsgData_INT.bit_52); 
    }

    if (memcmp(HMERecv.Data[54], "020", 3) == 0)
    {
        memcpy(MsgData_INT.bit_54, &HMERecv.Data[54][3], strlen(&HMERecv.Data[54][3]));
    }
    else
    {
        memcpy(MsgData_INT.bit_54, &HMERecv.Data[54][3+20], strlen(&HMERecv.Data[54][3+20]));
    }
    /* ?Ü¾?Á¤???? bit_104?? ??Ã¼ ???? . add 20180124 */
	memcpy(MsgData_INT.bit_104, &HMERecv.Data[54][3], strlen(&HMERecv.Data[54][3]));

	// add kbh 20170622 bit_55 : IC Á¤??
    memcpy(MsgData_INT.atm_mdm_d, "CD", sizeof(MsgData_INT.atm_mdm_d) );	// default ?? "CD"?? ?????Ï°? IC ?? ???ì¸¸ "IC"?? Set.
    if (strlen(&HMERecv.Data[55][3]) > 0)
    {
        //memcpy(MsgData.atm_ic_card_data_ctt, &HMERecv.Data[55][3], strlen(&HMERecv.Data[55][3]));
        memcpy(MsgData_INT.atm_ic_card_data_ctt, &HMERecv.Data[55][0], strlen(&HMERecv.Data[55][0]));
        memcpy(MsgData_INT.atm_mdm_d, "IC", sizeof(MsgData_INT.atm_mdm_d) );
	    memcpy(MsgData_INT.atm_card_ser, HMERecv.Data[23], strlen(HMERecv.Data[23]));
    }

    // 2019.08.14 Luke Added 
    // set Card Type ( parsing the Track2 data ) 
    if ( strlen(&HMERecv.Data[35][2]) > 0 )
    {
        LOGGER(LOG_DEBUG, "set card type start  : [%s", MsgData_INT.bit_35);
        char *ptrl = strchr(MsgData_INT.bit_35, '='); 
        if(ptrl){
	        if(ptrl[5] == '2' || ptrl[5] == '6'){
		        memcpy(MsgData_INT.atm_mdm_d, "IC", sizeof(MsgData_INT.atm_mdm_d) );
	        }
        }
        LOGGER(LOG_DEBUG, "set card type end card type=  : [%s",  MsgData_INT.atm_mdm_d );
    } // End of If 

    // add kbh 20171020 bit_62      // NAPAS Service Code
    if (strlen(&HMERecv.Data[62][2]) > 0)
    {
		memset(l_temp, 0x20, sizeof(l_temp) -1);
		sprintf(l_temp, "%-10s", &HMERecv.Data[62][2] );
        LOGGER(LOG_DEBUG, "62 l_temp : [%s", l_temp );
        memcpy(&MsgData_INT.filler[0], l_temp, 10);
    }
	else
	{
		memset(&MsgData_INT.filler[0], 0x20, 10);
    }

    // add kbh 20171020 bit_60      // Reserved
    if (strlen(&HMERecv.Data[60][3]) > 0)
    {
		memset(l_temp, 0x20, sizeof(l_temp) -1);
        sprintf(l_temp, "%-50s", &HMERecv.Data[60][3] );
        LOGGER(LOG_DEBUG, "60 l_temp : [%s", l_temp );
        memcpy(&MsgData_INT.filler[10], l_temp, 50);
    }
	else
	{
		memset(&MsgData_INT.filler[10], 0x20, 50);
    }

    // add kbh 20171020 bit_63      // Transaction Reference Number
    if (strlen(&HMERecv.Data[63][3]) > 0)
    {
		memset(l_temp, 0x20, sizeof(l_temp) -1);
        sprintf(l_temp, "%-16s", &HMERecv.Data[63][3] );
        LOGGER(LOG_DEBUG, "63 l_temp : [%s", l_temp );
        memcpy(&MsgData_INT.filler[60], l_temp, 16);
    }
	else
	{
		memset(&MsgData_INT.filler[50], 0x20, 16);
    }
	// logging(logfp, "%s] MsgData.filler : [%.*s", __func__, sizeof(MsgData.filler), MsgData.filler);

    /* 90 Original Data Elements  : AN42 */
    /* ???Ò°Å·??? ?Ýµ??? set */
    /* 1. Original message type                  (4 bytes) : 0200 */
    /* 2. Original trace number                    (6 bytes) : bitmap 11 */
    /* 3. Original transaction date/time        (10 bytes) : bitmap 7 */
    /* 4. Original accquirer ID                      (11 bytes) : bitmap 32 */
    memcpy(MsgData_INT.bit_90, HMERecv.Data[90], strlen(HMERecv.Data[90]));

    /* 100 BEN ID */ /* 2013.07.09 */
	if((!memcmp(msgtype, "0200", 4) || !memcmp(msgtype, "0420", 4)) &&
		(!memcmp(pcode, "43", 2) || !memcmp(pcode, "91", 2)))	
	{
		if (strlen(&HMERecv.Data[100][2]) > 0)
		{
			memcpy(MsgData_INT.bit_100, &HMERecv.Data[100][2], strlen(&HMERecv.Data[100][2]));
		}
	}
	
    memcpy(MsgData_INT.bit_102, &HMERecv.Data[102][2], strlen(&HMERecv.Data[102][2]));

    memcpy(MsgData_INT.bit_103, &HMERecv.Data[103][2], strlen(&HMERecv.Data[103][2]));

    // add kbh 20171020 bit_5   // Settlement Amount
	int len_5 = strlen(HMERecv.Data[5]);
	if(len_5 > 0) 
	{
		memcpy(&MsgData_INT.atm_ord_ctt[910], HMERecv.Data[5], 12);
	}
	// logging(logfp, "%s] 1. atm_ord_ctt : [%.*s] #5 : [%s], l_temp : [%s", __func__, sizeof(MsgData.atm_ord_ctt), MsgData.atm_ord_ctt, HMERecv.Data[5], l_temp);

	// add kbh 20171020 bit_6   // Cardholder billing Amount
	int len_6 = strlen(HMERecv.Data[6]);
	if(len_6 > 0) 
	{
		memcpy(&MsgData_INT.atm_ord_ctt[922], HMERecv.Data[6], 12);
	}
	// logging(logfp, "%s] 2. atm_ord_ctt : [%.*s] #6 : [%s], l_temp : [%s", __func__, sizeof(MsgData.atm_ord_ctt), MsgData.atm_ord_ctt, HMERecv.Data[6], l_temp);

	// add kbh 20171020 bit_9   // Settlement Conversion Rate
	int len_9 = strlen(HMERecv.Data[9]);
	if(len_9 > 0) 
	{
		memset(l_temp, 0x20, sizeof(l_temp) - 1 );
		sprintf(l_temp, "%-8s", HMERecv.Data[9] );
        LOGGER(LOG_DEBUG, "9 l_temp : [%s", l_temp);
		memcpy(&MsgData_INT.atm_ord_ctt[934], HMERecv.Data[9], 8);
	}
	// logging(logfp, "%s] 3. atm_ord_ctt : [%.*s] #9 : [%s], l_temp : [%s", __func__, sizeof(MsgData.atm_ord_ctt), MsgData.atm_ord_ctt, HMERecv.Data[9], l_temp);

    // add kbh 20171020 bit_10  // Settlement Conversion Rate
	int len_10 = strlen(HMERecv.Data[10]);
	if(len_10 > 0)
	{
		memset(l_temp, 0x20, sizeof(l_temp) - 1 );
		sprintf(l_temp, "%-10s", HMERecv.Data[10] );
        LOGGER(LOG_DEBUG, "10 l_temp : [%s", l_temp);
    	memcpy(&MsgData_INT.atm_ord_ctt[942], HMERecv.Data[10], 8);
	}
	// logging(logfp, "%s] 4. atm_ord_ctt : [%.*s] #10 : [%s], l_temp [%s", __func__, sizeof(MsgData.atm_ord_ctt), MsgData.atm_ord_ctt, HMERecv.Data[10], l_temp);

    // add kbh 20171020 bit_50  // Settlement Currency Code
	int len_50 = strlen(HMERecv.Data[50]);
	if(len_50 > 0)
	{
		memset(l_temp, 0x20, sizeof(l_temp) - 1 );
		sprintf(l_temp, "%-3s", HMERecv.Data[50] );
        LOGGER(LOG_DEBUG, "50 l_temp : [%s", l_temp);
    	memcpy(&MsgData_INT.atm_ord_ctt[950], HMERecv.Data[50], 3);
	}
	// logging(logfp, "%s] 5. atm_ord_ctt : [%.*s] #10 : [%s", __func__, sizeof(MsgData.atm_ord_ctt), MsgData.atm_ord_ctt, HMERecv.Data[50]);

    // add kbh 20171020 bit_51  // cardholder billing Currency Code
	int len_51 = strlen(HMERecv.Data[51]);
	if(len_51 > 0)
	{
		memset(l_temp, 0x20, sizeof(l_temp) - 1 );
		sprintf(l_temp, "%-3s", HMERecv.Data[51] );
        LOGGER(LOG_DEBUG, "51 l_temp : [%s", l_temp);
    	memcpy(&MsgData_INT.atm_ord_ctt[953], HMERecv.Data[51], 3);
	}
    /* 104 Narration */ /* 2013.10.11 */
	if(!memcmp(pcode, "43", 2) || !memcmp(pcode, "91", 2))	
	{
		if (strlen(&HMERecv.Data[120][3]) > 0)
		{
			memcpy(MsgData_INT.bit_120, &HMERecv.Data[120][3], strlen(&HMERecv.Data[120][3]));
		}
	}

	if (memcmp(msgtype, "0200", 4) == 0 &&
		memcmp(pcode, "94", 2) == 0) /* 2013.07.09 */
    {
    	memset(WorkClearKey, 0x00, sizeof(WorkClearKey));
    	memset(ClearPin    , 0x00, sizeof(ClearPin)    );
        memcpy(WorkClearKey, DesClearKey, strlen(DesClearKey));
        memset(MsgData_INT.newpindata, 0x20, sizeof(MsgData_INT.newpindata));

        /* PIN Block Pack */
        memset(pinbuf , 0x00, sizeof(pinbuf) );
        memset(pinpack, 0x00, sizeof(pinpack));
        memcpy(pinbuf, &HMERecv.Data[105][3], strlen(&HMERecv.Data[105][3]));

        pack(pinbuf, pinpack);
        
        setSmartDecrypt(pinpack, CardAccountNo, WorkClearKey, ClearPin);

        memcpy(MsgData_INT.newpindata, ClearPin, strlen(ClearPin));
        LOGGER(LOG_DEBUG, " New SHB [%.*s]", sizeof(MsgData_INT.newpindata), MsgData_INT.newpindata); 
    }

    return NORMAL;
}

/*----------------------------------------------------------------------------*/
/* FUNCTION : oasisAmount                                                     */
/* SUBJECT  : make oasisAmount                                                */
/* ARGUMENT :                                                                 */
/* RETURN   : SUCCESS: NORMAL                                                 */
/*----------------------------------------------------------------------------*/
int oasisAmount(char *inAmount, char *outAmount)
{
    char amount[30];
    int  len=0;

    sprintf(amount, "%d", atoi(inAmount));
    if (strlen(amount) < 3)
    {
        sprintf(amount, "%03d", atoi(inAmount));
    }

    len=strlen(amount);
    sprintf(outAmount, "%.*s.%.2s", len-2, amount, &amount[len-2]);

    return NORMAL;
}

/*----------------------------------------------------------------------------*/
/* FUNCTION : recvNetworkMgmt                                                 */
/* SUBJECT  : make Network Management Message                                 */
/* ARGUMENT : gb : send/recv bit, data : send/recv data                       */
/* RETURN   : SUCCESS: NORMAL                                                 */
/*----------------------------------------------------------------------------*/
int recvNetworkMgmt(msgtype)
char *msgtype;
{
    int   Cnt = 0;
    int   Length1 = 0;
    int   BitMapLength = 0;
    char  szTmpBit[1024];
    char  szTmpBuff[1024];
    char  szTmpData[1024];

    char  rst16[40];
    char  dat16[40];
    char  display_buf[NUM_STRING_BUF];
    char  key16[NUM_BUF*5];
    char  tmpack[128];
    int i = 0;

    LOGGER(LOG_DEBUG, " recvNetworkMgmt() [%s]", msgtype);

    memset(&HMERecv, 0, sizeof(HMERecv));
    memset(&HMESend, 0, sizeof(HMESend));

    /* Recv Data Field Length */
    HMERecv.DataLength[0]  = 4;
    HMERecv.DataLength[1]  = MAXBITMAP/8;
    HMERecv.DataLength[7]  = 10;
    HMERecv.DataLength[11] = 6;
    HMERecv.DataLength[32] = 99;
    HMERecv.DataLength[39] = 2;
    HMERecv.DataLength[70] = 3;

    BitMapLength = HMERecv.DataLength[1];

    /* 0 Transaction Kind */
    memcpy(HMERecv.Data[0], &RecvBuffer[Cnt], HMERecv.DataLength[0]);
    memcpy(MsgData_INT.msgtype, &RecvBuffer[Cnt], HMERecv.DataLength[0]);
    Cnt += HMERecv.DataLength[0];
/*
logging(logfp, " DBUG] #0 :[%d][%s]", HMERecv.DataLength[0], HMERecv.Data[0]);
*/
    /* 1 Bitmap */
    memcpy(HMERecv.Data[1], &RecvBuffer[Cnt], HMERecv.DataLength[1]);
/*
logging(logfp, " DBUG] #1 :[%d][%s]", HMERecv.DataLength[1], HMERecv.Data[1]);
*/

    memset(szTmpBit, 0, sizeof(szTmpBit));
    memcpy(szTmpBit, HMERecv.Data[1], HMERecv.DataLength[1]);
/*
logging(logfp, " DBUG] #szTmpBit :[%d][%s]", strlen(szTmpBit), szTmpBit);
*/

    for (i = 1; i < MAXBITMAP; i++)
    {
        char BT = 0x01;

        if ((szTmpBit[(i-1)/8] & (BT<<(7-(i-1)%8))) >> (7-(i-1)%8))
        {
            if (i == 1)  continue;

            if (99 == HMERecv.DataLength[i])
            {
                Length1 = 0;
                memset(szTmpBuff, 0, sizeof(szTmpBuff));
                memcpy(szTmpBuff, RecvBuffer + Cnt + BitMapLength, 2);
                Cnt += 2;
                memcpy(HMERecv.Data[i], szTmpBuff, 2);            /* LL */
                Length1 = atoi(szTmpBuff);
                memcpy(HMERecv.Data[i] + 2, &RecvBuffer[Cnt+BitMapLength], Length1);
                HMERecv.DataLength[i] = 2 + Length1;
                Cnt += Length1;
                continue;
            }
            else
            if (999 == HMERecv.DataLength[i])
            {
                Length1 = 0;
                memset(szTmpBuff, 0, sizeof(szTmpBuff));
                memcpy(szTmpBuff, RecvBuffer + Cnt + BitMapLength, 3);
                Cnt += 3;
                memcpy(HMERecv.Data[i], szTmpBuff, 3);            /* LLL */
                Length1 = atoi(szTmpBuff);
                memcpy(HMERecv.Data[i] + 3, &RecvBuffer[Cnt+BitMapLength], Length1);
                HMERecv.DataLength[i] = 3 + Length1;
                Cnt += Length1;
/*
logging(logfp, "DBUG] #%d:[%d][%s]", i, HMERecv.DataLength[i], HMERecv.Data[i]);
*/
                continue;
            }

            memcpy(HMERecv.Data[i], &RecvBuffer[Cnt+BitMapLength], HMERecv.DataLength[i]);
            Cnt += HMERecv.DataLength[i];
/*
logging(logfp, "DBUG] #%d:[%d][%s]", i, HMERecv.DataLength[i], HMERecv.Data[i]);
*/
        }
    }

    HMESend.DataLength[0] = 4;
    memset(HMESend.Data[0], '0', HMESend.DataLength[0]);
    memcpy(HMESend.Data[0], msgtype, HMESend.DataLength[0]);

    /* 7 Transmission Date and and Time */
    HMESend.DataLength[7] = HMERecv.DataLength[7];
    memcpy(HMESend.Data[7], HMERecv.Data[7], HMESend.DataLength[7]);

    /* 11 System Trace Audit Number */
    HMESend.DataLength[11] = HMERecv.DataLength[11];
    memcpy(HMESend.Data[11], HMERecv.Data[11], HMESend.DataLength[11]);

    HMESend.DataLength[32] = HMERecv.DataLength[32];
    memcpy(HMESend.Data[32], HMERecv.Data[32], HMESend.DataLength[32]);

    /* 39 Response Code */
    HMESend.DataLength[39] = 2;
    memcpy(HMESend.Data[39], "00", 2);

    /* 70 Network Management Identification Code */
    HMESend.DataLength[70] = HMERecv.DataLength[70];
    memcpy(HMESend.Data[70], HMERecv.Data[70], HMESend.DataLength[70]);


    makeBitmap2();

    return NORMAL;
}

/*----------------------------------------------------------------------------*/
/* FUNCTION : makeBitmap2                                                     */
/* SUBJECT  : make Bitmap                                                     */
/* ARGUMENT :                                                                 */
/* RETURN   : SUCCESS: NORMAL                                                 */
/*----------------------------------------------------------------------------*/
int makeBitmap2()
{
    int   nTmp = 1;
    int   Cnt = 0;
    char  szTmpBit[1024];
    char  szTmpData[1024];
    char  sendlen[5] = {0x00, };
    int i = 0;

    SendLength = 0;

    LOGGER(LOG_DEBUG, " makeBitmap2...start...","");

    memset(szTmpBit, 0, sizeof(szTmpBit));
    memset(SendBuffer, 0, sizeof(SendBuffer));

    /* Length + External Header + Msgtype */
    Cnt += 4;

	SecondFlag = 1;
/*
logging(logfp, "DBUG] SecondFlag [%d]", SecondFlag);
*/
    for (i = 0; i < MAXBITMAP; i++)
    {
        if (HMESend.Data[i][0] != 0x00 || i == 1)
        {
            if (i > 0) {
                szTmpBit[(i - 1) / 8] |= (nTmp << (7 - (i - 1) % 8));
            }

            if (i > 1)
            {
                memcpy(&SendBuffer[Cnt+16], HMESend.Data[i], HMESend.DataLength[i]);
            }
            else
            {
                memcpy(&SendBuffer[Cnt], HMESend.Data[i], HMESend.DataLength[i]);
            }

            Cnt += HMESend.DataLength[i];
/*
logging(logfp, "DBUG] #%d: [%d][%s]", i, HMESend.DataLength[i], HMESend.Data[i]);
*/
        }
    }

/*logging(logfp, "DBUG] szTmpBit2 SecondFlag[%d], (SecondFlag+1)*8)=[%d], Cnt=[%d]", SecondFlag, (SecondFlag+1)*8, Cnt);*/
    HMESend.DataLength[1] = (SecondFlag+1)*8;
    memcpy(HMESend.Data[1], szTmpBit, HMESend.DataLength[1]);

    SendLength = Cnt + HMESend.DataLength[1] - 4;
    sprintf(sendlen, "%04d", SendLength);
/*    
logging(logfp, "DBUG]sendlen2 [%s]", sendlen);
*/
    memcpy(SendBuffer, sendlen, 4);

    memcpy(&SendBuffer[4], HMESend.Data[0], HMESend.DataLength[0]);
    memcpy(&SendBuffer[4+4], HMESend.Data[1], HMESend.DataLength[1]);
/*
logging(logfp, "LOGG] SendBuffer Hexdump");
hexdump(logfp, SendBuffer, SendLength+4);
*/
    LOGGER(LOG_DEBUG, " makeNetBitmap2 End....","");
    return NORMAL;
}

/*----------------------------------------------------------------------------*/
/* FUNCTION : Iso8583toOasis                                                  */
/* SUBJECT  : Data Junction Main                                              */
/* ARGUMENT : gb : send/recv bit, data : send/recv data                       */
/* RETURN   : SUCCESS: NORMAL                                                 */
/*----------------------------------------------------------------------------*/
int makeHostSendMsg(gb, trdat)
int gb;
char *trdat;
{
    memset(&MsgData, 0x20, sizeof(MsgData));
    memcpy(&MsgData, &MsgData_INT, sizeof(MsgData_INT) - 2);
    
    if (gb == 1)
    {   
        makeHostSendMsgAtm(trdat);
    }
    else if (gb == 2)
    {   
        makeHostSendMsgExtAtm(trdat);
    } 

    return NORMAL;
}

/*----------------------------------------------------------------------------*/
/* FUNCTION : makeHostSendMsgAtm                                              */
/* SUBJECT  : make Host Send Message                                           */
/* RETURN   : SUCCESS: NORMAL                                                 */
/*----------------------------------------------------------------------------*/
int makeHostSendMsgAtm(trdat)
char *trdat;
{
    int ix = 0;
    char tdate  [16] = {0x00, };
    char tmdate  [21] = {0x00, };
    char bufflen[10] = {0x00, };
    char track2[100] = {0x00, };
    char sztemp[100] = {0x00, };
    
    memset(&ChnlHead, 0x20, sizeof(ChnlHead));
    memset(&MsgHead , 0x20, sizeof(MsgHead ));
    memset(&MsgFix  , 0x20, sizeof(MsgFix  ));
    memset(&MsgSys  , 0x20, sizeof(MsgSys  ));
    memset(&MsgCom  , 0x20, sizeof(MsgCom  ));
    
    getDate(tdate);
    getMDate(tmdate);
    ix = sizeof(ChnlHead)+sizeof(MsgHead)+sizeof(MsgFix)+sizeof(MsgSys)+sizeof(MsgCom)+sizeof(MsgData)-sizeof(ChnlHead.chl_len);
    sprintf(ChnlHead.chl_len, "%08d", ix);     
    
    memcpy(ChnlHead.trxbrno, MsgData.bit_41, 4);
    memcpy(ChnlHead.trx_trmno, &MsgData.bit_41[4], 4);

    ChnlHead.core_yn[0] = '0';
    ChnlHead.in_u[0] = '0';   
    ChnlHead.out_u[0] = '0';  
    
    memset(ChnlHead.buf_address, '0', sizeof(ChnlHead.buf_address));
    memset(ChnlHead.buf_record, '0', sizeof(ChnlHead.buf_record));

    memcpy(ChnlHead.chan_u, "C0", 2);
    
    ChnlHead.multi_scr_cnt[0] = '0';
    memcpy(ChnlHead.result, MsgData.bit_39, sizeof(MsgData.bit_39));    
    if (ChnlHead.result[0] == 0x20)
    {
        memcpy(ChnlHead.result, "00", 2);
    }
    
    ChnlHead.chgr_g[0] = '0';
    ChnlHead.online_yn[0] = '0';
                                      
    ChnlHead.enc_yn[0] = '0';   
    ChnlHead.comp_yn[0] = '0';  
    ChnlHead.send_mapping_yn[0] = '0';
    ChnlHead.recv_mapping_yn[0] = '0';
    
    memset(ChnlHead.chl_filler, '0', sizeof(ChnlHead.chl_filler));
    
    ix = sizeof(MsgHead)+sizeof(MsgFix)+sizeof(MsgSys)+sizeof(MsgCom)+sizeof(MsgData)-sizeof(MsgHead.msg_len);

    loadFifotoMsghead();
    
    memcpy(ChnlHead.recv_svc_c, MsgHead.target_svc_trx_c, sizeof(MsgHead.target_svc_trx_c));   
    MsgHead.rqst_resp_g[0] = 'R';    
    sprintf(sztemp, "%.14s00", tdate);
    memcpy(MsgHead.out_msg_ilsi, sztemp, 16);
    memcpy(MsgFix.data_type, "90", 2);    

    ix = 0;
    ix = sizeof(MsgFix)-sizeof(MsgFix.data_type)-sizeof(MsgFix.data_len)+sizeof(MsgSys)+sizeof(MsgCom)+sizeof(MsgData)-2;
    sprintf(bufflen, "%08d", ix);
    memcpy(MsgFix.data_len, bufflen, strlen(bufflen));    

    memcpy(MsgFix.tran_code, MsgHead.target_svc_trx_c, sizeof(MsgHead.target_svc_trx_c));    
    memcpy(MsgFix.screen_id, MsgHead.scr_id, sizeof(MsgHead.scr_id));    
    memcpy(MsgFix.filler, "00", 2);
    memcpy(MsgCom.atm_typecode, "1101", 4);    
    memcpy(MsgCom.atm_program_ver, "2010012701", 10);   
    memcpy(track2, MsgData.bit_35, sizeof(MsgData.bit_35));    
    getPAN(track2);
    memcpy(MsgCom.cashcard_no, track2, strlen(track2));    
    MsgCom.atm_check_gb[0] = '2';    
    memcpy(MsgCom.atm_trcode, MsgData.bit_3, sizeof(MsgData.bit_3));    
    memcpy(MsgCom.amount, MsgData.bit_4, sizeof(MsgData.bit_4));
    memcpy(MsgCom.atm_biz_detail, "00", 2);    
    memcpy(MsgCom.atm_tr_seqno, &MsgData.bit_11[2], 4);    
    memcpy(MsgData.etx, "ZZ", 2);  

    ix = 0;
    memcpy(&trdat[ix] , &ChnlHead, sizeof(ChnlHead ));  
    ix += sizeof(ChnlHead);
    memcpy(&trdat[ix] , &MsgHead, sizeof(MsgHead ));  
    ix += sizeof(MsgHead);
    memcpy(&trdat[ix]  , &MsgFix, sizeof(MsgFix  ));  
    ix += sizeof(MsgFix);
    memcpy(&trdat[ix] , &MsgSys , sizeof(MsgSys  ));  
    ix += sizeof(MsgSys);
    memcpy(&trdat[ix] , &MsgCom , sizeof(MsgCom  ));  
    ix += sizeof(MsgCom);
    memcpy(&trdat[ix] , &MsgData, sizeof(MsgData ));  

    return NORMAL;
}

/*----------------------------------------------------------------------------*/
/* FUNCTION : makeHostSendMsgExtAtm                                                 */
/* SUBJECT  : make Host Send Message External ATM                                          */
/* RETURN   : SUCCESS: NORMAL                                                 */
/*----------------------------------------------------------------------------*/
int makeHostSendMsgExtAtm(trdat)
char *trdat;
{
    int ix = 0;
    char tdate  [16] = {0x00, };
    char tmdate  [21] = {0x00, };
    char bufflen[10] = {0x00, };
    char track2[100] = {0x00, };
    char sztemp[100] = {0x00, };
    
    memset(&ChnlHead, 0x20, sizeof(ChnlHead));
    memset(&MsgHead , 0x20, sizeof(MsgHead ));
    memset(&MsgFix  , 0x20, sizeof(MsgFix  ));
    memset(&MsgSys  , 0x20, sizeof(MsgSys  ));
    memset(&MsgCom  , 0x20, sizeof(MsgCom  ));
    
    getDate(tdate);
    getMDate(tmdate);
    ix = sizeof(ChnlHead)+sizeof(MsgHead)+sizeof(MsgFix)+sizeof(MsgSys)+sizeof(MsgCom)+sizeof(MsgData)-sizeof(ChnlHead.chl_len);
    sprintf(ChnlHead.chl_len, "%08d", ix);     
    
    memset(ChnlHead.trxbrno, '0', sizeof(ChnlHead.trxbrno));
    memset(ChnlHead.trx_trmno, '0', sizeof(ChnlHead.trx_trmno));
    ChnlHead.core_yn[0] = '0';   
    ChnlHead.in_u[0] = '0';      
    ChnlHead.out_u[0] = '0';      
    memset(ChnlHead.buf_address, '0', sizeof(ChnlHead.buf_address));
    memset(ChnlHead.buf_record, '0', sizeof(ChnlHead.buf_record));
    memcpy(ChnlHead.chan_u, "K0", 2);  
    
    ChnlHead.multi_scr_cnt[0] = '0';
    memcpy(ChnlHead.result, MsgData.bit_39, sizeof(MsgData.bit_39));    
    if (ChnlHead.result[0] == 0x20)
    {
        memcpy(ChnlHead.result, "00", 2);
    }
    
    ChnlHead.chgr_g[0] = '0';
    ChnlHead.online_yn[0] = '0';
                                      
    ChnlHead.enc_yn[0] = '0';   
    ChnlHead.comp_yn[0] = '0';  
    ChnlHead.send_mapping_yn[0] = '0';
    ChnlHead.recv_mapping_yn[0] = '0';
    
    memset(ChnlHead.chl_filler, '0', sizeof(ChnlHead.chl_filler));
    
    ix = sizeof(MsgHead)+sizeof(MsgFix)+sizeof(MsgSys)+sizeof(MsgCom)+sizeof(MsgData)-sizeof(MsgHead.msg_len);
    sprintf(MsgHead.msg_len, "%08d", ix);    
    
    sprintf(MsgHead.glob_id, "%.8sSMLSS%.17s", tdate, tmdate); 
    memcpy(MsgHead.step, "00", 2);   
	LOGGER(LOG_DEBUG, ">>>>> MsgData.bit_3[%.*s]", sizeof(MsgData.bit_3), MsgData.bit_3);
    if (memcmp(MsgData.bit_3, "30", 2) == 0) 
    {   
        memcpy(MsgHead.recv_svc_c, "SGAT9460A001", 12);
        memcpy(MsgHead.scr_id, "4594600000", 10);
    }
    else if (memcmp(MsgData.bit_3, "01", 2) == 0) 
    {   
        memcpy(MsgHead.recv_svc_c, "SGAT9461A001", 12);
        memcpy(MsgHead.scr_id, "4594610000", 10);       
    }
    else if (memcmp(MsgData.bit_3, "40", 2) == 0) 
    {   
        memcpy(MsgHead.recv_svc_c, "SGAT9463A001", 12);
        memcpy(MsgHead.scr_id, "4594630000", 10);       
    }
    else if (memcmp(MsgData.bit_3, "38", 2) == 0 || memcmp(MsgData.bit_3, "94", 2) == 0) 
    {   
        memcpy(MsgHead.recv_svc_c, "SGAT9464A001", 12);
        memcpy(MsgHead.scr_id, "4594640000", 10);       
    }
    else if (memcmp(MsgData.bit_3, "00", 2) == 0) 
    {   
        memcpy(MsgHead.recv_svc_c, "SGAT9461A001", 12);
        memcpy(MsgHead.scr_id, "4594610000", 10);       
    }
    else if (memcmp(MsgData.bit_3, "43", 2) == 0 || memcmp(MsgData.bit_3, "91", 2) == 0) 
    {   
        memcpy(MsgHead.recv_svc_c, "SGAT9463B001", 12);
        memcpy(MsgHead.scr_id, "4594630000", 10);       
    }
    
    memcpy(ChnlHead.recv_svc_c, MsgHead.recv_svc_c, sizeof(MsgHead.recv_svc_c));   
    MsgHead.rqst_resp_g[0] = 'S';    
    memcpy(MsgHead.chan_u, ChnlHead.chan_u, sizeof(ChnlHead.chan_u));    
    memcpy(MsgHead.grpco_c, "S001", 4);    
    sprintf(MsgHead.msg_snd_msg_ilsi, "%.14s00", tdate); 
    MsgHead.sync_g[0] = 'S';        
    MsgHead.eai_trx_g[0] = '0';     
    MsgHead.ttl_use_yn[0] = '0';    
    memset(MsgHead.fst_sttim, '0', sizeof(MsgHead.fst_sttim));
    
    memcpy(MsgHead.keep_times, "000", 3);
    MsgHead.snd_infc_g[0] = '1';         
    MsgHead.msg_snd_g[0] = '2';          
    memset(MsgHead.trm_set_brno, '0', sizeof(MsgHead.trm_set_brno));      
    memset(MsgHead.trm_set_trmno, '0', sizeof(MsgHead.trm_set_trmno));    
    char trm_tong_no[20];
    sprintf(trm_tong_no, "%.*s", sizeof(MsgData.bit_11), MsgData.bit_11);
    sprintf(MsgHead.trm_tong_no, "%08d", atoi(trm_tong_no));    
    memset(MsgHead.trxbrno, '0', sizeof(MsgHead.trxbrno));      
    memset(MsgHead.trx_trmno, '0', sizeof(MsgHead.trx_trmno));  
    memset(sztemp, 0x20, sizeof(sztemp));
    sprintf(sztemp,"ATM%05d", OpHwnno);
    memcpy(MsgHead.op_hwnno, sztemp, sizeof(MsgHead.op_hwnno));
    OpHwnno++;
    if(OpHwnno > 100) OpHwnno = 1;
    
    MsgHead.trm_recv_ga_yn[0] = '0';    
    MsgHead.aprvr_cnt[0] = '0';         
    MsgHead.mgaf_yn[0] = '0';    
    MsgHead.msg_u[0] = '1';      
    memcpy(MsgHead.mach_set_grpco_c, "S001", 4);
    memset(MsgHead.msg_hdr_mdia_ctnt, '0', sizeof(MsgHead.msg_hdr_mdia_ctnt));   
    if (memcmp(MsgData.bit_3, "30", 2) == 0) 
    {   
        memcpy(MsgHead.target_svc_trx_c, "SGAT9460A001", 12);
        memcpy(MsgHead.scr_id, "4594600000", 10);
    }
    else if (memcmp(MsgData.bit_3, "01", 2) == 0) 
    {   
        memcpy(MsgHead.target_svc_trx_c, "SGAT9461A001", 12);
        memcpy(MsgHead.scr_id, "4594610000", 10);
    }
    else if (memcmp(MsgData.bit_3, "40", 2) == 0) 
    {   
        memcpy(MsgHead.target_svc_trx_c, "SGAT9463A001", 12);
        memcpy(MsgHead.scr_id, "4594630000", 10);
    }
    else if (memcmp(MsgData.bit_3, "38", 2) == 0 || memcmp(MsgData.bit_3, "94", 2) == 0) 
    {   
        memcpy(MsgHead.target_svc_trx_c, "SGAT9464A001", 12);
        memcpy(MsgHead.scr_id, "4594640000", 10);       
    }
    else if (memcmp(MsgData.bit_3, "00", 2) == 0) 
    {   
        memcpy(MsgHead.target_svc_trx_c, "SGAT9461A001", 12);
        memcpy(MsgHead.scr_id, "4594610000", 10);
    }
    else if (memcmp(MsgData.bit_3, "43", 2) == 0 || memcmp(MsgData.bit_3, "91", 2) == 0) 
    {   
        memcpy(MsgHead.target_svc_trx_c, "SGAT9463B001", 12);
        memcpy(MsgHead.scr_id, "4594630000", 10);       
    }
	
        
    MsgHead.upmu_stt_c[0] = '0';       
    MsgHead.msg_wrt_lan_g[0] = '9';    
    MsgHead.trm_lock_yn[0] = '0';      
    memset(MsgHead.snd_node_ctnt, '0', sizeof(MsgHead.snd_node_ctnt));
    
    memcpy(MsgHead.serv_mng_brno, "0000", 4); 
    memcpy(MsgHead.frnt_g, "00", 2);          
    memset(MsgHead.oprt_brno, '0', sizeof(MsgHead.oprt_brno));    
    
    memcpy(MsgHead.std_msg_hdr_g, "10", 2);   
    MsgHead.rst_yn[0] = '0';    
    MsgHead.out_u[0] = '0';     
    memset(MsgHead.msg_cont_no, '0', sizeof(MsgHead.msg_cont_no));
    
    MsgHead.can_tryn[0] = '0';    
    MsgHead.crt_tryn[0] = '0';    
    MsgHead.hsong_tryn[0] = '0';  
    memset(MsgHead.trx_k, '0', sizeof(MsgHead.trx_k));    
    
    memset(MsgHead.telr_trxno, '0', sizeof(MsgHead.telr_trxno));    
    MsgHead.telr_mg_yn[0] = '0';
    memset(MsgHead.acmt_trxbrno, '0', sizeof(MsgHead.acmt_trxbrno));    
    memset(MsgHead.brj_no, '0', sizeof(MsgHead.brj_no));    
    memset(MsgHead.trx_telrno, '0', sizeof(MsgHead.trx_telrno));   
    MsgHead.nfcng_chan_user_msg_out_yn[0] = '0';    
    memcpy(MsgHead.nat_c, "VN", 2);           
    memcpy(MsgHead.sh_lcl_corp_c, "130", 3);  
    memcpy(MsgHead.lang_c, "EN", 2);          
    memset(MsgHead.filler4, '0', sizeof(MsgHead.filler4));
    
    memcpy(MsgFix.data_type, "90", 2);    
    ix = 0;
    ix = sizeof(MsgFix)-sizeof(MsgFix.data_type)-sizeof(MsgFix.data_len)+sizeof(MsgSys)+sizeof(MsgCom)+sizeof(MsgData)-2;
    sprintf(bufflen, "%08d", ix);
    memcpy(MsgFix.data_len, bufflen, strlen(bufflen));    
    memcpy(MsgFix.tran_code, MsgHead.target_svc_trx_c, sizeof(MsgHead.target_svc_trx_c));    
    memcpy(MsgFix.screen_id, MsgHead.scr_id, sizeof(MsgHead.scr_id));    
    memcpy(MsgFix.filler, "00", 2);
    memcpy(track2, MsgData.bit_35, sizeof(MsgData.bit_35));    
    getPAN(track2);
    memcpy(MsgCom.cashcard_no, track2, strlen(track2));    
    MsgCom.atm_check_gb[0] = '2';
    memcpy(MsgCom.atm_trcode, MsgData.bit_3, sizeof(MsgData.bit_3));    
    memcpy(MsgCom.amount, MsgData.bit_4, sizeof(MsgData.bit_4));
    memcpy(MsgCom.atm_biz_detail, "00", 2);    
    memcpy(MsgCom.atm_tr_seqno, &MsgData.bit_11[2], 4);    
    memcpy(MsgData.etx, "ZZ", 2); 

    ix = 0;
    memcpy(&trdat[ix] , &ChnlHead, sizeof(ChnlHead )); 
    ix += sizeof(ChnlHead);
    memcpy(&trdat[ix] , &MsgHead, sizeof(MsgHead ));  
    ix += sizeof(MsgHead);
    memcpy(&trdat[ix]  , &MsgFix, sizeof(MsgFix  ));  
    ix += sizeof(MsgFix);
    memcpy(&trdat[ix] , &MsgSys , sizeof(MsgSys  ));  
    ix += sizeof(MsgSys);
    memcpy(&trdat[ix] , &MsgCom , sizeof(MsgCom  ));  
    ix += sizeof(MsgCom);
    memcpy(&trdat[ix] , &MsgData, sizeof(MsgData ));  

    return NORMAL;
}

void bit2str(char *sdata, char *s_ret, int ilen)
{
    int idx, idxi;
    char ch;

    for(idx = 0; idx < ilen; idx++)
    {
    
        for(idxi = 7; idxi >= 0; idxi--)
        {
            ch = (char)((*sdata >> idxi) & 0x01);
            *s_ret = (char)(ch | 0x30);
            s_ret++;
        }
        sdata++;
    }
}


void fc_2bitstrTO16bitstr(char *sdata, char *s_ret, int ilen)
{
    int idex, idxi;
    char ch;
    for(idex = 0; idex < ilen; idex += 4)
    {
    
	    if (  strncmp(sdata+idex ,  "0000",  4) == 0 )
        {
            *s_ret = '0';
        }
        else if (  strncmp(sdata+idex ,  "0001",  4) == 0 )
        {
            *s_ret = '1';
        }
        else if (  strncmp(sdata+idex ,  "0010",  4) == 0 )
        {
            *s_ret = '2';
        }
        else if (  strncmp(sdata+idex ,  "0011",  4) == 0 )
        {
            *s_ret = '3';
        }
        else if (  strncmp(sdata+idex ,  "0100",  4) == 0 )
        {
            *s_ret = '4';
        }
        else if (  strncmp(sdata+idex ,  "0101",  4) == 0 )
        {
            *s_ret = '5';
        }
        else if (  strncmp(sdata+idex ,  "0110",  4) == 0 )
        {
            *s_ret = '6';
        }
        else if (  strncmp(sdata+idex ,  "0111",  4) == 0 )
        {
            *s_ret = '7';
        }
        else if (  strncmp(sdata+idex ,  "1000",  4) == 0 )
        {
            *s_ret = '8';
        }
        else if (  strncmp(sdata+idex ,  "1001",  4) == 0 )
        {
            *s_ret = '9';
        }
        else if (  strncmp(sdata+idex ,  "1010",  4) == 0 )
        {
            *s_ret = 'A';
        }    
        else if (  strncmp(sdata+idex ,  "1011",  4) == 0 )
        {
            *s_ret = 'B';
        }
        else if (  strncmp(sdata+idex ,  "1100",  4) == 0 )
        {
            *s_ret = 'C';
        }
        else if (  strncmp(sdata+idex ,  "1101",  4) == 0 )
        {
            *s_ret = 'D';
        }
		else if (  strncmp(sdata+idex ,  "1110",  4) == 0 )
        {   
            *s_ret = 'E';
        }
        else if (  strncmp(sdata+idex ,  "1111",  4) == 0 )
        {
            *s_ret = 'F';   
        }   
			s_ret++;       
        
	}      
}

/*----------------------------------------------------------------------------*/
/*                        E N D   O F   F I L E                               */
/*----------------------------------------------------------------------------*/
