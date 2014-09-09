/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestValidate.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:04 $
  Version:   $Revision: 1.4 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmFile.h"
#include "gdcmValidator.h"


int main(int argc, char *argv[])
{
  if( argc < 2 )
  {
    std::cerr << "ouh les cornes / shame on you / !inocente!" << std::endl;
    return 1;
  }

   const char *filename = argv[1];

   GDCM_NAME_SPACE::File *input =  GDCM_NAME_SPACE::File::New( );
   input->SetFileName(filename);
   input->Load();
   GDCM_NAME_SPACE::Validator *v = GDCM_NAME_SPACE::Validator::New();
   v->SetInput( input );

   return 0;
}

