//
//  CTTLV.h
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

#include <iostream>

#include <string>
#include "CTError.h"
#include "CTPointer.h"
#include "CTTLV.h"


CTTLV::CTTLV()
:_type(0)
,_tag(0)
,_length(0)
,_size(0)
,_valid(false)
{
}


CTTLV::CTTLV(const string &s,unsigned int &pos, bool simple)
:_type(0)
,_tag(0)
,_length(0)
,_size(0)
,_valid(false)
,_simple(simple)
{
    unsigned int i;
    unsigned int opos;
    
    // skip "00" and "FF" fields at the beginning
    while (pos<s.length())
    {
        if ((s.at(pos)!=0x00) &&
            ((unsigned char)(s.at(pos))!=0xff))
            break;
        pos++;
    }
    opos=pos;
    
    // get tag type
    if (pos>=s.length())
        return;
    i=(unsigned char)s.at(pos);
    
    if (simple)
        // Simple-TLV uses only one byte for the tag code
        _tag=i;
    else
    {
        // BER-TLV uses up to two bytes for the tag code
        _type=i &0xe0;
        // get tag id
        if ((i &0x1f)==0x1f)
        {
            // two byte tag
            pos++;
            if (pos>=s.length())
                return;
            _tag=((unsigned char)s.at(pos))&0x7f;
        }
        else
            _tag=i & 0x1f;
    } // if BER-TLV
    
    // now get length
    pos++;
    if (pos>=s.length())
        return;
    i=(unsigned char)s.at(pos);
    if (simple)
    {
        // Simple-TLV
        if (i<255)
            // length encoded in one byte
            _length=i;
        else
        {
            // length encoded in three bytes
            pos++;
            if (pos+2>=s.length())
                return;
            pos++;
            _length=((unsigned char)s.at(pos))<<8;
            pos++;
            _length+=(unsigned char)s.at(pos);
        }
    }
    else
    {
        // BER-TLV
        if (i & 0x80)
        {
            // multiple bytes
            if (i==0x81)
            {
                // two bytes
                pos++;
                if (pos>=s.length())
                    return;
                _length=(unsigned char)s.at(pos);
            }
            else if (i==0x82)
            {
                if (pos+2>=s.length())
                    return;
                pos++;
                _length=((unsigned char)s.at(pos))<<8;
                pos++;
                _length+=(unsigned char)s.at(pos);
            }
            else
                return;
        }
        else
            _length=i;
    }
    pos++;
    
    // get value if any
    if (_length && pos<s.length())
        {
        _value=s.substr(pos,_length);
        pos+=_length;
        _size=pos-opos;
    }
    _valid=true;
}


CTTLV::CTTLV(unsigned int tag, unsigned int cl, bool cstrc,string d)
:_type(cl | (cstrc?0x20:0))
,_tag(tag)
,_length(d.length())
,_value(d)
,_valid(false)
,_simple(false)
{
    _size=d.length()+2;
    // if tag is 0x1f then the tag type occupies 2 bytes
    if ((tag&0x1f)==0x1f)
        _size++;
    // if length is higher than 127 we need two bytes
    if (_length>127)
        _size++;
    // if length is higher than 255 we need three bytes for the length
    if (_length>255)
        _size++;
    _valid=true;
}


CTTLV::CTTLV(unsigned int tag, string d)
:_type(0)
,_tag(tag)
,_length(d.length())
,_value(d)
,_valid(false)
,_simple(true)
{
    _size=d.length()+2;
    // if length is higher than 254 we need three bytes
    if (_length>254)
        _size+=2;
    _valid=true;
}


CTTLV::~CTTLV(){
}


string CTTLV::toString(){
    unsigned char c;
    string result;
    
    if (_simple)
        {
        // start with tag number
        result+=(char)_tag;
        
        // store length
        if (_length<255)
            // one byte
            result+=(char)_length;
        else
    {
            // three bytes
            c=_length>>8;
            result+=(char)c;
            c=_length & 0xff;
            result+=(char)c;
        }
    }
    else
    {
        // start with the tag number
        if (_tag>30)
        {
            // two byte tag number
            c=_type | 0x1f;
            result+=(char)c;
            result+=(char)_tag;
        }
        else
    {
            // one byte tag number
            c=_tag | _type;
            result+=(char)c;
        }
        
        // add tag length
        if (_length>255)
        {
            // three byte size
            c=0x82;
            result+=(char)c;
            // high byte
            c=_length>>8;
            result+=(char)c;
            c=_length & 0xff;
            result+=(char)c;
        }
        else if (_length>127)
        {
            // two byte size
            c=0x81;
            result+=(char)c;
            c=_length & 0xff;
            result+=(char)c;
        }
        else
    {
            // one byte size
            c=_length & 0x7f;
            result+=(char)c;
        }
    }
    
    // add value
    result+=_value.substr(0,_length);
    
    // store this size internally
    _size=result.length();
    
    // that's it
    return result;
}




CTTLV_FCI::CTTLV_FCI(string fci)
:fileSize(0)
,fileSizeRaw(0)
,fileId(0)
,fileType("unknown")
,isEF(false)
,fileDescription("")
,fileAttributes("")
,maxRecordLength(0)
,fileName("unset")
,isTransparent(false)
,isLinear(false)
,isFixed(false)
,isVariable(false)
,isCyclic(false)
,isSimpleTLV(false)
{
    string result;
    unsigned int pos;
    CTPointer<CTTLV> tfci;
    CTPointer<CTTLV> currtlv;
    string fcival;
    string currval;
    
    if (!fci.empty())
        {
        pos=0;
        tfci=new CTTLV(fci,pos);
        // is the envelope tag given ?
        if (tfci.ref().getTag()==0x0f)
            // yes, so the content of the FCI is the content of that tag
            fcival=tfci.ref().getValue();
        else
            // otherwise there is no envelope, just the content of the tag
            fcival=fci;
        pos=0;
        while(pos<fcival.length())
        {
            currtlv=new CTTLV(fcival,pos);
            parseTag(currtlv);
        } // while
    } // if !empty
}


CTTLV_FCI::CTTLV_FCI()
:fileSize(0)
,fileSizeRaw(0)
,fileId(0)
,fileType("unknown")
,isEF(false)
,fileDescription("")
,fileAttributes("")
,maxRecordLength(0)
,fileName("unset")
,isTransparent(false)
,isLinear(false)
,isFixed(false)
,isVariable(false)
,isCyclic(false)
,isSimpleTLV(false)
{
}


CTTLV_FCI::~CTTLV_FCI(){
}


void CTTLV_FCI::parseTag(CTPointer<CTTLV> currtlv){
    string currval;
    unsigned char c;
    
    switch(currtlv.ref().getTag())
        {
        case 0: // number of bytes in file
            currval=currtlv.ref().getValue();
            fileSizeRaw=((unsigned char)currval[0]<<8)+
            ((unsigned char)currval[1]);
            if (!fileSize)
                fileSize=fileSizeRaw;
            break;
            
        case 1: // number of bytes in file (including structural data)
            currval=currtlv.ref().getValue();
            fileSize=((unsigned char)currval[0]<<8)+
            ((unsigned char)currval[1]);
            if (!fileSizeRaw)
                fileSizeRaw=fileSize;
            break;
            
            case 2: // file descriptor (special: may have 1 to 4 bytes)
                currval=currtlv.ref().getValue();
                if (!currval.empty())
                {
                    c=currval.at(0);
                    // check type
                    if ((c & 0x38)==0x00)
                    {
                        fileType="Working EF";
                        isEF=true;
                    }
                    else if ((c & 0x38)==0x08)
                    {
                        fileType="Internal EF";
                        isEF=true;
                    }
                    else if ((c & 0x38)==0x38){
                        fileType="DF";
                        isEF=false;
                    }
                    
                    // check EF data
                    if (isEF)
                    {
                        if ((c & 0x7)==0x1)
                        {
                            fileDescription="EF, Transparent";
                            isTransparent=true;
                        }
                        else if ((c & 0x7)==0x2){
                            fileDescription="EF, Linear, fixed";
                            isLinear=true;
                            isFixed=true;
                        }
                        else if ((c & 0x7)==0x3){
                            fileDescription="EF, Linear, fixed, simple TLV";
                            isLinear=true;
                            isFixed=true;
                            isSimpleTLV=true;
                        }
                        else if ((c & 0x7)==0x4){
                            fileDescription="EF, Linear, variable";
                            isLinear=true;
                            isVariable=true;
                        }
                        else if ((c & 0x7)==0x5){
                            fileDescription="EF, Linear, variable, simple TLV";
                            isLinear=true;
                            isVariable=true;
                            isSimpleTLV=true;
                        }
                        else if ((c & 0x7)==0x6){
                            fileDescription="EF, Cyclic";
                            isCyclic=true;
                        }
                        else if ((c & 0x7)==0x7){
                            fileDescription="EF, Cyclic, simple TLV";
                            isCyclic=true;
                            isSimpleTLV=true;
                        }
                    }
                    else
                        fileDescription="DF";
                    // check byte 2
                    if (currval.length()>1)
                    {
                        c=(unsigned char)currval[1];
                        if ((c & 0x60)==0x00)
                            fileAttributes="one time write";
                        else if ((c & 0x60)==0x40)
                            fileAttributes="write OR";
                        else if ((c & 0x60)==0x60)
                            fileAttributes="write AND";
                    } // if more than 1 byte
                    // check byte 3
                    if (currval.length()==4)
                        maxRecordLength=(unsigned char)currval[0]<<8;
                    else if (currval.length()==3)
                        maxRecordLength=((unsigned char)currval[0]<<8)+
                        ((unsigned char)currval[1]);
                } // if !tag empty
                break;
                
            case 3: // file identifier
                fileId=((unsigned char)currval[0]<<8)+
                ((unsigned char)currval[1]);
                break;
                
            case 4: // DF name
                fileName=currtlv.ref().getValue();
                break;
                
            default: //
                break;
        } // switch
}






