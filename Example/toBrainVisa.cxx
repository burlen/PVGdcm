/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: toBrainVisa.cxx,v $
  Language:  C++
  Date:      $Date: 2007/09/13 12:48:37 $
  Version:   $Revision: 1.3 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmDocument.h"
#include "gdcmDataEntry.h"
#include "gdcmArgMgr.h"

   START_USAGE(usage)
   " \n toBrainVisa :\n                                                       ",
   "   Extracts user supplied tags (identified by group number-element number)",
   "   and writes their values in a text file.                                ",
   "   (file will be used as input by BrainVisa software )                    ",
   " usage :                                                                  ",
   " toBrainVisa      filein=inputFileName  (a DICOM file)                    ",
   "                  fileoutout=outputFileName                               ",
   "                  tags= list of tags to process                           ",
   "                  [verbose] [debug]                                       ",
   "                                                                          ",
   " e.g. : tags=0020-0052,0008-0021,0018-1030 (no space!)                    ",
   " dirout : will be created (as a text file) if doesn't exist               ",
   " verbose  : user wants to run the program in 'verbose mode'               ",
   " debug    : *developer*  wants to run the program in 'debug mode'         ",
   FINISH_USAGE
   
int main (int argc , char *argv[])
{
   // ============== Initialize Arguments Manager =============================
      
   GDCM_NAME_SPACE::ArgMgr *am = new GDCM_NAME_SPACE::ArgMgr(argc, argv);
  
   if (argc == 1 || am->ArgMgrDefined("usage")) 
   {
      am->ArgMgrUsage(usage); // Display 'usage'
      delete am;
      return 0;
   }

   const char *filein;   
   filein  = am->ArgMgrGetString("filein","."); 

   const char *fileout;   
   fileout  = am->ArgMgrGetString("fileout",".");
         
   int verbose  = am->ArgMgrDefined("verbose");
   
   int nbTags;
   uint16_t *tags = am->ArgMgrGetXInt16Enum("tags", &nbTags);   
   
   // if unused Param we give up
   if ( am->ArgMgrPrintUnusedLabels() )
   { 
      am->ArgMgrUsage(usage);
      delete am;
      return 0;
   }
   delete am;  // we don't need Argument Manager any longer

   // ==================== Begin Processing =====================================

   FILE *fp;
   fp=fopen(fileout, "w");
   if (fp == 0)
   {
      std::cout << "Failed to open [" << fileout << "] for writting" << std::endl;
      return 0;
   }      
   GDCM_NAME_SPACE::File *f = GDCM_NAME_SPACE::File::New();
   f->SetLoadMode( GDCM_NAME_SPACE::LD_ALL);
   f->SetFileName( filein );
   bool res = f->Load(); 
         
   if (!res) 
   {
       std::cout << "Sorry, [" << filein << "]  not a gdcm-readable "
                 << "DICOM / ACR File"
                 <<std::endl;
       f->Delete();
       return 0;
   }
   if (verbose)
      std::cout << "[" << filein << "] is readable " << std::endl;
   
   for(int i=0; i<nbTags; i++)
   {
      std::string toto = f->GetEntryString(tags[2*i], tags[2*i+1]);
      fprintf(fp, "%s\n", toto.c_str() );
      if (verbose)
         std::cout << "[" << toto << "]" << std::endl;
   }
   
  f->Delete();
  fclose (fp);        
   
}
