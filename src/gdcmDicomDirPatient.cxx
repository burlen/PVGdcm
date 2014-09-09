/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDicomDirPatient.cxx,v $
  Language:  C++
  Date:      $Date: 2007/10/08 15:20:17 $
  Version:   $Revision: 1.44 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#include "gdcmDicomDirPatient.h"
#include "gdcmDicomDirElement.h"
#include "gdcmGlobal.h"
#include "gdcmDicomDirStudy.h"
#include "gdcmSQItem.h"
#include "gdcmDebug.h"

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
// Constructor / Destructor
/**
 * \brief   Constructor
 * \note End user must use : DicomDir::NewPatient()
 */
DicomDirPatient::DicomDirPatient(bool empty)
                :DicomDirObject()
{
   if ( !empty )
   {
      ListDicomDirStudyElem const &elemList = 
         Global::GetDicomDirElements()->GetDicomDirPatientElements();
      FillObject(elemList);
   }
}

/**
 * \brief   Canonical destructor.
 */
DicomDirPatient::~DicomDirPatient() 
{
   ClearStudy();
}

//-----------------------------------------------------------------------------
// Public
/**
 * \brief   Writes the Object
 * @param fp ofstream to write to
 * @param t Type of the File (explicit VR, implicitVR, ...) 
 */ 
void DicomDirPatient::WriteContent(std::ofstream *fp, FileType t, bool , bool )
{
   DicomDirObject::WriteContent(fp, t, false, true);

   for(ListDicomDirStudy::iterator cc = Studies.begin();
                                   cc!= Studies.end();
                                 ++cc )
   {
      (*cc)->WriteContent( fp, t, false, true );
   }
}

/**
 * \brief   adds a new Patient at the beginning of the PatientList
 *          of a partially created DICOMDIR
 */
DicomDirStudy* DicomDirPatient::NewStudy()
{
   DicomDirStudy *dd = DicomDirStudy::New();
   Studies.push_back(dd);
   return dd; 
}   

/**
 * \brief  Remove all studies in the patient 
 */
void DicomDirPatient::ClearStudy()
{
   for(ListDicomDirStudy::const_iterator cc = Studies.begin();
                                         cc != Studies.end(); 
                                       ++cc )
   {
      (*cc)->Delete();
   }
   Studies.clear();
}

/**
 * \brief   Get the first entry while visiting the DicomDirStudy
 * \return  The first DicomDirStudy if found, otherwhise NULL
 */ 
DicomDirStudy *DicomDirPatient::GetFirstStudy()
{
   ItStudy = Studies.begin();
   if (ItStudy != Studies.end())
      return *ItStudy;
   return NULL;
}

/**
 * \brief   Get the next entry while visiting the DicomDirStudies
 * \note : meaningfull only if GetFirstEntry already called
 * \return  The next DicomDirStudies if found, otherwhise NULL
 */
DicomDirStudy *DicomDirPatient::GetNextStudy()
{
   gdcmAssertMacro (ItStudy != Studies.end())

   ++ItStudy;
   if (ItStudy != Studies.end())
      return *ItStudy;
   return NULL;
}

/**
 * \brief   Get the first entry while visiting the DicomDirStudy
 * \return  The first DicomDirStudy if found, otherwhise NULL
 */ 
DicomDirStudy *DicomDirPatient::GetLastStudy()
{
   ItStudy = Studies.end();
   if (ItStudy != Studies.begin())
   {
      --ItStudy;
      return *ItStudy;
   }
   return NULL;
}

/**
 * \brief Copies all the attributes from an other DocEntrySet 
 * @param set entry to copy from
 * @remarks The contained DocEntries a not copied, only referenced
 */
void DicomDirPatient::Copy(DocEntrySet *set)
{
   // Remove all previous childs
   ClearStudy();

   DicomDirObject::Copy(set);

   DicomDirPatient *ddEntry = dynamic_cast<DicomDirPatient *>(set);
   if( ddEntry )
   {
      Studies = ddEntry->Studies;
      for(ItStudy = Studies.begin();ItStudy != Studies.end();++ItStudy)
         (*ItStudy)->Register();
   }
}

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
 */ 
void DicomDirPatient::Print(std::ostream &os, std::string const & )
{
   os << "PATIENT" << std::endl;
   DicomDirObject::Print(os);

   for(ListDicomDirStudy::const_iterator cc = Studies.begin();
                                         cc != Studies.end(); 
                                       ++cc )
   {
      (*cc)->SetPrintLevel(PrintLevel);
      (*cc)->Print(os);
   }
}

//-----------------------------------------------------------------------------
} // end namespace gdcm
