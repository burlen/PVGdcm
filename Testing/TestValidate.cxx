/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestValidate.cxx,v $
  Language:  C++
  Date:      $Date: 2007/06/21 14:59:06 $
  Version:   $Revision: 1.11 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmFile.h"
#include "gdcmValidator.h"

#include "gdcmGlobal.h"
#include "gdcmDictSet.h"

//Generated file:
#include "gdcmDataImages.h"

int Validate(std::string const &filename);

int Validate(std::string const &filename)
{
   GDCM_NAME_SPACE::File *input =  GDCM_NAME_SPACE::File::New( );
   input->SetFileName(filename);
   input->Load();
   GDCM_NAME_SPACE::Validator *v = GDCM_NAME_SPACE::Validator::New();
   v->SetInput( input );
   input->Delete();
   v->Delete();
   return 1; // allways true (we don't want to break the test suite)
} 


int TestValidate(int argc, char *argv[])
{
   if ( argc == 2 )
   {
      // The test is specified a specific filename, use it instead of looping
      // over all images
      const std::string input = argv[1];
      return Validate( input );
   }
   else if ( argc > 2 || argc == 2 )
   {
      std::cout << "   Usage: " << argv[0]
                << " (no arguments needed)." << std::endl;
      std::cout << "or   Usage: " << argv[0]
                << " filename.dcm " << std::endl;
      return 1;
   }
   
   int i =0;
   int retVal = 0;  //by default : *no* error
   while( gdcmDataImages[i] != 0 )
   {
      std::string filename = GDCM_DATA_ROOT;
      filename += "/";  //doh!
      filename += gdcmDataImages[i];
      std::cout << filename << std::endl;
      if( Validate( filename ) != 0 )
      {
         retVal++;
      }

      i++;
   }
   retVal = 0; // Never break test suite
   return retVal;
}

