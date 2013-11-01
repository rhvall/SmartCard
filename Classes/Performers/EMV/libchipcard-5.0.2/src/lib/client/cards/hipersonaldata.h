/* This file is auto-generated from "hipersonaldata.xml" by the typemaker
   tool of Gwenhywfar. 
   Do not edit this file -- all changes will be lost! */
#ifndef HIPERSONALDATA_H
#define HIPERSONALDATA_H

/** @page P_LC_HI_PERSONAL_DATA_PUBLIC LC_HIPersonalData (public)
This page describes the properties of LC_HI_PERSONAL_DATA
@anchor LC_HI_PERSONAL_DATA_InsuranceId
<h3>InsuranceId</h3>
<p>
</p>
<p>
Set this property with @ref LC_HIPersonalData_SetInsuranceId, 
get it with @ref LC_HIPersonalData_GetInsuranceId
</p>

@anchor LC_HI_PERSONAL_DATA_Prename
<h3>Prename</h3>
<p>
</p>
<p>
Set this property with @ref LC_HIPersonalData_SetPrename, 
get it with @ref LC_HIPersonalData_GetPrename
</p>

@anchor LC_HI_PERSONAL_DATA_Name
<h3>Name</h3>
<p>
</p>
<p>
Set this property with @ref LC_HIPersonalData_SetName, 
get it with @ref LC_HIPersonalData_GetName
</p>

@anchor LC_HI_PERSONAL_DATA_Title
<h3>Title</h3>
<p>
</p>
<p>
Set this property with @ref LC_HIPersonalData_SetTitle, 
get it with @ref LC_HIPersonalData_GetTitle
</p>

@anchor LC_HI_PERSONAL_DATA_NameSuffix
<h3>NameSuffix</h3>
<p>
</p>
<p>
Set this property with @ref LC_HIPersonalData_SetNameSuffix, 
get it with @ref LC_HIPersonalData_GetNameSuffix
</p>

@anchor LC_HI_PERSONAL_DATA_Sex
<h3>Sex</h3>
<p>
</p>
<p>
Set this property with @ref LC_HIPersonalData_SetSex, 
get it with @ref LC_HIPersonalData_GetSex
</p>

@anchor LC_HI_PERSONAL_DATA_DateOfBirth
<h3>DateOfBirth</h3>
<p>
</p>
<p>
Set this property with @ref LC_HIPersonalData_SetDateOfBirth, 
get it with @ref LC_HIPersonalData_GetDateOfBirth
</p>

@anchor LC_HI_PERSONAL_DATA_AddrZipCode
<h3>AddrZipCode</h3>
<p>
</p>
<p>
Set this property with @ref LC_HIPersonalData_SetAddrZipCode, 
get it with @ref LC_HIPersonalData_GetAddrZipCode
</p>

@anchor LC_HI_PERSONAL_DATA_AddrCity
<h3>AddrCity</h3>
<p>
</p>
<p>
Set this property with @ref LC_HIPersonalData_SetAddrCity, 
get it with @ref LC_HIPersonalData_GetAddrCity
</p>

@anchor LC_HI_PERSONAL_DATA_AddrState
<h3>AddrState</h3>
<p>
</p>
<p>
Set this property with @ref LC_HIPersonalData_SetAddrState, 
get it with @ref LC_HIPersonalData_GetAddrState
</p>

@anchor LC_HI_PERSONAL_DATA_AddrCountry
<h3>AddrCountry</h3>
<p>
</p>
<p>
Set this property with @ref LC_HIPersonalData_SetAddrCountry, 
get it with @ref LC_HIPersonalData_GetAddrCountry
</p>

@anchor LC_HI_PERSONAL_DATA_AddrStreet
<h3>AddrStreet</h3>
<p>
</p>
<p>
Set this property with @ref LC_HIPersonalData_SetAddrStreet, 
get it with @ref LC_HIPersonalData_GetAddrStreet
</p>

@anchor LC_HI_PERSONAL_DATA_AddrHouseNum
<h3>AddrHouseNum</h3>
<p>
</p>
<p>
Set this property with @ref LC_HIPersonalData_SetAddrHouseNum, 
get it with @ref LC_HIPersonalData_GetAddrHouseNum
</p>

*/
#ifdef __cplusplus
extern "C" {
#endif

typedef struct LC_HI_PERSONAL_DATA LC_HI_PERSONAL_DATA;

#ifdef __cplusplus
} /* __cplusplus */
#endif

#include <gwenhywfar/db.h>
/* headers */
#include <chipcard/chipcard.h>
#include <gwenhywfar/gwentime.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  LC_HIPersonalData_SexUnknown=-1,
  /** Male.
  */
  LC_HIPersonalData_SexMale,
  /** Female.
  */
  LC_HIPersonalData_SexFemale
} LC_HI_PERSONAL_DATA_SEX;

CHIPCARD_API LC_HI_PERSONAL_DATA_SEX LC_HIPersonalData_Sex_fromString(const char *s);
CHIPCARD_API const char *LC_HIPersonalData_Sex_toString(LC_HI_PERSONAL_DATA_SEX v);


/** Creates a new object.
*/
CHIPCARD_API LC_HI_PERSONAL_DATA *LC_HIPersonalData_new();
/** Creates an object from the data in the given GWEN_DB_NODE
*/
CHIPCARD_API LC_HI_PERSONAL_DATA *LC_HIPersonalData_fromDb(GWEN_DB_NODE *db);
/** Creates and returns a deep copy of thegiven object.
*/
CHIPCARD_API LC_HI_PERSONAL_DATA *LC_HIPersonalData_dup(const LC_HI_PERSONAL_DATA*st);
/** Destroys the given object.
*/
CHIPCARD_API void LC_HIPersonalData_free(LC_HI_PERSONAL_DATA *st);
/** Increments the usage counter of the given object, so an additional free() is needed to destroy the object.
*/
CHIPCARD_API void LC_HIPersonalData_Attach(LC_HI_PERSONAL_DATA *st);
/** Reads data from a GWEN_DB.
*/
CHIPCARD_API int LC_HIPersonalData_ReadDb(LC_HI_PERSONAL_DATA *st, GWEN_DB_NODE *db);
/** Stores an object in the given GWEN_DB_NODE
*/
CHIPCARD_API int LC_HIPersonalData_toDb(const LC_HI_PERSONAL_DATA*st, GWEN_DB_NODE *db);
/** Returns 0 if this object has not been modified, !=0 otherwise
*/
CHIPCARD_API int LC_HIPersonalData_IsModified(const LC_HI_PERSONAL_DATA *st);
/** Sets the modified state of the given object
*/
CHIPCARD_API void LC_HIPersonalData_SetModified(LC_HI_PERSONAL_DATA *st, int i);


/**
* Returns the property @ref LC_HI_PERSONAL_DATA_InsuranceId
*/
CHIPCARD_API const char *LC_HIPersonalData_GetInsuranceId(const LC_HI_PERSONAL_DATA *el);
/**
* Set the property @ref LC_HI_PERSONAL_DATA_InsuranceId
*/
CHIPCARD_API void LC_HIPersonalData_SetInsuranceId(LC_HI_PERSONAL_DATA *el, const char *d);

/**
* Returns the property @ref LC_HI_PERSONAL_DATA_Prename
*/
CHIPCARD_API const char *LC_HIPersonalData_GetPrename(const LC_HI_PERSONAL_DATA *el);
/**
* Set the property @ref LC_HI_PERSONAL_DATA_Prename
*/
CHIPCARD_API void LC_HIPersonalData_SetPrename(LC_HI_PERSONAL_DATA *el, const char *d);

/**
* Returns the property @ref LC_HI_PERSONAL_DATA_Name
*/
CHIPCARD_API const char *LC_HIPersonalData_GetName(const LC_HI_PERSONAL_DATA *el);
/**
* Set the property @ref LC_HI_PERSONAL_DATA_Name
*/
CHIPCARD_API void LC_HIPersonalData_SetName(LC_HI_PERSONAL_DATA *el, const char *d);

/**
* Returns the property @ref LC_HI_PERSONAL_DATA_Title
*/
CHIPCARD_API const char *LC_HIPersonalData_GetTitle(const LC_HI_PERSONAL_DATA *el);
/**
* Set the property @ref LC_HI_PERSONAL_DATA_Title
*/
CHIPCARD_API void LC_HIPersonalData_SetTitle(LC_HI_PERSONAL_DATA *el, const char *d);

/**
* Returns the property @ref LC_HI_PERSONAL_DATA_NameSuffix
*/
CHIPCARD_API const char *LC_HIPersonalData_GetNameSuffix(const LC_HI_PERSONAL_DATA *el);
/**
* Set the property @ref LC_HI_PERSONAL_DATA_NameSuffix
*/
CHIPCARD_API void LC_HIPersonalData_SetNameSuffix(LC_HI_PERSONAL_DATA *el, const char *d);

/**
* Returns the property @ref LC_HI_PERSONAL_DATA_Sex
*/
CHIPCARD_API LC_HI_PERSONAL_DATA_SEX LC_HIPersonalData_GetSex(const LC_HI_PERSONAL_DATA *el);
/**
* Set the property @ref LC_HI_PERSONAL_DATA_Sex
*/
CHIPCARD_API void LC_HIPersonalData_SetSex(LC_HI_PERSONAL_DATA *el, LC_HI_PERSONAL_DATA_SEX d);

/**
* Returns the property @ref LC_HI_PERSONAL_DATA_DateOfBirth
*/
CHIPCARD_API const GWEN_TIME *LC_HIPersonalData_GetDateOfBirth(const LC_HI_PERSONAL_DATA *el);
/**
* Set the property @ref LC_HI_PERSONAL_DATA_DateOfBirth
*/
CHIPCARD_API void LC_HIPersonalData_SetDateOfBirth(LC_HI_PERSONAL_DATA *el, const GWEN_TIME *d);

/**
* Returns the property @ref LC_HI_PERSONAL_DATA_AddrZipCode
*/
CHIPCARD_API const char *LC_HIPersonalData_GetAddrZipCode(const LC_HI_PERSONAL_DATA *el);
/**
* Set the property @ref LC_HI_PERSONAL_DATA_AddrZipCode
*/
CHIPCARD_API void LC_HIPersonalData_SetAddrZipCode(LC_HI_PERSONAL_DATA *el, const char *d);

/**
* Returns the property @ref LC_HI_PERSONAL_DATA_AddrCity
*/
CHIPCARD_API const char *LC_HIPersonalData_GetAddrCity(const LC_HI_PERSONAL_DATA *el);
/**
* Set the property @ref LC_HI_PERSONAL_DATA_AddrCity
*/
CHIPCARD_API void LC_HIPersonalData_SetAddrCity(LC_HI_PERSONAL_DATA *el, const char *d);

/**
* Returns the property @ref LC_HI_PERSONAL_DATA_AddrState
*/
CHIPCARD_API const char *LC_HIPersonalData_GetAddrState(const LC_HI_PERSONAL_DATA *el);
/**
* Set the property @ref LC_HI_PERSONAL_DATA_AddrState
*/
CHIPCARD_API void LC_HIPersonalData_SetAddrState(LC_HI_PERSONAL_DATA *el, const char *d);

/**
* Returns the property @ref LC_HI_PERSONAL_DATA_AddrCountry
*/
CHIPCARD_API const char *LC_HIPersonalData_GetAddrCountry(const LC_HI_PERSONAL_DATA *el);
/**
* Set the property @ref LC_HI_PERSONAL_DATA_AddrCountry
*/
CHIPCARD_API void LC_HIPersonalData_SetAddrCountry(LC_HI_PERSONAL_DATA *el, const char *d);

/**
* Returns the property @ref LC_HI_PERSONAL_DATA_AddrStreet
*/
CHIPCARD_API const char *LC_HIPersonalData_GetAddrStreet(const LC_HI_PERSONAL_DATA *el);
/**
* Set the property @ref LC_HI_PERSONAL_DATA_AddrStreet
*/
CHIPCARD_API void LC_HIPersonalData_SetAddrStreet(LC_HI_PERSONAL_DATA *el, const char *d);

/**
* Returns the property @ref LC_HI_PERSONAL_DATA_AddrHouseNum
*/
CHIPCARD_API const char *LC_HIPersonalData_GetAddrHouseNum(const LC_HI_PERSONAL_DATA *el);
/**
* Set the property @ref LC_HI_PERSONAL_DATA_AddrHouseNum
*/
CHIPCARD_API void LC_HIPersonalData_SetAddrHouseNum(LC_HI_PERSONAL_DATA *el, const char *d);


#ifdef __cplusplus
} /* __cplusplus */
#endif


#endif /* HIPERSONALDATA_H */
