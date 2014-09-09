/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmMacro.h,v $
  Language:  C++
  Date:      $Date: 2007/08/22 16:14:04 $
  Version:   $Revision: 1.7 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMMACRO_H_
#define _GDCMMACRO_H_

//-----------------------------------------------------------------------------
#define gdcmTypeMacro(type)               \
   private :                              \
      type(type &); /* Not implemented */ \
      type &operator=(type &) /* Not implemented */

#define gdcmNewMacro(type)                \
   public :                               \
      static type *New() {return new type(); } /* Not implemented */

//-----------------------------------------------------------------------------
//
// Define GDCM_LEGACY macro to mark legacy methods where they are
// declared in their class.
//
// WARNING : Don't try to use it with 'inline' methods !
//
//Example usage:
//
//   // @deprecated Replaced by MyOtherMethod() as of gdcm 2.0.
//   GDCM_LEGACY(void MyMethod())
#if defined(GDCM_LEGACY_REMOVE)
  // Remove legacy methods completely.
# define GDCM_LEGACY(method)
#elif defined(GDCM_LEGACY_SILENT)
  // Provide legacy methods with no warnings.
# define GDCM_LEGACY(method) method;
#else
  // Setup compile-time warnings for uses of deprecated methods if
  // possible on this compiler.
# if defined(__GNUC__) && !defined(__INTEL_COMPILER) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
#if defined(__APPLE__) && (__GNUC__ == 3) && (__GNUC_MINOR__ == 3)
// Seems like there is a bug in APPLE gcc for deprecated attribute and ctor
// This is fixed in g++ 4.0 (Tiger)
#  define GDCM_LEGACY(method) method;
#else
#  define GDCM_LEGACY(method) method __attribute__((deprecated));
#endif
# elif defined(_MSC_VER) && _MSC_VER >= 1300
#  define GDCM_LEGACY(method) __declspec(deprecated) method;
# else
#  define GDCM_LEGACY(method) method;
# endif
#endif

// Macros to create runtime deprecation warning messages in function
// bodies.  Example usage:
//
//   void MyClass::MyOldMethod()
//   {
//     GDCM_LEGACY_BODY(MyClass::MyOldMethod, 2.0);
//   }
//
//   void MyClass::MyMethod()
//   {
//     GDCM_LEGACY_REPLACED_BODY(MyClass::MyMethod, 5.0,
//                               MyClass::MyOtherMethod);
//   }
#if defined(GDCM_LEGACY_REMOVE) || defined(GDCM_LEGACY_SILENT)
# define GDCM_LEGACY_BODY(method, version)
# define GDCM_LEGACY_REPLACED_BODY(method, version, replace)
#else
# define GDCM_LEGACY_BODY(method, version) \
  gdcmWarningMacro(#method " was deprecated for gdcm" #version " and will be removed in a future version.")
# define GDCM_LEGACY_REPLACED_BODY(method, version, replace) \
  gdcmWarningMacro(#method " was deprecated for gdcm" #version " and will be removed in a future version.  Use " #replace " instead.")
#endif

//-----------------------------------------------------------------------------
#endif
