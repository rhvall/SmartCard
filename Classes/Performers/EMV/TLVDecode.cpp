//
//  TLVDecode.cpp
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

#include <iostream>
#include "TLVDecode.h"

void TLVPackage::construct(unsigned char *buffer, unsigned int bufferLength, TLVEntity *tlvEntity, unsigned int &entityLength, unsigned int status)
{
	int currentTLVIndex = 0;
	int currentIndex = 0;
	int currentStatus = 'T';
	unsigned long valueSize = 0;
    
	while(currentIndex < bufferLength)
	{
		switch(currentStatus)
		{
            case 'T':
                valueSize = 0;
                //Determines whether is a single structure
                if(status == 1 && ((buffer[currentIndex] & 0x20) != 0x20))
                {
                    tlvEntity[currentTLVIndex].sub_TLVEntity = NULL; //Single structure, sub-tag empty
                    //Check whether is multy-byte
                    if((buffer[currentIndex] & 0x1f) == 0x1f)
                    {
                        int endTagIndex = currentIndex;
                        while((buffer[++endTagIndex] & 0x80) == 0x80); //Judgment the most significant bit of the second byte 1
                        int tagSize = endTagIndex - currentIndex + 1; //The calculated Tag contains the number of bytes
                        
                        tlvEntity[currentTLVIndex].tag = new unsigned char[tagSize];
                        memcpy(tlvEntity[currentTLVIndex].tag, buffer + currentIndex, tagSize);
                        tlvEntity[currentTLVIndex].tag[tagSize] = 0;
                        
                        tlvEntity[currentTLVIndex].tagSize = tagSize;
                        
                        currentIndex += tagSize;
                    }
                    else
                    {
                        tlvEntity[currentTLVIndex].tag = new unsigned char[1];
                        memcpy(tlvEntity[currentTLVIndex].tag, buffer + currentIndex, 1);
                        tlvEntity[currentTLVIndex].tag[1] = 0;
                        
                        tlvEntity[currentTLVIndex].tagSize = 1;
                        
                        currentIndex += 1;
                    }
                }
                else
                {
                    //Determine whether multi-byte tag
                    if((buffer[currentIndex] & 0x1f) == 0x1f)
                    {
                        int endTagIndex = currentIndex;
                        while((buffer[++endTagIndex] & 0x80) == 0x80); //Judgment the most significant bit of the second byte 1
                        int tagSize = endTagIndex - currentIndex + 1; //The calculated Tag contains the number of bytes
                        
                        tlvEntity[currentTLVIndex].tag = new unsigned char[tagSize];
                        memcpy(tlvEntity[currentTLVIndex].tag, buffer + currentIndex, tagSize); 
                        tlvEntity[currentTLVIndex].tag[tagSize] = 0;
                        
                        tlvEntity[currentTLVIndex].tagSize = tagSize;
                        
                        currentIndex += tagSize;
                    }
                    else
                    {
                        tlvEntity[currentTLVIndex].tag = new unsigned char[1];
                        memcpy(tlvEntity[currentTLVIndex].tag, buffer + currentIndex, 1);
                        tlvEntity[currentTLVIndex].tag[1] = 0;
                        
                        tlvEntity[currentTLVIndex].tagSize = 1;
                        
                        currentIndex += 1;				
                    }
                    
                    //SubTag Analysis
                    int subLength = 0;	
                    
                    unsigned char* temp;
                    if((buffer[currentIndex] & 0x80) == 0x80)
                    {
                        for (int index = 0; index < 2; index++)
                        {
                            subLength += buffer[currentIndex + 1 + index] << (index * 8); //Calculate the length domains
                        }
                        
                        temp = new unsigned char[subLength];
                        
                        memcpy(temp, buffer + currentIndex + 3, subLength);
                    }
                    else
                    {
                        subLength = buffer[currentIndex];
                        
                        temp = new unsigned char[subLength];
                        
                        memcpy(temp, buffer + currentIndex + 1, subLength);
                    }
                    
                    temp[subLength] = 0;
                    unsigned int oLength;
                    tlvEntity[currentTLVIndex].sub_TLVEntity = new TLVEntity[1];
                    construct(temp, subLength, tlvEntity[currentTLVIndex].sub_TLVEntity, oLength);
                }
                
                currentStatus = 'L';
                break;
            case 'L':		
                //To determine the maximum length byte bit, if 1, then the byte length of the extension byte, decided by the beginning of the next byte length
                if((buffer[currentIndex] & 0x80) != 0x80)
                {
                    tlvEntity[currentTLVIndex].length = new unsigned char[1];
                    memcpy(tlvEntity[currentTLVIndex].length, buffer + currentIndex, 1);
                    tlvEntity[currentTLVIndex].length[1] = 0;
                    tlvEntity[currentTLVIndex].lengthSize = 1;
                    
                    valueSize = tlvEntity[currentTLVIndex].length[0];
                    
                    currentIndex += 1;
                }
                else
                {
                    //Check because is 1
                    
                    unsigned int lengthSize = buffer[currentIndex] & 0x7f;
                    
                    currentIndex += 1; //Length field from the beginning of the next byte count
                    
                    for (int index = 0; index < lengthSize; index++)
                    {
                        valueSize += buffer[currentIndex + index] << (index * 8); //Calculate the Length domains
                    }
                    
                    tlvEntity[currentTLVIndex].length = new unsigned char[lengthSize];
                    memcpy(tlvEntity[currentTLVIndex].length, buffer + currentIndex, lengthSize);
                    tlvEntity[currentTLVIndex].length[lengthSize] = 0;
                    
                    tlvEntity[currentTLVIndex].lengthSize = lengthSize;
                    
                    currentIndex += lengthSize;
                }
                
                currentStatus = 'V';
                break;
            case 'V':
                tlvEntity[currentTLVIndex].value = new unsigned char[valueSize];
                memcpy(tlvEntity[currentTLVIndex].value, buffer + currentIndex, valueSize);
                tlvEntity[currentTLVIndex].value[valueSize] = 0;
                
                currentIndex += valueSize;
                
                //Into the next TLV structure cycle
                currentTLVIndex += 1;
                
                currentStatus = 'T';
                break;
            default:
                return;
		}
	}
    
	entityLength = currentTLVIndex;
}

void TLVPackage::parse(TLVEntity *tlvEntity, unsigned int entityLength, unsigned char *buffer, unsigned int &bufferLength)
{
	int currentIndex = 0;
	int currentTLVIndex = 0;
	unsigned long valueSize = 0;
    
	while(currentTLVIndex < entityLength)
	{
		valueSize = 0;
		TLVEntity entity = tlvEntity[currentTLVIndex];
		
		memcpy(buffer + currentIndex, entity.tag, entity.tagSize);
		currentIndex += entity.tagSize;
        
		for (int index = 0; index < entity.lengthSize; index++)
		{
			valueSize += entity.length[index] << (index * 8); //Calculate the Length domains 
		}
		
        if(valueSize > 127)
		{
			buffer[currentIndex] = 0x80 | entity.lengthSize;
			currentIndex += 1;
		}
		
		memcpy(buffer + currentIndex, entity.length, entity.lengthSize);
		currentIndex += entity.lengthSize;
		//To determine whether it contains sub-nested TLV
		if(entity.sub_TLVEntity == NULL)
		{
			memcpy(buffer + currentIndex, entity.value, valueSize);
			currentIndex += valueSize;
		}
		else
		{
			unsigned int oLength;
			parse(entity.sub_TLVEntity, 1, buffer + currentIndex, oLength);	//Resolve sub-nested TLV
			currentIndex += oLength;
		}
        
		currentTLVIndex++;
	}
	buffer[currentIndex] = 0;
	bufferLength = currentIndex;
}