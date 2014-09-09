/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: WriteRead.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:05 $
  Version:   $Revision: 1.16 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmFile.h"
#include "gdcmFileHelper.h"

#include <iostream>

int main(int argc, char *argv[])
{  
   std::string fileNameToWrite;

   GDCM_NAME_SPACE::File *e1;
   GDCM_NAME_SPACE::File *e2;
   GDCM_NAME_SPACE::FileHelper *f1;
   GDCM_NAME_SPACE::FileHelper *f2;
   uint8_t* imageData, *imageData2;
   int dataSize, dataSize2;
     
   if( argc < 2 )
   {
      std::cerr << "Usage " << argv[0] << " image.dcm" << std::endl;
      return 1;
   }

   std::string fileName = argv[1];

// --------------------- we read the input image

   std::cout << argv[1] << std::endl;
   e1 = GDCM_NAME_SPACE::File::New( );
   e1->SetFileName( fileName );
   e1->Load();
   if (!e1->IsReadable()) 
   {
      std::cerr << "Sorry, " << fileName <<"  not a Readable DICOM / ACR File"
                <<std::endl;
      e1->Delete();
      return 1;
   }
   
   f1 = GDCM_NAME_SPACE::FileHelper::New(e1);
   imageData= f1->GetImageData();
   dataSize = f1->GetImageDataSize();

// --------------------- we write it as an Explicit VR DICOM file

   fileNameToWrite = "temp.XDCM";
   std::cout << "WriteDCM Explicit VR" << std::endl;
   f1->WriteDcmExplVR(fileNameToWrite);

// --------------------- we read the written image
   e2 = GDCM_NAME_SPACE::File::New( );
   e2->SetFileName( fileNameToWrite );
   e2->Load();
   if (!e2->IsReadable()) 
   {
      std::cerr << "Sorry, " << fileNameToWrite << " not a Readable DICOM / ACR File"  
               <<std::endl;
      e1->Delete();
      e2->Delete();
      f1->Delete();
      return 1;
   }
   f2 = GDCM_NAME_SPACE::FileHelper::New(e2);
   imageData2= f2->GetImageData();
   dataSize2 = f2->GetImageDataSize();

// --------------------- we compare the pixel areas

   if (dataSize != dataSize2) 
   {
      std::cout << " ----------------------------------------- " 
                << "Bad shot! Lengthes are different : " 
                << dataSize << " # " << dataSize2
                << " for file : " << fileName << std::endl;

      e1->Delete();
      e2->Delete();
      f1->Delete();
      f2->Delete();
      return 1;
   }
   if (int res=memcmp(imageData,imageData2,dataSize) !=0) 
   {
      std::cout << " ----------------------------------------- " 
                << "Bad shot! Pixels are different : " 
                << " for file : " << fileName << std::endl;
      std::cout << "memcmp(imageData,imageData2,dataSize) = " << res << std::endl;
      e1->Delete();
      e2->Delete();
      f1->Delete();
      f2->Delete();
      return 1;
   }
  
   e1->Delete();
   e2->Delete();
   f1->Delete();
   f2->Delete();
   return 0;
}

