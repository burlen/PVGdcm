/*=========================================================================
 
  Program:   gdcm
  Module:    $RCSfile: gdcmValidator.h,v $
  Language:  C++
  Date:      $Date: 2007/08/22 16:14:05 $
  Version:   $Revision: 1.7 $
 
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
 
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef _GDCMVALIDATOR_H_
#define _GDCMVALIDATOR_H_

#include "gdcmRefCounter.h"

namespace GDCM_NAME_SPACE
{
/**
 * \brief Class to perform some verifications on a gdcm::Document
 */
class ElementSet;
class GDCM_EXPORT Validator : public RefCounter
{
   gdcmTypeMacro(Validator);
   
public:

/// \brief Constructs a Validator with a RefCounter
   static Validator *New() {return new Validator();}
   void SetInput(ElementSet *input);

protected:
   Validator();
   ~Validator();
};

} // end namespace gdcm

//-----------------------------------------------------------------------------
#endif
