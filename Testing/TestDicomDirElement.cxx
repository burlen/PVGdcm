/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestDicomDirElement.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:06 $
  Version:   $Revision: 1.7 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmDicomDirElement.h"
#include "gdcmDebug.h"
#include "gdcmGlobal.h"
#include "gdcmCommon.h"
#include <iostream>

int TestDicomDirElement(int , char *[])
{
   GDCM_NAME_SPACE::DicomDirElement *ddElt = GDCM_NAME_SPACE::DicomDirElement::New();
   if (ddElt == 0)
   {
      std::cout << "new DicomDirElement failed" << std::endl;
      return 1;
   }  
   ddElt->Print( std::cout );

   // Let's allow User to add an Entry (e.g Patient Weight )

   // We can add an Entry to the default list of any of the Element
 
   // Add Patient Weight to the default list

   // FIXME : Why doesn't it compile ?!?
   // FIXME :`DD_PATIENT' undeclared (first use this function)
   // FIXME : see gdcmCommon.h !!

   //ddElt->AddDicomDirElement ( DD_PATIENT, 0x0010, 0x1010 );

   // We could add others

   std::cout << " -------- DicomDirElement After modif --------" <<std::endl;
   ddElt->Print( std::cout );

   ddElt->Delete();
   return 0;
}
