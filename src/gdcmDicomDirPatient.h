/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDicomDirPatient.h,v $
  Language:  C++
  Date:      $Date: 2007/08/29 15:30:48 $
  Version:   $Revision: 1.35 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMPATIENT_H_
#define _GDCMPATIENT_H_

#include "gdcmDicomDirObject.h"

namespace GDCM_NAME_SPACE 
{
class DicomDirStudy;

//-----------------------------------------------------------------------------
typedef std::list<DicomDirStudy*> ListDicomDirStudy;

//-----------------------------------------------------------------------------
/**
 * \brief   describes a PATIENT within a DICOMDIR (DicomDir)
 */

class GDCM_EXPORT DicomDirPatient : public DicomDirObject 
{
   gdcmTypeMacro(DicomDirPatient);

public:
/// \brief Constructs a DicomDirPatient with a RefCounter
   static DicomDirPatient *New(bool empty=false) {return new DicomDirPatient(empty);}

   void Print(std::ostream &os = std::cout, std::string const &indent = "" );
   void WriteContent(std::ofstream *fp, FileType t, bool insideMetaElements, bool insideSequence );
  
   // Patient methods
   /// \brief Adds a new gdcm::DicomDirStudy to the Patient
   void AddStudy(DicomDirStudy *obj) { Studies.push_back(obj); }
   DicomDirStudy *NewStudy(); 
   void ClearStudy();

   DicomDirStudy *GetFirstStudy();
   DicomDirStudy *GetNextStudy();
   DicomDirStudy *GetLastStudy();
   /// returns the number of Studies currently held in the gdcm::DicomDirPatient
   int            GetNumberOfStudies() { return Studies.size();}
   virtual void Copy(DocEntrySet *set);

protected:
   DicomDirPatient(bool empty=false); 
   ~DicomDirPatient();

private:
   /// chained list of DicomDirStudy  (to be exploited hierarchicaly)
   ListDicomDirStudy Studies;
   /// iterator on the DicomDirStudies of the current DicomDirPatient
   ListDicomDirStudy::iterator ItStudy;
};
} // end namespace gdcm

//-----------------------------------------------------------------------------
#endif
