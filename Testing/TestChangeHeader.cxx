/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestChangeHeader.cxx,v $
  Language:  C++
  Date:      $Date: 2008/09/15 15:49:21 $
  Version:   $Revision: 1.36 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmFileHelper.h"
#include "gdcmFile.h"

// Writting of a DICOM file, using a correct gdcmHeader.
// and pixels of an other image


// NO SYNTAX UPDATED
// Doesn't compile !

int TestChangeHeader(int argc, char *argv[])
{
   if (argc < 3)
   {
      std::cerr << "usage :" << std::endl << 
      argv[0] << " FileForHeader FileForPixels" << std::endl;
      return 1;
   }

   std::string firstArgument = argv[1];
   
   GDCM_NAME_SPACE::File  *f1 = new GDCM_NAME_SPACE::File();
   f1->SetFileName(firstArgument);
   f1->Load( );
   std::string secondArgument = argv[2];
   GDCM_NAME_SPACE::File  *f2 = new GDCM_NAME_SPACE::File(secondArgument);

   //f1->PrintPubElVal();

   // We suppose the DICOM Entries of the second file *does* exist !

   std::string nbFrames = f2->GetHeader()->GetEntryValue(0x0028, 0x0008);
   if(nbFrames != GDCM_UNFOUND)
   {
      f1->GetHeader()->Insert( nbFrames, 0x0028, 0x0008);
   }


// WARNING : If user tries to 'merge' two mismatching images
// e.g. a LUT image and a RGB image
// or a compressed and an uncompressed one
// or a single frame and a multiframe,
// or a '12 Bits Allocated' image and anything else, etc
// Probabely TestChangeHeader will fail !
// It was not designed as a 'Test' program, but as a utility
// provided to 'transform' an image 'Siemens MRI New version' into an image 'Siemens MRI old version'
         
   f1->GetHeader()->Insert(
      f2->GetHeader()->GetEntryValue(0x0028, 0x0010), 0x0028, 0x0010);// nbLig
   f1->GetHeader()->Insert( 
      f2->GetHeader()->GetEntryValue(0x0028, 0x0011), 0x0028, 0x0011);// nbCol
   f1->GetHeader()->Insert( 
      f2->GetHeader()->GetEntryValue(0x0028, 0x0100), 0x0028, 0x0100);// BitsAllocated
   f1->GetHeader()->Insert( 
      f2->GetHeader()->GetEntryValue(0x0028, 0x0101), 0x0028, 0x0101);// BitsStored
   f1->GetHeader()->Insert( 
      f2->GetHeader()->GetEntryValue(0x0028, 0x0102), 0x0028, 0x0102);// HighBit
   f1->GetHeader()->Insert( 
      f2->GetHeader()->GetEntryValue(0x0028, 0x0103), 0x0028, 0x0103);// Pixel Representation
// Probabely some more to update (?)

// TODO : add a default value
// TODO : add a method that receives a list of pairs  (gr,el), 
//                        and that does the work.

   int dataSize = f2->GetImageDataSize();
   printf ("dataSize %d\n",dataSize);
   uint8_t *imageData= f2->GetImageData();

// TODO : Why don't we merge these 2 functions ?

   f1->SetImageData(imageData,dataSize);

   f1->GetHeader()->Print();

   std::string s0 =f2->GetHeader()->GetEntryValue(0x7fe0, 0x0000);
   std::string s10=f2->GetHeader()->GetEntryValue(0x7fe0, 0x0010);
   printf("lgr 7fe0, 0000 %s\n",s0.c_str());
   printf("lgr 7fe0, 0010 %s\n",s10.c_str());

   char resultat[512];

   sprintf(resultat, "%s.vol", deuxieme.c_str());
   printf ("WriteDCM\n");
  //f1->WriteDcmExplVR("itk-gdcm-ex.dcm");
  //f1->WriteDcmImplVR(resultat);
   f1->WriteAcr(resultat);

   return 0;
}
