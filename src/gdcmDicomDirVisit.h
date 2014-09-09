/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDicomDirVisit.h,v $
  Language:  C++
  Date:      $Date: 2007/08/22 16:14:03 $
  Version:   $Revision: 1.6 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMDICOMDIRVISIT_H_
#define _GDCMDICOMDIRVISIT_H_

#include "gdcmDicomDirObject.h"

namespace GDCM_NAME_SPACE 
{

/**
 * \brief   describes a VISIT  within a within a STUDY
 * (DicomDirStudy) of a given DICOMDIR (DicomDir)
 */
class GDCM_EXPORT DicomDirVisit : public DicomDirObject 
{
   gdcmTypeMacro(DicomDirVisit);

public:
/// \brief Constructs a DicomDirVisit with a RefCounter
   static DicomDirVisit *New(bool empty=false) {return new DicomDirVisit(empty);}

   void Print( std::ostream &os = std::cout, std::string const &indent = "" );

protected:
   DicomDirVisit(bool empty=false); 
   ~DicomDirVisit();
};
} // end namespace gdcm
//-----------------------------------------------------------------------------
#endif
