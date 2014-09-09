/*=========================================================================
  
  Program:   gdcm
  Module:    $RCSfile: gdcmDicomDir.h,v $
  Language:  C++
  Date:      $Date: 2007/09/17 12:16:02 $
  Version:   $Revision: 1.82 $
  
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
  
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
  
=========================================================================*/

#ifndef _GDCMDICOMDIR_H_
#define _GDCMDICOMDIR_H_

#include "gdcmDocument.h"
#include "gdcmDebug.h"

#include <list>
#include <vector>

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
class DicomDirPatient;
class DicomDirMeta;
class DicomDirElement;
class DicomDirStudy;
class DicomDirSerie;
class DicomDirVisit;
class DicomDirImage;
class DicomDirPrivate;
class SQItem;

typedef std::list<DicomDirPatient *> ListDicomDirPatient;
typedef std::vector<Document *> VectDocument;

//-----------------------------------------------------------------------------
/**
 * \brief   DicomDir defines an object representing a DICOMDIR in memory
 *  as a tree-like structure :
 *
 *        ->DicomDirPatient 
 *           -> DicomDirStudy 
 *           -> DicomDirVisit 
 *              -> DicomDirSerie
 *                 -> DicomDirImage
 *                 -> DicomDirPrivate (for Siemens CSA non image)
 */
class GDCM_EXPORT DicomDir: public Document
{
   gdcmTypeMacro(DicomDir);

public:
/// \brief Constructs a DicomDir with a RefCounter
   static DicomDir *New() {return new DicomDir();}

//   GDCM_LEGACY( bool Load(std::string const &filename) )
   bool Load( );
   void Print(std::ostream &os = std::cout, std::string const &indent = "" );

   /// Sets the root Directory name to parse, recursively
   void SetDirectoryName(std::string const &dirName)
        { ParseDir = true; if (Filename != dirName)
                               Filename = dirName, IsDocumentModified = true;}
   /// Accessor to Filename
   virtual void SetFileName(std::string const &fileName)
                   { ParseDir = false; if (Filename != fileName)
                              Filename = fileName, IsDocumentModified = true;}

   // Informations contained in the parser
   virtual bool IsReadable();

   // Meta
   DicomDirMeta *NewMeta();
   /// Returns a pointer to the DicomDirMeta for this DICOMDIR. 
   DicomDirMeta *GetMeta() { return MetaElems; }

   // Patients
   DicomDirPatient *NewPatient();
   void ClearPatient();

   DicomDirPatient *GetFirstPatient();
   DicomDirPatient *GetNextPatient();
   /// returns the number of Patients currently held in the gdcm::DicomDir
   int              GetNumberOfPatients() { return Patients.size();}
   
   // Parsing
   void ParseDirectory();

   /// GetProgress GetProgress
   float GetProgress() const { return Progress; }
   /// AbortProgress AbortProgress
   void  AbortProgress() { Abort = true; }
   /// IsAborted IsAborted
   bool  IsAborted() { return Abort; }

   // Write
   bool Write(std::string const &fileName);

   bool Anonymize();

   virtual void Copy(DocEntrySet *set);

   /// Types of the DicomDirObject within the DicomDir
   typedef enum
   {
      GDCM_DICOMDIR_NONE,
      GDCM_DICOMDIR_META,
      GDCM_DICOMDIR_PATIENT,
      GDCM_DICOMDIR_STUDY,
      GDCM_DICOMDIR_SERIE,
      GDCM_DICOMDIR_VISIT,
      GDCM_DICOMDIR_IMAGE,
      GDCM_DICOMDIR_PRIVATE
   } DicomDirType;

protected:
   DicomDir();
   ~DicomDir();
//   GDCM_LEGACY( DicomDir(std::string const &filename, bool parseDir = false) )

   void CreateDicomDirChainedList(std::string const &path);

private:
   void Initialize();
   void CreateDicomDir();
   bool DoTheLoadingJob();
   bool AddPatientToEnd(DicomDirPatient *dd);   
   bool AddStudyToEnd  (DicomDirStudy   *dd);
   bool AddSerieToEnd  (DicomDirSerie   *dd);
   bool AddVisitToEnd  (DicomDirVisit   *dd);
   bool AddImageToEnd  (DicomDirImage   *dd);
   bool AddPrivateToEnd(DicomDirPrivate *dd);
   
   void SetElements(std::string const &path, VectDocument const &list);
   void SetElement (std::string const &path, DicomDirType type,
                    Document *header);
   void MoveSQItem(DocEntrySet *dst, DocEntrySet *src);

   static bool HeaderLessThan(Document *header1, Document *header2);

// Variables

   /// Pointer on *the* DicomDirObject 'DicomDirMeta Elements'
   DicomDirMeta *MetaElems;

   /// Chained list of DicomDirPatient (to be exploited hierarchicaly) 
   ListDicomDirPatient Patients;
   ListDicomDirPatient::iterator ItPatient;

   /// value of the ??? for any progress bar   
   bool ParseDir;

};
} // end namespace gdcm
//-----------------------------------------------------------------------------
#endif
