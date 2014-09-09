/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: exGrey2RGB.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:05 $
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
#include "gdcmDocument.h"
#include "gdcmDebug.h"

#ifndef _WIN32
#include <unistd.h> //for access, unlink
#else
#include <io.h> //for _access
#endif

// return true if the file exists
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

// Here we load a gdcmFile (8 or 16 Bits) and convert it into a 8 or 16 RGB file

int main(int argc, char *argv[])
{
   if (argc < 3)
   {
      std::cerr << "Usage :" << std::endl << 
      argv[0] << " input_dicom output_dicom" << std::endl;
      return 1;
   }

   //GDCM_NAME_SPACE::Debug::DebugOn();

   std::string filename = argv[1];
   std::string output   = argv[2];

   if( FileExists( output.c_str() ) )
   {
      std::cerr << "Don't try to cheat, I am removing the file anyway" << std::endl;
      if( !RemoveFile( output.c_str() ) )
      {
         std::cerr << "Ouch, the file exist, but I cannot remove it" << std::endl;
         return 1;
      }
   }
 
   GDCM_NAME_SPACE::FileHelper *fh = GDCM_NAME_SPACE::FileHelper::New( );
   fh->SetFileName( filename );
   fh->Load();
      
   size_t dataSize    = fh->GetImageDataSize();
   uint8_t *imageData = fh->GetImageData();

   uint8_t *imageDataRGB = new uint8_t[dataSize*3];

   if (fh->GetFile()->GetEntryString(0x0028,0x0100) == "8" )
   {
      for (unsigned int i=0;i<dataSize;i++)
      {
         imageDataRGB[i*3]=imageDataRGB[i*3+1]
                          =imageDataRGB[i*3+2]
                          =imageData[i]; 
      }
   }
   else
   {
      for (unsigned int i=0;i<dataSize/2;i++)
      {
        //std::cout << i << std::endl;
         ((uint16_t *)imageDataRGB)[i*3]=((uint16_t *)imageDataRGB)[i*3+1]
                                        =((uint16_t *)imageDataRGB)[i*3+2]
                                        =((uint16_t *)imageData)[i]; 
      }
   }
   // Samples Per Pixel  
   fh->GetFile()->InsertEntryString( "3 " ,0x0028,0x0002);
   // Photometric Interpretation
   fh->GetFile()->InsertEntryString( "RGB ",0x0028,0x0004 );
   // Planar Configuration
   fh->GetFile()->InsertEntryString( "1 ",0x0028,0x0006 );

   // TODO  : free existing PixelData first !

   fh->SetImageData(imageDataRGB, dataSize*3);   
   fh->WriteDcmExplVR( output );

   fh->Delete();

   return 0;
}


