/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestDictGroupName.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:06 $
  Version:   $Revision: 1.5 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmDictGroupName.h"
#include "gdcmGlobal.h"
#include "gdcmCommon.h"

#include <iomanip>

int CompareDictGroupName(GDCM_NAME_SPACE::DictGroupName *groupName,
                         uint16_t group,std::string ref)
{
   std::string val = groupName->GetName(group);
   std::cout << "Group : 0x" << std::hex << std::setw(4) << group 
             << std::dec << " : " << val << " - "
             << (bool)(val==ref) << std::endl;

   return val!=ref;
}

int TestDictGroupName(int , char *[])
{
   GDCM_NAME_SPACE::DictGroupName *groupName = GDCM_NAME_SPACE::DictGroupName::New();
   groupName->Print( std::cout );

   int ret = 0;

   std::cout << std::endl;
   ret += CompareDictGroupName(groupName,0x0002,"Meta Elements");
   ret += CompareDictGroupName(groupName,0x7fe0,"Pixels");
   ret += CompareDictGroupName(groupName,0x0007,GDCM_NAME_SPACE::GDCM_UNFOUND);

   groupName->Delete();

   return ret;
}
