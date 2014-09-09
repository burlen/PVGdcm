/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: FlatHashTablePrint.cxx,v $
  Language:  C++
  Date:      $Date: 2007/06/21 15:01:00 $
  Version:   $Revision: 1.11 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmFile.h"
#include "gdcmDocEntry.h"

// Iterate on all the Dicom entries encountered in the gdcmFile (given
// as line argument) and print them. This is an illustration of the
// usage of \ref gdcmDocument::BuildFlatHashTable().

int main(int, char*[])
{
/*   if (argc < 2)
   {
      std::cerr << "Usage :" << std::endl << 
      argv[0] << " input_dicom " << std::endl;
      return 1;
   }
   GDCM_NAME_SPACE::File *header = new GDCM_NAME_SPACE::File();
   header->SetFileName(argv[1] );
   header->Load();
 
   GDCM_NAME_SPACE::TagDocEntryHT *Ht = header->BuildFlatHashTable();
   
   for (GDCM_NAME_SPACE::TagDocEntryHT::iterator tag = Ht->begin(); tag != Ht->end(); ++tag)
   {
      tag->second->Print(); 
      std::cout << std::endl;
   }*/
   std::cout<<"Usefull example ???\n";

   return 0;
}
