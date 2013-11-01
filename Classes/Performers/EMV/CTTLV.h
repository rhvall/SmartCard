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

#ifndef CTTLV_H
#define CTTLV_H

class CTTLV;
class CTTLV_FCI;

#define k_CTTLV_CLASS_UNIVERSAL   0x00
#define k_CTTLV_CLASS_APPLICATION 0x40
#define k_CTTLV_CLASS_CONTEXT     0x80
#define k_CTTLV_CLASS_PRIVATE     0xc0
#define k_CTTLV_TYPE_CONSTRUCTED  0x20

#include <string>

/**
 * This class handles BER-TLV tags also used by ASN.1. Such a tag consists
 * of at least two bytes:
 * <ul>
 *  <li>Tag (including class bits, bit telling if the object is constructed)</li>
 *  <li>Length (1 to 3 bytes giving the data length)</li>
 *  <li>Value (only if length !=0) </li>
 * </ul>
 * @ingroup misc
 */
class CTTLV
{
private:
    unsigned int _type;
    unsigned int _tag;
    unsigned int _length;
    string _value;
    unsigned int _size;
    bool _valid;
    bool _simple;
    
public:
    CTTLV();
    
    /**
     * Constructor for a tag from a string.
     * @author Martin Preuss<martin@libchipcard.de>
     * @param s string which is assumed to contain a tag
     * @param pos reference to a variable containing the position start
     * position within the given string. Upon return this variable will
     * contain the first position behind the tag.
     * @param simple if true then the tag in the given string is assumed to
     * be a Simple-TLV tag. Otherwise a BER-TLV tag is assumed.
     */
    CTTLV(const string &s, unsigned int &pos, bool simple=false);
    
    /**
     * Constructor for a BER-TLV tag.
     * A BER_TLV tag is a rather advanced tag compared to Simple-TLV.
     * Such a tag contains information about the class this tag belongs to,
     * or if this tag consists of sub tags.
     */
    CTTLV(unsigned int tag, unsigned int cl, bool cstrc,string d);
    
    /**
     * Constructor for a Simple-TLV tag.
     * A Simple-TLV tag has no encodings for the class type, it is very
     * simple.
     */
    CTTLV(unsigned int tag, string d);
    ~CTTLV();
    unsigned int getTag() const { return _tag;};
    unsigned int getLength() const { return _length;};
    string getValue() const { return _value;};
    unsigned int getSize() const { return _size;};
    bool isValid() const { return _valid;};
    
    /**
     * This is only usefull with BER-TLV tags, since Simple-TLV tags don't
     * tell if they are constructed. Constructed means that this tag is
     * assumed to have sub tags in it.
     */
    bool isConstructed() const { return _type &0x20;};
    
    bool isSimple() const { return _simple;};
    
    /**
     * This is only usefull with BER-TLV tags, since Simple-TLV tags don't
     * have classes.
     */
    unsigned int getClass() const { return _type & 0xc0;};
    
    /**
     * Writes the content of this tag into a string.
     */
    string toString();
    
};



/**
 * @ingroup misc
 */
class  CTTLV_FCI
{
private:
protected:
    /**
     * This method is called on all tags which are found inside a FCI
     * If you want to support additional tags you should overload this method.
     */
    virtual void parseTag(CTPointer<CTTLV> tag);
public:
    unsigned int fileSize;
    unsigned int fileSizeRaw;
    unsigned int fileId;
    string fileType;
    bool isEF;
    string fileDescription;
    string fileAttributes;
    unsigned int maxRecordLength;
    string fileName;
    bool isTransparent;
    bool isLinear;
    bool isFixed;
    bool isVariable;
    bool isCyclic;
    bool isSimpleTLV;
    
    /**
     * This parses a string which is assumed to include an FCI.
     * The string given here can either be a full FCI alike tag containing
     * FCI subtags or it can simply be the content of an FCI alike tag.
     * This feature is used by inheriting classes.
     */
    CTTLV_FCI(string fci);
    
    CTTLV_FCI();
    virtual ~CTTLV_FCI();
};
#endif
