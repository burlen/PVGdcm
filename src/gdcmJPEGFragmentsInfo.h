/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmJPEGFragmentsInfo.h,v $
  Language:  C++
  Date:      $Date: 2007/09/17 12:16:01 $
  Version:   $Revision: 1.24 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/


#ifndef _GDCMJPEGFRAGMENTSINFO_H_
#define _GDCMJPEGFRAGMENTSINFO_H_

#include "gdcmJPEGFragment.h"

#include <list>
#include <iostream>

namespace GDCM_NAME_SPACE
{
/**
 * \brief Utility class for gathering the informations of the collection
 *        of JPEG fragment[s] (see JPEGFragment) when handling
 *        "Encapsulated JPEG Compressed Images". 
 *        The informations on each frame are obtained during the pixel parsing
 *        of a gdcm::File (refer to File::ComputeJPEGFragmentInfo() ).
 *        They shall be used when (if necessary) decoding the fragments.
 *
 *        This class is simply a stl list<> of JPEGFragment.
 */
class GDCM_EXPORT JPEGFragmentsInfo
{
friend class File;
friend class PixelReadConvert;

private:
   JPEGFragmentsInfo();
   ~JPEGFragmentsInfo();
   void Print( std::ostream &os = std::cout, std::string const &indent = "" );

   void DecompressFromFile(std::ifstream *fp, uint8_t *buffer, int nBits,
                           int numBytes, int length);

   void AddFragment(JPEGFragment *fragment);
   JPEGFragment *GetFirstFragment();
   JPEGFragment *GetNextFragment();
   unsigned int GetFragmentCount();

//private:
   typedef std::list<JPEGFragment *> JPEGFragmentsList;

    //Some mathieu hack:
   int StateSuspension;
   void *SampBuffer;
   char *pimage;
   JPEGFragmentsList Fragments;
   JPEGFragmentsList::iterator ItFragments;
};
} // end namespace gdcm

//-----------------------------------------------------------------------------
#endif

