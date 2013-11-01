//
//  DUKPT.h
//  SmartCard
//
//  Created by Chenfan on 31/10/11.
//  Modified by Ra渓 Valencia on 17/12/12
//  Copyright (c) 2009 RHVT. All rights reserved.
//
//
//	           (\___/)
//             (o\ /o)
//            /|:.V.:|\
//		      \\::::://
//        -----`"" ""`-----

#include <stdlib.h>  
#include <stdio.h>  
#include <memory.h>  
#include <string.h>  
#include <ctype.h>  

#include "3DES.h"
#include "DUKPT.h"

unsigned char g_IPEK[16]={0x6A,0xC2,0x92,0xFA,0xA1,0x31,0x5B,0x4D,0x85,0x8A,0xB3,0xA3,0xD7,0xD5,0x93,0x3A};
unsigned char g_cryptKey[16]={0x00,};
unsigned char g_PINKey[16]={0x00,};
unsigned char g_MACKey[16]={0x00,};

unsigned long getSR(unsigned long SRValue,int lastpos,int* pos)
{
    int loop;
    SRValue=SRValue<<(32-lastpos);
    SRValue=SRValue>>(32-lastpos);
    for(loop=lastpos;loop>=0;loop--)
    {
        if(SRValue>>loop)
        {
            SRValue=SRValue>>loop;
            SRValue=SRValue<<loop;
            break;
        }
    }  
    *pos=loop;
    return SRValue;
}
void Gen_CryptoKey(unsigned char* t_IPEK,unsigned char* KSN)
{

    unsigned char R8[9];
    unsigned char R8A[9];
    unsigned char R8B[9];
    unsigned long R3=0;
    unsigned long SR=0x0;
//    int lastpos=21;
    int currentpos=21;
    char random[9]={0x00,};
    unsigned char crc1[17]={0xc0,0xc0,0xc0,0xc0,0x00,0x00,0x00,0x00,0xc0,0xc0,0xc0,0xc0,0x00,0x00,0x00,0x00};
    memcpy(R8,KSN+2,8);
    R8[5]&=0xe0;            //右边21位清0
    R8[6]=0x00;
    R8[7]=0x00;
    memcpy(&R3,KSN+7,4);    //加密计数器
    R3=(KSN[7]&0x1f)<<16;
    R3|=(KSN[8]<<8);
    R3|=KSN[9];
    SR=R3;
    SR=SR&0x1fffff;         //保留21位，其它位清0   //101
//    int loop;
//    int crycount=0;
	memcpy(t_IPEK,g_IPEK,16);
    while (1) 
    {
        SR=getSR(R3,currentpos,&currentpos);
        if(!(SR&R3))break;
        R8[5]|=(unsigned char)(SR>>16);
        R8[6]|=(unsigned char)(SR>>8);
        R8[7]|=(unsigned char)SR;
        int i;
        for(i=0;i<8;i++)
        {
            R8A[i]=t_IPEK[i+8]^R8[i];
            
        }
        unsigned char KEYR[9];
        //int result=Run3Des(ENCRYPT,ECB, (const char*)R8A,8, (const char*)t_IPEK, KEY_LEN_8, (char*)KEYR, 8,random);
        Run3Des(ENCRYPT,ECB, (const char*)R8A,8, (const char*)t_IPEK, KEY_LEN_8, (char*)KEYR, 8,random);
        for(i=0;i<8;i++)
        {
            R8A[i]=KEYR[i]^t_IPEK[i+8];
        }
        for(i=0;i<16;i++)
        {
            t_IPEK[i]^=crc1[i];
        }
        for(i=0;i<8;i++)
        {
            R8B[i]=t_IPEK[i+8]^R8[i];
        }
        unsigned char KEYL[9];

        Run3Des(ENCRYPT,ECB, (const char*)R8B,8, (const char*)t_IPEK, KEY_LEN_8, (char*)KEYL, 8, random);
        for(i=0;i<8;i++)
        {
            R8B[i]=KEYL[i]^t_IPEK[i+8];
        }
        memcpy(g_cryptKey,R8B,8);
        memcpy(g_cryptKey+8,R8A,8);
        SR=SR>>1;
        if(!SR)break;
    }
	
}
static unsigned char *general_PINKEY()
{
    unsigned char i;
	unsigned char crc[17]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff};
    for(i=0;i<16;i++)
    {
        g_PINKey[i]=g_cryptKey[i]^crc[i];
    }
	return g_PINKey;  
}

unsigned short decrypt_apdu(unsigned char* cryptapdu,int len)
{
	unsigned char random[8]={0x00,};
	Run3Des(DECRYPT,ECB, (const char*)cryptapdu,len,(const char*)g_cryptKey, KEY_LEN_16, (char*)cryptapdu, 264, (const char*)random);
	return	RunRsm((char*)cryptapdu,len);
}
unsigned short encrypt_apdu(unsigned char* planapdu,int len)
{
	unsigned char random[8]={0x00,};
	int padlen=0; 
	general_PINKEY();
	RunPad(PAD_PKCS_7,(const char*)planapdu,len,(char*)planapdu,&padlen);
	Run3Des(ENCRYPT,ECB, (const char*)planapdu,padlen, (const char*)g_cryptKey, KEY_LEN_16, (char*)planapdu, 264, (const char*)random);
	return padlen;
}
