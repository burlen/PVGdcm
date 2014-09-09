/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestWriteSimple.cxx,v $
  Language:  C++
  Date:      $Date: 2007/09/28 14:20:22 $
  Version:   $Revision: 1.53 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

/**
 * Write a dicom file from nothing
 * The written image is 256x256, 8 bits, unsigned char
 * The image content is a horizontal grayscale from 
 * 
 */
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmDebug.h"
#include "gdcmGlobal.h"
#include "gdcmDictSet.h"

#include <iostream>
#include <sstream>

typedef struct
{
   int sizeX;         // Size X of the image
   int sizeY;         // Size Y of the image
   int sizeZ;         // Size Z of the image
   int components;    // Number of components for a pixel
   int componentSize; // Component size (in bits : 8, 16) // Bits Allocated
   int componentUse ; // Component size (in bits)         // Bits Stored
   int sign;          // Sign of components
   char writeMode;    // Write mode
                      //  - 'a' : ACR
                      //  - 'e' : Explicit VR
                      //  - 'i' : Implicit VR
} Image;

Image Images [] = {
// these ones are use to check further oddities.

   {63, 127, 1, 1, 16,  8,  0 ,'e'},
   {63, 127, 1, 1, 16,  9,  0 ,'e'},
   {63, 127, 1, 1, 16, 10,  0 ,'e'},
   {63, 127, 1, 1, 16, 11,  0 ,'e'},
   {63, 127, 1, 1, 16, 12,  0 ,'e'},
   {63, 127, 1, 1, 16, 13,  0 ,'e'},
   {63, 127, 1, 1, 16, 14,  0 ,'e'},
   {63, 127, 1, 1, 16, 15,  0 ,'e'}, 
   {63, 127, 1, 1, 16, 16,  0 ,'e'},
   
   {63, 127, 1, 1, 32, 32,  0 ,'e'},   // Pixel VR should be OL?
      
   {128, 128, 1, 1, 8,  8,  0, 'e'},
   {256, 128, 1, 1, 8,  8,  0, 'a'},
   {128, 128, 1, 1, 8,  8,  0, 'i'},

   {128, 128, 1, 1, 8,  8,  0, 'a'},    
   {256, 128, 1, 1, 8,  8,  0, 'i'},
     

   {256, 128, 1, 1, 8,  8,  0, 'e'},
   {128, 128, 1, 1, 16, 16, 0, 'e'},      
   {128, 256, 1, 1, 16, 16, 0, 'e'},   
   
   {128, 256, 1, 1, 8,  8,  0, 'e'},
   {128, 256, 1, 1, 8,  8,  0, 'i'},
   {128, 256, 1, 1, 8,  8,  0, 'a'},
   
   {128, 256, 1, 1, 16, 16, 0, 'i'},
   {128, 256, 1, 1, 16, 16, 0, 'i'},
   {128, 256, 1, 1, 16, 16, 0, 'a'},
   {128, 256, 1, 1, 16, 16, 0, 'a'},

   {256, 128, 10, 1, 8, 8,  0, 'e'},
   {256, 128, 10, 3, 8, 8,  0, 'e'},
   {256, 128, 10, 3, 8, 8,  0, 'i'},
   {256, 128, 10, 1, 8, 8,  0, 'i'},
   {256, 128, 10, 1, 8, 8,  0, 'a'},
   {256, 128, 10, 3, 8, 8,  0, 'a'},
      
   {128, 128, 1, 1, 8,  8,  1, 'e'},
   {128, 128, 1, 1, 8,  8,  1, 'i'},
   {128, 128, 1, 1, 8,  8,  1, 'a'},
   
   {256, 128, 1, 1, 8,  8,  1, 'e'},
   {256, 128, 1, 1, 8,  8,  1, 'i'},
   {256, 128, 1, 1, 8,  8,  1, 'a'},
   
   {128, 256, 1, 1, 8,  8,  1, 'a'},
   {128, 256, 1, 1, 8,  8,  1, 'e'},
   {128, 256, 1, 1, 8,  8,  1, 'i'},

   {128, 256, 1, 1, 16, 16, 1, 'e'},
   {128, 256, 1, 1, 16, 16, 1, 'e'},
   {128, 256, 1, 1, 16, 16, 1, 'i'},
   {128, 256, 1, 1, 16, 16, 1, 'i'},
   {128, 256, 1, 1, 16, 16, 1, 'a'},
   {128, 256, 1, 1, 16, 16, 1, 'a'},     

   {256, 128, 10, 1, 8, 8,  1, 'e'},
   {256, 128, 10, 1, 8, 8,  1, 'i'},
   {256, 128, 10, 1, 8, 8,  1, 'a'},   
   
   {256, 128, 10, 3, 8, 8,  1, 'e'},
   {256, 128, 10, 3, 8, 8,  1, 'i'},
   {256, 128, 10, 3, 8, 8,  1, 'a'},
   {0,   0,   1,  1, 8, 8,  0, 'i'} // to find the end
};


const unsigned int MAX_NUMBER_OF_DIFFERENCE = 10;

int WriteSimple(Image &img)
{
   
   std::cout << "======================= WriteSimple =========(begin of processing current image)" << std::endl;
   std::ostringstream fileName;
   fileName.str("");
   fileName << "TestWriteSimple";

// Step 1 : Create an empty FileHelper

   std::cout << "        1...";
   GDCM_NAME_SPACE::FileHelper *fileH = GDCM_NAME_SPACE::FileHelper::New();
 
 //  Get the (empty) image header.  
   GDCM_NAME_SPACE::File *fileToBuild = fileH->GetFile();
   std::ostringstream str;

   // Set the image size
   str.str("");
   str << img.sizeX;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0011,"US"); // Columns
   str.str("");
   str << img.sizeY;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0010,"US"); // Rows

   if(img.sizeZ>1)
   {
      str.str("");
      str << img.sizeZ;
      fileToBuild->InsertEntryString(str.str(),0x0028,0x0008, "IS"); // Number of Frames
   }

   fileName << "-" << img.sizeX << "-" << img.sizeY << "-" << img.sizeZ;

   // Set the pixel type
   str.str("");
   str << img.componentSize;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0100,"US"); // Bits Allocated

   str.str("");
   str << img.componentUse;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0101,"US"); // Bits Stored

   str.str("");
   str << ( img.componentUse - 1 );
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0102,"US"); // High Bit

   // Set the pixel representation
   str.str("");
   str << img.sign;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0103,"US"); // Pixel Representation

   fileName << "-" << img.componentSize;
   if(img.sign == 0)
      fileName << "U";
   else
      fileName << "S";
      
   fileName << "-" << img.componentSize << "-" << img.componentUse;
 
   fileToBuild->InsertEntryString("0",0x0008,0x0000,"UL"); // Should be removed
                                                            // except for ACR 
   switch (img.writeMode)
   {
      case 'a' :
         fileName << ".ACR";
         break; 
      case 'e' :
         fileName << ".EXPL"; 
         break; 
      case 'i' :
         fileName << ".IMPL"; 
         break;
   } 

   if(img.componentSize == 32)
   {
      // Create a Private DataElement; VR =OL
      fileToBuild->InsertEntryString("gdcm test OL",0x0009,0x0010,"LO");
      uint32_t binArea[4];
      binArea[0] = 0x01234567;
      binArea[1] = 0x89abcdef;      
      binArea[2] = 0x2468ace0;
      binArea[2] = 0xfdb97531;
      fileToBuild->InsertEntryBinArea((uint8_t *)binArea,16,0x0009,0x0100,"OL");              
   }
   
   
   std::cout << "[" << fileName.str() << "]...";
   // Set the samples per pixel
   str.str("");
   str << img.components;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0002,"US"); // Samples per Pixel

// Step 2 : Create the output image
   std::cout << "2...";
   if( img.componentSize%8 > 0 )
   {
      img.componentSize += 8-img.componentSize%8;
   }
   size_t size = img.sizeX * img.sizeY * img.sizeZ 
               * img.components * img.componentSize / 8;
   unsigned char *imageData = new unsigned char[size];

   // FIXME : find a better heuristic to create the image
   unsigned char *tmp = imageData;
   for(int k=0;k<img.sizeZ;k++)
   {
      for(int j=0;j<img.sizeY;j++)
      {
         for(int i=0;i<img.sizeX;i++)
         {
            for(int c=0;c<img.components;c++)
            {
               *tmp = (unsigned char)(j%256);
               if( img.componentSize>8 )
               {
                  *(tmp+1) = (unsigned char)(j/256);
               }
               tmp += img.componentSize/8;
            }
         }
      }
   }

// Step 3 : Set the image Pixel Data
   std::cout << "3...";
   fileH->SetImageData(imageData,size);

// Step 4 : Set the writting mode and write the image

   fileH->SetWriteModeToRaw();
   std::cout << "4'...";   
   switch (img.writeMode)
   {
      case 'a' : // Write an ACR file
         fileH->SetWriteTypeToAcr();
         break;

      case 'e' : // Write a DICOM Explicit VR file
         fileH->SetWriteTypeToDcmExplVR();
         break;

      case 'i' : // Write a DICOM Implicit VR file
         fileH->SetWriteTypeToDcmImplVR();
         break;

      default :
         std::cout << "Failed for [" << fileName.str() << "]\n"
                   << "        Write mode '"<<img.writeMode<<"' is undefined\n";

         fileH->Delete();
         delete[] imageData;
         return 1;
   }

   std::cout << std::endl;
   //fileToBuild->Print();

   if( !fileH->Write(fileName.str()) )
   {
      std::cout << "Failed for [" << fileName.str() << "]\n"
                << "           File is unwrittable" << std::endl;

      fileH->Delete();

      delete[] imageData;
      return 1;
   }

// Step 5 : Read the written image
   std::cout << "5..." << std::endl;
   // old form.
   //GDCM_NAME_SPACE::FileHelper *reread = new GDCM_NAME_SPACE::FileHelper( fileName.str() );
   // Better use :
   GDCM_NAME_SPACE::File *f = GDCM_NAME_SPACE::File::New( );
   f->SetLoadMode(GDCM_NAME_SPACE::LD_ALL);
   f->SetFileName( fileName.str() );
   
   //reread->SetFileName( fileName.str() );
   //reread->SetLoadMode(GDCM_NAME_SPACE::LD_ALL); // Load everything
                           // Possible values are 
                           //              GDCM_NAME_SPACE::LD_ALL, 
                           //              GDCM_NAME_SPACE::LD_NOSEQ, 
                           //              GDCM_NAME_SPACE::LD_NOSHADOW,
                           //              GDCM_NAME_SPACE::LD_NOSEQ|GDCM_NAME_SPACE::LD_NOSHADOW, 
                           //              GDCM_NAME_SPACE::LD_NOSHADOWSEQ
   
   f->Load();
  // reread->Load();
   GDCM_NAME_SPACE::FileHelper *reread = GDCM_NAME_SPACE::FileHelper::New( f );  

   //reread->Print();

   if( !reread->GetFile()->IsReadable() )
   {
      std::cerr << "Failed" << std::endl
                << "Could not read written image : " << fileName.str() << std::endl;
      fileToBuild->Delete();
      fileH->Delete();
      reread->Delete();
      delete[] imageData;
      return 1;
   }

// Step 6 : Compare to the written image
   std::cout << "6..." << std::endl;
   size_t dataSizeWritten = reread->GetImageDataSize();
   uint8_t *imageDataWritten = reread->GetImageData();

   // Test the image write mode
   if (reread->GetFile()->GetFileType() != fileH->GetWriteType())
   {
      std::cout << "Failed" << std::endl
         << "        File type differ: "
         << fileH->GetWriteType() << " # " 
         << reread->GetFile()->GetFileType() << std::endl;
      fileToBuild->Delete();
      fileH->Delete();
      reread->Delete();
      delete[] imageData;

      return 1;
   }
   std::cout << "6.1..." << std::endl;
   // Test the image size
   if (fileToBuild->GetXSize() != reread->GetFile()->GetXSize() ||
       fileToBuild->GetYSize() != reread->GetFile()->GetYSize() ||
       fileToBuild->GetZSize() != reread->GetFile()->GetZSize())
   {
      std::cout << "Failed for [" << fileName.str() << "]" << std::endl
         << "        X Size differs: "
         << "X: " << fileToBuild->GetXSize() << " # " 
                  << reread->GetFile()->GetXSize() << " | "
         << "Y: " << fileToBuild->GetYSize() << " # " 
                  << reread->GetFile()->GetYSize() << " | "
         << "Z: " << fileToBuild->GetZSize() << " # " 
                  << reread->GetFile()->GetZSize() << std::endl;
      fileToBuild->Delete();
      fileH->Delete();
      reread->Delete();
      delete[] imageData;

      return 1;
   }
 std::cout << "6.2..." << std::endl;
   // Test the data size
   if (size != dataSizeWritten)
   {
      std::cout << "Failed" << std::endl
         << "        Pixel areas lengths differ: "
         << size << " # " << dataSizeWritten << std::endl;
      fileToBuild->Delete();
      fileH->Delete();
      reread->Delete();
      delete[] imageData;

      return 1;
   }
std::cout << "6.3..." << std::endl;
   // Test the data content
   if ( memcmp(imageData, imageDataWritten, size) !=0 )
   {
      std::cout << fileName.str() << " Failed " << std::endl
                << "        Pixel differ (as expanded in memory)." << std::endl;
      std::cout << "        list of the first " << MAX_NUMBER_OF_DIFFERENCE
                  << " pixels differing (pos : test - ref) :" 
                  << std::endl;
      unsigned int i;
      unsigned int j;
      for(i=0, j=0;i<dataSizeWritten && j<MAX_NUMBER_OF_DIFFERENCE;i++)
      {
         if(imageDataWritten[i]!=imageData[i])
            {
            std::cout << std::hex << "(" << i << " : " 
                        << std::hex << (int)(imageDataWritten[i]) << " - "
                        << std::hex << (int)(imageData[i]) << ") "
                        << std::dec;
            ++j;
            }
      }
      std::cout << std::endl;
      fileToBuild->Delete();
      fileH->Delete();
      reread->Delete();
      delete[] imageData;

      return 1;
   }
std::cout << "======================= OK" << std::endl;
   fileH->Delete();
   reread->Delete();
   delete[] imageData;
   return 0;
}

int TestWriteSimple(int argc, char *argv[])
{
   if (argc < 1) 
   {
      std::cerr << "usage: \n" 
                << argv[0] << " (without parameters) " << std::endl 
                << std::endl;
      return 1;
   }

  // GDCM_NAME_SPACE::Debug::DebugOn();

   int ret=0;
   int i=0;
   while( Images[i].sizeX>0 && Images[i].sizeY>0 )
   {
      std::cout << "Test n :" << i <<std::endl;; 
      ret += WriteSimple(Images[i] );
      i++;

   }

   return ret;
}
