/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: WriteDicomAsJPEG2000.cxx,v $
  Language:  C++
  Date:      $Date: 2007/08/29 08:13:40 $
  Version:   $Revision: 1.10 $
                                                                                
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
#include "gdcmDebug.h"

// Open a dicom file and compress it as JPEG2000 stream
int main(int argc, char *argv[])
{
  if( argc < 2)
    {
    std::cerr << argv[0] << " inputfilename.dcm [ outputfilename.dcm"
              << " quality debug]\n";
    return 1;
    }

   std::string filename = argv[1];
   std::string outfilename = "/tmp/bla.dcm";
   if( argc >= 3 )
     outfilename = argv[2];
   int quality = 100;
   if( argc >= 4 )
     quality = atoi(argv[3]);
   std::cerr << "Using quality: " << quality << std::endl;
   
   if (argc > 4)
      GDCM_NAME_SPACE::Debug::DebugOn();
      
// Step 1 : Read the image
   GDCM_NAME_SPACE::File *f = GDCM_NAME_SPACE::File::New();
   f->SetLoadMode ( GDCM_NAME_SPACE::LD_ALL ); // Load everything
   f->SetFileName( filename );
   f->Load();

   GDCM_NAME_SPACE::FileHelper *tested = GDCM_NAME_SPACE::FileHelper::New( f );
   std::string PixelType = tested->GetFile()->GetPixelType();
   int xsize = f->GetXSize();
   int ysize = f->GetYSize();
   int zsize = f->GetZSize();
   //tested->Print( std::cout );

   int samplesPerPixel = f->GetSamplesPerPixel();
   size_t testedDataSize    = tested->GetImageDataRawSize(); // Raw : Don't convert gray pixels+LUT to RBG pixels
   uint8_t *testedImageData = tested->GetImageDataRaw();
   
   if( GDCM_NAME_SPACE::Debug::GetDebugFlag() ) { 
      tested->Print( std::cout );
      std::cout << "-------------------------------------------------------------------------------" << std::endl;
   }
// Step 1 : Create the header of the new file
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

   int bitsallocated = f->GetBitsAllocated();
   int bitsstored    = f->GetBitsStored();
   int highbit       = f->GetHighBitPosition();
   //std::string pixtype = f->GetPixelType();
   int sign = f->IsSignedPixelData();

   // Set the pixel type
   str.str("");
   str << bitsallocated;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0100,"US");// Bits Allocated
   str.str("");
   str << bitsstored;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0101,"US");  // Bits Stored

   str.str("");
   str << highbit;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0102,"US"); // High Bit

   // Set the pixel representation
   str.str("");
   str << sign;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0103,"US"); // Pixel Representation

   // Set the samples per pixel
   str.str("");
   str << samplesPerPixel; //img.components;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0002,"US"); // Samples per Pixel

// The image may be displayed uncorectly if these fields are missing

   // Set the Pixel Aspect Ratio, if any
   std::string par = f->GetEntryString(0x0028,0x0034);
   std::cerr <<"Pixel Aspect Ratio [" << par << "]" << std::endl;
   if ( par != GDCM_NAME_SPACE::GDCM_UNFOUND )
      fileToBuild->InsertEntryString(par,0x0028,0x0034,"IS"); // Pixel Aspect Ratio

   // Set the Modality, if any
   std::string modality = f->GetEntryString(0x0008,0x0060);
   std::cerr <<"Modality [" << modality << "]" << std::endl;
   if ( modality != GDCM_NAME_SPACE::GDCM_UNFOUND )
      fileToBuild->InsertEntryString(modality,0x0008,0x0060,"CS"); // Modality

   // Set the Media Storage SOP Class UID, if any
   std::string mssop = f->GetEntryString(0x0002,0x0002);
   std::cerr <<"Media Storage SOP Class UID [" << mssop << "]" << std::endl;
   if ( mssop != GDCM_NAME_SPACE::GDCM_UNFOUND )
      fileToBuild->InsertEntryString(mssop,0x0002,0x0002,"UI"); // Media Storage SOP Class UID

   // This one is mandatory to deal with Pixel Aspect Ratio, in ultrasound images !
   // Set the SOP Class UID, if any
   std::string sop = f->GetEntryString(0x0008,0x0016);
   std::cerr <<"SOP Class UID [" << sop << "]" << std::endl;
   if ( sop != GDCM_NAME_SPACE::GDCM_UNFOUND )
      fileToBuild->InsertEntryString(sop,0x0008,0x0016,"UI"); // SOP Class UID
     
// Step 2 : Create the output image
   size_t size = xsize * ysize * zsize
               * samplesPerPixel  * bitsallocated / 8;

   GDCM_NAME_SPACE::FileHelper *fileH = GDCM_NAME_SPACE::FileHelper::New(fileToBuild);

   // Consider that pixels are unmodified
   fileH->SetContentType(GDCM_NAME_SPACE::UNMODIFIED_PIXELS_IMAGE);
   std::cerr << "xsize " << xsize << " ysize " << ysize << " zsize " << zsize << " samplesPerPixel " << samplesPerPixel
             << " bitsallocated " << bitsallocated << std::endl;
   std::cerr << "size " << size << " testedDataSize " << testedDataSize <<
                 std::endl;
   assert(abs (size-testedDataSize) <= 1 );
   fileH->SetWriteTypeToJPEG2000(  );
   //fileH->SetImageData(testedImageData, testedDataSize);

   // SetUserData will ensure the compression
   fileH->SetUserData(testedImageData, testedDataSize);
   if( !fileH->Write(outfilename) )
     {
     std::cerr << "write fails" << std::endl;
     }
   
   f->Delete();
   tested->Delete();
   fileToBuild->Delete();
   fileH->Delete();

   return 0;
}

