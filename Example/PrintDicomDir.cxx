/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: PrintDicomDir.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:04 $
  Version:   $Revision: 1.36 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmDocument.h"
#include "gdcmDicomDir.h"
#include "gdcmDicomDirPatient.h"
#include "gdcmDicomDirStudy.h"
#include "gdcmDicomDirVisit.h"
#include "gdcmDicomDirSerie.h"
#include "gdcmDicomDirImage.h"
#include "gdcmDicomDirPrivate.h"
#include "gdcmTS.h"
#include "gdcmDebug.h"

#include "gdcmArgMgr.h"

#include <fstream>
#include <iostream>

int main(int argc, char* argv[])
{
   START_USAGE(usage)
   " \n PrintDicomDir :\n                                                  ",
   " Display the tree-like structure of a DICOMDIR File                    ",
   " usage: PrintDicomDir filein=fileName [level=n][detail=m][debug]       ",
   "        detail = 1 : Patients, 2 : Studies, 3 : Series, 4 : Images     ",
   "                 5 : Full Content                                      ",
   "        level = 0,1,2 : depending on user (what he wants to see)       ",
   "        warning  : user wants to run the program in 'warning mode'     ",   
   "        debug    : developper wants to run the program in 'debug mode' ",
   FINISH_USAGE

   // Initialize Arguments Manager   
   GDCM_NAME_SPACE::ArgMgr *am= new GDCM_NAME_SPACE::ArgMgr(argc, argv);
  
   if (argc == 1 || am->ArgMgrDefined("usage") )
   {
      am->ArgMgrUsage(usage); // Display 'usage'
      delete am;
      return 0;
   }
  
   GDCM_NAME_SPACE::DicomDir *f;
   GDCM_NAME_SPACE::TSKey v;

   GDCM_NAME_SPACE::DicomDirPatient *pa;
   GDCM_NAME_SPACE::DicomDirStudy *st;
   GDCM_NAME_SPACE::DicomDirSerie *se;
   GDCM_NAME_SPACE::DicomDirVisit *vs;
   GDCM_NAME_SPACE::DicomDirImage *im;
   GDCM_NAME_SPACE::DicomDirPrivate *pr;  
   char *fileName;
   fileName  = am->ArgMgrWantString("filein",usage); 

   int level  = am->ArgMgrGetInt("level", 2);

   int detailLevel = am->ArgMgrGetInt("detail", 2);

   if (am->ArgMgrDefined("debug"))
      GDCM_NAME_SPACE::Debug::DebugOn();

   if (am->ArgMgrDefined("warning"))
      GDCM_NAME_SPACE::Debug::WarningOn();
      
   /* if unused Param we give up */
   if ( am->ArgMgrPrintUnusedLabels() )
   { 
      am->ArgMgrUsage(usage);
      delete am;
      return 0;
   } 

   // new gdcm2 style 

   f = GDCM_NAME_SPACE::DicomDir::New();
   f->SetFileName ( fileName );
   f->Load( );

   // Test if the DicomDir is readable
   if( !f->IsReadable() )
   {
      std::cout<<"          DicomDir '"<<fileName
               <<"' is not readable"<<std::endl
               <<"          ...Failed"<<std::endl;

      f->Delete();
      return 1;
   }

   f->SetPrintLevel(level);

   // Test if the DicomDir contains any Patient
   pa = f->GetFirstPatient();
   if ( pa == 0)
   {
      std::cout<<"          DicomDir '"<<fileName
               <<" has no patient"<<std::endl
               <<"          ...Failed"<<std::endl;

      f->Delete();
      return 1;
   }

// Structure use Examples 

   switch (detailLevel)
   { 
      case 1:
         std::cout << std::endl << std::endl  
         << " =  PATIENT List ==========================================" 
         << std::endl<< std::endl;

         pa = f->GetFirstPatient();
         while (pa) 
         {
            std::cout << "Pat.Name:[" << pa->GetEntryString(0x0010, 0x0010) <<"]"; // Patient's Name
            std::cout << " Pat.ID:[";
            std::cout << pa->GetEntryString(0x0010, 0x0020) << "]" << std::endl; // Patient ID
            pa = f->GetNextPatient();    
         }
         break;

      case 2:    
         std::cout << std::endl << std::endl  
         << " = PATIENT/STUDY List =======================================" 
         << std::endl<< std::endl;

         pa = f->GetFirstPatient();
         while ( pa ) // on degouline les PATIENT de ce DICOMDIR
         {  
            std::cout << "Pat.Name:[" << pa->GetEntryString(0x0010, 0x0010) <<"]"; // Patient's Name
            std::cout << " Pat.ID:[";
            std::cout << pa->GetEntryString(0x0010, 0x0020) << "]" << std::endl; // Patient ID
            st = pa->GetFirstStudy();
            while ( st ) 
            { // on degouline les STUDY de ce patient
               std::cout << "--- Stud.descr:["    << st->GetEntryString(0x0008, 0x1030) << "]"; // Study Description 
               std::cout << " Stud.ID:["          << st->GetEntryString(0x0020, 0x0010) << "]"; // Study ID
               std::cout << " Stud.Inst.UID:["    << st->GetEntryString(0x0020, 0x000d) << "]"; // Study Instance UID
               std::cout << std::endl; 
               st = pa->GetNextStudy();
            }
            pa = f->GetNextPatient();    
         }   
         break;

      case 3: 
         std::cout << std::endl << std::endl  
         << " =  PATIENT/STUDY/SERIE List ==================================" 
         << std::endl<< std::endl;

         pa = f->GetFirstPatient(); 
         while ( pa )   // on degouline les PATIENT de ce DICOMDIR
         {
            // Patient's Name, Patient ID 
            std::cout << "Pat.Name:[" << pa->GetEntryString(0x0010, 0x0010) <<"]"; // Patient's Name
            std::cout << " Pat.ID:[";
            std::cout << pa->GetEntryString(0x0010, 0x0020) << "]" << std::endl; // Patient ID

            st = pa->GetFirstStudy();
            while ( st ) 
            { // on degouline les STUDY de ce patient
               std::cout << "--- Stud.descr:["    << st->GetEntryString(0x0008, 0x1030) << "]"; // Study Description 
               std::cout << " Stud.ID:["          << st->GetEntryString(0x0020, 0x0010) << "]"; // Study ID
               std::cout << " Stud.Inst.UID:["    << st->GetEntryString(0x0020, 0x000d) << "]"; // Study Instance UID       
               std::cout << std::endl;

               se = st->GetFirstSerie();
               while ( se ) 
               { // on degouline les SERIES de cette study
                  std::cout << "--- --- Ser.Descr:["<< se->GetEntryString(0x0008, 0x103e)<< "]";  // Series Description
                  std::cout << " Ser.nb:["          << se->GetEntryString(0x0020, 0x0011);        // Series number
                  std::cout << "] Mod.:["           << se->GetEntryString(0x0008, 0x0060) << "]"; // Modality
                  std::cout << " Serie Inst.UID.:[" <<  se->GetEntryString(0x0020, 0x000e) << "]";  // Series Instance UID
                  std::cout << std::endl;    
                  se = st->GetNextSerie();   
               }

               vs = st->GetFirstVisit();
               while ( vs ) 
               { // on degouline les VISIT de cette study
                  std::cout << "--- --- VISIT: ";
                  std::cout << " Ref. File ID :[" << vs->GetEntryString(0x0004, 0x1500) << "]"; // Referenced File ID
                  std::cout << " Inst.Name:["    << vs->GetEntryString(0x0008,0x0080)   << "]"; // Institution Name
                  std::cout << " Adm.ID:["      << vs->GetEntryString(0x0038, 0x0010)   << "]"; // Admission ID
                  std::cout << " Adm. date:["   << vs->GetEntryString(0x0038, 0x0020)   << "]"; // Admitting Date
                  std::cout << std::endl;    
                  vs = st->GetNextVisit();    
               }

               st = pa->GetNextStudy();
            }
            pa = f->GetNextPatient();    
         } 
         break;

      case 4:  
         std::cout << std::endl << std::endl  
         << " = PATIENT/STUDY/SERIE/IMAGE List ============================" 
         << std::endl<< std::endl;

         pa = f->GetFirstPatient(); 
         while ( pa )
         {  // les PATIENT de ce DICOMDIR
            // Patient's Name, Patient ID 
            std::cout << "Pat.Name:[" << pa->GetEntryString(0x0010, 0x0010) <<"]"; // Patient's Name
            std::cout << " Pat.ID:[";
            std::cout << pa->GetEntryString(0x0010, 0x0020) << "]" << std::endl; // Patient ID

            st = pa->GetFirstStudy();
            while ( st ) 
            { // on degouline les STUDY de ce patient
               std::cout << "--- Stud.descr:["    << st->GetEntryString(0x0008, 0x1030) << "]"; // Study Description 
               std::cout << " Stud.ID:["          << st->GetEntryString(0x0020, 0x0010) << "]"; // Study ID
               std::cout << " Stud.Inst.UID:["    << st->GetEntryString(0x0020, 0x000d) << "]"; // Study Instance UID
               std::cout << std::endl;

               vs = st->GetFirstVisit();
               while ( vs ) 
               { // on degouline les VISIT de cette study
                  std::cout << "--- --- VISIT: ";
                  std::cout << " Ref. File ID :[" << vs->GetEntryString(0x0004, 0x1500) << "]"; // Referenced File ID
                  std::cout << " Inst.Name:["     << vs->GetEntryString(0x0008,0x0080)  << "]"; // Institution Name
                  std::cout << " Adm.ID:["        << vs->GetEntryString(0x0038, 0x0010) << "]"; // Admission ID
                  std::cout << " Adm. date:["     << vs->GetEntryString(0x0038, 0x0020) << "]"; // Admitting Date
                  std::cout << std::endl;    
                  vs = st->GetNextVisit();    
               }

               se = st->GetFirstSerie();
               while ( se ) 
               { // on degouline les SERIES de cette study
                  std::cout << "--- --- Ser.Descr:["<< se->GetEntryString(0x0008, 0x103e) << "]";  // Series Description
                  std::cout << " Ser.nb:["          <<  se->GetEntryString(0x0020, 0x0011) << "]";  // Series number
                  std::cout << " Mod.:["            <<  se->GetEntryString(0x0008, 0x0060) << "]";  // Modality
                  std::cout << " Serie Inst.UID.:[" <<  se->GetEntryString(0x0020, 0x000e) << "]";  // Series Instance UID
                  std::cout << std::endl;    

                  im = se->GetFirstImage();
                  while ( im ) 
                  { // on degouline les Images de cette serie
                     std::cout << "--- --- --- "<< " IMAGE Ref. File ID :[" << im->GetEntryString(0x0004, 0x1500) 
                               << "]" << std::endl; // File name (Referenced File ID)
                     im = se->GetNextImage();   
                  }
 
                  pr = se->GetFirstPrivate();
                  while ( pr ) 
                  { // on degouline les 'Privates' de cette serie
                     std::cout << "--- --- --- "<< " PRIVATE Ref. File ID :[" << pr->GetEntryString(0x0004, 0x1500) 
                               << "]" << std::endl; // File name (Referenced File ID)
                     pr = se->GetNextPrivate();   
                  }
 
                  se = st->GetNextSerie();   
               }
               st = pa->GetNextStudy();
            }     
            pa = f->GetNextPatient();    
         }
         break;

      case 5:
         std::cout << std::endl << std::endl  
         << " = DICOMDIR full content ==========================================" 
         << std::endl<< std::endl;
         f->Print();
         break;

   }  // end switch


 /*
   // Previous code.
   // Kept as an example. Please don't remove
 
   GDCM_NAME_SPACE::ListDicomDirPatient::const_iterator  itPatient;
   GDCM_NAME_SPACE::ListDicomDirStudy::const_iterator    itStudy;
   GDCM_NAME_SPACE::ListDicomDirSerie::const_iterator    itSerie;
   GDCM_NAME_SPACE::ListDicomDirImage::const_iterator    itImage;
   cout << std::endl << std::endl
        << " = Liste des PATIENT/STUDY/SERIE/IMAGE ===================================" 
        << std::endl<< std::endl;
 
   itPatient = f->GetDicomDirPatients().begin();
   while ( itPatient != f->GetDicomDirPatients().end() ) {  // on degouline les PATIENT de ce DICOMDIR
      std::cout << (*itPatient)->GetEntryString(0x0010, 0x0010) << std::endl; // Patient's Name
      itStudy = ((*itPatient)->GetDicomDirStudies()).begin();
      while (itStudy != (*itPatient)->GetDicomDirStudies().end() ) { // on degouline les STUDY de ce patient
         std::cout << "--- "<< (*itStudy)->GetEntryString(0x0008, 0x1030) << std::endl; // Study Description
         itSerie = ((*itStudy)->GetDicomDirSeries()).begin();
         while (itSerie != (*itStudy)->GetDicomDirSeries().end() ) { // on degouline les SERIES de cette study
            std::cout << "--- --- "<< (*itSerie)->GetEntryString(0x0008, 0x103e) << std::endl; // Serie Description
            itImage = ((*itSerie)->GetDicomDirImages()).begin();
            while (itImage != (*itSerie)->GetDicomDirImages().end() ) { // on degouline les IMAGES de cette serie
               std::cout << "--- --- --- "<< (*itImage)->GetEntryString(0x0004, 0x1500) << std::endl; // File name
               ++itImage;   
            }
            ++itSerie;   
         }
         ++itStudy;
      }  
      itPatient ++;    
   }   
 */  


   if(f->IsReadable())
      std::cout <<std::endl<<fileName<<" is Readable"<<std::endl;
   else
      std::cout <<std::endl<<fileName<<" is NOT Readable"<<std::endl;
   std::cout<<std::flush;
   f->Delete();

   return(0);
}
