//
//  CTError.h
//  SmartCard
//
//  Created by Chenfan on 11/8/11.
//  Modified by Raúl Valencia on 5/12/12
//  Another author: Martin Preuss<martin@libchipcard.de>
//  Homepage: http://www2.aquamaniac.de/sites/home/index.php
//  Copyright (c) 2009 RHVT. All rights reserved.
//
//
//	           (\___/)
//             (o\ /o)
//            /|:.V.:|\
//		      \\::::://
//        -----`"" ""`-----

#ifndef CTERROR_H
#define CTERROR_H

#include <string>

class CTError;

//  These errors are directly returned by CTAPI drivers
//  all other drivers will map their error codes to these codes
#define k_CTERROR_OK         0
#define k_CTERROR_INVALID_D 1 //-1
#define k_CTERROR_CT        8 //-8
#define k_CTERROR_TRANS     10 //-10
#define k_CTERROR_MEMORY    11 //-11
#define k_CTERROR_HTSI      128 //-128

#define k_CTERROR_PARAM     10
#define k_CTERROR_INVALID   11
#define k_CTERROR_NULL      12
#define k_CTERROR_NETWORK   14
#define k_CTERROR_LOCK      15
#define k_CTERROR_DRIVER    16
#define k_CTERROR_LIBLOADER 17
#define k_CTERROR_POINTER   18
#define k_CTERROR_DEBUG     19
#define k_CTERROR_FILE      20
#define k_CTERROR_IMPL      21
#define k_CTERROR_AUTH      22
#define k_CTERROR_SERVICE   23
#define k_CTERROR_API       24

using namespace std;

/**
 * You can use this one to return an error code or you can throw
 * an object of this class. This class holds information about errors, like
 * <ul>
 * <li>Where the error occurred (@ref where())</li>
 * <li>Error code (@ref code())</li>
 * <li>Subcode1 (called SW1 in CTAPI docs, the major result of operations
 * with a chipcard/terminal, @ref subcode1())</li>
 * <li>Subcode2 (called SW2 in CTAPI docs, gives more specific information
 * about the result, @ref subcode2())</li>
 * <li>Additional information (@ref info())</li>
 * </ul>
 * @short This class can be used to flag errors.
 * @author Martin Preuss<martin@libchipcard.de>
 * @ingroup status
 */

class CTError
{    
public:
    /**
     * Use this constructor if an error occurred.
     * @author Martin Preuss<martin@libchipcard.de>
     * @param where the location of the error, e.g. "CTError::isOk()"
     * @param info additional information about the error.
     * @param code error code, this can be one defined in CTTerminal, like:
     * <ul>
     * <li>k_CTERROR_INVALID </li>
     * <li>k_CTERROR_CT </li>
     * <li>k_CTERROR_TRANS </li>
     * <li>k_CTERROR_MEMORY </li>
     * <li>k_CTERROR_HTSI </li>
     * </ul>
     * @param subcode1 this is the first CTAPI return code. this and subcode2
     * are returned if the code shows "k_CTERROR_OK". This code holds the
     * major code, while subcode2 is more specific about the error. (SW1)
     * @param subcode2 this is the second CTAPI return code (SW2)
     * @param info additional information
     * @param explanation Here you can explan the error. If this is omitted
     * then this class will try to provide an explanation of its own.
     */
    CTError(const string &where,
            unsigned char code,
            unsigned char subcode1,
            unsigned char subcode2,
            const string &info = "",
            const string &explanation = "");
    
    CTError(const string &where,
            const CTError &err);
    
    /**
     * Use this constructor if NO error occurred. This way you can return
     * more complex return values. The caller may ask this object with
     * @ref isOk() to determine if an error occurred.
     * @author Martin Preuss<martin@libchipcard.de>
     */
    CTError();
    
    ~CTError();
    
    /**
     * Tells you if this object shows an error or if all was ok.
     * You may give additional values for subcode1 and subcode2 which are to
     * be treated "OK". This is needed, when checking the result of
     * @ref CTCard::closeCard(), which sometimes returns SW1=0x62, thus
     * showing that no card is inserted. Well, this is an error on all other
     * methods, but when calling closeCard() this result is desired.
     * So for checking the result of closeCard() use this:
     * err.isOk(0x62);
     * @param ad1 subcode1 to be treated ok
     * @param ad2 subcode2 to be treated ok when occuring together with
     * ad1
     * @author Martin Preuss<martin@libchipcard.de>
     * @return true if there was no error, false otherwise
     */
    bool isOk(unsigned char ad1 = 0, unsigned char ad2 = 0);
    
//  Return the location of the error, e.g. "CTError::CTError()".
    const string &where() const {return _where; };
    
//  Returns the error code
    int code() const { return _code; };
    
//  Returns subcode1
    int subcode1() const { return _subcode1; };
    
//  Returns subcode2
    int subcode2() const { return _subcode2; };
    
//  Returns additional info about the error.
    const string &info() const { return _info; };
    
//  Returns a short explanation of tis error.
    const string &explanation() const { return _explanation;};
    
//  Returns a short error string containing all important information.
    string errorString();
    
//  Returns the path of methods/functions which reported this error.
    const string &reportedFrom() const { return _reportedFrom;};

private:
    string _where;
    unsigned char _code;
    unsigned char _subcode1;
    unsigned char _subcode2;
    string _info;
    string _explanation;
    string _reportedFrom;
    string _textFromCode(unsigned char code,
                         unsigned char sw1,
                         unsigned char sw2);
    string _num2string(int n, const string &format = "%d");
    int _string2num(const string &n, const string &format = "%d");
};
#endif



