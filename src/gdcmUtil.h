/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmUtil.h,v $
  Language:  C++
  Date:      $Date: 2008/01/02 14:58:00 $
  Version:   $Revision: 1.72 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMUTIL_H_
#define _GDCMUTIL_H_

#include "gdcmCommon.h"
#include <vector>
#include <string>

namespace GDCM_NAME_SPACE 
{
/**
 * \brief    Here are some utility functions, belonging to the gdcm::Util class,
 *           dealing with strings, file names... that can be called
 *           from anywhere by whomsoever they can help.
 */

//-----------------------------------------------------------------------------

class GDCM_EXPORT Util
{
public:
   static std::string Format(const char *format, ...);
   static void        Tokenize (const std::string &str,
                                std::vector<std::string> &tokens,
                                const std::string &delimiters = " ");
   static int         CountSubstring (const std::string &str,
                                      const std::string &subStr);

   static std::string CreateCleanString(std::string const &s);
   static std::string CreateCleanString(uint8_t *s, int l);
   static bool IsCleanString(std::string const &s);
   static bool IsCleanArea(uint8_t *s, int l);
   static void ReplaceSpecChar(std::string &s, std::string &rep);
   static std::string NormalizePath(std::string const &name);
   static std::string GetPath(std::string const &fullName);
   static std::string GetName(std::string const &fullName);
   static std::string GetCurrentDate();
   static std::string GetCurrentTime();
   static std::string GetCurrentDateTime();
   /// Provides a simple static GetVersion() function
   static std::string GetVersion() 
                      { return GDCM_VERSION;}
   static unsigned int GetCurrentThreadID();
   static unsigned int GetCurrentProcessID();
   static bool         IsCurrentProcessorBigEndian();

   static std::string DicomString(const char *s, size_t l);
   static std::string DicomString(const char *s);
   static bool        DicomStringEqual(const std::string &s1, const char *s2);
   static bool        CompareDicomString(const std::string &s1, 
                                         const char *s2, int op);
   static std::string GetMACAddress();

   static std::string CreateUniqueUID(const std::string &root = "");
   static void SetRootUID(const std::string &root = "");
   static const std::string &GetRootUID();

   static const uint8_t *GetFileMetaInformationVersion() 
                     { return FileMetaInformationVersion;}
   static void SetFileMetaInformationVersion( uint16_t fmiv )
                     { FileMetaInformationVersion = (uint8_t *)&fmiv; }

// ----------------- to be removed later --------------------------     
//To perform a full check of inline functions on all the platforms, 
// we put here the two 'static' functions
// hifpswap and  hNoifpswap in a .h
// They will be remove ASAP

   inline void hifpswap(double *a, double *b)     
   {
      double tmp;
      tmp = *a;
      *a = *b;
      *b = tmp;
   }
   void hNoifpswap(double *a, double *b)    
   {
      double tmp;
      tmp = *a;
      *a = *b;
      *b = tmp;
   }    
   void hfpswap(double *a, double *b);
      
   static inline void sthifpswap(double *a, double *b)     
   {
      double tmp;
      tmp = *a;
      *a = *b;
      *b = tmp;
   }
   static void sthNoifpswap(double *a, double *b)    
   {
      double tmp;
      tmp = *a;
      *a = *b;
      *b = tmp;
   }    
// ------------ end of functions to remove --------------------

// For MD5
static std::string ConvertToMD5 (std::string &stringToCrypt);

private:
   static std::string GetIPAddress(); //Do not expose this method

   static std::string RootUID;
   static const std::string GDCM_UID;
   static uint8_t *FileMetaInformationVersion;
   
   static const uint16_t FMIV;
   static std::string GDCM_MAC_ADDRESS;

// For MD5

/*
  Copyright (C) 1999, 2002 Aladdin Enterprises.  All rights reserved.
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  L. Peter Deutsch
  ghost@aladdin.com
*/

/* $Id: gdcmUtil.h,v 1.72 2008/01/02 14:58:00 malaterre Exp $ */
/*
  Independent implementation of MD5 (RFC 1321).
  This code implements the MD5 Algorithm defined in RFC 1321, whose
  text is available at
 http://www.ietf.org/rfc/rfc1321.txt
  The code is derived from the text of the RFC, including the test suite
  (section A.5) but excluding the rest of Appendix A.  It does not include
  any code or documentation that is identified in the RFC as being
  copyrighted.
  The original and principal author of md5.h is L. Peter Deutsch
  <ghost@aladdin.com>.  Other authors are noted in the change history
  that follows (in reverse chronological order):
  2002-04-13 lpd Removed support for non-ANSI compilers; removed
       references to Ghostscript; clarified derivation from RFC 1321;
       now handles byte order either statically or dynamically.
  1999-11-04 lpd Edited comments slightly for automatic TOC extraction.
  1999-10-18 lpd Fixed typo in header comment (ansi2knr rather than md5);
       added conditionalization for C++ compilation from 
       Martin Purschke <purschke@bnl.gov>.
  1999-05-03 lpd Original version.
 */

/*
 * This package supports both compile-time and run-time determination of CPU
 * byte order.  If ARCH_IS_BIG_ENDIAN is defined as 0, the code will be
 * compiled to run only on little-endian CPUs; if ARCH_IS_BIG_ENDIAN is
 * defined as non-zero, the code will be compiled to run only on big-endian
 * CPUs; if ARCH_IS_BIG_ENDIAN is not defined, the code will be compiled to
 * run on either big- or little-endian CPUs, but will run slightly less
 * efficiently on either one than if ARCH_IS_BIG_ENDIAN is defined.
 */

/* Define the state of the MD5 Algorithm. */
typedef struct md5_state_s {
    uint32_t count[2];   /* message length in bits, lsw first */
    uint32_t abcd[4];    /* digest buffer    */
    uint8_t buf[64];     /* accumulate block */
} md5_state_t;

   static void md5_process(md5_state_t *pms, const uint8_t *data /*[64]*/);
   /* Initialize the algorithm. */
   static void md5_init(md5_state_t *pms);
   /* Append a string to the message. */
   static void md5_append(md5_state_t *pms, const uint8_t *data, int nbytes);
   /* Finish the message and return the digest. */
   static void md5_finish(md5_state_t *pms, uint8_t digest[16]);

};

GDCM_EXPORT std::ostream &binary_write(std::ostream &os, const uint16_t &val);
GDCM_EXPORT std::ostream &binary_write(std::ostream &os, const uint32_t &val);
GDCM_EXPORT std::ostream &binary_write(std::ostream &os, const double &val);
GDCM_EXPORT std::ostream &binary_write(std::ostream &os, const char *val);
GDCM_EXPORT std::ostream &binary_write(std::ostream &os, std::string const &val);
GDCM_EXPORT std::ostream &binary_write(std::ostream &os, const uint8_t *val, size_t len);
GDCM_EXPORT std::ostream &binary_write(std::ostream &os, const uint16_t *val, size_t len);

} // end namespace gdcm
//-----------------------------------------------------------------------------
#endif
