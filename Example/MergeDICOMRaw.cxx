/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: MergeDICOMRaw.cxx,v $
  Language:  C++
  Date:      $Date: 2007/12/03 17:16:25 $
  Version:   $Revision: 1.2 $
                                                                                
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
#include "gdcmUtil.h"
#include "gdcmDocEntry.h"
#include "gdcmDataEntry.h"

#include <iomanip>

/*
 * Take a template DICOM and merge in the Pixel Data from a RAW file
 */
int main(int argc, char *argv[])
{  
   GDCM_NAME_SPACE::File *f;
 
   if( argc < 3 )
   {
      std::cerr << "Usage :" << argv[0] << " input.dcm inputrawfile" << std::endl;
      std::cerr << "  Ex: " << argv[0] << " template.dcm data.raw" << std::endl;
      return 1;
   }
   std::string fileName = argv[1];
   
// ============================================================
//   Read the input image.
// ============================================================

   f = GDCM_NAME_SPACE::File::New( );

   //f->SetLoadMode(GDCM_NAME_SPACE::LD_NOSEQ | GDCM_NAME_SPACE::LD_NOSHADOW);
   f->SetFileName( fileName );
      
   bool res = f->Load();  

   GDCM_NAME_SPACE::Debug::DebugOn();
   if( GDCM_NAME_SPACE::Debug::GetDebugFlag() )
   {
      std::cout << "---------------------------------------------" << std::endl;
      f->Print();
      std::cout << "---------------------------------------------" << std::endl;
   }
   if (!res) {
       std::cerr << "Sorry, " << fileName << " not a gdcm-readable "
           << "DICOM / ACR File"
           << std::endl;
      f->Delete();
      return 1;
   }
   std::cout << " ... is readable " << std::endl;

     // (Find the dicom tag, and) extract the string
     
   // Read in the input data as a file:
   const char *inputraw = argv[2];
   std::ifstream i(inputraw);
   if( !i )
   {
      std::cerr << "Problem opening file: " << inputraw << std::endl;
      f->Delete();
      return 1;
   }
   const unsigned int dims[3] = { 256, 364, 100 };
   unsigned int pixelsize = 2; // 16bits => 2 bytes
   unsigned long length = dims[0] * dims[1] * dims[2] * pixelsize;
   uint8_t *pixeldata = new uint8_t[length];
   memset(pixeldata,0,length);
   if( ! i.read((char*)pixeldata, length) )
     {
      std::cerr << "Problem reading raw data" << std::endl;
     }
   i.close();

   std::ostringstream str;

   // Set the image size
   str.str("");
   str << dims[0];
   f->InsertEntryString(str.str(),0x0028,0x0011,"US"); // Columns

   str.str("");
   str << dims[1];
   f->InsertEntryString(str.str(),0x0028,0x0010,"US"); // Rows

   str.str("");
   str << dims[2];
   f->InsertEntryString(str.str(),0x0028,0x0008, "IS"); // Number of Frames

   // Set the pixel type
   str.str("");
   str << pixelsize * 8;
   f->InsertEntryString(str.str(),0x0028,0x0100,"US"); // Bits Allocated
   f->InsertEntryString(str.str(),0x0028,0x0101,"US"); // Bits Stored

   str.str("");
   str << ( pixelsize * 8 ) - 1;
   f->InsertEntryString(str.str(),0x0028,0x0102,"US"); // High Bit

   // Set the pixel representation
   str.str("");
   str << "0"; // Unsigned
   f->InsertEntryString(str.str(),0x0028,0x0103,"US"); // Pixel Representation

   // Set the samples per pixel
   str.str("");
   str << 1; // grayscale
   f->InsertEntryString(str.str(),0x0028,0x0002,"US"); // Samples per Pixel





   GDCM_NAME_SPACE::FileHelper *fh = GDCM_NAME_SPACE::FileHelper::New(f);
   // Convert Media Storage SOP Class if needed
   std::string mssop = f->GetEntryString(0x0002,0x0002);
   // See http://www.toshiba-europe.com/medical/Materials/PDF/Dicom/MIIUS0026EA.pdf
   if ( GDCM_NAME_SPACE::Util::DicomStringEqual(mssop, "1.2.392.200036.9116.7.8.1.1.1") ) // Toshiba US Private Data Storage
     {
     fh->InsertEntryString("1.2.840.10008.5.1.4.1.1.3.1",0x0002,0x0002,"UI"); // Media Storage SOP Class UID
     fh->InsertEntryString("1.2.840.10008.5.1.4.1.1.3.1",0x0008,0x0016,"UI"); // SOP Class UID
     }

  // TODO
  // for UltrasoundMultiframeImageStorage we may need also:
  // (0028,0009) AT (0018,1063)                              #   4, 1 FrameIncrementPointer
  // and
  // Pixel Aspect Ratio



   fh->SetContentType(GDCM_NAME_SPACE::UNMODIFIED_PIXELS_IMAGE);
   fh->SetImageData(pixeldata,length);
   fh->SetWriteModeToRaw(); // no LUT, no compression.
   fh->SetWriteTypeToDcmExplVR();
   const char fileOut[] = "out.dcm";
   if( !fh->Write(fileOut) )
   {
      std::cerr<< "-------------------------------\n"
                   << "Error when writting the file " << fileOut << "\n"
                << "No file written\n";
   }


   fh->Delete();
   f->Delete();
   return 0;
}


