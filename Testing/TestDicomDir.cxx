/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestDicomDir.cxx,v $
  Language:  C++
  Date:      $Date: 2008/09/15 15:49:21 $
  Version:   $Revision: 1.47 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmDocEntry.h"
#include "gdcmDataEntry.h"
#include "gdcmDicomDir.h"
#include "gdcmDicomDirPatient.h"
#include "gdcmDicomDirStudy.h"
#include "gdcmDicomDirSerie.h"
#include "gdcmDicomDirImage.h"
#include "gdcmTS.h"

#include <iostream>
#include <fstream>

#include <stdlib.h>

// check *all* the dicom elements (GDCM_NAME_SPACE::DocEntry)
// of this GDCM_NAME_SPACE::DicomDirObject
int CompareSQItem(GDCM_NAME_SPACE::SQItem *pa1, GDCM_NAME_SPACE::SQItem *pa2 )
{
   GDCM_NAME_SPACE::DocEntry *e1;
   GDCM_NAME_SPACE::DocEntry *e2;

   e2 = pa2->GetFirstEntry();
   while (!e2)
   {
      // locate the corresponding element in 'source' file 
      e1 = pa1->GetDocEntry( e2->GetGroup(),e2->GetElement() );

      // an element doesn't exist in origin file
      if (!e1)
      {
       std::cout << "DicomDir element " << std::hex 
                 << e2->GetGroup() << "," <<e2->GetElement() << std::endl;
       return 1; 
      }
      // skip SeqEntries (I don't want to deal with 'recursion pbs' here)
      if ( !dynamic_cast<GDCM_NAME_SPACE::DataEntry *>(e1) ||
           !dynamic_cast<GDCM_NAME_SPACE::DataEntry *>(e2) )
         continue;

      // a value is read as GDCM_UNFOUND 
      if ( ((GDCM_NAME_SPACE::DataEntry *)e1)->GetString() == GDCM_NAME_SPACE::GDCM_UNFOUND )
      {
         std::cout << "for gdcm source DicomDir : element (" << std::hex 
                   << e1->GetGroup() << "," <<e1->GetElement() 
                   << ") has values [" << GDCM_NAME_SPACE::GDCM_UNFOUND << "]"
                   << std::endl;
         return 1;
      }

      // values differ in source file and destination file
      if ( ((GDCM_NAME_SPACE::DataEntry *)e1)->GetString() != 
           ((GDCM_NAME_SPACE::DataEntry *)e2)->GetString() )
      {
 
 /// \todo : check the value *written on disc*, not the value converted as a std::string
 ///         (this comparison doesn't make the difference between Ox(ff) and "255" ...)
 
         // serious trouble : values differ in source and destination file
         std::cout << "for gdcm DicomDir element (" << std::hex 
                   << e2->GetGroup() << "," <<e2->GetElement() 
                   << ") values differ [" 
                   << ((GDCM_NAME_SPACE::DataEntry *)e1)->GetString() << "] != [" 
                   << ((GDCM_NAME_SPACE::DataEntry *)e2)->GetString() << "]"
                   << std::endl;
          return 1;
      }
   }
   return 0;
} 
 
int TestDicomDir(int argc, char *argv[])
{  
   GDCM_NAME_SPACE::DicomDir *dicomdir;
   
   GDCM_NAME_SPACE::DicomDirPatient *pa1;
   GDCM_NAME_SPACE::DicomDirStudy   *st1;
   GDCM_NAME_SPACE::DicomDirSerie   *se1;
   GDCM_NAME_SPACE::DicomDirImage   *im1;

   GDCM_NAME_SPACE::TSKey v;
    
   std::string file; 
   if (argc > 1) 
      file = argv[1];    
   else 
   {
      file += GDCM_DATA_ROOT;
      file += "/DICOMDIR";
   }

   std::cout << "DicomDir we're going to deal with : ["<< file << "]" 
             << std::endl;

   dicomdir = GDCM_NAME_SPACE::DicomDir::New();
   dicomdir->SetFileName(file);
   dicomdir->Load();
   if (argc > 2) 
   {
      int level = atoi(argv[2]);   
      dicomdir->SetPrintLevel(level);
   }

   // Test if the DICOMDIR file is readable
   if( !dicomdir->IsReadable() )
   {
      std::cout<<"          DicomDir '"<<file
               <<"' is not readable"<<std::endl
               <<"          ...Failed"<<std::endl;

      dicomdir->Delete();
      return 1;
   }
   else
   {
      std::cout<<"          DicomDir '"<<file
               <<"' is readable"<<std::endl;
   }

   // Test if the GDCM_NAME_SPACE::DicomDir contains any patient
   if( !dicomdir->GetFirstPatient() )
   {
      std::cout<<"          DicomDir '"<<file
               <<" has no patient"<<std::endl
               <<"          ...Failed"<<std::endl;

      dicomdir->Delete();
      return 1;
   }

   // step by step structure full exploitation
   std::cout << std::endl << std::endl  
             << " = PATIENT/STUDY/SERIE/IMAGE List ============================" 
             << std::endl<< std::endl;

   pa1 = dicomdir->GetFirstPatient(); 
   while ( pa1 ) 
   {  // we process all the PATIENT of this GDCM_NAME_SPACE::DicomDir 
      std::cout << pa1->GetEntryString(0x0010, 0x0010) << std::endl; // Patient's Name

      st1 = pa1->GetFirstStudy();
      while ( st1 ) 
      { // we process all the STUDY of this patient
         std::cout << "--- "<< st1->GetEntryString(0x0008, 0x1030) // Study Description
         << std::endl;  
         std::cout << " Stud.ID:[" << st1->GetEntryString(0x0020, 0x0010) // Study ID
         << "]"; 

         se1 = st1->GetFirstSerie();
         while ( se1 ) 
         { // we process all the SERIES of this study
            std::cout << "--- --- "<< se1->GetEntryString(0x0008, 0x103e) << std::endl;      // Serie Description
            std::cout << " Ser.nb:["         <<  se1->GetEntryString(0x0020, 0x0011);        // Series number
            std::cout << "] Mod.:["          <<  se1->GetEntryString(0x0008, 0x0060) << "]"; // Modality

            im1 = se1->GetFirstImage();
            while ( im1 ) { // we process all the IMAGE of this serie
               std::cout << "--- --- --- "<< im1->GetEntryString(0x0004, 0x1500) << std::endl; // File name
               im1 = se1->GetNextImage();   
            }
            se1 = st1->GetNextSerie();   
         }
         st1 = pa1->GetNextStudy();
      } 
      pa1 = dicomdir->GetNextPatient();
   }  

/*
// this one is *really* too much verbose!

   std::cout << std::endl << std::endl  
             << " = DICOMDIR full content ====================================" 
             << std::endl<< std::endl;
   dicomdir->Print();
   std::cout << std::endl << std::endl  
             << " = end of DICOMDIR full content ====================================" 
             << std::endl<< std::endl;
*/
   // ------------------------- second stage ---------------------------
 
 
 GDCM_NAME_SPACE::Debug::DebugOn();
 
    
   // Write on disc what we read
   dicomdir->Write("NewDICOMDIR");

   std::cout << std::endl << std::endl  
             << "NewDICOMDIR written on disc =================================" 
             << std::endl<< std::endl;
  // Read what we wrote  
   GDCM_NAME_SPACE::DicomDir *d2 = GDCM_NAME_SPACE::DicomDir::New();
   d2->SetFileName("NewDICOMDIR");
   d2->Load();
   if (!d2->IsReadable())
   {
      std::cout << std::endl << std::endl  
                << "Read NewDicomDir from disc failed ========================" 
                << std::endl<< std::endl;
      return 1;
   }
   std::cout << std::endl << std::endl  
             << "NewDICOMDIR successfully read from disc =================================" 
             << std::endl<< std::endl;
  
   GDCM_NAME_SPACE::DicomDirPatient *pa2;
   GDCM_NAME_SPACE::DicomDirStudy   *st2;
   GDCM_NAME_SPACE::DicomDirSerie   *se2;
   GDCM_NAME_SPACE::DicomDirImage   *im2;

   pa1 = dicomdir->GetFirstPatient(); 
   pa2 = d2->GetFirstPatient(); 

   if (!d2)
   {
      std::cout << "NewDICOMDIR contains no Patient ?!?" << std::endl;
      dicomdir->Delete();
      d2->Delete();
      return 1;
   }
   
   while ( pa1 && pa2 ) 
   {  // we process all the PATIENT of this DICOMDIR

      if ( CompareSQItem(pa2,pa1) == 1 )
      {
         dicomdir->Delete();
         d2->Delete();
         return 1;
      }
  
      // just to allow human reader to be sure ...
      std::cout << pa2->GetEntryString(0x0010, 0x0010) 
                << std::endl; // Patient's Name
 
      st1 = pa1->GetFirstStudy();
      st2 = pa2->GetFirstStudy();

      while ( st1 && st2 )   
      {
         if ( CompareSQItem(st2,st1) == 1 )
         {
            dicomdir->Delete();
            d2->Delete();
            return 1;
         }

         // just to allow human reader to be sure ...
         std::cout << "--- "<< st2->GetEntryString(0x0008, 0x1030);
         // << std::endl;    // Study Description
         std::cout << " Stud.ID:["          
                   << st2->GetEntryString(0x0020, 0x0010)
                   << "]" << std::endl; // Study ID
  
         se1 = st1->GetFirstSerie();
         se2 = st2->GetFirstSerie();

         while ( se1 && se2 ) 
         { // we process all the SERIE of this study
            if ( CompareSQItem(se2,se1) == 1 )
              return 1;

            std::cout << "--- --- " << se2->GetEntryString(0x0008, 0x103e);      // Serie Description
            std::cout << " Ser.nb:["<< se2->GetEntryString(0x0020, 0x0011);        // Series number
            std::cout << "] Mod.:[" << se2->GetEntryString(0x0008, 0x0060) << "]" << std::endl; // Modality
            im1 = se1->GetFirstImage();
            im2 = se2->GetFirstImage();

            while ( im1 && im2 ) // we process all the IMAGE of this serie
            {
               if ( CompareSQItem(im2,im1) == 1 )
               {
                  dicomdir->Delete();
                  d2->Delete();
                  return 1; 
               }

               im1 = se1->GetNextImage();   
               im2 = se2->GetNextImage();   
            }
            se1 = st1->GetNextSerie();   
            se2 = st2->GetNextSerie();   
         }
         st1 = pa1->GetNextStudy();
         st2 = pa2->GetNextStudy();
      }
      pa1 = dicomdir->GetNextPatient();
      pa2 = dicomdir->GetNextPatient();
   }
   
   std::cout << std::flush;
   dicomdir->Delete();
   d2->Delete();

   return 0;
}
