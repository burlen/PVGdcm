/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmSystem.h,v $
  Language:  C++
  Date:      $Date: 2008/02/13 18:53:34 $
  Version:   $Revision: 1.6 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMSYSTEM_H_
#define _GDCMSYSTEM_H_

#include "gdcmConfigure.h"

//-----------------------------------------------------------------------------
//This is needed when compiling in debug mode
#ifdef _MSC_VER
// 'identifier' : class 'type' needs to have dll-interface to be used by
// clients of class 'type2'
#pragma warning ( disable : 4251 )
// non dll-interface class 'type' used as base for dll-interface class 'type2'
#pragma warning ( disable : 4275 )
// 'identifier' : identifier was truncated to 'number' characters in the
// debug information
#pragma warning ( disable : 4786 )
//'identifier' : decorated name length exceeded, name was truncated
#pragma warning ( disable : 4503 )
// C++ exception specification ignored except to indicate a 
// function is not __declspec(nothrow)
#pragma warning ( disable : 4290 )
// signed/unsigned mismatch
#pragma warning ( disable : 4018 )
// return type for 'identifier' is '' (ie; not a UDT or reference to UDT. Will
// produce errors if applied using infix notation
#pragma warning ( disable : 4284 )
// 'type' : forcing value to bool 'true' or 'false' (performance warning)
// //#pragma warning ( disable : 4800 )
#endif //_MSC_VER

//-----------------------------------------------------------------------------
#ifdef CMAKE_HAVE_STDINT_H
   #include <stdint.h>
#else
#ifdef CMAKE_HAVE_INTTYPES_H
   // Old system only have this
   #include <inttypes.h>   // For uint8_t uint16_t and uint32_t
#else
// Broken plateforms do not respect C99 and do not provide those typedef
// Special case for recent Borland compiler, comes with stdint.h
#if defined(_MSC_VER) || defined(__BORLANDC__) && (__BORLANDC__ < 0x0560)  \
                      || defined(__MINGW32__)
typedef  signed char         int8_t;
typedef  signed short        int16_t;
typedef  signed int          int32_t;
/// \fixme : what about 64 bits ?
typedef  unsigned char       uint8_t;
typedef  unsigned short      uint16_t;
typedef  unsigned int        uint32_t;
#else
#error "Sorry your plateform is not supported"
#endif // defined(_MSC_VER) || defined(__BORLANDC__) && (__BORLANDC__ < 0x0560)  || defined(__MINGW32__)
#endif // CMAKE_HAVE_INTTYPES_H
#endif // CMAKE_HAVE_STDINT_H

// Basically for VS6 and bcc 5.5.1:
#ifndef UINT32_MAX
#define UINT32_MAX    (4294967295U)
#endif

//-----------------------------------------------------------------------------
#endif
