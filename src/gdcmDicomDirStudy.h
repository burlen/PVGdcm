/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDicomDirStudy.h,v $
  Language:  C++
  Date:      $Date: 2007/08/29 15:30:49 $
  Version:   $Revision: 1.37 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMDICOMDIRSTUDY_H_
#define _GDCMDICOMDIRSTUDY_H_

#include "gdcmDicomDirObject.h"

namespace GDCM_NAME_SPACE 
{
class DicomDirSerie;
class DicomDirVisit;
//-----------------------------------------------------------------------------
typedef std::list<DicomDirSerie *> ListDicomDirSerie;


// For future use (Full DICOMDIR)
typedef std::list<DicomDirVisit *> ListDicomDirVisit;
/*
typedef std::list<DicomDirResult *> ListDicomDirResult;
typedef std::list<DicomDirStudyComponent *> ListDicomDirStudyComponent;
*/
//-----------------------------------------------------------------------------
/**
 * \brief   describes a STUDY within a within a PATIENT
 * (DicomDirPatient) of a given DICOMDIR (DicomDir)
 */
class GDCM_EXPORT DicomDirStudy : public DicomDirObject
{
   gdcmTypeMacro(DicomDirStudy);

public:
/// \brief Constructs a DicomDirStudy with a RefCounter
   static DicomDirStudy *New(bool empty=false) {return new DicomDirStudy(empty);}

   void Print(std::ostream &os = std::cout, std::string const &indent = "" );
   void WriteContent(std::ofstream *fp, FileType t, bool insideMetaElements,bool insideSequence );

   // 'Serie' methods
   DicomDirSerie *NewSerie();
   /// Adds a gdcm::DicomDirSerie to a gdcm::DicomDirStudy
   void AddSerie(DicomDirSerie *obj) { Series.push_back(obj); }
   void ClearSerie();
   DicomDirSerie *GetFirstSerie();
   DicomDirSerie *GetNextSerie();
   DicomDirSerie *GetLastSerie();
   /// returns the number of Series currently held in the gdcm::DicomDirStudy
   int            GetNumberOfSeries() { return Series.size();}
   
   // 'Visit' methods
   DicomDirVisit *NewVisit();
   /// Adds a gdcm::DicomDirVisit to a gdcm::DicomDirStudy
   void AddVisit(DicomDirVisit *obj) { Visits.push_back(obj); }
   void ClearVisit();
   DicomDirVisit *GetFirstVisit();
   DicomDirVisit *GetNextVisit();
   DicomDirVisit *GetLastVisit();
   /// returns the number of Visit currently held in the gdcm::DicomDirStudy
   int            GetNumberOfVisits() { return Visits.size();}
      
   // for future use (Full DICOMDIR)
/*
   DicomDirResult *GetFirstResult();
   DicomDirResult *GetNextResult();
   DicomDirResult *GetLastResult();

   DicomDirStudyComponent *GetFirstStudyComponent();
   DicomDirStudyComponent *GetNextStudyComponent();
   DicomDirStudyComponent *GetLastStudyComponent();
*/    

   virtual void Copy(DocEntrySet *set);

protected:
   DicomDirStudy(bool empty=false); 
   ~DicomDirStudy();

private:
   /// chained list of DicomDirSeries (to be exploited hierarchicaly)
   ListDicomDirSerie Series;
   /// iterator on the DicomDirSeries of the current DicomDirStudy
   ListDicomDirSerie::iterator ItSerie;

   /// chained list of DicomDirVisits(single level)
   ListDicomDirVisit Visits;
   /// iterator on the DicomDirVisits of the current DicomDirStudy
   ListDicomDirVisit::iterator ItVisit;
   

   // for future use (Full DICOMDIR)
/*
   /// chained list of DicomDirResults(single level)
   ListDicomDirResult Results;
   /// iterator on the DicomDirResults of the current DicomDirStudy
   ListDicomDirResult::iterator ItResult;

   /// chained list of DicomDirStudyComponents(single level)
   ListDicomDirStudyComponent StudyComponents;
   /// iterator on the DicomDirStudyComponents of the current DicomDirStudy
   ListDicomDirStudyComponent::iterator ItStudyComponents;
*/
};
} // end namespace gdcm

//-----------------------------------------------------------------------------
#endif
