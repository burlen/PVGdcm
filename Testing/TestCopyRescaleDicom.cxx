/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestCopyRescaleDicom.cxx,v $
  Language:  C++
  Date:      $Date: 2007/10/30 09:13:03 $
  Version:   $Revision: 1.23 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmDataEntry.h"

#include <time.h>
#include <sys/times.h>
#include <iomanip> // for std::ios::left, ...

//Generated file:
#include "gdcmDataImages.h"

#ifndef _WIN32
#include <unistd.h> //for access, unlink
#else
#include <io.h> //for _access on Win32
#endif

bool FileExists(const char *filename)
{
#ifdef _MSC_VER
# define access _access
#endif
#ifndef R_OK
# define R_OK 04
#endif
  if ( access(filename, R_OK) != 0 )
    {
    return false;
    }
  else
    {
    return true;
    }
}

bool RemoveFile(const char *source)
{
#ifdef _MSC_VER
#define _unlink unlink
#endif
  return unlink(source) != 0 ? false : true;
}

int CopyRescaleDicom(std::string const &filename, 
                     std::string const &output )
{
   std::cout << "   Testing: " << filename << std::endl;
   if( FileExists( output.c_str() ) )
   {
     // std::cerr << "Don't try to cheat, I am removing the file anyway" << std::endl;
      if( !RemoveFile( output.c_str() ) )
      {
         std::cout << "Ouch, the file exist, but I cannot remove it" << std::endl;
         return 1;
      }
   }
 
   //////////////// Step 1:
   std::cout << "      1...";
   GDCM_NAME_SPACE::File *originalF = GDCM_NAME_SPACE::File::New( );
   originalF->SetFileName( filename );
   originalF->Load();
   
   GDCM_NAME_SPACE::File *copyF     = GDCM_NAME_SPACE::File::New( );

   //First of all copy the file, field by field

   //////////////// Step 2:
   std::cout << "2...";
   // Copy of the file content
   GDCM_NAME_SPACE::DocEntry *d = originalF->GetFirstEntry();
   while(d)
   {
      if ( GDCM_NAME_SPACE::DataEntry *de = dynamic_cast<GDCM_NAME_SPACE::DataEntry *>(d) )
      {              
         copyF->InsertEntryBinArea( de->GetBinArea(),de->GetLength(),
                                   de->GetGroup(),de->GetElement(),
                                   de->GetVR() );
      }
      else
      {
       // We skip pb of SQ recursive exploration
      }

      d = originalF->GetNextEntry();
   }

   GDCM_NAME_SPACE::FileHelper *original = GDCM_NAME_SPACE::FileHelper::New( originalF );
   GDCM_NAME_SPACE::FileHelper *copy     = GDCM_NAME_SPACE::FileHelper::New( copyF );

   size_t dataSize = original->GetImageDataSize();

   size_t rescaleSize;
   uint8_t *rescaleImage;

   const std::string &bitsStored = originalF->GetEntryString(0x0028,0x0101);
   if( bitsStored == "16" )
   {
      std::cout << "Rescale...";
      copyF->InsertEntryString( "8", 0x0028, 0x0100, "US"); // Bits Allocated
      copyF->InsertEntryString( "8", 0x0028, 0x0101, "US"); // Bits Stored
      copyF->InsertEntryString( "7", 0x0028, 0x0102, "US"); // High Bit
      copyF->InsertEntryString( "0", 0x0028, 0x0103, "US"); // Pixel Representation

      // We assume the value were from 0 to uint16_t max
      rescaleSize = dataSize / 2;
      rescaleImage = new uint8_t[dataSize];

      uint16_t *imageData16 = (uint16_t*)original->GetImageData();
      uint16_t *tmpImage = imageData16;
      uint8_t *tmpRescale = rescaleImage;
      for(unsigned int i=0; i<rescaleSize; i++)
      {
         *tmpRescale = (uint8_t)( (*tmpImage)>>8 );
         tmpImage++;
         tmpRescale++;
      }
   }
   else
   {
      std::cout << "Same...";
      rescaleSize = dataSize;
      rescaleImage = new uint8_t[dataSize];
      memcpy(rescaleImage,original->GetImageData(),dataSize);
   }

   copy->SetImageData(rescaleImage, rescaleSize);

   //////////////// Step 3:
   std::cout << "3...";
   copy->SetWriteModeToRGB();
   if( !copy->WriteDcmExplVR(output) )
   {
      std::cout << " Failed" << std::endl
                << "        " << output << " not written" << std::endl;

      delete[] rescaleImage;

      return 1;
   }

   //////////////// Step 4:
   std::cout << "4...";
   GDCM_NAME_SPACE::FileHelper *copy2 = GDCM_NAME_SPACE::FileHelper::New( output );

   //Is the file written still gdcm parsable ?
   if ( !copy2->GetFile()->IsReadable() )
   { 
      std::cout << " Failed" << std::endl
                << "        " << output << " not readable" << std::endl;

      delete[] rescaleImage;

      return 1;
   }

   //////////////// Step 5:
   std::cout << "5...";
   size_t    dataSizeWritten = copy2->GetImageDataSize();
   uint8_t *imageDataWritten = copy2->GetImageData();

   if (originalF->GetXSize() != copy2->GetFile()->GetXSize() ||
       originalF->GetYSize() != copy2->GetFile()->GetYSize() ||
       originalF->GetZSize() != copy2->GetFile()->GetZSize())
   {
      std::cout << "Failed" << std::endl
         << "        X Size differs: "
         << "X: " << originalF->GetXSize() << " # " 
                  << copy2->GetFile()->GetXSize() << " | "
         << "Y: " << originalF->GetYSize() << " # " 
                  << copy2->GetFile()->GetYSize() << " | "
         << "Z: " << originalF->GetZSize() << " # " 
                  << copy2->GetFile()->GetZSize() << std::endl;
      delete[] rescaleImage;

      return 1;
   }

   if (rescaleSize != dataSizeWritten)
   {
      std::cout << " Failed" << std::endl
                << "        Pixel areas lengths differ: "
                << dataSize << " # " << dataSizeWritten << std::endl;

      delete[] rescaleImage;

      return 1;
   }

   if (int res = memcmp(rescaleImage, imageDataWritten, rescaleSize) !=0)
   {
      (void)res;
      std::cout << " Failed" << std::endl
                << "        Pixel differ (as expanded in memory)." << std::endl;

      delete[] rescaleImage;

      return 1;
   }
   std::cout << "OK." << std::endl ;

   delete[] rescaleImage;

   return 0;
}

// Here we load a gdcmFile and then try to create from scratch a copy of it,
// copying field by field the dicom image

int TestCopyRescaleDicom(int argc, char *argv[])
{
   if ( argc == 3 )
   {
      // The test is specified a specific filename, use it instead of looping
      // over all images
      const std::string input     = argv[1];
      const std::string reference = argv[2];
      return CopyRescaleDicom( input, reference );
   }
   else if ( argc > 3 || argc == 2 )
   {
      std::cout << "   Usage: " << argv[0]
                << " (no arguments needed)." << std::endl;
      std::cout << "or   Usage: " << argv[0]
                << " filename.dcm reference.dcm" << std::endl;
      return 1;
   }
   // else other cases:

   std::cout << "   Description (Test::TestCopyDicom): "
             << std::endl;
   std::cout << "   For all images in gdcmData (and not blacklisted in "
                "Test/CMakeLists.txt)"
             << std::endl;
   std::cout << "   apply the following to each filename.xxx: "
             << std::endl;
   std::cout << "   step 1: parse the image (as gdcmFile) and call"
             << " IsReadable(). After that, call GetImageData() and "
             << "GetImageDataSize() "
             << std::endl;
   std::cout << "   step 2: create a copy of the readed file and the new"
             << " pixel data are set to the copy"
             << std::endl;
   std::cout << "   step 3: write the copy of the image"
             << std::endl;
   std::cout << "   step 4: read the copy and call IsReadable()"
             << std::endl;
   std::cout << "   step 5: compare (in memory with memcmp) that the two "
             << "images " << std::endl
             << "           match (as expanded by gdcm)." << std::endl;
   std::cout << std::endl;


   clock_t r1,r2, r3,r4;
   struct tms tms1,tms2, tms3,tms4;

   r3 = times(&tms3);
   int i =0;
   int retVal = 0;  //by default this is an error
   while( gdcmDataImages[i] != 0 )
   {
      std::string filename = GDCM_DATA_ROOT;
      filename += "/";  //doh!
      filename += gdcmDataImages[i];

      std::string output = "output.dcm";

      r1 = times(&tms1);
      if( CopyRescaleDicom( filename, output ) != 0 )
      {
         retVal++;
      }
      r2 = times(&tms2);

      std::cout 
        << std::setw(150-strlen(gdcmDataImages[i]))
        << gdcmDataImages[i] << " user time: " 
        << (long) ((tms2.tms_utime)  - (tms1.tms_utime)) 
        << " system time: "
        << (long) ((tms2.tms_stime)  - (tms1.tms_stime)) 
        << "\t elapsed time: " << r2 - r1
        << std::endl;

      i++;
   }
   r4 = times(&tms4);

   std::cout 
        << std::setw(150-strlen("Gross Total")) << " --> "
        << "Gross Total" << " user time: " 
        << (long) ((tms4.tms_utime)  - (tms3.tms_utime))
        << " system time: "
        << (long) ((tms4.tms_stime)  - (tms3.tms_stime)) 
        << "\t elapsed time: " << (long) (r4 - r3)
        << std::endl;
   return retVal;
}

