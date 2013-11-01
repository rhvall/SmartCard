//
//  CRPointer.h
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
/***************************************************************************
 $RCSfile: ctpointer.h,v $
 -------------------
 cvs         : $Id: ctpointer.h,v 1.3 2003/01/10 20:02:16 aquamaniac Exp $
 begin       : Tue Dec 13 2001
 copyright   : (C) 2001 by Martin Preuss
 email       : martin@aquamaniac.de
 
 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CTPOINTER_H
#define CTPOINTER_H

#include <stdio.h> /* DEBUG */
#include <string>

#include "CTError.h"

class CTPointerBase;
/* template <class T> class CHIPCARD_API CTPointer; */
class CTPointerObject;
template <class T> class CTPointerCastBase;
template <class T, class U> class CTPointerCast;

#ifndef DOXYGEN
/**
 * This internal class is created by CTPointer. It holds the real pointer
 * and an usage counter. You can neither create nor detroy such an object.
 * @author Martin Preuss<martin@libchipcard.de>
 */
class  CTPointerObject
{
    friend class CTPointerBase;
private:
    
    CTPointerObject(void *obj, string descr = ""):
                    _object(obj),
                    _counter(0),
                    _delete(true),
                    _descr(descr)
    {
    };
    
    ~CTPointerObject()
    {
    };
    
    void setDescription(string descr)
    {
        _descr=descr;
    };
    
    const string &description() const
    {
        return _descr;
    };
	
    int counter() const
    {
        return _counter;
    };
    
    void *_object;
    int _counter;
    bool _delete;
    string _descr;
};
#endif /* DOXYGEN */


/**
 * @short Base class for the smart pointer template class.
 *
 * This is the base class to be inherited by a template class. This
 * cannot be used directly.
 *
 * @author Martin Preuss<martin@libchipcard.de>
 * @ingroup misc
 */

class  CTPointerBase
{
public:
    /**
     * Destructor.
     * If this one gets called, it automagically decrements the usage
     * counter of the object pointed to. If it reaches zero, then no other
     * pointer points to the object and the object faces deletion.
     * @author Martin Preuss<martin@libchipcard.de>
     */
    virtual ~CTPointerBase()
    {
    };
    
    /**
     *  Set the description of this pointer.
     *
     * Useful for debugging purposes.
     * @author Martin Preuss<martin@libchipcard.de>
     */
    void setDescription(string descr)
    {
        _descr=descr;
    };
    
    /**
     *  Get the description of this pointer.
     *
     * Useful for debugging purposes.
     * @author Martin Preuss<martin@libchipcard.de>
     */
    const string &description() const
    {
        return _descr;
    };
    
    /**
     *  Set the description of the object this pointer points to.
     *
     * Useful for debugging purposes.
     * @author Martin Preuss<martin@libchipcard.de>
     */
    void setObjectDescription(string descr)
    {
        if (!descr.empty())
            if (_ptr)
                _ptr->setDescription(descr);
    };
    
    /**
     *  Returns the description of the object.
     *
     * Useful for debugging purposes.
     * @author Martin Preuss<martin@libchipcard.de>
     */
    string objectDescription() const
    {
        if (_ptr)
            return _ptr->description();
        else
            return "";
    };
    
    /**
     *  Returns the reference counter of the object.
     *
     * Useful for debugging purposes.
     * @author Martin Preuss<martin@libchipcard.de>
     */
    int referenceCount() const
    {
        if (_ptr)
            return _ptr->counter();
        else
            return -1;
    };
    
    /**
     *  Equality operator for the object pointed to.
     *
     * This operator checks whether another pointer and this one are
     * pointing to the same data.
     *
     * @author Martin Preuss<martin@libchipcard.de> */
    bool operator==(const CTPointerBase &p) const
    {
        if (_ptr && p._ptr)
            return _ptr->_object==p._ptr->_object;
        else
            return false;
    };
    
    /**
     *  Checks whether both pointers share their data object.
     *
     * @author Martin Preuss<martin@libchipcard.de>
     */
    bool sharingData(const CTPointerBase &p) const
    {
        return (_ptr==p._ptr);
    };
    
    /**
     *  Inequality operator for the object pointed to.
     *
     * This operator checks whether another pointer and this one are
     * not pointing to the same data.
     *
     * @author Martin Preuss<martin@libchipcard.de> */
    bool operator!=(const CTPointerBase &p) const
    {
        if (_ptr && p._ptr)
            return _ptr->_object!=p._ptr->_object;
        else
            return true;
    };
    
    /**
     *  Returns a raw pointer to the stored data.
     *
     * You should not really use this, but if you do so please NEVER
     * delete the object the pointer points to !  AND you should make
     * sure that as long as you are using the pointer returned there
     * is still a CTPointer pointing to it (because if the last CTPointer
     * stops pointing to an object that object gets deleted) !!
     *
     * @author Martin Preuss<martin@libchipcard.de> */
    virtual void* voidptr() const
    {
        if (!_ptr)
            return 0;
        if (!(_ptr->_object))
            return 0;
        return _ptr->_object;
    };
    
    
    /**
     *  Set the auto-deletion behaviour.
     *
     * Set the auto-deletion behaviour of the CTPointerObject (the
     * wrapper object around the "real" object pointed to) that is
     * pointed to by this CTPointer.
     *
     * By default, this is set to setAutoDeletion(true), i.e. the
     * object will automatically be deleted when its last
     * CTPointer gets deleted. On the other hand, when you call
     * this with b=false, then the object this pointer points to will
     * not be deleted by the last CTPointer.
     *
     * This might be useful if you are pointing to constant objects,
     * or if you need to continue using this object through raw
     * pointers elsewhere.
     *
     * This flag is a property of the CTPointerObject, i.e. even for
     * multiple CTPointer's pointing to the same object there is
     * only *one* autoDelete flag per object. Changes to this flag
     * affect all of the CTPointer's at the same time.
     *
     * This CTPointer MUST already point to an object (a NULL
     * pointer is not allowed at this point) since the autodelete flag
     * is a property of the class CTPointerObject. If called on an
     * invalid CTPointer, this method will throw an CTError.
     *
     * @param b True to set automatic deletion to be enabled,
     * false to disable it.
     * @author Martin Preuss<martin@libchipcard.de> */
    void setAutoDelete(bool b)
    {
        if (_ptr)
    {
            if (_ptr->_object)
                _ptr->_delete=b;
        }
        else
            throw CTError("CTCTPointer::setAutoDelete()",
                          k_CTERROR_POINTER,0,0,
                          "No object for "+description());
    };
    
    /**
     *  Returns true if this CTPointer is valid.
     *
     * This tells you if this pointer is pointing to accessible data.
     * @author Martin Preuss<martin@libchipcard.de>
     * @return true if data is accessible, false if no data
     */
    bool isValid() const
    {
        if (_ptr)
            if (_ptr->_object)
                return true;
        return false;
    };
#ifndef DOXYGEN
protected:
    void _attach(CTPointerObject &p)
    {
        _ptr=&p;
        if (_ptr)
    {
            _ptr->_counter++;
            if (_descr.empty())
                _descr=_ptr->_descr;
        }
        else
            throw CTError("CTCTPointer::_attach(&)",
                          k_CTERROR_POINTER,0,0,
                          "No object for "+_descr);
    };
    
    void _attach(CTPointerObject *p)
    {
        _ptr=p;
        if (_ptr)
    {
            _ptr->_counter++;
            if (_descr.empty())
                _descr=_ptr->_descr;
        }
        else
            throw CTError("CTCTPointer::_attach(*)",
                          k_CTERROR_POINTER,0,0,
                          "No object for "+_descr);
    };
    
    void _detach()
    {
        if (_ptr)
    {
            if (_ptr->_counter>0)
    {
                _ptr->_counter--;
                if (_ptr->_counter<1)
    {
                    if (_ptr->_delete)
                        _deleteObject(_ptr->_object);
                    delete _ptr;
                }
            }
        }
        _ptr=0;
    };
    
    /**
     * This method actually deletes the object. Since the base class
     * does not know the type of this object, we have to make this method
     * virtual. The template class MUST override this.
     */
    virtual void _deleteObject(void *p)
    {
    };
    
    CTPointerBase(CTPointerBase &p): _ptr(0)
    {
        if (p._ptr)
            _attach(p._ptr);
    };
    
    CTPointerBase(const CTPointerBase &p) : _ptr(0)
    {
        if (p._ptr)
            _attach(p._ptr);
    };
    
    /**
     * This operator handles the case where you give another pointer as argument.
     * (like pointer1=pointer2).
     * @author Martin Preuss<martin@libchipcard.de>
     */
    void operator=(CTPointerBase &p)
    {
        _detach();
        if (_descr.empty())
            _descr=p._descr;
        if (p._ptr)
            _attach(p._ptr);
            };
    
    void operator=(const CTPointerBase &p)
    {
        _detach();
        if (_descr.empty())
            _descr=p._descr;
        if (p._ptr)
            _attach(p._ptr);
            };
    
    /**
     * This operator handles the case where you do something like this:<BR>
     * pointer=new structXYZ;<BR>
     * @author Martin Preuss<martin@libchipcard.de>
     */
    void operator=(void* obj)
    {
        CTPointerObject *p;
        if (_ptr)
            _detach();
            _ptr=0;
        if (obj==0)
            return;
        p=new CTPointerObject(obj,_descr);
        _attach(p);
    };
    
    /**
     * Constructor.
     */
    CTPointerBase(): _ptr(0)
    {};
    
    CTPointerBase(void *obj): _ptr(0)
    {
        CTPointerObject *p;
        p=new CTPointerObject(obj,_descr);
        _attach(p);
    };
#endif
private:
    CTPointerObject *_ptr;
    string _descr;
};


/**
 * @short A smart pointer template class.
 *
 * This class serves as a smart pointer class that is used in OpenHBCI
 * to avoid memory leaks. It does automatic reference counting for the
 * objects pointed to, like so: Each time a new CTPointer to the same
 * object is created, the reference counter is incremented. Each time
 * a CTPointer to an object is deleted, the reference counter is
 * decremented. When the reference counter reaches zero, the object is
 * deleted.
 *
 * Use it instead of normal pointers, for example:
 * instead of
 *
 * <code>structXYZ *pointer; <BR>
 * pointer = new structXYZ; </code>
 *
 * use this one:
 *
 * <code>CTPointer<structXYZ> pointer; <BR>
 * pointer = new structXYZ; </code>
 *
 * You can access the data easily by using the "*" operator, e.g:
 *
 * <code>structXYZ xyz = *pointer;</code>
 *
 * To access members of the object, either use the "*" operator or the
 * ref() method:
 *
 * <code>a = (*pointer).a; </code> or<br>
 * <code>b = pointer.ref().a;</code>
 *
 * @author Martin Preuss<martin@libchipcard.de>
 * @ingroup misc
 */
template <class T> class  CTPointer: public CTPointerBase {
    friend class CTPointerCastBase<T>;
private:
protected:
    /**
     * This method actually deletes the object. Since the base class
     * does not know the type of this object, we have to make this method
     * virtual. The template class MUST override this.
     */
    virtual void _deleteObject(void *p)
    {
        delete (T*) p;
    };
    
    CTPointer(const CTPointerBase &p): CTPointerBase(p)
    {
    };
    
public:
    /**
     *  Empty Constructor.
     */
    CTPointer(): CTPointerBase(){};
    
    /**
     *  Constructor with object to be pointing to.
     */
    CTPointer(T *obj): CTPointerBase(obj)
    {
    };
    
    /**  Copy constructor */
    CTPointer(const CTPointer<T> &p) : CTPointerBase(p)
    {
    };
    
    /**
     *  Destructor.
     *
     * If this one gets called, it automagically decrements the usage
     * counter of the object pointed to. If it reaches zero, then no other
     * pointer points to the object and the object will be deleted.
     * @author Martin Preuss<martin@libchipcard.de>
     */
    virtual ~CTPointer()
    {
        _detach();
    };
    
    /** @name Copy Operators */
    //@{
    /**
     *  Copy operator with object pointed to.
     *
     * This operator handles the case where you do something like this:<BR>
     * pointer=new structXYZ;<BR>
     * @author Martin Preuss<martin@libchipcard.de>
     */
    void operator=(T* obj)
    {
        CTPointerBase::operator=(obj);
    };
    
    /**
     *  Copy operator with another CTPointer.
     *
     * This operator handles the case where you give another pointer
     * as argument.  (like pointer1=pointer2).
     *
     * @author Martin Preuss<martin@libchipcard.de> */
    void operator=(CTPointer<T> &p)
    {
        CTPointerBase::operator=(p);
    };
    
    /**
     *  Copy operator with another const CTPointer.
     *
     * This operator handles the case where you give another pointer
     * as argument.  (like pointer1=pointer2).
     *
     * @author Martin Preuss<martin@libchipcard.de> */
    void operator=(const CTPointer<T> &p)
    {
        CTPointerBase::operator=(p);
    };
    //@}
    
    /** @name Object Access */
    //@{
    /**
     *  Returns a reference to the object pointed to.
     *
     * If the CTPointer is invalid, this throws a CTError.
     */
    T& ref() const
    {
        T* p;
        
        p=ptr();
        if (!p)
            throw CTError("CTCTPointer::ref()",
                          k_CTERROR_POINTER,0,0,
                          "No object for "+description());
        return *p;
    };
    
    /**
     *  Returns a reference to the object pointed to.
     *
     * If the CTPointer is invalid, this throws a CTError.
     */
    T& operator*() const
    {
        return ref();
    };
    
    /**  Returns a raw pointer to the stored data.
     *
     * If you can continue using only CTPointer's, you should not
     * really need to use this. This method is necessary if and only
     * if you need to use a "raw C pointer" of the object pointed to.
     *
     * So if you need to use this method while there is still a
     * CTPointer pointing to it, please <i>never</i> delete the object
     * returned. The last remaining CTPointer's will take care of
     * deletion.
     *
     * On the other hand, if you need to use this pointer longer than
     * the last CTPointer would exist, then either try to keep a CTPointer
     * around long enough, or you need to consider setting
     * CTPointerBase::setAutoDelete appropriately. (Because if the last
     * CTPointer stops pointing to an object, then that object will get
     * deleted unless CTPointerBase::setAutoDelete was changed.)
     *
     * @author Martin Preuss<martin@libchipcard.de> */
    virtual T* ptr() const
    {
        return (T*)CTPointerBase::voidptr();
    };
    //@}
    
    /** @name Type cast */
    //@{
    /**
     * @short Returns a type-safe casted CTPointer of the given type.
     *
     * This method returns a type-safe casted CTPointer of the given
     * type.  This obeys the same rules as a
     * <code>dynamic_cast<TARGET_TYPE></code>, and in fact internally
     * a <code>dynamic_cast</code> is used.
     *
     * Use it like this:
     * <pre>
     * class type_X;
     * class type_Y : public type_X;
     *
     * CTPointer<type_X> pX;
     * CTPointer<type_Y> pY = new type_Y;
     * pX = pY.cast<type_X>();
     * </pre>
     *
     * The casting fails if it is impossibe to safely cast the
     * "type_Y" to "type_X". In that case, an CTError is
     * thrown. Also, if you call this method on an invalid
     * CTPointer, a CTError is thrown.
     *
     * @author Martin Preuss<martin@libchipcard.de> */
    template <class U> CTPointer<U> cast() const
    {
        return CTPointerCast<U,T>::cast(*this);
        /* return CTPointer<U>(*this); */
        
    };
    
    //@}
    
    /** @name Equality */
    //@{
    /**
     *  Equality operator for the object pointed to.
     *
     * This operator checks whether another pointer and this one are
     * pointing to the same data.
     *
     * @author Martin Preuss<martin@libchipcard.de> */
    bool operator==(const CTPointer<T> &p) const
    {
        return CTPointerBase::operator==(p);
    };
    
    /**
     *  Inequality operator for the object pointed to.
     *
     * This operator checks whether another pointer and this one are
     * not pointing to the same data.
     *
     * @author Martin Preuss<martin@libchipcard.de> */
    bool operator!=(const CTPointer<T> &p) const
    {
        return CTPointerBase::operator!=(p);
    };
    
    /**
     *  Checks whether both pointers share their data object.
     *
     * This method checks whether another pointer and this one share
     * the same internal data object and thus also the same data
     * pointed to. This is a stronger condition than
     * <code>operator==</code>. This method returns true only if one
     * CTPointer has been copied to other CTPointer's. But as soon as some
     * "raw C pointers" have been assigned to different CTPointer, this
     * method would return false.  In that latter case, the
     * <code>operator==</code> would still return true, so that is why
     * the <code>operator==</code> is more likely to be useful.
     *
     * @author Martin Preuss<martin@libchipcard.de> */
    bool sharingData(const CTPointer<T> &p) const
    {
        return CTPointerBase::sharingData(p);
    };
    //@}
    
};


#ifndef DOXYGEN

/**
 *
 */
template <class T> class  CTPointerCastBase {
protected:
    CTPointerCastBase();
    ~CTPointerCastBase();
    
    static CTPointer<T> makeCTPointer(const CTPointerBase &p)
    {
        return CTPointer<T>(p);
    };
};


/**
 * This class lets you safely cast one CTPointer to another one.
 * It will automatically perform type checking. If this class is unable to
 * cast then it throws an CTError.
 * You can use this if you have a CTPointer for an object but
 * you need a pointer for a base object. For example:
 * <pre>
 * class BaseClass {
 *   BaseClass();
 *   ~BaseClass();
 * };
 *
 * class InheritingClass: public BaseClass {
 *   InheritingClass();
 *   ~InheritingClass();
 * };
 *
 * CTPointer<InheritingClass> pInheriting;
 * CTPointer<BaseClass> pBase;
 *
 * pInheriting=new InheritingClass();
 * pBase==CTPointerCast<BaseClass,InheritingClass>::cast(pInheriting);
 * </pre>
 * @author Martin Preuss<martin@libchipcard.de>
 */
template <class T, class U> class  CTPointerCast
:public CTPointerCastBase<T>
{
public:
    /**
     * If the first template parameter is the base class to the second
     * template parameter then this method will cast a pointer to an
     * inheriting class into a pointer to the base class (downcast).
     * If you want just the opposite then you only need to exchange the order
     * of the template parameters (upcast).
     * @author Martin Preuss<martin@libchipcard.de>
     */
    static  CTPointer<T> cast(const CTPointer<U> &u)
    {
        U *uo;
        T *t;
        
        /* check if given pointer is valid */
        if (!u.isValid())
            throw CTError("CTPointerCast::cast()",
                          k_CTERROR_POINTER,0,0,
                          "No object for "+u.description());
        
        /* then try to cast the pointer */
        uo=u.ptr();
        t=dynamic_cast<T*>(uo);
        
        /* could we cast it ? */
        if (t==0)
        /* no, throw */
            throw CTError("CTCTPointerCast::cast()",
                          k_CTERROR_POINTER,0,0,
                          "Bad cast "+u.description());
        /* otherwise create a new pointer */
        return makeCTPointer(u);
    };
    
};

#endif /* DOXYGEN */

#endif /* CTPOINTER_H */

