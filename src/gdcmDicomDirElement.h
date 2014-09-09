/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDicomDirElement.h,v $
  Language:  C++
  Date:      $Date: 2007/08/22 16:14:03 $
  Version:   $Revision: 1.41 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMDICOMDIRELEMENT_H_
#define _GDCMDICOMDIRELEMENT_H_

#include "gdcmRefCounter.h"
#include "gdcmVRKey.h"

#include <list>

namespace GDCM_NAME_SPACE 
{

//-----------------------------------------------------------------------------

typedef std::list<DicomElement> ListDicomDirElem;
typedef std::list<DicomElement> ListDicomDirMetaElem;
typedef std::list<DicomElement> ListDicomDirPatientElem;
typedef std::list<DicomElement> ListDicomDirStudyElem;
typedef std::list<DicomElement> ListDicomDirVisitElem;
typedef std::list<DicomElement> ListDicomDirSerieElem;
typedef std::list<DicomElement> ListDicomDirImageElem;
typedef std::list<DicomElement> ListDicomDirPrivateElem; // For "CSA Non Image"

// For future use (Full DICOMDIR)

/*
typedef std::list<DicomElement> ListDicomDirResultElem;
typedef std::list<DicomElement> ListDicomDirStudyComponentElem;

typedef std::list<DicomElement> ListDicomDirOverlayElem;
typedef std::list<DicomElement> ListDicomDirModalityLutElem;
typedef std::list<DicomElement> ListDicomDirModalityLutElem;
typedef std::list<DicomElement> ListDicomDirCurveElem;
typedef std::list<DicomElement> ListDicomDirStoredPrintElem;
typedef std::list<DicomElement> ListDicomDirRtDoseElem;
typedef std::list<DicomElement> ListDicomDirRtStructureSetElem;
typedef std::list<DicomElement> ListDicomDirRtPlanElem;
typedef std::list<DicomElement> ListDicomDirRtTreatRecordElem;
typedef std::list<DicomElement> ListDicomDirPresentationElem;
typedef std::list<DicomElement> ListDicomDirSrDocumentElem;
typedef std::list<DicomElement> ListDicomDirKeyObjectDocElem;
typedef std::list<DicomElement> ListDicomDirSpectroscopyElem;
typedef std::list<DicomElement> ListDicomDirRawDataElem;
typedef std::list<DicomElement> ListDicomDirRegistrationElem;
typedef std::list<DicomElement> ListDicomDirFiducialElem;
*/

//-----------------------------------------------------------------------------
/**
 * \brief   Represents elements contained in a DicomDir class
 *          for the chained lists from the file 'Dicts/DicomDir.dic'
 */
class GDCM_EXPORT DicomDirElement : public RefCounter
{
   gdcmTypeMacro(DicomDirElement);

public:
/// \brief Contructs a DicomDirElement with a RefCounter
   static DicomDirElement *New() {return new DicomDirElement();}

   /**
    * \brief   canonical Printer 
    */ 
   virtual void Print(std::ostream &os = std::cout, 
                      std::string const &indent = "" );

   /**
    * \brief   returns a reference to the chained List 
    *          related to the META Elements of a DICOMDIR.
    */
   ListDicomDirMetaElem const &GetDicomDirMetaElements() const
      { return DicomDirMetaList; }

   /**
    * \brief   returns a reference to the chained List 
    *          related to the PATIENT Elements of a DICOMDIR.
    */      
   ListDicomDirPatientElem const &GetDicomDirPatientElements() const
      { return DicomDirPatientList; }

   /**
    * \brief   returns a reference to the chained List 
    *          related to the STUDY Elements of a DICOMDIR.
    */      
   ListDicomDirStudyElem const &GetDicomDirStudyElements() const
      { return DicomDirStudyList; }

   /**
    * \brief   returns a reference to the chained List 
    *          related to the VISIT Elements of a DICOMDIR.
    */      
   ListDicomDirVisitElem const &GetDicomDirVisitElements() const
      { return DicomDirVisitList; }
   /**
    * \brief   returns a reference to the chained List 
    *          related to the SERIE Elements of a DICOMDIR.
    */
   ListDicomDirSerieElem const &GetDicomDirSerieElements() const
      { return DicomDirSerieList; }

   /**
    * \brief   returns a reference to the chained List 
    *          related to the IMAGE Elements of a DICOMDIR.
    */
   ListDicomDirImageElem const &GetDicomDirImageElements() const
      { return DicomDirImageList; }

   /**
    * \brief   returns a reference to the chained List 
    *          related to the PRIVATE Elements of a DICOMDIR.
    */      
   ListDicomDirPrivateElem const &GetDicomDirPrivateElements() const
      { return DicomDirPrivateList; }
 
      
   // Public method to add an element
   bool AddEntry(DicomDirType type, DicomElement const &elem);

   // Only one instance of ddElem 
   void AddDicomDirElement(DicomDirType type,
                           uint16_t group, uint16_t elem, VRKey vr);

protected:
   DicomDirElement();
   ~DicomDirElement();

private:
   /// Elements chained list, related to the MetaElements of DICOMDIR
   ListDicomDirMetaElem    DicomDirMetaList;
   /// Elements chained list, related to the PatientElements of DICOMDIR
   ListDicomDirPatientElem DicomDirPatientList;
   /// Elements chained list, related to the StudyElements of DICOMDIR
   ListDicomDirStudyElem   DicomDirStudyList;
   /// Elements chained list, related to the VisitElements of DICOMDIR
   ListDicomDirVisitElem   DicomDirVisitList;
   /// Elements chained list, related to the SerieElements of DICOMDIR
   ListDicomDirSerieElem   DicomDirSerieList;
   /// Elements chained list, related to the ImageElements of DICOMDIR
   ListDicomDirImageElem   DicomDirImageList;
   /// Elements chained list, related to the PrivateElements of DICOMDIR
   ListDicomDirPrivateElem   DicomDirPrivateList;
};
} // end namespace gdcm
//-----------------------------------------------------------------------------
#endif
