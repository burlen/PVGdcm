/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestDicomString.cxx,v $
  Language:  C++
  Date:      $Date: 2005/02/02 10:05:26 $
  Version:   $Revision: 1.5 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmUtil.h"
#include <assert.h>

int TestDicomString(int, char *[])
{
  int i;
  const char *s = "\0\0";
  std::string a(s,s+2); // will copy 2 '\0' 
  assert( a.size() == 2 );
  for(i=0;i<2;i++)
  {
    assert( a.c_str()[i] == '\0' );
    assert( a.data()[i] == '\0' );
  }
  assert( a.c_str()[2] == '\0' );

/*
std::string zeros(x, 0);
char s1[] = "\0";
char s2[] = "\0\0";
char s3[] = "\0\0\0";
char s4[] = "\0abc";
*/

  return 0;
}
