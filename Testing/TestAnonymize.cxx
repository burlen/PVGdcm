/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestAnonymize.cxx,v $
  Language:  C++
  Date:      $Date: 2008/09/15 15:49:21 $
  Version:   $Revision: 1.4 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmGlobal.h"
#include "gdcmDebug.h"

//Generated file:
#include "gdcmDataImages.h"

#ifndef _WIN32
#include <unistd.h> //for access, unlink
#else
#include <io.h> //for _access on Win32
#endif


bool FileExists(const char *filename);
bool RemoveFile(const char *source);

// ---------------------------------------------------------

int Anonymize(std::string const &filename, 
              std::string const &output )
{
      std::cout << "   Testing: " << filename << std::endl;

      if( FileExists( output.c_str() ) )
      {
         if( !RemoveFile( output.c_str() ) )
         {
            std::cout << "Ouch, the file exist, but I cannot remove it" 
                      << std::endl;
            return 1;
         }
      }

      //////////////// Step 1:
      std::cout << "      1...";
      std::cout << std::endl;
      
      GDCM_NAME_SPACE::File *f;
      f = new GDCM_NAME_SPACE::File( );
      f->SetFileName( filename );
      f->Load();
      
      std::cout << " ... Read !" << std::endl;   
      // ============================================================
      //   Load the pixels in memory.
      //   Write a new file
      // ============================================================

      // We need a GDCM_NAME_SPACE::FileHelper, since we want to load the pixels        
      GDCM_NAME_SPACE::FileHelper *fh = new GDCM_NAME_SPACE::FileHelper(f);

      // --- Don't forget to load the Pixels ...
      // We shall not use them, but we have to load them
  
      fh->GetImageData();

      std::cout << " Image Data... Got ! " << std::endl;

      // Institution name 
      f->AddAnonymizeElement( 0x0008, 0x0080, "Xanadoo" ); 
      // Patient's name 
      f->AddAnonymizeElement( 0x0010, 0x0010, "Fantomas" );   
      // Patient's ID
      f->AddAnonymizeElement( 0x0010, 0x0020,"1515" );   
      // Study Instance UID
      f->AddAnonymizeElement( 0x0020, 0x000d, "9.99.999.9999" );
      // Telephone
      f->AddAnonymizeElement( 0x0010, 0x2154, "3615" );

      std::cout << " Anonymize list... Done ! " << std::endl;

      f->AnonymizeFile();

      std::cout << " Anonymize File... Done ! " << std::endl;
 
      fh->WriteDcmExplVR(output);  
      std::cout << " Anonymized File... Written ! " << std::endl;

      f->ClearAnonymizeList();
      std::cout << " Anonymize list... Cleared ! " << std::endl;
    
      delete f;
      delete fh;

      // Read the file we just wrote
      f = new GDCM_NAME_SPACE::File( output );

      std::cout << " Anonymized File... Re-Read ! " << std::endl;

      std::string v;
      bool plouf = false;

      // Compare and abort if different.
      v = f->GetEntryValue(0x0008, 0x0080);
      if ( v != GDCM_NAME_SPACE::GDCM_UNFOUND ) 
         if (v.find("Xanadoo") >= v.length() )
            plouf = true;

      v = f->GetEntryValue(0x0010, 0x0010);
      if ( v != GDCM_NAME_SPACE::GDCM_UNFOUND ) 
         if (v.find("Fantomas") >= v.length() )
            plouf = true;

      if ( v != GDCM_NAME_SPACE::GDCM_UNFOUND )
         v = f->GetEntryValue(0x0010, 0x0020);
         if (v.find("1515") >= v.length() )
            plouf = true;

       if ( v != GDCM_NAME_SPACE::GDCM_UNFOUND )
         v = f->GetEntryValue(0x0010, 0x000d);
         if (v.find("9.99.999.9999") >= v.length() )
            plouf = true;
      
      delete f;

      if ( !plouf)
      { 
         return 1;
      } 

 
   // ============================================================
   //   Don't load the pixels in memory.
   //   Overwrite the file
   // ============================================================

      // Read the file we just anonymize and check

      f = new GDCM_NAME_SPACE::File( output );

      // First, we set values to replace the ones we want to hide
   
      // Patient's name 
      f->AddAnonymizeElement(0x0010, 0x0010, "XXL");  
      std::cout << " replace Patient's Name " << std::endl;
      // Patient's ID
      f->AddAnonymizeElement( 0x0010, 0x0020,"007" );
      std::cout << " replace Patient's ID " << std::endl;
      // Study Instance UID
      f->AddAnonymizeElement(0x0020, 0x000d, "6.66.666.6666" );
      std::cout << " replace Study ID " << std::endl;

      // --------------------- we overwrite the file

      // No need to load the pixels.
      // The GDCM_NAME_SPACE::File remains untouched in memory

      std::cout <<"Let's AnonymizeNoLoad " << std::endl;;
      f->AnonymizeNoLoad();

      std::cout <<"End AnonymizeNoLoad" << std::endl;
      // No need to write the File : modif were done on disc !
 
      delete f;
      f = new GDCM_NAME_SPACE::File( output );

      std::string val;
      plouf = false;

      val = f->GetEntryValue(0x0010, 0x0010);
      if ( val != GDCM_NAME_SPACE::GDCM_UNFOUND ) 
         if (val.find("XXL") >= v.length() )
            plouf = true;
     
      if ( val != GDCM_NAME_SPACE::GDCM_UNFOUND )
         val = f->GetEntryValue(0x0010, 0x0020);
         if (val.find("007") >= v.length() )
      plouf = true;

      if ( val != GDCM_NAME_SPACE::GDCM_UNFOUND )
         val = f->GetEntryValue(0x0010, 0x000d);
         if (val.find("6.66.666.6666") >= v.length() )
      plouf = true;
      
      delete f;

      if ( !plouf)
      { 
         return 1;
      }                       

      return 0; // no fail
}

// Here we load a gdcmFile we anonymize it with and without loading the pixels,

int TestAnonymize(int argc, char *argv[])
{
   //GDCM_NAME_SPACE::Debug::DebugOn();

   if ( argc == 3 )
   {
      // The test is specified a specific filename, use it instead of looping
      // over all images
      const std::string input = argv[1];
      const std::string reference = argv[2];
      return Anonymize( input, reference );
   }
   else if ( argc > 3 || argc == 2 )
   {
      std::cout << "   Usage: " << argv[0]
                << " (no arguments needed)." << std::endl;
      std::cout << "or   Usage: " << argv[0]
                << " filename.dcm reference.dcm" << std::endl;
      return 1;
   }
   // else other cases:

   std::cout << "   Description (Test::TestAnonymize): "
             << std::endl;
   std::cout << "   For all images in gdcmData (and not blacklisted in "
                "Test/CMakeLists.txt)"
             << std::endl;
   std::cout << "   apply the following to each filename.xxx: "
             << std::endl;
   std::cout << "   - Create a new file " 
             << std::endl;
   std::cout << "   - Anonymize the file (creates a new file) " 
             << std::endl;
   std::cout << "   - Read this new file " 
             << std::endl;
   std::cout << "   - Check if the written values are ok" 
             << std::endl;
   std::cout << "   - AnonymizeNoLoad the new file (it s overwritten)" 
             << std::endl;
   std::cout << "   - Check if the written values are ok" 
             << std::endl;
   std::cout << std::endl;

   int i =0;
   int retVal = 0;  //by default this is an error
   while( gdcmDataImages[i] != 0 )
   {
      std::string filename = GDCM_DATA_ROOT;
      filename += "/";  //doh!
      filename += gdcmDataImages[i];

      std::string output = "output.dcm";

      if( Anonymize( filename, output ) != 0 )
      {
         retVal++;
      }
      i++;
   }
   return retVal;
}
