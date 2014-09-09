/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDicomDirObject.h,v $
  Language:  C++
  Date:      $Date: 2007/08/22 16:14:03 $
  Version:   $Revision: 1.20 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMDICOMDIROBJECT_H_
#define _GDCMDICOMDIROBJECT_H_

#include "gdcmSQItem.h"
#include "gdcmDicomDirElement.h"

#include <string>
#include <list>

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
class DicomDirObject;

//-----------------------------------------------------------------------------
typedef std::list<DicomDirObject *> ListContent;
//-----------------------------------------------------------------------------
/**
 * \brief   Parent object for DicomDirPatient, DicomDirStudy, 
 *                            DicomDirSerie, DicomDirImage, of a DicomDir
 */
class GDCM_EXPORT DicomDirObject : public SQItem
{
   gdcmTypeMacro(DicomDirObject);

public:

protected:
   // Constructor and destructor are protected to avoid end user to
   // instanciate from this class. 
   DicomDirObject(int depth = 1);
   ~DicomDirObject();

   void FillObject(ListDicomDirMetaElem const &elemList);

};
} // end namespace gdcm

//-----------------------------------------------------------------------------
#endif
