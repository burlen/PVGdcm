/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestBug.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:06 $
  Version:   $Revision: 1.24 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
// The following crashes on Win32
// We have to be carefull when the code both use cout + printf VC++ gets
// confused, thus make sure we use only one kind of iostream

#include "gdcmFile.h"

int TestBug(int argc, char *argv[])
{  
std::cout << "==========test" <<std::endl;
  GDCM_NAME_SPACE::File *f;
  f = GDCM_NAME_SPACE::File::New();
  
   if (argc > 1)
      f->SetFileName( argv[1] );
   else {
      std::string filename = GDCM_DATA_ROOT;
      filename += "/test.acr";
      f->SetFileName( filename.c_str() );
   }
   f->Load( );
  // f->Print();
   std::cout << "-------------End of Print file ------------" << std::cout;   
   //f->GetPubDict()->Print();
   std::cout << "-------------End of Print Dict ------------" << std::cout;   
   f->Delete();

   return 0;
}
