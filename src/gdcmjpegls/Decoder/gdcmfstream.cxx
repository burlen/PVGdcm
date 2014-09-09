/*=========================================================================
  
  Program:   gdcm
  Module:    $RCSfile: gdcmfstream.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:13 $
  Version:   $Revision: 1.2 $
  
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
  
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
  
=========================================================================*/

#include "gdcmfstream.h"
#include <iostream>

namespace GDCM_NAME_SPACE {


ifstream::ifstream(const char* filename,openmode mode)
   :std::ifstream(filename,mode)
{
}

std::istream& ifstream::read(char* s, std::streamsize n )
{
  std::cerr << "my read" << std::endl;
  this->std::ifstream::read(s,n);
}

} //end namespace gdcm

