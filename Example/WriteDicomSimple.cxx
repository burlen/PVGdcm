/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: WriteDicomSimple.cxx,v $
  Language:  C++
  Date:      $Date: 2007/06/26 15:42:14 $
  Version:   $Revision: 1.20 $
                                                                                
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
#include "gdcmArgMgr.h"
 
#include <iostream>
#include <sstream>

// Image size
 uint16_t SIZE_X;
 uint16_t SIZE_Y;
// Number of components in the image (3 for RGB)
 uint16_t  COMPONENT;
// Size of each component (in byte)
 uint16_t  COMPONENT_SIZE;


int main(int argc, char *argv[])
{


   START_USAGE(usage)
   " \n exWriteDicomSimple : \n                                               ",
   " Creates a Dicom image File                                               ",
   " usage: exWriteDicomSimple {fileout=outputFileName}                       ",
   "                       [nx=number of colomns] [ny=number of lines]        ",
   "                       [components= 1: grey, 3 : RGB]                     ",
   "                       [pixelsize= Pixel Size in Bytes : 1/2] } ] [debug] ",
   FINISH_USAGE

   // Initialize Arguments Manager   
   GDCM_NAME_SPACE::ArgMgr *am= new GDCM_NAME_SPACE::ArgMgr(argc, argv);
  
   if (argc == 1 || am->ArgMgrDefined("usage") )
   {
      am->ArgMgrUsage(usage); // Display 'usage'
      delete am;
      return 1;
   }

   if (am->ArgMgrDefined("debug"))
      GDCM_NAME_SPACE::Debug::DebugOn();
      
   std::string fileOut = am->ArgMgrGetString("fileout",(char *)"WriteDicomSimple.dcm");   
   SIZE_X = am->ArgMgrGetInt("NX", 128);
   SIZE_Y = am->ArgMgrGetInt("NY", 128);
   COMPONENT = am->ArgMgrGetInt("components", 1);
   COMPONENT_SIZE = am->ArgMgrGetInt("size", 1);
   
   /* if unused Param we give up */
   if ( am->ArgMgrPrintUnusedLabels() )
   {
      am->ArgMgrUsage(usage);
      delete am;
      return 1;
   } 

   delete am;  // we don't need Argument Manager any longer

   // ----------- End Arguments Manager ---------
   
   
// Step 1 : Create an empty GDCM_NAME_SPACE::FileHelper for the image
//          (it deals with the acces to the pixels)
   GDCM_NAME_SPACE::FileHelper *fileH = GDCM_NAME_SPACE::FileHelper::New();
   
//         Get the empty GDCM_NAME_SPACE::File of the image 
//         (it deals with the 'entries' od the image header)  
   GDCM_NAME_SPACE::File *header = fileH->GetFile();
    
   std::ostringstream str;

   // Set the image size
   str.str("");
   str << SIZE_X;
   header->InsertEntryString(str.str(),0x0028,0x0011,"US"); // Columns

   str.str("");
   str << SIZE_Y;
   header->InsertEntryString(str.str(),0x0028,0x0010,"US"); // Rows

   // Set the pixel type
   str.str("");
   str << COMPONENT_SIZE * 8;
   header->InsertEntryString(str.str(),0x0028,0x0100,"US"); // Bits Allocated
   header->InsertEntryString(str.str(),0x0028,0x0101,"US"); // Bits Stored

   str.str("");
   str << ( COMPONENT_SIZE * 8 ) - 1;
   header->InsertEntryString(str.str(),0x0028,0x0102,"US"); // High Bit

   // Set the pixel representation
   str.str("");
   str << "0"; // Unsigned
   header->InsertEntryString(str.str(),0x0028,0x0103,"US"); // Pixel Representation

   // Set the samples per pixel
   str.str("");
   str << COMPONENT;
   header->InsertEntryString(str.str(),0x0028,0x0002,"US"); // Samples per Pixel


// Step 2 : Create the output image
   size_t size = SIZE_X * SIZE_Y * COMPONENT * COMPONENT_SIZE;
   unsigned char *imageData = new unsigned char[size];

   unsigned char *tmp = imageData;
   for(int j=0;j<SIZE_Y;j++)
   {
      for(int i=0;i<SIZE_X;i++)
      {
         for(int c=0;c<COMPONENT;c++)
         {
            *tmp = (unsigned char)j;
            tmp += COMPONENT_SIZE; 
         }
      }
   }

// Step 3 : Set the 'Pixel Area' of the image

   fileH->SetImageData(imageData,size);
   header->Print();
   std::cout << "-------------------------------" << std::endl;
   
      // Step 4 : Set the writting mode and write the image

/*
// Warning : SetImageData does *not* add the 7FE0|0010 Element!
//           IsReadable() is always false
   if( !header->IsReadable() )
   {
      std::cerr << "-------------------------------\n"
                << "Error while creating the file\n"
                << "This file is considered to be not readable\n";
      return 1;
   }
*/
   fileH->SetWriteModeToRaw(); // no LUT, no compression.

    // Write a DICOM Explicit VR file

    fileH->SetWriteTypeToDcmExplVR();
    std::cout << "Write DCM Explicit VR" << std::endl
              << "File :" << fileOut << std::endl;


   if( !fileH->Write(fileOut) )
   {
      std::cout << "-------------------------------\n"
                   << "Error when writting the file " << fileOut << "\n"
                << "No file written\n";
   }

   header->Print();

   delete[] imageData;
   fileH->Delete();

   return 0;
}

