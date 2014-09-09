/*=========================================================================
                                                                                 
  Program:   gdcm
  Module:    $RCSfile: AnonymizeReWriteMultiPatient.cxx,v $
  Language:  C++
  Date:      $Date: 2008/03/18 13:37:54 $
  Version:   $Revision: 1.4 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmDocEntry.h"
#include "gdcmDicomDir.h"
#include "gdcmDicomDirPatient.h"
#include "gdcmDicomDirStudy.h"
#include "gdcmDicomDirVisit.h"
#include "gdcmDicomDirSerie.h"
#include "gdcmDicomDirImage.h"
#include "gdcmDirList.h"
#include "gdcmDebug.h"
#include "gdcmUtil.h"
#include "gdcmFile.h"
#include "gdcmFileHelper.h"

#include "gdcmArgMgr.h"

#include <iostream>

/**
  * \brief   Explores recursively the given directory
  *          orders the gdcm-readable found Files
  *          according their Patient/Study/Serie/Image characteristics
  *          and anomymizes (rewrite) them, creates a different name for each Patient.
  */  

int main(int argc, char *argv[]) 
{
   START_USAGE(usage)
   " \n AnonymizeReWriteMultiPatient :\n                                      ",
   " AnonymizeReWriteMultiPatient a full gdcm-readable Dicom image            ",
   "         optionnaly, creates the DICOMDIR                                 ",
   "         Warning : the image is OVERWRITTEN                               ",
   "                   to preserve image integrity, use a copy.               ",
   " usage: AnonymizeReWriteMultiPatient dirin=inputDirectoryName  dicomdir   ",
   "       dirin : directory holding images to anonymize                      ",
   "       rootname : root for the 'anonymized' name                          ",
   "       listOfElementsToRubOut : group-elem,g2-e2,... (in hexa, no space)  ",
   "                                of extra Elements to rub out              ",
   "       dicomdir   : user wants to generate a DICOMDIR                     ",
   "       verbose    : user wants to run the program in 'verbose mode'       ",   
   "       debug      : user wants to run the program in 'debug mode'         ",   
   FINISH_USAGE
   


   // ----- Initialize Arguments Manager ------   
   GDCM_NAME_SPACE::ArgMgr *am = new GDCM_NAME_SPACE::ArgMgr(argc, argv);
  
   if (argc == 1 || am->ArgMgrDefined("usage")) 
   {
      am->ArgMgrUsage(usage); // Display 'usage'
      delete am;
      return 0;
   }
   const char * name  = am->ArgMgrGetString("dirin");
   if ( name == NULL )
   {
      delete am;
      return 0;
   }
   
   std::string dirName = name;
   
   bool verbose  = ( 0 != am->ArgMgrDefined("verbose") ); 
   bool dicomdir = ( 0 != am->ArgMgrDefined("dicomdir") );
   
   const char *rootname  = am->ArgMgrGetString("rootname","Patient");
     
   if (am->ArgMgrDefined("debug"))
     GDCM_NAME_SPACE::Debug::DebugOn();

   int loadMode = GDCM_NAME_SPACE::LD_ALL;

   int rubOutNb;
   uint16_t *elemsToRubOut = am->ArgMgrGetXInt16Enum("rubout", &rubOutNb);
 
   // ----------- if unused Param we give up
   if ( am->ArgMgrPrintUnusedLabels() )
   { 
      am->ArgMgrUsage(usage);
      delete am;
      return 0;
   }

   delete am;  // we don't need Argument Manager any longer

 
  // ---------------------------------------------------------- 
  

   // ----- Begin Processing -----

   GDCM_NAME_SPACE::DicomDir *dcmdir;

   // we ask for Directory parsing

   dcmdir = GDCM_NAME_SPACE::DicomDir::New( );
   dcmdir->SetLoadMode(loadMode);
   dcmdir->SetDirectoryName(dirName);
   dcmdir->Load();

   if ( verbose )
      std::cout << "======================= End Parsing Directory" << std::endl;
      
    // ----- Check the result
    
   if ( !dcmdir->GetFirstPatient() ) 
   {
      std::cout << "No patient found (?!?). Exiting."
                << std::endl;
      dcmdir->Delete();
      return 1;
   }

   GDCM_NAME_SPACE::DicomDirPatient *pa;
   GDCM_NAME_SPACE::DicomDirStudy *st;
   GDCM_NAME_SPACE::DicomDirSerie *se;
   GDCM_NAME_SPACE::DicomDirImage *im;

   std::string codedName;
   std::string codedID;
   std::string fullFileName;
   std::string patName;
   std::string patID;
        
   GDCM_NAME_SPACE::File *f;
 
 
  //GDCM_NAME_SPACE::Debug::DebugOn();
  
   
   int sequentialPatientNumber = 0;
   char  char_sequentialPatientNumber[10]; // 999999999 patients in a directory should be enough?
   pa = dcmdir->GetFirstPatient();    
   while ( pa )
   {  // on degouline les PATIENT du DICOMDIR
   
      sequentialPatientNumber++;
      sprintf (char_sequentialPatientNumber, "%d", sequentialPatientNumber);
      //patName = pa->GetEntryString(0x0010, 0x0010);
      
     
      //codedName = "g^" + GDCM_NAME_SPACE::Util::ConvertToMD5(patName);
      codedName = "g^" + std::string(rootname) + std::string(char_sequentialPatientNumber);
      patID = pa->GetEntryString(0x0010, 0x0020);
      codedID = GDCM_NAME_SPACE::Util::ConvertToMD5(patID);
      
      if (verbose) {
         std::cout << "[" << patName << "] --> [" << codedName << "]" << std::endl;
         std::cout << "[" << patID   << "] --> [" << codedID   << "]"  << std::endl;                 
      } 
      st = pa->GetFirstStudy();
      while ( st ) 
      { // on degouline les STUDY de ce patient
         se = st->GetFirstSerie();
         while ( se ) 
         { // on degouline les SERIES de cette study
            im = se->GetFirstImage();
            while ( im ) 
            { // on degouline les Images de cette serie       
               fullFileName = dirName;
               const char lastChar = dirName.c_str()[strlen(dirName.c_str())-1];
               if ( lastChar != '/' && lastChar != '\\')
                  fullFileName +=  GDCM_NAME_SPACE::GDCM_FILESEPARATOR;
               fullFileName += im->GetEntryString(0x0004, 0x1500);
               
               // -- remove trailing space, if any
               int pos=fullFileName.length()-1;
               if (fullFileName[pos] == ' ')
               {
                  fullFileName.erase(pos);
               }
               // --
               
               if (verbose)
                  std::cout << "FileName [" << fullFileName << "]" << std::endl;

               f = GDCM_NAME_SPACE::File::New( );
                  f->SetMaxSizeLoadEntry(0x7fff);  // we want to load entries of any length !
                  f->SetLoadMode(loadMode);
                  f->SetFileName( fullFileName );
               if ( !f->Load() )
               {
                 std::cout << "Load failed for [" << fullFileName << "]" << std::endl;
                 //f->Delete();
                 //continue;  // or return 0; ?
               }
               else
               {
                  if (verbose)
                     std::cout << "Load successed for [" << fullFileName << "]" << std::endl;
               }
               // 
               //  Choose the fields to anonymize.
               // 

               // Institution name 
               f->AddAnonymizeElement( 0x0008, 0x0080, "Xanadoo" );

               // Patient's name 
               f->AddAnonymizeElement( 0x0010, 0x0010, codedName ); 
    
               // Patient's ID
               //f->AddAnonymizeElement( 0x0010, 0x0020,"1515" );
               f->AddAnonymizeElement( 0x0010, 0x0020,codedID );
               // Patient's Birthdate
               f->AddAnonymizeElement( 0x0010, 0x0030,"11111111" );
               // Patient's Adress
               f->AddAnonymizeElement( 0x0010, 0x1040,"Sing-sing" );
               // Patient's Mother's Birth Name
               f->AddAnonymizeElement( 0x0010, 0x1060,"g^Vampirella" );
       
               // Study Instance UID
               // we may not brutaly overwrite it
               //f->AddAnonymizeElement( 0x0020, 0x000d, "9.99.999.9999" );
   
               // Telephone
               f->AddAnonymizeElement(0x0010, 0x2154, "3615" );

               // deal with user defined Elements set

               for (int ri=0; ri<rubOutNb; ri++)
               {
                  f->AddAnonymizeElement((uint32_t)elemsToRubOut[2*ri], 
                                         (uint32_t)elemsToRubOut[2*ri+1],"*" ); 
               }

   // ============================================================
   //   Load the pixels in memory.
   // ============================================================

               // We need a gdcm::FileHelper, since we want to load the pixels        
               GDCM_NAME_SPACE::FileHelper *fh = GDCM_NAME_SPACE::FileHelper::New(f);

               // unit8_t DOESN'T mean it's mandatory for the image to be a 8 bits one !
               // Feel free to cast if you know it's not.

               uint8_t *imageData = fh->GetImageData();

               if ( imageData == 0 )
               {
                   std::cerr << "Sorry, Pixels of [" << fullFileName <<"]  are not "
                             << " gdcm-readable."       << std::endl
                             << "Use exAnonymizeNoLoad" << std::endl;
                   f->Delete();
                   fh->Delete();
                   return 0;
               }
               
   // ============================================================
              f->AnonymizeFile();    
              f->ClearAnonymizeList();
              
              fh->SetContentType(GDCM_NAME_SPACE::UNMODIFIED_PIXELS_IMAGE);
              fh->Write(fullFileName);  // WARNING : overwrites the file!
              
              im = se->GetNextImage(); 
              f->Delete();
              fh->Delete();                  
           }
           se = st->GetNextSerie();   
        }
        st = pa->GetNextStudy();
     }     
     pa = dcmdir->GetNextPatient();      
   }

   dcmdir->Delete();
   
   if (dicomdir)
   {
      std::cout << "DICOMDIR creation in progress ..." << std::endl;      
      dcmdir = GDCM_NAME_SPACE::DicomDir::New( );
      dcmdir->SetLoadMode(loadMode);
      dcmdir->SetDirectoryName(dirName);
      dcmdir->Load();
      dcmdir->Write("DICOMDIR");   
   }
             
   return 0;
   
}
