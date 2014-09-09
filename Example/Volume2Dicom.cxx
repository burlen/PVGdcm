/*=========================================================================
                                                                                 
  Program:   gdcm
  Module:    $RCSfile: Volume2Dicom.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/30 11:05:47 $
  Version:   $Revision: 1.13 $
                                                                                 
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                 
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                 
=========================================================================*/

/**
 * This example was proposed by Jean-Michel Rouet
 * It was patched by Mathieu Malaterre to remove ITK reference and be more
 * independant from other toolkit
 * It's aim is to show people how to write their data volume into DICOM slices
 */

#include "gdcmFile.h"
#include "gdcmDocEntry.h"
#include "gdcmFileHelper.h"
#include "gdcmUtil.h"

#define USAGE "USAGE: Input3DImage OutputDirectory"

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
   #define stat _stat
#endif

#if defined(__BORLANDC__)
 #include <mem.h> // for memset, memcpy
#endif 

//const unsigned int Dimension = 3;

void gdcmwrite(const char *inputfile, std::string directory);
void GetFileDateAndTime(const char *inputfile, 
                        std::string &date, std::string &time);

int main( int argc, char *argv[] )
{
   if (argc < 2) 
   {
      std::cerr << argv[0] << USAGE << std::endl;
      return 1;
   }    

   //const char *inputfile = argv[1];
   std::string directory = argv[1];
   //   itksys::SystemTools::ConvertToUnixSlashes( directory );
   if (directory[directory.size()-1] != '/') 
   directory += '/';

   std::cout << "Converting image into dicom in " << directory << std::endl;

   ////////////////////////////////////////////////////////////
   // Reading input image and getting some information       //
   ////////////////////////////////////////////////////////////
   //std::cout << "Loading image " << inputfile << std::endl;
   //PixelType* imageData = input->GetPixelContainer()->GetImportPointer();
   uint8_t *imageData = new uint8_t[256*256*10];
   memset( imageData, 0, 256*256*10);
   std::cout << "Image Loaded." << std::endl;

   int   sizex      = 256;
   int   sizey      = 256;
   int   sizez      = 10;
   //float spacing[3] = { 1.0, 1.0, 1.5 };
   //float orig[3]    = { 0.0, 0.0, 0.0 };
   int   sliceSize  = sizex*sizey*sizeof(uint8_t);

   ////////////////////////////////////////////////////////////
   // compute window center and window width                 //
   ////////////////////////////////////////////////////////////
   uint8_t min, max; min = max = imageData[0];
   for (int i=1; i<sizex*sizey*sizez; i++) 
   {
      uint8_t val = imageData[i];
      if (val > max) 
         max = val;
      if (val < min) 
         min = val;
   }
   //float wcenter = (max+min) / 2.;
   //float wwidth  = (max-min)>0 ? (max-min) : 1.;

   ////////////////////////////////////////////////////////////
   // Get file date and time                                 //
   ////////////////////////////////////////////////////////////
   std::string filedate, filetime;    
   //GetFileDateAndTime(inputfile, filedate, filetime);

   ////////////////////////////////////////////////////////////
   // Create a new dicom header and fill in some info        //
   ////////////////////////////////////////////////////////////
   GDCM_NAME_SPACE::File *f = GDCM_NAME_SPACE::File::New();

   ////////////////////////////////////////////////////////////
   // Create a new dicom file object from the header         //
   ////////////////////////////////////////////////////////////
   GDCM_NAME_SPACE::FileHelper  *fh = GDCM_NAME_SPACE::FileHelper::New(f);
   uint8_t *myData = fh->GetImageData(); // Get an Image pointer
   fh->SetImageData( myData, sliceSize); // This callback ensures that the internal
                                        // Pixel_Data of fh is set correctly


   ////////////////////////////////////////////////////////////
   // Iterate through the slices and save them to file       //
   ////////////////////////////////////////////////////////////
   for (int z=0; z<sizez; z++) 
   {
      // Set dicom relevant information for that slice
      //f->SetImageUIDFromSliceNumber(z);
      //f->SetImageLocation(orig[0],orig[1],orig[2]+z*spacing[2]);

      // copy image slice content
      memcpy(myData,imageData+z*sizex*sizey,sliceSize);

      // write the image
      std::string filename = directory + GDCM_NAME_SPACE::Util::Format("%Image_%05d.dcm", z);
      std::cout << "Writing file " << filename;
      fh->WriteDcmExplVR(filename);
      std::cout << " OK" << std::endl;
   }

   ////////////////////////////////////////////////////////////
   // Free the allocated objects                             //
   ////////////////////////////////////////////////////////////
   fh->Delete();
   f->Delete();

   return 0;
}


// just an utility function to retrieve date and time of a file
void GetFileDateAndTime(const char *inputfile, std::string &date, 
                                               std::string &time)
{
   struct stat buf;    
   if (stat(inputfile, &buf) == 0) 
   {
      char tmp[512];

      strftime(tmp,512,"%Y%m%d", localtime(&buf.st_mtime) );
      date = tmp;

      strftime(tmp,512,"%H%M%S", localtime(&buf.st_mtime) );
      time = tmp;
   }
   else
   {
      date = "20040101";
      time = "120000";
   }
}
