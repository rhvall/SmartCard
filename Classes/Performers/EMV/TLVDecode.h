//
//  TLVDecode.h
//  SmartCard
//
//  Created by Chenfan on 11/8/11.
//  Modified by Raúl Valencia on 5/12/12
//  Copyright (c) 2009 RHVT. All rights reserved.
//
//
//	           (\___/)
//             (o\ /o)
//            /|:.V.:|\
//		      \\::::://
//        -----`"" ""`-----

#ifndef TLVDecode_h
#define TLVDecode_h
#include <stdio.h>

#define TLV_MAX_LENGTH  1024

// TLV Structure
typedef struct TLVEntity
{
	unsigned char *tag;			//Mark
	unsigned char *length;		//Data Length
	unsigned char *value;		//Data
	unsigned int tagSize;		//Tag length
	unsigned int lengthSize;	//Length usage
	TLVEntity *sub_TLVEntity;	//Reference to other TVL's
}TLVEntity;

class TLVPackage
{
public:
    TLVPackage();
    virtual ~TLVPackage();
    static void construct(unsigned char *buffer, unsigned int bufferLength, TLVEntity *tlvEntity, unsigned int &entityLength, unsigned int status = 0);
//  TLV Matrix parsing
    static void parse(TLVEntity *tlvEntity, unsigned int entityLength, unsigned char *buffer, unsigned int &bufferLength);
};


#endif
