//
//  cterror.cpp
//  iBankSpace
//
//  Created by Chenfan on 11/8/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//
//
//	           (\___/)
//             (o\ /o)
//            /|:.V.:|\
//		      \\::::://
//        -----`"" ""`-----

#include <stdio.h>
#include <string>

#include "CTError.h"
//#include "libchipcard.h"
//#include <engine/chameleon/error.h>
//#include <engine/chameleon/debug.h>

//----------------------------------------
//	INITIALIZATION
//----------------------------------------
#pragma mark -
#pragma mark INITIALIZATION

CTError::CTError(const string &where,
                 unsigned char code,
                 unsigned char subcode1,
                 unsigned char subcode2,
                 const string &info,
                 const string &expl):
_where(where)
,_code(code)
,_subcode1(subcode1)
,_subcode2(subcode2)
,_info(info.empty()?_textFromCode(code,subcode1,subcode2):info)
,_explanation(expl.empty()?_textFromCode(code,subcode1,subcode2):expl)
{
    if (!isOk())
    {
        printf("CTError: Created an error with these values:\n"
                  " %s\n"
                  "This is not necessarily an error that will be reported,\n"
                  "this is just for debugging purposes.",
                  errorString().c_str());
    }
}


CTError::CTError():
_code(0)
,_subcode1(0)
,_subcode2(0)
{
}


CTError::CTError(const string &where,
                 const CTError &err)
{
    *this=err;
    if (_reportedFrom.empty())
    {
        _reportedFrom = where;
    }
    else
    {
        _reportedFrom = where + "/" + _reportedFrom;
    }
}

CTError::~CTError()
{
}

//----------------------------------------
//	FUNCTIONALITY
//----------------------------------------
#pragma mark -
#pragma mark FUNCTIONALITY

string CTError::errorString()
{
    string result;
    char buffer[32];
    
    if (isOk())
    {
        return "Ok.";
    }

    result = "ERROR ";
//  TODO: verbose error message
    result += " (";
    sprintf(buffer, "%4i", (int)((char)_code));
    result += buffer;
    result += ", ";
    sprintf(buffer, "%2x", _subcode1);
    result += buffer;
    result += ", ";
    sprintf(buffer, "%2x", _subcode2);
    result += buffer;
    result += ") at ";
    result += _where;
    result += " Info: ";
    result += _info;
    
    if (!_explanation.empty())
    {
        result += " Expl: ";
        result += _explanation;
    }
    
    if (!_reportedFrom.empty())
    {
        result += " reported from ";
        result += _reportedFrom;
    }
    
    return result;
}


bool CTError::isOk(unsigned char ad1, unsigned char ad2)
{
    /* this code works like this:
     * - if the main return code indicates a CTAPI error, which means the
     *   chip card never even saw the last command, then FALSE is returned
     * - the following return codes are treated as ok:
     *   - 90XX (general OK)
     *   - 91XX (GSM: normal ending of the command, with extra information from
     *           the proactive SIM containing a command for the ME. Length 'XX'
     *           of the response data)
     *   - 9FXX (GSM: length 'XX' of the response data)
     *   - 61XX (warning about size, ignoreable since it only tells how many
     *           bytes are still available)
     *   - 6282 (premature end of file, this is not treated as an error here,
     *           since the caller in most cases checks the size of returned
     *           data itself)
     *   - 0000 (this means that this object is created with CTError(), so
     *           there can be no error)
     * - if ad1 == SW1 (which means the actual returncode SW1 is that one the
     *   caller wants us to ignore)
     *   AND ad2 = SW2 (or ad2 is zero, meaning that the caller wants to ignore
     *   ALL returncodes which have SW1==ad1, regardless of what SW2 actually
     *   is)
     */
    if (_code != k_CTERROR_OK)
        return false;
    if (_subcode1 == 0x90 ||  // ok
        _subcode1 == 0x91 ||  // ok (GSM)
        _subcode1 == 0x9f ||  // ok (GSM)
        _subcode1 == 0x61 ||  // warning: size
        (_subcode1 == 0x62 && // premature end of file
         _subcode2 == 0x82) ||
        (_subcode1 == ad1 && (!ad2 || _subcode2 == ad2)) ||
        (_subcode1 == 0 && _subcode2 == 0)
        )
        return true;
    return false;
}

string CTError::_textFromCode(unsigned char code,
                              unsigned char sw1,
                              unsigned char sw2)
{
    string result;
    
    switch (code)
    {
        case k_CTERROR_OK: // SW1 and SW2 are valid
            switch(sw1) {
                case 0x90:
                    result += "success";
                    break;
                case 0x91:
                    result += "success (GSM)";
                    break;
                case 0x9f:
                    result += "success (GSM)";
                    break;
                case 0x61:
                    result += "more bytes available";
                    break;
                case 0x62: // memory unchanged
                    switch (sw2)
                    {
                        case 0x81:
                            result += "returned data may be corrupted";
                            break;
                        case 0x82:
                            result += "premature end of file";
                            break;
                        case 0x83:
                            result += "selected file invalidated";
                            break;
                        case 0x84:
                            result += "bad FCI format";
                            break;
                        default:
                            result += "memory unchanged";
                            break;
                    } // switch sw2
                    break;
                case 0x63: // memory unchanged
                    switch (sw2)
                    {
                        case 0x00:
                            result += "memory unchanged";
                            break;
                        case 0x81:
                            result += "file already filled up";
                            break;
                        case 0xc0:
                            result += "bad pin, no bad try left!!!";
                            break;
                        case 0xc1:
                            result += "bad pin, one bad try left!!";
                            break;
                        case 0xc2:
                            result += "bad pin, two bad tries left!";
                            break;
                        case 0xc3:
                            result += "bad pin, three bad tries left";
                            break;
                        default:
                            result += "memory unchanged, maybe bad pin ?";
                            break;
                    } // switch sw2
                    break;
                case 0x64:
                    result += "memory unchanged";
                    break;
                case 0x65: // memory unchanged
                    result += "memory unchanged";
                    switch (sw2)
                    {
                        case 0x00:
                            result += " (no info given)";
                            break;
                        case 0x81:
                            result += " (memory failure)";
                            break;
                        default:
                            break;
                    } // switch sw2
                    break;
                case 0x66:
                    result += "security violation";
                    break;
                case 0x67:
                    result += "wrong length of ISO command (check LR)";
                    break;
                case 0x68: // function in CLA not supported
                    switch (sw2)
                    {
                        case 0x81:
                            result += "logical channel not supported";
                            break;
                        case 0x82:
                            result += "secure messaging not supported";
                            break;
                        default:
                            result += "command is not part of given class";
                            break;
                    } // switch sw2
                    break;
                case 0x69: // command not allowed
                    switch (sw2)
                    {
                        case 0x81:
                            result += "command incompatible with file structure";
                            break;
                        case 0x82:
                            result += "security status not satisfied";
                            break;
                        case 0x83:
                            result += "authentification method blocked";
                            break;
                        case 0x84:
                            result += "referenced data invalidated";
                            break;
                        case 0x85:
                            result += "conditions of use not satisfied";
                            break;
                        case 0x86:
                            result += "command needs a selected EF";
                            break;
                        case 0x87:
                            result += "expected SecureMessaging data objects missing";
                            break;
                        case 0x88:
                            result += "SecureMessaging data objects incorrect";
                            break;
                        default:
                            result += "command not allowed";
                            break;
                    } // switch sw2
                    break;
                case 0x6a:
                    switch (sw2)
                    {
                        case 0x80:
                            result += "incorrect parameters in data field";
                            break;
                        case 0x81:
                            result += "function not supported";
                            break;
                        case 0x82:
                            result += "file not found";
                            break;
                        case 0x83:
                            result += "record not found";
                            break;
                        case 0x84:
                            result += "not enough memory space in file";
                            break;
                        case 0x85:
                            result += "Lc inconsistent with TLV structure";
                            break;
                        case 0x86:
                            result += "incorrect paramters p1-p2";
                            break;
                        case 0x87:
                            result += "Lc inconistent with p1-p2";
                            break;
                        case 0x88:
                            result += "referenced data not found";
                            break;
                        default:
                            result += "wrong parameters p1-p2";
                            break;
                    } // switch sw2
                    break;
                case 0x6b:
                    result += "wrong parameters p1/p2";
                    break;
                case 0x6c:
                    result += "wrong length, SW2=correct length";
                    break;
                case 0x6d:
                    result += "command not supported";
                    break;
                case 0x6e:
                    result += "class not supported";
                    break;
                case 0x6f:
                    result += "no precise diagnosis";
                    break;
                default:
                    result += "unkown SW codes";
            } // switch sw1
            break;
        case k_CTERROR_INVALID_D: // invalid params ot whatever
            result += "invalid arguments";
            break;
        case k_CTERROR_CT: // CT error
            result += "error within CTAPI";
            break;
        case k_CTERROR_TRANS: // transmission error
            result += "transmission error";
            break;
        case k_CTERROR_MEMORY: // error in memory allocation
            result += "error on memory allocation within CTAPI";
            break;
        case k_CTERROR_HTSI: // HTSI error
            result += "HTSI error";
            break;
        case k_CTERROR_API:
            result += "API error ( ";
            switch(sw1)
            {
                /*case CHIPCARD_SUCCESS:
                    result += "Ok";
                    break;
                case CHIPCARD_ERROR_INVALID:
                    result += "invalid arguments";
                    break;
                case CHIPCARD_ERROR_BUFFER:
                    result += "invalid buffer size";
                    break;
                case CHIPCARD_ERROR_CARD_REMOVED:
                    result += "Card has been removed";
                    break;
                case CHIPCARD_ERROR_NO_REQUEST:
                    result += "No request";
                    break;
                case CHIPCARD_ERROR_NO_MESSAGE:
                    result += "no message";
                    break;
                case CHIPCARD_ERROR_BAD_CHANNEL_STATUS:
                    result += "bad channel status";
                    break;
                case CHIPCARD_ERROR_NO_COMMANDS:
                    result += "no commands";
                    break;
                case CHIPCARD_ERROR_NO_CONFIG:
                    result += "bad or missing configuration file";
                    break;
                case CHIPCARD_ERROR_UNREACHABLE:
                    result += "card server is not reachable (maybe down ?)";
                    break;
                case CHIPCARD_ERROR_DRIVER:
                    result += "internal driver error";
                    break;
                case CHIPCARD_ERROR_NO_READER:
                    result += "No reader/reader not available";
                    break;
                case CHIPCARD_ERROR_COMMAND_NOT_FOUND:
                    result += "command not found";
                    break;
                case CHIPCARD_ERROR_BAD_RESPONSE:
                    result += "malformed response";
                    break;
                case CHIPCARD_ERROR_NO_CARD:
                    result += "no card";
                    break;
                case CHIPCARD_ERROR_ABORTED:
                    result += "aborted";
                    break;
                case CHIPCARD_ERROR_INTERRUPTED:
                    result += "interrupted";
                    break;
                case CHIPCARD_ERROR_INTERNAL:
                    result += "internal error";
                    break;*/
                default:
                    result += "unknown k_CTERROR_API error";
                    break;
            } // switch sw1
            result += ")";
            break;
        default: // unknown
            result="unknown k_CTERROR_OK error";
            break;
    } // switch code
    
    return result;
}

string CTError::_num2string(int n, const string &format)
{
    char buffer[32];
    
    sprintf(buffer, format.c_str(), n);
    return buffer;
}

int CTError::_string2num(const string &n, const string &format)
{
    int result;
    sscanf(n.c_str(),format.c_str(),result);
    // TODO: maybe scan for error
    return result;
}





