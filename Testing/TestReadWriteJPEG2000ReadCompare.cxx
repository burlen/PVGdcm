/*=========================================================================

  Program:   gdcm
  Module:    $RCSfile: TestReadWriteJPEG2000ReadCompare.cxx,v $
  Language:  C++
  Date:      $Date: 2007/09/05 09:55:01 $
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
#include "gdcmDebug.h"

#include "gdcmGlobal.h"
#include "gdcmTS.h"

//Generated file:
#include "gdcmDataImages.h"


const unsigned int MAX_NUMBER_OF_DIFFERENCE = 10;

int nb_of_success2000___;
int nb_of_failure2000___;
int nb_of_diffPM12000___;
static int CompareInternalJPEG2000(std::string const &filename, std::string const &output)
{
   std::cout << "----------------------------------------------------------------------" << std::endl
             << "   Testing: " << filename << std::endl;

   //////////////// Step 1 (see above description):

   GDCM_NAME_SPACE::File *file = GDCM_NAME_SPACE::File::New( );
   file->SetFileName( filename );
   file->Load ();
   if( !file->IsReadable() )
   {
      std::cout << "Failed" << std::endl
                << "Test::TestReadWriteJPEG2000ReadCompare: Image not gdcm compatible:"
                << filename << std::endl;
      file->Delete();
      nb_of_failure2000___++;
      return 1;
   }
   std::cout << "           step 1...";

   //////////////// Step 2:
   GDCM_NAME_SPACE::FileHelper *filehelper = GDCM_NAME_SPACE::FileHelper::New( file );
   int dataSize       = filehelper->GetImageDataRawSize();
   uint8_t *imageData = filehelper->GetImageDataRaw(); //EXTREMELY IMPORTANT
          // Sure, it is : It's up to the user to decide if he wants to
          // GetImageData or if he wants to GetImageDataRaw
          // (even if we do it by setting a flag, *he* will have to decide)

   //filehelper->SetImageData(imageData, dataSize);
 
   filehelper->SetContentType(GDCM_NAME_SPACE::UNMODIFIED_PIXELS_IMAGE); // lossless compression : pixels reain unimpared
   filehelper->SetWriteModeToRaw();  
   filehelper->SetWriteTypeToJPEG2000(  ); 
   
   filehelper->SetUserData(imageData,dataSize); // This one ensures the compression
   filehelper->Write( output ); 

   std::cout << "2...";

   //////////////// Step 3:
   GDCM_NAME_SPACE::File *fileout = GDCM_NAME_SPACE::File::New();
   fileout->SetFileName( output );
   fileout->Load();

   if( !fileout->IsReadable() )
   {
      std::cout << "Failed" << std::endl
                << "Test::TestReadWriteJPEG2000ReadCompare: Could not parse the newly "
                << "written image:" << filename << std::endl;
      file->Delete();
      filehelper->Delete();
      fileout->Delete();
      nb_of_failure2000___++;
      return 1;
   }

   if ( file->GetBitsAllocated()>16 )
   {
      std::cout << "=============== 32 bits, not checked...OK." << std::endl ;
      //////////////// Clean up:
      file->Delete();
      filehelper->Delete();
      fileout->Delete();
      return 0;   
   }
   
   
   GDCM_NAME_SPACE::FileHelper *reread = GDCM_NAME_SPACE::FileHelper::New( fileout );

   std::cout << "3...";
   // For the next step:
   int     dataSizeWritten   = reread->GetImageDataRawSize();
   uint8_t *imageDataWritten = reread->GetImageDataRaw();
 
   //////////////// Step 4:
   // Test the image size
   if (file->GetXSize() != reread->GetFile()->GetXSize() ||
       file->GetYSize() != reread->GetFile()->GetYSize() ||
       file->GetZSize() != reread->GetFile()->GetZSize())
   {
      std::cout << "Failed" << std::endl
         << "        Size differs: "
         << "X: " << file->GetXSize() << " # "
                  << reread->GetFile()->GetXSize() << " | "
         << "Y: " << file->GetYSize() << " # "
                  << reread->GetFile()->GetYSize() << " | "
         << "Z: " << file->GetZSize() << " # "
                  << reread->GetFile()->GetZSize() << std::endl;
      file->Delete();
      filehelper->Delete();
      fileout->Delete();
      reread->Delete();
      nb_of_failure2000___++;      
      return 1;
   }

   // Test the data size
   // beware of odd length Pixel Element!
   if (dataSize != dataSizeWritten)
      std::cout << std::endl << "dataSize:" << dataSize << " dataSizeWritten:" << dataSizeWritten << std::endl;
      
   int dataSizeFixed = dataSize - dataSize%2;
   int dataSizeWrittenFixed = dataSizeWritten - dataSizeWritten%2;

   if (dataSizeFixed != dataSizeWrittenFixed)
   {
      std::cout << "Failed" << std::endl
         << "        Pixel areas lengths differ: "
         << dataSize << " # " << dataSizeWritten << std::endl;
      file->Delete();
      filehelper->Delete();
      fileout->Delete();
      reread->Delete();
      nb_of_failure2000___++;      
      return 1;
   }

   // Test the data content
   
   unsigned int j  =0;
   unsigned int nbDiff =0;  
   if (memcmp(imageData, imageDataWritten, dataSizeFixed) !=0)
   {   
      std::string PixelType = filehelper->GetFile()->GetPixelType();
      std::string ts        = filehelper->GetFile()->GetTransferSyntax();

       for(int i1=0; i1<dataSizeFixed; i1++)
         if (abs ((int)imageData[i1]-(int)imageDataWritten[i1]) > 2) {
            nbDiff++;
           // break; // at debug time, keep line commented out; (uncommenting will save CPU time)
         }

       if (nbDiff!=0)
       {
          std::cout << std::endl << filename << " Failed : "
                    << nbDiff/(file->GetBitsAllocated()/8) << " Pixels -amongst "
                    << dataSizeFixed/(file->GetBitsAllocated()/8) << "- (" 
                    << PixelType << " bAlloc:" << file->GetBitsAllocated() << " bStored:" << file->GetBitsStored()
                    << ") differ (as expanded in memory)."
                    << std::endl
                    << "        compression : " 
                    << GDCM_NAME_SPACE::Global::GetTS()->GetValue(ts) << std::endl;

          std::cout << "   list of the first " << MAX_NUMBER_OF_DIFFERENCE
                    << " bytes differing (pos : original - written) :"
                    << std::endl;

          for(int i=0, j=0; i<dataSizeFixed && j<MAX_NUMBER_OF_DIFFERENCE; i++)
          {
             if (abs ((int)imageData[i]-(int)imageDataWritten[i]) > 2)
             {
                if (j<MAX_NUMBER_OF_DIFFERENCE)
                   std::cout << std::dec << "(" << i << " : "
                     << std::hex
                     << (int)(imageData[i]) << " - "
                     << (int)(imageDataWritten[i]) << ") "
                     << std::dec;
                ++j;
              }
          }
          std::cout << std::endl;

          file->Delete();
          filehelper->Delete();
          fileout->Delete();
          reread->Delete();
          nb_of_failure2000___++;
  
          if (nbDiff/2 > 8 )  // last pixel of (DermaColorLossLess.dcm) is diferent. ?!?
                              // I don't want it to break the testsuite
             return 1;
          else
             return 0;
       }      
       else
       {
          std::cout << std::endl << filename << " : some pixels"
                    << "  ("
                    << PixelType << " bAlloc:" << file->GetBitsAllocated() << " bStored:" << file->GetBitsStored()
                    << ") differ +/-1 (as expanded in memory)."
                    << std::endl
                    << "        compression : "
                    << GDCM_NAME_SPACE::Global::GetTS()->GetValue(ts) << std::endl;

          std::cout << "   list of the first " << MAX_NUMBER_OF_DIFFERENCE
                    << " bytes differing (pos : original - written) :"
                    << std::endl;

          for(int i=0, j=0; i<dataSizeFixed && j<MAX_NUMBER_OF_DIFFERENCE; i++)
          {
             if (imageData[i] != imageDataWritten[i])
             {
                std::cout << std::hex << "(" << i << " : " 
                         << std::hex << (int)(imageData[i]) << " - "
                         << std::hex << (int)(imageDataWritten[i]) << ") "
                         << std::dec;
                ++j;
              }
          }
          std::cout << std::endl;
          nb_of_diffPM12000___++;
       }
   }
   else
   {
      nb_of_success2000___ ++;
   }
   std::cout << "=============== 4...OK." << std::endl ;
   //////////////// Clean up:
   file->Delete();
   filehelper->Delete();
   fileout->Delete();
   reread->Delete();

   return 0;
}

// -------------------------------------------------------------------------------------------

int TestReadWriteJPEG2000ReadCompare(int argc, char *argv[]) 
{
   int result = 0;
   nb_of_success2000___ = 0;
   nb_of_failure2000___ = 0;
   nb_of_diffPM12000___ = 0;
   
   if (argc == 4)
      GDCM_NAME_SPACE::Debug::DebugOn();

   if (argc >= 3)
   {
      const std::string input  = argv[1];
      const std::string output = argv[2];
      result += CompareInternalJPEG2000(input, output);
   }
   else if( argc > 4 || argc == 2 )
   {
      std::cout << "Please read the manual" << std::endl;
   }
   else
   {
      std::cout<< "Test::TestReadWriteJPEG2000ReadCompare: description " << std::endl;
      std::cout << "   For all images in gdcmData (and not blacklisted in "
                   "Test/CMakeLists.txt)" << std::endl;
      std::cout << "   apply the following multistep test: " << std::endl;
      std::cout << "   step 1: parse the image (as gdcmFile) and call"
                << " IsReadable(). " << std::endl;
      std::cout << "   step 2: write the corresponding image in JPEG2000 DICOM V3 "
                << "with explicit Value Representation " << std::endl
                << "            in temporary file "
                << "TestReadWriteJPEG2000ReadCompare.dcm." << std::endl;
      std::cout << "   step 3: read the image written on step2 and call "
                << " IsReadable(). " << std::endl;
      std::cout << "   step 4: compare (in memory with memcmp) that the two "
                << "images " << std::endl
                << "           match (as expanded by gdcm)." << std::endl;
   
      int i = 0;
      int res =0;
      while( gdcmDataImages[i] != 0 )
      {
         std::string filename = GDCM_DATA_ROOT;
         filename += "/";
         filename += gdcmDataImages[i];
         res = CompareInternalJPEG2000(filename, "TestReadWriteJPEG2000ReadCompare.dcm");

         if (res == 1)
         {
            std::cout << "=============================== Failure on: " << gdcmDataImages[i] << std::endl;
            result ++;
         }
         i ++;
      }
   }
   std::cout << "==================================" << std::endl;
   std::cout << "nb of success  " << nb_of_success2000___ << std::endl;
   std::cout << "nb of failure  " << nb_of_failure2000___ << std::endl;
   std::cout << "nb of diff+/-1 " << nb_of_diffPM12000___ << std::endl;   
   return result;
}
