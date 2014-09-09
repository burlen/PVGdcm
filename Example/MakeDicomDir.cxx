/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: MakeDicomDir.cxx,v $
  Language:  C++
  Date:      $Date: 2007/09/28 14:09:20 $
  Version:   $Revision: 1.25 $
                                                                                
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
#include "gdcmDirList.h"
#include "gdcmDebug.h"
#include "gdcmArgMgr.h"

//#include <sys/times.h> // Linux Only

#include <iostream>

/**
  * \brief   Explores recursively the given directory
  *          orders the gdcm-readable found Files
  *          according their Patient/Study/Serie/Image characteristics
  *          makes the gdcmDicomDir 
  *          and writes a file named DICOMDIR. (user may choose an other name)
  */  

int main(int argc, char *argv[]) 
{
   START_USAGE(usage)
   " \n MakeDicomDir :\n                                                      ",
   " Explores recursively the given directory, makes the relevant DICOMDIR    ",
   "          and writes it as 'DICOMDIR'                                     ",
   "                                                                          ", 
   " usage: MakeDicomDir dirname=rootDirectoryName                            ",
   "                     name=DICOMDIR file name                              ",
   "        [noshadowseq][noshadow][noseq] [debug] [check]                    ",
   "                                                                          ",
   "        name : the default name for the generated dicomdir is 'DICOMDIR'  ",
   "        noshadowseq: user doesn't want to load Private Sequences          ",
   "        noshadow : user doesn't want to load Private groups (odd number)  ",
   "        noseq    : user doesn't want to load Sequences                    ",
   "        debug    : developper wants to run the program in 'debug mode'    ",
   "        check    : the dicomdir is checked as 'gdcm readable'             ",
   FINISH_USAGE

   // ----- Initialize Arguments Manager ------   
   GDCM_NAME_SPACE::ArgMgr *am = new GDCM_NAME_SPACE::ArgMgr(argc, argv);
  
   if (argc == 1 || am->ArgMgrDefined("usage")) 
   {
      am->ArgMgrUsage(usage); // Display 'usage'
      delete am;
      return 0;
   }

   const char *dirName;   
   dirName  = am->ArgMgrGetString("dirName","."); 

   const char *name;
   name  = am->ArgMgrGetString("name","DICOMDIR");
   
   int loadMode = GDCM_NAME_SPACE::LD_ALL;
   if ( am->ArgMgrDefined("noshadowseq") )
      loadMode |= GDCM_NAME_SPACE::LD_NOSHADOWSEQ; 
   else 
   {
   if ( am->ArgMgrDefined("noshadow") )
         loadMode |= GDCM_NAME_SPACE::LD_NOSHADOW;
      if ( am->ArgMgrDefined("noseq") )
         loadMode |= GDCM_NAME_SPACE::LD_NOSEQ;
   }

   if (am->ArgMgrDefined("debug"))
      GDCM_NAME_SPACE::Debug::DebugOn();
      
   int check = am->ArgMgrDefined("check"); 
   
   // if unused Param we give up
   if ( am->ArgMgrPrintUnusedLabels() )
   { 
      am->ArgMgrUsage(usage);
      delete am;
      return 0;
   }
   
   delete am;  // we don't need Argument Manager any longer

   // ----- Begin Processing -----

   GDCM_NAME_SPACE::DicomDir *dcmdir;
   
   // we ask for Directory parsing

   dcmdir = GDCM_NAME_SPACE::DicomDir::New( );

   dcmdir->SetLoadMode(loadMode);
   dcmdir->SetDirectoryName(dirName);
   //dcmdir->SetParseDir(true);
   
     // struct tms tms1, tms2; // Time measurements
     // times(&tms1);
       
   dcmdir->Load(); // Reads all the files and creates the GDCM_NAME_SPACE::DicomDir
   
      //times(&tms2);      
      //std::cout 
      //  << (long) ((tms2.tms_utime)  - (tms1.tms_utime)) 
      // << std::endl;

   if ( GDCM_NAME_SPACE::Debug::GetDebugFlag() )
      std::cout << "======================= End Parsing Directory" << std::endl;
      
    // ----- Check the result
    
   if ( !dcmdir->GetFirstPatient() ) 
   {
      std::cout << "makeDicomDir: no patient found. Exiting."
                << std::endl;
      dcmdir->Delete();
      return 1;
   }
    
   // ----- Writes the corresponding DICOMDIR file (from the GDCM_NAME_SPACE::DicomDir)

   dcmdir->Write(name);
   dcmdir->Delete();
   
   if (check) 
   {
      if ( GDCM_NAME_SPACE::Debug::GetDebugFlag() )
         std::cout << "======================= End Writting DICOMDIR" 
                   << std::endl;

     // Read from disc the just written DicomDir
    
      GDCM_NAME_SPACE::DicomDir *newDicomDir = GDCM_NAME_SPACE::DicomDir::New();
      newDicomDir->SetFileName( name );
      newDicomDir->Load();
      if ( GDCM_NAME_SPACE::Debug::GetDebugFlag() )
         std::cout << "======================= End Parsing DICOMDIR" 
                   << std::endl;   
      if( !newDicomDir->IsReadable() )
      {
         std::cout<<"          Written DicomDir [" << name << "] "
                  <<" is not readable"<<std::endl
                  <<"          ...Failed"<<std::endl;

         newDicomDir->Delete();
         return 1;
      }

      if( !newDicomDir->GetFirstPatient() )
      {
         std::cout <<"          Written DicomDir [" << name << "] "
                   <<" has no patient"<<std::endl
                   <<"          ...Failed"<<std::endl;

         newDicomDir->Delete();
         return(1);
      } 
      std::cout<<std::flush;
      newDicomDir->Delete();        
   }
   return 0;
}
