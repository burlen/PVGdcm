/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: WriteDicomAsMPEG.cxx,v $
  Language:  C++
  Date:      $Date: 2007/06/21 15:01:00 $
  Version:   $Revision: 1.3 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmUtil.h"
 

// Open a dicom file and compress it as JPEG stream
int main(int argc, char *argv[])
{
  const int xsize = 352; //352x240 DirectClass 248kb 0.000u 0:01
  const int ysize = 240;
  const int zsize = 10;
  const int samplesPerPixel = 3;
  (void)argc;
  (void)argv;

   // Step 1 : Create the header of the image

   GDCM_NAME_SPACE::File *fileToBuild = GDCM_NAME_SPACE::File::New();
   std::ostringstream str;

   // Set the image size
   str.str("");
   str << xsize;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0011,"US"); // Columns
   str.str("");
   str << ysize;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0010,"US"); // Rows

   if(zsize>1)
   {
      str.str("");
      str << zsize;
      fileToBuild->InsertEntryString(str.str(),0x0028,0x0008,"IS"); // Number of Frames
   }

   // Set the pixel type
   str.str("");
   str << 8; //img.componentSize;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0100,"US"); // Bits Allocated

   str.str("");
   str << 8; //img.componentUse;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0101,"US"); // Bits Stored

   str.str("");
   str << 7; //( img.componentSize - 1 );
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0102,"US"); // High Bit

   // Set the pixel representation
   str.str("");
   str << 0; //img.sign;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0103,"US"); // Pixel Representation

   // Set the samples per pixel
   str.str("");
   str << samplesPerPixel; //img.components;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0002,"US"); // Samples per Pixel

// Step 2 : Create the output image
//   std::cout << "2...";
//   if( img.componentSize%8 > 0 )
//   {
//      img.componentSize += 8-img.componentSize%8;
//   }
   size_t size = xsize * ysize * zsize
               * samplesPerPixel /* * img.componentSize / 8*/;

   uint8_t *imageData = new uint8_t[size];
   GDCM_NAME_SPACE::FileHelper *fileH = GDCM_NAME_SPACE::FileHelper::New(fileToBuild);
   //fileH->SetImageData(imageData,size);
   std::ifstream mpeg("/tmp/ts.mpg");
   mpeg.seekg(0, std::ios::end);
   size = mpeg.tellg();
   std::cerr << "Size MPEG:" << size << std::endl;
   mpeg.seekg(0, std::ios::beg);
   mpeg.read((char*)imageData, size);
   fileH->SetImageData(imageData, size);
   fileH->SetWriteTypeToJPEG(  );
   std::string outfilename = "/tmp/ts.dcm";
   if( !fileH->Write(outfilename) )
     {
     std::cerr << "Badddd" << std::endl;
     }

   fileToBuild->Delete();
   fileH->Delete();

   return 0;
}

