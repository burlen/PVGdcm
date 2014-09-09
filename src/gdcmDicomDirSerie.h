/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDicomDirSerie.h,v $
  Language:  C++
  Date:      $Date: 2007/08/29 15:30:49 $
  Version:   $Revision: 1.38 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMDICOMDIRSERIE_H_
#define _GDCMDICOMDIRSERIE_H_

#include "gdcmDicomDirObject.h"

namespace GDCM_NAME_SPACE 
{
class DicomDirImage;
class DicomDirPrivate;

//-----------------------------------------------------------------------------
typedef std::list<DicomDirImage *> ListDicomDirImage;
typedef std::list<DicomDirPrivate *> ListDicomDirPrivate;
//-----------------------------------------------------------------------------
/**
 * \brief   describes a SERIE  within a within a STUDY
 * (DicomDirStudy) of a given DICOMDIR (DicomDir)
 */
class GDCM_EXPORT DicomDirSerie : public DicomDirObject 
{
   gdcmTypeMacro(DicomDirSerie);

public:
/// \brief Constructs a DicomDirSerie with a RefCounter
   static DicomDirSerie *New(bool empty=false) {return new DicomDirSerie(empty);}

   void Print( std::ostream &os = std::cout, std::string const &indent = "" );
   void WriteContent( std::ofstream *fp, FileType t, bool insideMetaElements,
                                                     bool insideSequence );

   // 'Image' methods
   DicomDirImage *NewImage();
   /// Add a new gdcm::DicomDirImage to the Serie
   void AddImage(DicomDirImage *obj) { Images.push_back(obj); }
   void ClearImage();

   DicomDirImage *GetFirstImage();
   DicomDirImage *GetNextImage();
   /// returns the number of Images currently held in the gdcm::DicomDirSerie
   int            GetNumberOfImages() { return Images.size();}
   
   // 'Private' methods (For SIEMENS 'CSA non image')
   DicomDirPrivate *NewPrivate();
   /// Add a new gdcm::DicomDirPrivate to the Serie
   void AddPrivate(DicomDirPrivate *obj) { Privates.push_back(obj); }
   void ClearPrivate();

   DicomDirPrivate *GetFirstPrivate();
   DicomDirPrivate *GetNextPrivate();   
   /// returns the number of 'Privates' currently held in the gdcm::DicomDirSerie
   int             GetNumberOfPrivates() { return Privates.size();}   
   
   virtual void Copy(DocEntrySet *set);

protected:
   DicomDirSerie(bool empty=false); 
   ~DicomDirSerie();

private:
   ///chained list of DicomDirImages (to be exploited recursively)
   ListDicomDirImage Images;
   /// iterator on the DicomDirImages of the current DicomDirSerie
   ListDicomDirImage::iterator ItImage;

   ///chained list of DicomDirPrivates (to be exploited recursively)
   ListDicomDirPrivate Privates;
   /// iterator on the DicomDirPrivates of the current DicomDirSerie
   ListDicomDirPrivate::iterator ItPrivate;
/*
// for future use  (Full DICOMDIR):

   /// chained list of DicomDirOverlays(single level)
   ListDicomDirOverlay Overlays;
   /// iterator on the DicomDirOverlays of the current DicomDirSerie
   ListDicomDirOverlay::iterator ItOverlay;

   /// chained list of DicomDirModalityLuts(single level)
   ListDicomDirModalityLut ModalityLuts;
   /// iterator on the DicomDirModalityLuts of the current DicomDirSerie
   ListDicomDirModalityLut::iterator ItModalityLuts;

   /// chained list of DicomDirCurves(single level)
   ListDicomDirCurve Curves;
   /// iterator on the DicomDirCurves of the current DicomDirSerie
   ListDicomDirCurve::iterator ItCurves;

   /// chained list of DicomDirStoredPrints(single level)
   ListDicomDirStoredPrint StoredPrints;
   /// iterator on the DicomDirStoredPrints of the current DicomDirSerie
   ListDicomDirStoredPrint::iterator ItStoredPrints;

   /// chained list of DicomDirRtDoses(single level)
   ListDicomDirRtDose RtDoses;
   /// iterator on the DicomDirRtDoses of the current DicomDirSerie
   ListDicomDirRtDose::iterator ItRtDoses;

   /// chained list of DicomDirRtStructureSets(single level)
   ListDicomDirRtStructureSet RtStructureSets;
   /// iterator on the DicomDirRtStructureSets of the current DicomDirSerie
   ListDicomDirRtStructureSet::iterator ItRtStructureSets;

   /// chained list of DicomDirRtPlans(single level)
   ListDicomDirRtPlan RtPlans;
   /// iterator on the DicomDirRtPlans of the current DicomDirSerie
   ListDicomDirPlan::iterator ItRtPlans;

   /// chained list of DicomDirRtTreatRecords(single level)
   ListDicomDirRtTreatRecord RtTreatRecords;
   /// iterator on the DicomDirRtTreatRecords of the current DicomDirSerie
   ListDicomDirRtTreatRecord::iterator ItRtTreatRecords;

   /// chained list of DicomDirPresentations(single level)
   ListDicomDirPresentation Presentations;
   /// iterator on the DicomDirPresentations of the current DicomDirSerie
   ListDicomDirPresentation::iterator ItPresentations;

   /// chained list of DicomDirWaveForms(single level)
   ListDicomDirWaveForm WaveForms;
   /// iterator on the DicomDirWaveForms of the current DicomDirSerie
   ListDicomDirWaveForm::iterator ItWaveForms;

   /// chained list of DicomDirSrDocuments(single level)
   ListDicomDirSrDocument SrDocuments;
   /// iterator on the DicomDirSrDocuments of the current DicomDirSerie
   ListDicomDirSrDocument::iterator ItSrDocuments;

   /// chained list of DicomDirKeyObjectDocs(single level)
   ListDicomDirKeyObjectDoc KeyObjectDocs;
   /// iterator on the DicomDirKeyObjectDocs of the current DicomDirSerie
   ListDicomDirKeyObjectDoc::iterator ItKeyObjectDocs;

   /// chained list of DicomDirSpectroscopys(single level)
   ListDicomDirSpectroscopy Spectroscopys;
   /// iterator on the DicomDirSpectroscopys of the current DicomDirSerie
   ListDicomDirSpectroscopy::iterator ItSpectroscopys;

   /// chained list of DicomDirRawDatas(single level)
   ListDicomDirRawData RawDatas;
   /// iterator on the DicomDirRawDatas of the current DicomDirSerie
   ListDicomDirRawData::iterator ItRawDatas;

   /// chained list of DicomDirRegistrations(single level)
   ListDicomDirRegistration Registrations;
   /// iterator on the DicomDirRegistrations of the current DicomDirSerie
   ListDicomDirRegistration::iterator ItRegistrations;

   /// chained list of DicomDirFiducials(single level)
   ListDicomDirFiducial Fiducials;
   /// iterator on the DicomDirFiducials of the current DicomDirSerie
   ListDicomDirFiducial::iterator ItFiducials;
*/

};
} // end namespace gdcm
//-----------------------------------------------------------------------------
#endif
