/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDictEntry.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:09 $
  Version:   $Revision: 1.61 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#include "gdcmDictEntry.h"
#include "gdcmDebug.h"
#include "gdcmUtil.h"

#include <iomanip> // for std::ios::left, ...
#include <fstream>
#include <stdio.h> // for sprintf

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
// Constructor / Destructor
/**
 * \brief   Constructor
 * @param   group      DICOM-Group Number
 * @param   elem       DICOM-Element Number
 * @param   vr         Value Representation
 * @param   vm         Value Multiplicity 
 * @param   name       description of the element
*/
DictEntry::DictEntry(uint16_t group, uint16_t elem,
                     VRKey const &vr, 
                     TagName const &vm,
                     TagName const &name)
{
   Tag.SetGroup(group);
   Tag.SetElement(elem);
   VR      = vr;
   VM      = vm;
   Name    = name;
}

/**
 * \brief   Destructor
 */
DictEntry::~DictEntry()
{
}
//-----------------------------------------------------------------------------
// Public
/**
 * \brief Class allocator
 * @param   group      DICOM-Group Number
 * @param   elem       DICOM-Element Number
 * @param   vr         Value Representation
 * @param   vm         Value Multiplicity 
 * @param   name       description of the element
*/
DictEntry *DictEntry::New(uint16_t group, uint16_t elem,
                          VRKey const &vr,
                          TagName const &vm,
                          TagName const &name)
{
   return new DictEntry(group,elem,vr,vm,name);
}

/**
 * \brief   concatenates 2 uint16_t (supposed to be a Dicom group number 
 *                                              and a Dicom element number)
 * @param  group the Dicom group number used to build the tag
 * @param  elem the Dicom element number used to build the tag
 * @return the built tag
 */
TagKey DictEntry::TranslateToKey(uint16_t group, uint16_t elem)
{
   // according to 'Purify', TranslateToKey is one of the most
   // time consuming methods.
   // Let's try to shorten it !
   return TagKey(group,elem);
}

//-----------------------------------------------------------------------------
// Protected

//-----------------------------------------------------------------------------
// Private

//-----------------------------------------------------------------------------
// Print
/**
 * \brief   Prints an entry of the Dicom DictionaryEntry
 * @param   os ostream we want to print in
 * @param indent Indentation string to be prepended during printing
 */
void DictEntry::Print(std::ostream &os, std::string const & )
{
   os << GetKey(); 
   os << " [" << VR  << "] ";

   std::ostringstream s;

   if ( PrintLevel >= 1 )
   {
      s.setf(std::ios::left);
      s << std::setw(66-GetName().length()) << " ";
   }

   s << "[" << GetName()<< "]";
   os << s.str() << std::endl;
}

//-----------------------------------------------------------------------------
} // end namespace gdcm

