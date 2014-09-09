/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDicomEntry.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:09 $
  Version:   $Revision: 1.5 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#include "gdcmDicomEntry.h"
//#include "gdcmDebug.h"
//#include "gdcmUtil.h"

//#include <iomanip> // for std::ios::left, ...
#include <fstream>
//#include <stdio.h> // for sprintf

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
// Constructor / Destructor
/**
 * \brief   Constructor
 * @param   group      DICOM-Group Number
 * @param   elem       DICOM-Element Number
 * @param   vr         Value Representation
*/
DicomEntry::DicomEntry(const uint16_t &group,const uint16_t &elem,
                       const VRKey &vr)
{
   //Tag.SetGroupElement(group);
   //Tag.SetElement(elem);
   Tag.SetGroupElement(group,elem);
   VR = vr;
}

/**
 * \brief   Destructor
 */
DicomEntry::~DicomEntry()
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
 * \brief   Prints an entry of the Dicom DictionaryEntry
 * @param   os ostream we want to print in
 * @param indent Indentation string to be prepended during printing
 */
void DicomEntry::Print(std::ostream &os, std::string const & )
{
   os << GetKey(); 
   os << " [" << VR  << "] ";
}

//-----------------------------------------------------------------------------
} // end namespace gdcm

