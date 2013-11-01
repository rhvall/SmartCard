//
//  DUKPT.h
//  SmartCard
//
//  Created by Chenfan on 31/10/11.
//  Modified by Raúl Valencia on 17/12/12
//  Copyright (c) 2009 RHVT. All rights reserved.
//
//
//	           (\___/)
//             (o\ /o)
//            /|:.V.:|\
//		      \\::::://
//        -----`"" ""`-----

#ifndef __DUKPT__H
#define __DUKPT__H
unsigned long getSR(unsigned long SRValue,int lastpos,int* pos);
void Gen_CryptoKey(unsigned char* CURKEY,unsigned char* KSN);
static unsigned char *general_PINKEY();
unsigned short encrypt_apdu(unsigned char* planapdu,int len);
unsigned short decrypt_apdu(unsigned char* cryptapdu,int len);
#endif