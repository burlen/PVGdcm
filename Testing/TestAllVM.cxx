/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestAllVM.cxx,v $
  Language:  C++
  Date:      $Date: 2007/06/21 14:59:06 $
  Version:   $Revision: 1.15 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#include "gdcmFile.h"
#include "gdcmDataEntry.h"

//Generated file:
#include "gdcmDataImages.h"

int DoTheVMTest(std::string const &filename)
{
   GDCM_NAME_SPACE::File *file = GDCM_NAME_SPACE::File::New();
   // - Do not test unknow VM in shadow groups (if element 0x0000 is present)
   // - Skip Sequences (if they are 'True Length'); loading will be quicker
   //                  (anyway, Sequences are skipped at processing time ...)
   file->SetLoadMode( GDCM_NAME_SPACE::LD_NOSHADOW | GDCM_NAME_SPACE::LD_NOSEQ );

   file->SetFileName( filename );
   if( !file->Load() ) //would be really bad...
      return 1;

   GDCM_NAME_SPACE::DocEntry *d = file->GetFirstEntry();
   std::cerr << "Testing file : " << filename << std::endl;
   GDCM_NAME_SPACE::DataEntry *de;
   while(d)
   {
      if ( (de = dynamic_cast<GDCM_NAME_SPACE::DataEntry *>(d)) )
      {
         if ( !(de->GetGroup() % 2) ) // Don't check shadow elements. Righ now,
                                      // Private Dictionnary are not dealt with
         {    
            // We know OB and OW VM is always 1, whatever the actual
            //  found value is.
     
            if (de->GetVR() != "OB" && de->GetVR() != "OW" )
               if( !de->IsValueCountValid() )
               {
                  std::cerr << "Element: " << de->GetKey() <<
                    " (" << de->GetName() << ") " <<
                    "Contains a wrong VM: " << de->GetValueCount() 
                    << " should be: " << de->GetVM() << std::endl;;
               }
         }
      }
      else
      {
          // We skip pb of SQ recursive exploration
      }
      d = file->GetNextEntry();
   }
   file->Delete();

   return 0;
}

int TestAllVM(int argc, char *argv[])
{
   int i = 0;
   if( argc >= 2 )
     {
     const char *filename = argv[1];
     if( DoTheVMTest( filename ) )
       return 1;
     return 0;
     }
   // else

   while( gdcmDataImages[i] != 0 )
   {
      std::string filename = GDCM_DATA_ROOT;
      filename += "/";
      filename += gdcmDataImages[i];
      
      if (!strcmp(gdcmDataImages[i],"00191113.dcm")) // Track bug on Darwin
          GDCM_NAME_SPACE::Debug::DebugOn();
      else
         GDCM_NAME_SPACE::Debug::DebugOff();

      if( DoTheVMTest( filename ) )
        return 1;
      i++;
      std::cerr << std::endl; // skip a line after each file
   }

   return 0;
}

