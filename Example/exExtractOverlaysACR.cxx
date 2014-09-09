/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: exExtractOverlaysACR.cxx,v $
  Language:  C++
  Date:      $Date: 2007/10/30 09:15:57 $
  Version:   $Revision: 1.4 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmCommon.h"
#include "gdcmDebug.h"
#include "gdcmDocEntry.h"
#include "gdcmArgMgr.h"

#include <iostream>
#include <stdlib.h> // for atoi

 /* SIEMENS_GBS_III-16-ACR_NEMA_1.acr
 
 // Example (sorry, we've got no more than this one ...)
 
0028|0010[US] [Rows] [256] x(100)
0028|0011[US] [Columns] [256] x(100)
0028|0030[DS] [Pixel Spacing] [01.56\1.56]
0028|0100[US] [Bits Allocated] [16] x(10)
0028|0101[US] [Bits Stored] [12] x(c)
0028|0102[US] [High Bit] [11] x(b)
0028|0103[US] [Pixel Representation] [0] x(0)
 
6000|0000[UL] [Group Length] [96] x(60)
6000|0010[US] [Rows] [256] x(100)
6000|0011[US] [Columns] [256] x(100)
6000|0040[CS] [Overlay Type] [R ]
6000|0050[SS] [Overlay Origin] [23601\8241] x(5c31)
6000|0100[US] [Overlay Bits Allocated] [16] x(10)
6000|0102[US] [Overlay Bit Position] [12] x(c)
...
...
6006|0000[UL] [Group Length] [96] x(60)
6006|0010[US] [Rows] [256] x(100)
6006|0011[US] [Columns] [256] x(100)
6006|0040[CS] [Overlay Type] [R ]
6006|0050[SS] [Overlay Origin] [23601\8241] x(5c31)
6006|0100[US] [Overlay Bits Allocated] [16] x(10)
6006|0102[US] [Overlay Bit Position] [15] x(f)
 */
 
int main(int argc, char *argv[])
{
   START_USAGE(usage)
   " \n exExtractOverlaysACR :\n                                              ",
   " Extract ACR-NEMA style overlays from an image                            ",
   "         Resulting image name(s) are postpended with .ovly.dcm            ",
   " usage: exExtractOverlaysACR filein=inputFileName  [debug] [warning]      ",
   "        warning  : user wants to run the program in 'warning mode'        ",
   "        debug    : developper wants to run the program in 'debug mode'    ",
   FINISH_USAGE

   // ----- Initialize Arguments Manager ------

   GDCM_NAME_SPACE::ArgMgr *am = new GDCM_NAME_SPACE::ArgMgr(argc, argv);

   if (argc == 1 || am->ArgMgrDefined("usage"))
   {
      am->ArgMgrUsage(usage); // Display 'usage'
      delete am;
      return 0;
   }

   const char *fileName  = am->ArgMgrWantString("filein", usage);

   if (am->ArgMgrDefined("debug"))
      GDCM_NAME_SPACE::Debug::DebugOn();

   if (am->ArgMgrDefined("warning"))
      GDCM_NAME_SPACE::Debug::WarningOn();

   // if unused Param we give up
   if ( am->ArgMgrPrintUnusedLabels() )
   {
      am->ArgMgrUsage(usage);
      delete am;
      return 0;
   }

   delete am;  // we don't need Argument Manager any longer

   // ========================== Now, we can do the job! ================

   GDCM_NAME_SPACE::File *f;

// ============================================================
//   Read the input image.
// ============================================================

   f = GDCM_NAME_SPACE::File::New( );

   f->SetLoadMode(GDCM_NAME_SPACE::LD_NOSEQ | GDCM_NAME_SPACE::LD_NOSHADOW);
   f->SetFileName( fileName );
   bool res = f->Load();

   if (!res) {
       std::cout << "Sorry, " << fileName <<"  not a gdcm-readable "
           << "DICOM / ACR File"
           <<std::endl;
      f->Delete();
      return 0;
   }
   std::cout << fileName << " ... is readable " << std::endl;

// ============================================================
//   Check whether image contains Overlays ACR-NEMA style.
// ============================================================

   int bitsAllocated = f->GetBitsAllocated();
   if ( bitsAllocated <= 8 )
   {
      std::cout << " 8 bits pixel image cannot contain Overlays " << std::endl;
      f->Delete();
      return 0;
   }
   std::string s1 = f->GetEntryString(0x6000, 0x0102);
   if (s1 == GDCM_NAME_SPACE::GDCM_UNFOUND)
   {
      std::cout << " Image doesn't contain any Overlay " << std::endl;
      f->Delete();
      return 0;
   }
   std::cout << fileName << " is read! " << std::endl;


// ============================================================
//   Load the pixels in memory.
// ============================================================

   GDCM_NAME_SPACE::FileHelper *fh1 = GDCM_NAME_SPACE::FileHelper::New(f);
   fh1->SetKeepOverlays(true);
   uint16_t *pixels = (uint16_t *)fh1->GetImageDataRaw();
   int lgt = fh1->GetImageDataRawSize();

   if( GDCM_NAME_SPACE::Debug::GetDebugFlag() )
      std::cout << "Pixels read as expected : length = " << lgt << std::endl;

// ============================================================
//   Prepare the stuff
// ============================================================

   unsigned int nx = f->GetXSize();
   unsigned int ny = f->GetYSize();
   unsigned int nxy=nx*ny;
   uint16_t currentOvlGroup;
   int i;

   std::ostringstream str;

   uint8_t *outputData = new uint8_t[nxy]; // uint8 is enought to hold 1 bit !

   std::string strOvlBitPosition;
   int ovlBitPosition;
   uint16_t mask;
   uint16_t overlayLocation;
   std::string strOverlayLocation;

   GDCM_NAME_SPACE::File *fileToBuild = 0;
   GDCM_NAME_SPACE::FileHelper *fh = 0;

// ============================================================
//   Get each overlay Bit into the image
// ============================================================
   for(i=0, currentOvlGroup=0x6000; i<32; i+=2 ,currentOvlGroup+=2)
   {
      if ( (strOvlBitPosition = f->GetEntryString(currentOvlGroup, 0x0102)) 
                                                 == GDCM_NAME_SPACE::GDCM_UNFOUND )
          continue;

      if( GDCM_NAME_SPACE::Debug::GetWarningFlag() )
         std::cout << "Current Overlay Group " << std::hex << currentOvlGroup
                   << " OvlBitPosition " << strOvlBitPosition << std::endl;

      strOverlayLocation = f->GetEntryString(currentOvlGroup, 0x0200);
      if ( strOverlayLocation != GDCM_NAME_SPACE::GDCM_UNFOUND )
      {
         overlayLocation = atoi(strOverlayLocation.c_str());
         if ( overlayLocation != f->GetGrPixel() )
         {
            std::cout << "Big Trouble : Overlays are NOT in the Pixels Group "
                      << std::hex << "(" << overlayLocation << " vs " 
                      << f->GetGrPixel() << std::endl;
            // Actually, here, we should (try to) read the overlay location
            // and go on the job.
            continue;
         }
      }

      // ============================================================
      //  DICOM Overlay Image data generation
      // ============================================================

      ovlBitPosition = atoi(strOvlBitPosition.c_str());
      mask = 1 << ovlBitPosition;

      if( GDCM_NAME_SPACE::Debug::GetDebugFlag() )
         std::cout << "Mask :[" <<std::hex << mask << "]" << std::endl;
      for (unsigned int j=0; j<nxy; j++)
      {
         if( GDCM_NAME_SPACE::Debug::GetDebugFlag() )
            if (pixels[j] >= 0x1000)// if it contains at least one overlay bit
               printf("%d : %04x\n",j, pixels[j]);

         if ( (pixels[j] & mask) == 0 )
            outputData[j] = 0;
         else
            outputData[j] = 128;
      }
   // ============================================================
   //   Write a new file
   // ============================================================

      fileToBuild = GDCM_NAME_SPACE::File::New();
      str.str("");
      str << nx;
      fileToBuild->InsertEntryString(str.str(),0x0028,0x0011, "US"); // Columns
      str.str("");
      str << ny;
      fileToBuild->InsertEntryString(str.str(),0x0028,0x0010, "US"); // Rows

      fileToBuild->InsertEntryString("8",0x0028,0x0100, "US"); // Bits Allocated
      fileToBuild->InsertEntryString("8",0x0028,0x0101, "US"); // Bits Stored
      fileToBuild->InsertEntryString("7",0x0028,0x0102, "US"); // High Bit
      fileToBuild->InsertEntryString("0",0x0028,0x0103, "US"); // Pixel Representation
      fileToBuild->InsertEntryString("1",0x0028,0x0002, "US"); // Samples per Pixel
      fileToBuild->InsertEntryString("MONOCHROME2 ",0x0028,0x0004, "LO");

      // feel free to add any field (Dicom Data Entry) you like, here.
      // ...
      // Other mandatory fields will be set automatically,
      // just before Write(), by FileHelper::CheckMandatoryElements()

      fh = GDCM_NAME_SPACE::FileHelper::New(fileToBuild);

      fh->SetImageData(outputData,nx*ny);
      fh->SetWriteTypeToDcmExplVR();

std::ostringstream tmp;
tmp <<std::hex;
tmp <<currentOvlGroup;

      str.str("");
// -> Why doesn't it work ?!?
      //str << fileName << std::hex << currentOvlGroup << ".dcm" << std::ends;

str << fileName << ".ovly." << tmp.str() << ".dcm" << std::ends;

      //   Write the current 'overlay' file

      if( !fh->Write(str.str()) )
      {
         std::cout << "Failed\n"
                   << "File [" << str.str() << "] is unwrittable" << std::endl;
      }
      else
      {
         std::cout << "File written successfully [" << str.str()  << "]" << std::endl;
      }

   } // end on loop on 60xx

   if (f)
      fh->Delete();
   if (fileToBuild)
      fileToBuild->Delete();
   f->Delete();
   delete pixels;
   delete outputData;

   return 0;
}

