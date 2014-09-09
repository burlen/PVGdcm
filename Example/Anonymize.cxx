/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: Anonymize.cxx,v $
  Language:  C++
  Date:      $Date: 2007/06/15 13:18:50 $
  Version:   $Revision: 1.13 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmCommon.h"
#include "gdcmDebug.h"

#include "gdcmArgMgr.h"

#include <iostream>


/// \todo : AnonymizeDirectory
///         That should split the images : Patient/Study/Serie
///         and keeps coherent the StudyInstanceUID, SeriesInstanceUID
///         (Now, a new one is generated fore each image :-( )
int main(int argc, char *argv[])
{
   START_USAGE(usage)
   " \n Anonymize :\n                                                         ",
   " Anonymize a full gdcm-readable Dicom image                               ",
   "          Warning : probably segfaults if pixels are not gdcm readable.   ",
   "                    Use AnonymizeNoLoad instead.                          ",
   " usage: Anonymize filein=inputFileName fileout=anonymizedFileName[debug]  ",
   "        debug    : user wants to run the program in 'debug mode'          ",
   FINISH_USAGE

   // ----- Initialize Arguments Manager ------   
   GDCM_NAME_SPACE::ArgMgr *am = new GDCM_NAME_SPACE::ArgMgr(argc, argv);
  
   if (argc == 1 || am->ArgMgrDefined("usage")) 
   {
      am->ArgMgrUsage(usage); // Display 'usage'
      delete am;
      return 0;
   }
   char *fileName = am->ArgMgrWantString("filein",usage);
   if ( fileName == NULL )
   {
      delete am;
      return 0;
   }

   char *outputFileName = am->ArgMgrWantString("fileout",usage);
   if ( outputFileName == NULL )
   {
      delete am;
      return 0;
   }
   if (am->ArgMgrDefined("debug"))
      GDCM_NAME_SPACE::Debug::DebugOn();
 
   // if unused Param we give up
   if ( am->ArgMgrPrintUnusedLabels() )
   { 
      am->ArgMgrUsage(usage);
      delete am;
      return 0;
   }

   delete am;  // we don't need Argument Manager any longer

   // ============================================================
   //   Read the input file.
   // ============================================================

   GDCM_NAME_SPACE::File *f;

   f = GDCM_NAME_SPACE::File::New(  );
   f->SetLoadMode( GDCM_NAME_SPACE::LD_ALL );
   f->SetFileName( fileName );
   int res = f->Load();

   if ( !res ) 
   {
       std::cerr << "Sorry, " << fileName <<"  not a gdcm-readable "
                 << "DICOM / ACR File" <<std::endl;
       f->Delete();
       return 0;
   }
   std::cout << " ... is readable " << std::endl;

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
       std::cerr << "Sorry, Pixels of" << fileName <<"  are not "
                 << " gdcm-readable."       << std::endl
                 << "Use exAnonymizeNoLoad" << std::endl;
       f->Delete();
       fh->Delete();
       return 0;
   } 

   // ============================================================
   //  Choose the fields to anonymize.
   // ============================================================
   // Institution name 
   f->AddAnonymizeElement(0x0008, 0x0080, "Xanadoo"); 
   // Patient's name 
   f->AddAnonymizeElement(0x0010, 0x0010, "Fantomas^X");   
   // Patient's ID
   f->AddAnonymizeElement( 0x0010, 0x0020,"1515" );   
   // Study Instance UID
   f->AddAnonymizeElement(0x0020, 0x000d, "9.99.999.9999" );
   // Telephone
   f->AddAnonymizeElement(0x0010, 0x2154, "3615" );

   // Aware user will add here more fields to anonymize here

   // The gdcm::File is modified in memory

   f->AnonymizeFile();

   // ============================================================
   //   Write a new file
   // ============================================================
   
   // Since we just Anonymized the file, we know that no modification 
   // was performed on the pixels.
   // The written image will not appear as a 'Secondary Captured image'
   // nor as a DERIVED one  

   fh->SetContentType(GDCM_NAME_SPACE::UNMODIFIED_PIXELS_IMAGE);
   
   fh->WriteDcmExplVR(outputFileName);
   std::cout <<"End Anonymize" << std::cout;

   // ============================================================
   //   Remove the Anonymize list
   // ============================================================  
   f->ClearAnonymizeList();
    
   f->Delete();
   fh->Delete();
   return 0;
}

