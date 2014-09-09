/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmBase.h,v $
  Language:  C++
  Date:      $Date: 2007/10/04 17:50:42 $
  Version:   $Revision: 1.13 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMBASE_H_
#define _GDCMBASE_H_

#include "gdcmCommon.h"
#include <iostream>

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
/**
 * \brief Base class of all gdcm classes.
 * Contains the Print related methods :
 *  - Print 
 *  - SetPrintLevel / GetPrintLevel 
 */
class GDCM_EXPORT Base
{
public:
   /// \brief Constructor
   Base( ) : PrintLevel(0) { }
   /// \brief Canonical Destructor   
   virtual ~Base() {}
   /// \brief Printer
   virtual void Print(std::ostream & = std::cout, 
                      std::string const & = "" ) {};

   /// \brief Sets the print level for the Dicom Header Elements
   /// \note 0 for Light Print; 1 for 'medium' Print, 2 for Heavy Print
   void SetPrintLevel(int level) { PrintLevel = level; }

   /// \brief Gets the print level for the Dicom Entries
   int GetPrintLevel() { return PrintLevel; }

protected:
   /// \brief Amount of printed details for each Dicom Entries :
   /// 0 : stands for the least detail level.
   int PrintLevel;
};
} // end namespace gdcm

//-----------------------------------------------------------------------------
#endif
