//
//  CardBytesAnalysis.h
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


#ifndef CardBytesAnalysis_h
#define CardBytesAnalysis_h

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <vector>
#include "wintypes.h"
#include "CardList.h"

using namespace std;

//Transforms an standard string to its hexadecimal representation, in string as well
string changeFromStrToHex(string fullStr);
//Find a command secuence in raw data
uInt getCommandPosition(const void *data, uInt dtaLenght, const uint8_t *command, const uInt &cmmLenght);
//Transfrom the binary data into an string, hexadecimal or something readable
std::string getDataRepresentation(const void *data, uInt dtaLenght, const uint8_t *cmmTag, const uInt &cmmLength, const DataRepresentation dtaRep = STRING_REPRESENTATION, uInt stringSize = 0);
//Loop through a list of Tags to parse the information
vector<string> getCardData(const void *data, uInt dtaLenght);

#endif
