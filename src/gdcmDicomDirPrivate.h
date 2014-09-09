/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDicomDirPrivate.h,v $
  Language:  C++
  Date:      $Date: 2007/08/22 16:14:03 $
  Version:   $Revision: 1.3 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMDICOMDIRPRIVATE_H_
#define _GDCMDICOMDIRPRIVATE_H_

#include "gdcmDicomDirObject.h"

namespace GDCM_NAME_SPACE 
{

/**
 * \brief   describes a PRIVATE  within a within a SERIE
 * (DicomDirSerie) of a given DICOMDIR (DicomDir)
 */
class GDCM_EXPORT DicomDirPrivate : public DicomDirObject 
{
   gdcmTypeMacro(DicomDirPrivate);

public:
/// \brief Constructs a DicomDirPrivate with a RefCounter
   static DicomDirPrivate *New(bool empty=false) {return new DicomDirPrivate(empty);}

   void Print( std::ostream &os = std::cout, std::string const &indent = "" );
  // void WriteContent( std::ofstream *fp, FileType t );

protected:
   DicomDirPrivate(bool empty=false); 
   ~DicomDirPrivate();
};
} // end namespace gdcm
//-----------------------------------------------------------------------------
#endif
