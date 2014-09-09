%module gdcm
#pragma SWIG nowarn=504,510
%{
#include <iostream>

#include "gdcmCommon.h"
#include "gdcmBase.h"
#include "gdcmRefCounter.h"
#include "gdcmCommand.h"
#include "gdcmCommandPy.h"
#include "gdcmDebug.h"
#include "gdcmCommandManager.h"
#include "gdcmTagKey.h"
#include "gdcmVRKey.h"
#include "gdcmDict.h"
#include "gdcmDictEntry.h"
#include "gdcmDictSet.h"
#include "gdcmDicomDir.h"
#include "gdcmDicomDirElement.h"
#include "gdcmDicomDirImage.h"
#include "gdcmDicomDirMeta.h"
#include "gdcmDicomDirObject.h"
#include "gdcmDicomDirPatient.h"
#include "gdcmDicomDirStudy.h"
#include "gdcmDicomDirSerie.h"
#include "gdcmDicomDirVisit.h"
#include "gdcmDicomDirPrivate.h"
#include "gdcmDirList.h"
#include "gdcmDocEntrySet.h"
#include "gdcmDocument.h"
#include "gdcmElementSet.h"
#include "gdcmFileHelper.h"
#include "gdcmGlobal.h"
#include "gdcmFile.h"
#include "gdcmSerieHelper.h"
#include "gdcmRLEFramesInfo.h"
#include "gdcmJPEGFragmentsInfo.h"
#include "gdcmSQItem.h"
#include "gdcmUtil.h"
#include "gdcmDocEntry.h"
#include "gdcmDataEntry.h"
#include "gdcmSeqEntry.h"
#include "gdcmVR.h"
#include "gdcmTS.h"
#include "gdcmDictGroupName.h"

/// This is required in order to avoid %including all the gdcm include files.
using namespace GDCM_NAME_SPACE;
%}


///////////////////////  typemap section  ////////////////////////////////////

////////////////////////////////////////////////
// Redefine all types used
typedef char               int8_t;
typedef unsigned char      uint8_t;
typedef short              int16_t;
typedef unsigned short     uint16_t;
typedef int                int32_t;
typedef unsigned int       uint32_t;
typedef long long          int64_t;
typedef unsigned long long uint64_t;

////////////////////////////////////////////////
// Convert a DocEntry * to the real derived class
%typemap(out) GDCM_NAME_SPACE::DocEntry *
{
   PyObject *newEntry;

   if($1)
   {
      if(dynamic_cast<SeqEntry *>($1)) // SeqEntry *
         newEntry = SWIG_NewPointerObj($1,SWIGTYPE_p_GDCM_NAME_SPACE__SeqEntry,0);
      else if(dynamic_cast<DataEntry *>($1)) // DataEntry *
         newEntry = SWIG_NewPointerObj($1,SWIGTYPE_p_GDCM_NAME_SPACE__DataEntry,0);
      else
         newEntry = NULL;
   }
   else
   {
      newEntry = Py_BuildValue("");
   }
   $result = newEntry;
}

////////////////////  STL string versus Python str  ////////////////////////
// Convertion returning a C++ string.
%typemap(out) std::string
{
    $result = PyString_FromString(($1).c_str());
}

%typemap(out) string
{
    $result = PyString_FromString(($1).c_str());
}

%typemap(out) std::string const &
{
    $result = PyString_FromString(($1)->c_str());
}

// Convertion of incoming Python str to STL string
%typemap(python, in) const std::string, std::string
{
  $1 = PyString_AsString($input);
}

// Same convertion as above but references (since swig converts C++
// references to pointers)
%typemap(python, in) std::string const &
{
   $1 = new std::string( PyString_AsString( $input ) );
}

////////////////////  gdcm.TagName versus Python str  //////////////////////
%typemap(out) GDCM_NAME_SPACE::TagName, const GDCM_NAME_SPACE::TagName &
{
    $result = PyString_FromString(($1)->c_str());
}

// Convertion of incoming Python str to STL string
%typemap(python, in) const GDCM_NAME_SPACE::TagName, GDCM_NAME_SPACE::TagName
{
  $1 = PyString_AsString($input);
}

// Same convertion as above but references (since swig converts C++
// refererences to pointers)
%typemap(python, in) GDCM_NAME_SPACE::TagName const &
{
   $1 = new std::string( PyString_AsString( $input ) );
}

////////////////////////////////////////////////////////////////////////////
// Because overloading and %rename don't work together (see below Note 1)
// we need to ignore some methods (e.g. the overloaded default constructor).
// The GDCM_NAME_SPACE::File class doesn't have any SetFilename method anyhow, and
// this constructor is only used internaly (not from the API) so this is
// not a big loss.
%ignore GDCM_NAME_SPACE::binary_write(std::ostream &,uint32_t const &);
%ignore GDCM_NAME_SPACE::binary_write(std::ostream &,uint16_t const &);

%ignore GDCM_NAME_SPACE::VRKey::operator=(const VRKey &_val);
%ignore GDCM_NAME_SPACE::VRKey::operator=(const std::string &_val);
%ignore GDCM_NAME_SPACE::VRKey::operator=(const char *_val);
%ignore GDCM_NAME_SPACE::VRKey::operator[](const unsigned int &_id) const;
%ignore GDCM_NAME_SPACE::VRKey::operator[](const unsigned int &_id);

%ignore GDCM_NAME_SPACE::TagKey::operator=(const TagKey &_val);
%ignore GDCM_NAME_SPACE::TagKey::operator[](const unsigned int &_id) const;
%ignore GDCM_NAME_SPACE::TagKey::operator[](const unsigned int &_id);

// Ignore all placed in gdcmCommon.h
%ignore GDCM_UNKNOWN;
%ignore GDCM_UNFOUND;
%ignore GDCM_BINLOADED;
%ignore GDCM_NOTLOADED;
%ignore GDCM_UNREAD;
%ignore GDCM_NOTASCII;
%ignore GDCM_PIXELDATA;
%ignore GDCM_LEGACY;
%ignore GDCM_VRUNKNOWN;
%ignore GDCM_FILESEPARATOR;

%constant const char *UNKNOWN        = "GDCM::Unknown";
%constant const char *UNFOUND        = "GDCM::Unfound";
%constant const char *BINLOADED      = "GDCM::Binary data";
%constant const char *NOTLOADED      = "GDCM::NotLoaded";
%constant const char *UNREAD         = "GDCM::UnRead";
%constant const char *GDCM_NOTASCII  = "GDCM::NotAscii";
%constant const char *GDCM_PIXELDATA = "GDCM::Pixel Data to be loaded";
%constant const char *VRUNKNOWN      = "  ";
%constant const char GDCM_FILESEPARATOR = '\\';
////////////////////////////////////////////////////////////////////////////
// Warning: Order matters !
%include "gdcmCommon.h"
%include "gdcmBase.h"
%include "gdcmRefCounter.h"
%include "gdcmCommand.h"
%include "gdcmCommandPy.h"
%include "gdcmDebug.h"
%include "gdcmCommandManager.h"
%include "gdcmTagKey.h"
%include "gdcmVRKey.h"
%include "gdcmDictEntry.h"
%include "gdcmDict.h"
%include "gdcmDictSet.h"
%include "gdcmDocEntrySet.h"
%include "gdcmElementSet.h"
%include "gdcmSQItem.h"
%include "gdcmDicomDirElement.h"
%include "gdcmDicomDirObject.h"
%include "gdcmDicomDirImage.h"
%include "gdcmDicomDirPrivate.h"
%include "gdcmDicomDirSerie.h"
%include "gdcmDicomDirVisit.h"
%include "gdcmDicomDirStudy.h"
%include "gdcmDicomDirPatient.h"
%include "gdcmDicomDirMeta.h"
%include "gdcmDocument.h"
%include "gdcmFile.h"
%include "gdcmSerieHelper.h"
%include "gdcmFileHelper.h"
%include "gdcmUtil.h"
%include "gdcmGlobal.h"
%include "gdcmDicomDir.h"
%include "gdcmDocEntry.h"
%include "gdcmDataEntry.h"
%include "gdcmSeqEntry.h"
%include "gdcmVR.h"
%include "gdcmTS.h"
%include "gdcmDictGroupName.h"
