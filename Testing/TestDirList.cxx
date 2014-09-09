/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestDirList.cxx,v $
  Language:  C++
  Date:      $Date: 2007/06/21 14:59:06 $
  Version:   $Revision: 1.4 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmDirList.h"

int TestDirList(int , char *[])
{  
   std::string path = GDCM_DATA_ROOT;
   std::cerr << path << std::endl;

   GDCM_NAME_SPACE::DirList list(path, true);
   list.Print();

   return 0;
}
