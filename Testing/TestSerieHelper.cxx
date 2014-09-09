/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestSerieHelper.cxx,v $
  Language:  C++
  Date:      $Date: 2008/09/15 15:49:21 $
  Version:   $Revision: 1.13 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmSerieHelper.h"
#include "gdcmFile.h"
#include "gdcmDebug.h"
#include <iostream>

int TestSerieHelper(int argc, char *argv[])
{  
   GDCM_NAME_SPACE::SerieHelper *s;
  
   std::string dirName; 
   if (argc > 1) 
      dirName = argv[1];    
   else 
   {
      dirName = GDCM_DATA_ROOT;
   }

   //if (argc > 2)
   //   GDCM_NAME_SPACE::Debug::DebugOn();

  
   std::cout << "-------------- Dir Name :[" << dirName << "]" << std::endl;

   s = GDCM_NAME_SPACE::SerieHelper::New();
   s->SetLoadMode(GDCM_NAME_SPACE::LD_ALL);     // Load everything for each File
   //s->AddRestriction(tagKey, valueToCheck); // Keep only files where
                                              // restriction is true
   s->SetDirectory(dirName, true); // true : recursive exploration
   std::cout << " ---------------------------------------- Finish parsing :["
             << dirName << "]" << std::endl;

   s->Print();
   std::cout << " ---------------------------------------- Finish printing (1)"
             << std::endl;

   int nbFiles;
   // For all the SingleSerieUID filesets of the GDCM_NAME_SPACE::Serie
   GDCM_NAME_SPACE::FileList *l = s->GetFirstSingleSerieUIDFileSet();
   while (l)
   { 
      nbFiles = l->size() ;
      if ( l->size() > 3 ) // Why not ? Just an example, for testing
      {
         std::cout << "Sort list : " << nbFiles << " long" << std::endl;
         s->OrderFileList(l);  // sort the list
      }
      l = s->GetNextSingleSerieUIDFileSet();
   } 
   std::cout << " -------------------------------------------- Finish sorting"
             << std::endl;
   s->Print(); // Prints all the SingleSerieUID filesets (sorted or not)
   std::cout << " -------------------------------------------- Finish printing"
             << std::endl;


   // Only for the first SingleSerieUID fileset 
   // ( Why not ? Just an example, for testing )
   // Display all the file names

   std::string fileName; 
   l = s->GetFirstSingleSerieUIDFileSet();
   for (std::vector<GDCM_NAME_SPACE::File* >::iterator it =  l->begin();
                                            it != l->end();
                                          ++it)
   {
      fileName = (*it)->GetFileName();
      std::cout << fileName << std::endl;
      // Remark : vtkGdcmReader users can use this loop to AddFileName(fileName)
      //          in the right order 
   } 
     

   s->Delete();

   return 0;
}
