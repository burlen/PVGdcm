/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: exExtractOverlaysDCM.cxx,v $
  Language:  C++
  Date:      $Date: 2009/05/28 15:44:34 $
  Version:   $Revision: 1.6 $
                                                                                
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
#include "gdcmDataEntry.h"
#include "gdcmDirList.h"
#include "gdcmArgMgr.h"

#include <iostream>

// Each BIT of Overlay Data (0x60xx,0x3000) corresponds
// to a BYTE of overlay image.

void explodeByte(unsigned char byte, unsigned char* result)
{
   unsigned char mask = 1;
   for (int i=0;i<8;i++)
   {
      if ((byte & mask)==0)
         result[i]=0;
      else
         result[i]=1;
      mask<<=1;
   }
   return;
}

int main(int argc, char *argv[])
{
   START_USAGE(usage)
   " \n exExtractOverlaysDCM :\n                                              ",
   " Extract DICOM style overlays from an image                               ",
   "         Resulting image name(s) are postpended with .ovly.dcm            ",
   " usage: exExtractOverlaysDCM filein=inputFileName  [debug] [warning]      ",
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

   f->AddForceLoadElement(0x6000,0x3000);  // Overlay Data
   f->AddForceLoadElement(0x6002,0x3000); 
   f->AddForceLoadElement(0x6004,0x3000); 
   f->AddForceLoadElement(0x6006,0x3000);    
   f->AddForceLoadElement(0x6008,0x3000);    
   f->AddForceLoadElement(0x600a,0x3000); 
   f->AddForceLoadElement(0x600c,0x3000); 
   f->AddForceLoadElement(0x600e,0x3000);
   f->AddForceLoadElement(0x6010,0x3000);
   f->AddForceLoadElement(0x6012,0x3000);             
   f->AddForceLoadElement(0x6014,0x3000);             
   f->AddForceLoadElement(0x6016,0x3000); 
   f->AddForceLoadElement(0x6018,0x3000); 
   f->AddForceLoadElement(0x601a,0x3000);                
   f->AddForceLoadElement(0x601c,0x3000); 
   f->AddForceLoadElement(0x601e,0x3000); // Hope it's enought : Dicom says 60xx ...

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
//   Check whether image contains Overlays DICOM style.
// ============================================================

   unsigned int nx = f->GetXSize();
   unsigned int ny = f->GetYSize();
   unsigned int nxy=nx*ny;
   uint16_t currentOvlGroup;
   int i;

   std::ostringstream str;

   uint8_t *outputData = new uint8_t[nxy]; // uint8 is enought to hold 1 bit !

   GDCM_NAME_SPACE::File *fileToBuild = 0;
   GDCM_NAME_SPACE::FileHelper *fh = 0;

// ============================================================
//   Get each overlay group into the image header
// ============================================================
   for(i=0, currentOvlGroup=0x6000; i<32; i+=2 ,currentOvlGroup+=2)
   {
      GDCM_NAME_SPACE::DataEntry *e10 = f->GetDataEntry(currentOvlGroup, 0x0010); // nb Row Ovly
      if (e10 == 0)
      {
         if( GDCM_NAME_SPACE::Debug::GetWarningFlag() )
            std::cout << " Image doesn't contain Overlay on " <<std::hex
                      << currentOvlGroup+i << std::endl;
         continue;
      }

      GDCM_NAME_SPACE::DataEntry *e = f->GetDataEntry(currentOvlGroup, 0x3000);
      if (e == 0)
      {
         if( GDCM_NAME_SPACE::Debug::GetWarningFlag() )
            std::cout << " Image doesn't contain DICOM Overlay Data " <<std::hex
                      << currentOvlGroup+i << std::endl;
      }
      else
      {
         uint8_t *overlay = (uint8_t *)(e->GetBinArea());
         if ( overlay == 0 )
         {
            std::cerr << "Sorry, Overlays of [" << fileName << "] are not "
                       << " gdcm-readable."    << std::endl;
             continue;
         }
         if( GDCM_NAME_SPACE::Debug::GetWarningFlag() )
            std::cout << " Overlay on group [" << std::hex << currentOvlGroup<< "] is read! " << std::endl;

      // ============================================================
      //  DICOM Overlay Image data generation
      // ============================================================

         unsigned char *result=outputData;
         for (unsigned int i2=0;i2<(nxy/8);i2++)
         {
            explodeByte(overlay[i2], result);
            result+=8;
         }
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

   if (fh)
      fh->Delete();
   if (fileToBuild)
      fileToBuild->Delete();
   f->Delete();

   return 0;
}

