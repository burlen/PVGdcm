/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: exSerieHelper.cxx,v $
  Language:  C++
  Date:      $Date: 2009/05/28 15:44:34 $
  Version:   $Revision: 1.18 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmSerieHelper.h"
#include "gdcmFile.h"
#include "gdcmDirList.h" // for FileList
#include "gdcmDebug.h"
#include <iostream>
#include "gdcmArgMgr.h"

int main(int argc, char *argv[])
{
   START_USAGE(usage)
   "\n exSerieHelper :\n                                                      ",
   "Example on how to use the methodes of gdcm::SerieHelper                   ",
   "usage: exSerieHelper {dirin=inputDirectoryName}                           ",
   "                       [ { [noshadowseq] | [noshadow][noseq] } ] [debug]  ",
   "                                                                          ",
   "       dirin : user wants to analyze *all* the files                      ",
   "                            within the directory                          ",
   "       noshadowseq: user doesn't want to load Private Sequences           ",
   "       noshadow   : user doesn't want to load Private groups (odd number) ",
   "       noseq      : user doesn't want to load Sequences                   ",
   "       verbose    : user wants to run the program in 'verbose mode'       ",
   "       warning    : user wants to run the program in 'warning mode'       ",   
   "       debug      : developper wants to run the program in 'debug mode'   ",
   FINISH_USAGE

   // ----- Initialize Arguments Manager ------
  
   GDCM_NAME_SPACE::ArgMgr *am = new GDCM_NAME_SPACE::ArgMgr(argc, argv);
  
   if (am->ArgMgrDefined("usage") || argc == 1) 
   {
      am->ArgMgrUsage(usage); // Display 'usage'
      delete am;
      return 0;
   }

   if (am->ArgMgrDefined("debug"))
      GDCM_NAME_SPACE::Debug::DebugOn();
      
   if (am->ArgMgrDefined("warning"))
      GDCM_NAME_SPACE::Debug::WarningOn();      

   bool verbose = ( 0 != am->ArgMgrDefined("verbose") );
      
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
   
   const char *dirName  = am->ArgMgrGetString("dirin");
   if (dirName == 0)
   {
       std::cout <<std::endl
                 << "'dirin=' must be present;" 
                 <<  std::endl;
       am->ArgMgrUsage(usage); // Display 'usage'  
       delete am;
       return 0;
   }

  
   std::cout << "Dir Name :[" << dirName << "]" << std::endl;
   //   
   // Sometimes using only SerieHelper is not enought !
   // See also exXcoherentFileSet
   //
   
   
   GDCM_NAME_SPACE::SerieHelper *s;
   s = GDCM_NAME_SPACE::SerieHelper::New();
   s->SetLoadMode(GDCM_NAME_SPACE::LD_ALL);     // Load everything for each File
   //GDCM_NAME_SPACE::TagKey t(0x0020,0x0013);
   //s->AddRestriction(t, "340", GDCM_NAME_SPACE::GDCM_LESS); // Keep only files where
                                              // restriction is true
   s->SetDirectory(dirName, true); // true : recursive exploration

   if (verbose) {
      std::cout << " ---------------------------------------- Finish parsing :["
                << dirName << "]" << std::endl;

      s->Print();
     std::cout << " ---------------------------------------- Finish printing (1)"
                << std::endl;
   }

   GDCM_NAME_SPACE::FileList::const_iterator it;
   GDCM_NAME_SPACE::FileList *l;
   std::cout << std::endl << " ---------------------------------------- Recap"
             << std::endl;  
   l = s->GetFirstSingleSerieUIDFileSet();
   while (l)
   { 
      it = l->begin();
      std::cout << "SerieUID [" <<  (*it)->GetEntryString(0x0020,0x000e) <<"]   Serie Description ["
                << (*it)->GetEntryString(0x0008,0x103e) << "] "  
                << " : " << l->size() << " files" << std::endl;
      l = s->GetNextSingleSerieUIDFileSet();
   } 
    std::cout << " ----------------------------------------End Recap"
             << std::endl << std::endl;

   int nbFiles;
   double zspacing = 0.;
   // For all the Single SerieUID Files Sets of the GDCM_NAME_SPACE::Serie
   l = s->GetFirstSingleSerieUIDFileSet();
   while (l)
   { 
      nbFiles = l->size() ;
      if ( nbFiles > 5 ) // Why not ? Just an example, for testing
      {
         std::cout << "List to sort : " << nbFiles << " long" << std::endl;  
         //---------------------------------------------------------
         s->OrderFileList(l);  // sort the list (and compute ZSpacing !)
         //---------------------------------------------------------
         std::cout << "List after sorting : " << l->size() << " long" << std::endl;
           
          zspacing = s->GetZSpacing();
         // Just to show : GetZSpacing from a GDCM_NAME_SPACE::SerieHelper is right  
         std::cout << "GetZSpacing() of sorted SingleSerieUIDFileSet "
                   << "from GDCM_NAME_SPACE::SerieHelper: " << zspacing << std::endl;
         std::cout << " ('-1' means all the files have the same position)" << std::endl;
         
         // Check the vector content
         int fileCount = 0;
      // for (std::vector<GDCM_NAME_SPACE::File* >::iterator it2 =  l->begin();
         for (GDCM_NAME_SPACE::FileList::const_iterator it2 = l->begin();
                                            it2 != l->end();
                                          ++it2)
         {
          // Just to show : GetZSpacing from a GDCM_NAME_SPACE::File may be different        
             std::cout << (*it2)->GetFileName() << " -->  Get{X/Y/Z}Spacing() from GDCM_NAME_SPACE::File : " 
                       << (*it2)->GetXSpacing() << " " 
                       << (*it2)->GetYSpacing() << " " 
                       << (*it2)->GetZSpacing() << std::endl; 
           fileCount++;      
         }
         std::cout << "Iterate trough vector, nb of files : " << fileCount << std::endl;  

         //break; // we only deal with the first one ... Why not ?
      }
      l = s->GetNextSingleSerieUIDFileSet();
   } 
   std::cout << std::endl
             << " ------------------Prints all the Single SerieUID File Sets (sorted or not) -----"
             << std::endl;
   s->Print(); // Prints all the Single SerieUID File Sets (sorted or not)
   std::cout << " -------------------------------------------- Finish printing"
             << std::endl;
     
   s->Delete();

   return 0;
}
