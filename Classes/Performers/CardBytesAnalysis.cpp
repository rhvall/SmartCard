//
//  CardBytesAnalysis.m
//  SmartCard
//
//  Created by RHVT on 12/12/12.
//  Copyright (c) 2012 Mittra Software. All rights reserved.
//
//  Class Description: Helper C/C++ functions to read Hexadecimal
//  streams from the Card reader
//
//	           (\___/)
//             (o\ /o)
//            /|:.V.:|\
//		      \\::::://
//        -----`"" ""`-----

#include "CardBytesAnalysis.hpp"

string changeFromStrToHex(string fullStr)
{
    vector<char> pivotStr;
    int pivotHex = 3;
    for (uInt i = 0; i < fullStr.size(); i++)
    {   
        if (pivotHex == 3)
        {
            pivotStr.push_back(',');
            pivotStr.push_back('0');
            pivotStr.push_back('x');
            pivotHex = 1;
        }
        if (fullStr.at(i) != ' ')
        {
            pivotStr.push_back(fullStr.at(i));
            pivotHex++;
        }
    }
    
    ostringstream strFormatter;
    std::copy(pivotStr.begin(), pivotStr.end(), ostream_iterator<char>(strFormatter));
    return strFormatter.str();
}

uInt getCommandPosition(const void *data, uInt dtaLenght, const uint8_t *command, const uInt &cmmLenght)
{
    if (data == NULL || command == NULL)
    {
        return 0;
    }
    
    for (uInt i = 0; i < dtaLenght; i++)
    {
        const uint8_t *castData = reinterpret_cast<const uint8_t *>(data);
        if (castData[i] == command[0])
        {
            uInt position = i;
            bool isEqual = true;
            for (int j = 1; j < cmmLenght; ++j)
            {
                i++;
                if (castData[i] != command[j])
                {
                    isEqual = false;
                    break;
                }
            }
            
            if (isEqual == true)
            {
                return position;
            }
        }
    }
    
    return 0;
}

string getDataRepresentation(const void *data, uInt dtaLenght, const uint8_t *cmmTag, const uInt &cmmLength, const DataRepresentation dtaRep, uInt stringSize)
{
    uInt cmmPosition = getCommandPosition(data, dtaLenght, cmmTag, cmmLength);

    if (cmmPosition != 0)
    {
        const uint8_t *castData = reinterpret_cast<const uint8_t *>(data);
        
        int sizeOffset = 0;
        if (stringSize <= 0)
        {
            stringSize = castData[cmmPosition + cmmLength];
            sizeOffset = 1;
        }
        
        switch (dtaRep)
        {
            //Translates the binary information into an ASCII reaable string
            case STRING_REPRESENTATION:
            {
                
                string aStr(reinterpret_cast<const char*>(&(castData[cmmPosition + cmmLength + sizeOffset])), stringSize);
                return aStr;
                break;
            }
            //Translates the binary information into a decimal (0-9) reaable string
            case DECIMAL_REPRESENTATION:
            {
                std::stringstream ss;
                for (int k = 0; k < stringSize; k++)
                {
                    ss << std::hex << setfill('0') << setw(2) <<  static_cast<unsigned int>( castData[cmmPosition + cmmLength + sizeOffset + k]);
                }
                return ss.str();
                break;
            }
            case HEX_REPRESENTATION:
            {
                
                string aStr(reinterpret_cast<const char*>((int)&(castData[cmmPosition + cmmLength + sizeOffset])), stringSize);
                return aStr;
                break;
            }   
            default:
                break;
        }
    }
    
    return "\0";
}

vector<string> getCardData(const void *data, uInt dtaLenght)
{
    vector<string> cardData;
    CardList aCardList;
    for (int i = 0; i < CARD_TAG_LIST_NUMBER; i++)
    {
        const uint8_t *cmmTag = aCardList.cardTags[i];
        uInt cmmTagSize = aCardList.cardTagsSize[i];
        DataRepresentation cmmRepresentation = DECIMAL_REPRESENTATION;
        if (i == CARDHOLDER_NAME)
        {
            cmmRepresentation = STRING_REPRESENTATION;
        }
        
        string cardName = getDataRepresentation(data, dtaLenght, cmmTag, cmmTagSize, cmmRepresentation);
        if (cardName.size() > 0)
        {
           cardData.push_back(cardName);
        }
    }
    
    return cardData;
}
