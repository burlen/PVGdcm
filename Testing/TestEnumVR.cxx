/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestEnumVR.cxx,v $
  Language:  C++
  Date:      $Date: 2005/11/10 18:02:26 $
  Version:   $Revision: 1.2 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
/*
 * Proof of concept for using enum instead of string for representing VR
 * This allows us to:
 * 1. Qickly check against another VR (faster than string comparison)
 * 2. Support the advance VR like 'OB or OW'
 */

#include <iostream>
typedef enum {
  AE = 1,
  AS = 2,
  AT = 4,
  CS = 8,
  DA = 16,
  DS = 32,
  DT = 64,
  FL = 128,
  FD = 256,
  IS = 512,
  LO = 1024,
  LT = 2048,
  OB = 4096,
  OW = 8192,
  PN = 16384,
  SH = 32768,
  SL = 65536,
  SQ = 131072,
  SS = 262144,
  ST = 524288,
  TM = 1048576,
  UI = 2097152,
  UL = 4194304,
  UN = 8388608,
  US = 16777216,
  UT = 33554432,
  OB_OW = OB | OW,
  US_SS = US | SS,
  US_SS_OW = US | SS | OW
} VR;

static const char *VRStrings[] = {
  "AE",
  "AS",
  "AT",
  "CS",
  "DA",
  "DS",
  "DT",
  "FL",
  "FD",
  "IS",
  "LO",
  "LT",
  "OB",
  "OW",
  "PN",
  "SH",
  "SL",
  "SQ",
  "SS",
  "ST",
  "TM",
  "UI",
  "UL",
  "UN",
  "US",
  "UT",
  "OB or OW",
  "US or SS",
  "US or SS or OW"
};


int get_index_vr(VR vr)
{
  int l;
  switch(vr)
    {
  case OB_OW:
    l =  26;
    break;
  case US_SS:
    l =  27;
    break;
  case US_SS_OW:
    l =  28;
    break;
  default:
      {
      int a = (int)vr;
      for (l = 0; a > 1; ++l)
        a >>= 1;
      }
    }
  return l;
}

int TestEnumVR(int , char *[])
{
  for(int i=0; i<26; i++)
    {
    int j = 1 << i;
    int k = get_index_vr((VR)j);
    std::cout << k << "," << VRStrings[k] << std::endl;
    }

   VR vr = OB_OW;
   int k = get_index_vr(vr);
   std::cout << k << "," << VRStrings[k] << std::endl;
   vr = US_SS;
   k = get_index_vr(vr);
   std::cout << k << "," << VRStrings[k] << std::endl;
   vr = US_SS_OW;
   k = get_index_vr(vr);
   std::cout << k << "," << VRStrings[k] << std::endl;

   return 0;
}
