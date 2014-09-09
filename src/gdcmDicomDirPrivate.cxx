/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDicomDirPrivate.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:08 $
  Version:   $Revision: 1.2 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#include "gdcmDicomDirPrivate.h"
#include "gdcmGlobal.h"
#include "gdcmDataEntry.h"

namespace GDCM_NAME_SPACE
{
//-----------------------------------------------------------------------------
// Constructor / Destructor
/**
 * \brief  Constructor 
 * \note End user must use : DicomDirSerie::NewPrivate()
 */
DicomDirPrivate::DicomDirPrivate(bool empty):
   DicomDirObject()
{
   if ( !empty )
   {
      ListDicomDirPrivateElem const &elemList = 
         Global::GetDicomDirElements()->GetDicomDirPrivateElements();
      FillObject(elemList);
   }
}

/**
 * \brief   Canonical destructor.
 */
DicomDirPrivate::~DicomDirPrivate() 
{
}

//-----------------------------------------------------------------------------
// Public

//-----------------------------------------------------------------------------
// Protected

//-----------------------------------------------------------------------------
// Private

//-----------------------------------------------------------------------------
// Print
/**
 * \brief   Prints the Object
 * @param os ostream to write to
 * @param indent Indentation string to be prepended during printing
 * @return
 */ 
void DicomDirPrivate::Print(std::ostream &os, std::string const & )
{
   os << "PRIVATE : ";
   for(ListDocEntry::iterator i = DocEntries.begin();
                              i!= DocEntries.end();
                              ++i)
   {
      if ( (*i)->GetGroup() == 0x0004 && (*i)->GetElement() == 0x1500 )
      {
         if( dynamic_cast<DataEntry *>(*i) )
            os << (dynamic_cast<DataEntry *>(*i))->GetString();
      }
   }
   os << std::endl;

   DicomDirObject::Print(os);
}

//-----------------------------------------------------------------------------
} // end namespace gdcm

