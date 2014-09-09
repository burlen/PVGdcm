/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: exGC.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:05 $
  Version:   $Revision: 1.11 $
                                                                                
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
#include "gdcmDataEntry.h"
#include "gdcmSeqEntry.h"

#include <stdlib.h> // for exit

typedef struct  // Maybe we should add it to gdcm ?
{
   uint8_t r;
   uint8_t g;
   uint8_t b;
} rgb8_t;


// This small application, for a given Creatis user (G.C.)
// may be taken as an example

int main(int argc, char *argv[])
{

   // we need a user friendly way for passign parameters on the command line !

   std::cout << "------------------------------------------------" << std::endl;
   std::cout << "Transforms a full gdcm-readable 'color' Dicom image "
          << " (e.g Palette Color, YBR, 3-Planes RGB) "
          << " into an 'RGB_Pixels' Dicom Image " << std::endl
          << " Blacks out any 'grey' pixel (r=g=b) "
          << " Blacks out any 'dark' pixel (r,g,b < threshold) " 
          << " "            << std::endl;
   std::cout << "------------------------------------------------" << std::endl;

   if (argc < 3)
   {
      std::cerr << "Usage :" << std::endl << 
      argv[0] << " input_dicom output_dicom threshold background" << std::endl;
      return 1;
   }

   std::string fileName = argv[1];
   std::string output   = argv[2];

   int threshold = 0;
   if (argc > 3)
      threshold        = atoi( argv[3] );
  
   int background = 0;
   if (argc > 4)
      background        = atoi( argv[4] );

// ============================================================
//   Read the input image.
// ============================================================
   // a GDCM_NAME_SPACE::File contains all the Dicom Field but the Pixels Element

   std::cout << argv[1] << std::endl;

   GDCM_NAME_SPACE::File *f = GDCM_NAME_SPACE::File::New();
   f->SetLoadMode( GDCM_NAME_SPACE::LD_ALL);
   f->SetFileName( fileName );
   bool res = f->Load();        

   if (!res) {
       std::cerr << "Sorry, " << fileName <<"  not a gdcm-readable "
                 << "DICOM / ACR File"
                 <<std::endl;
       f->Delete();
       return 0;
   }
   std::cout << " ... is readable " << std::endl;

/*
   if (!f->IsMonochrome()) {
       std::cerr << "Sorry, " << fileName <<"  not a 'color' File "
           << " "
                 <<std::endl;
       return 0;
   }
*/

// ============================================================
//   Load the pixels in memory.
// ============================================================

   // We need a GDCM_NAME_SPACE::FileHelper, since we want to load the pixels        
   GDCM_NAME_SPACE::FileHelper *fh = GDCM_NAME_SPACE::FileHelper::New(f);

   // (unit8_t DOESN'T mean it's mandatory for the image to be a 8 bits one) 

   uint8_t *imageData = fh->GetImageData();

   if ( imageData == 0 )
   {
       std::cerr << "Sorry, Pixels of" << fileName <<"  are not "
                 << " gdcm-readable."  << std::endl;
       f->Delete();
       fh->Delete();
       return 0;
   }

   // ------ User wants write a new image without shadow groups -------------
   // ------                              without Sequences     -------------

 
   GDCM_NAME_SPACE::FileHelper *copy = GDCM_NAME_SPACE::FileHelper::New( );
   copy->SetFileName( output );
   copy->Load();
 
   GDCM_NAME_SPACE::DocEntry *d = f->GetFirstEntry();
   while(d)
   {
      // We skip SeqEntries, since user cannot do much with them
      if ( !(dynamic_cast<GDCM_NAME_SPACE::SeqEntry*>(d))
      // We skip Shadow Groups, since nobody knows what they mean
           && !( d->GetGroup()%2 ) )
      { 

         if ( GDCM_NAME_SPACE::DataEntry *de = dynamic_cast<GDCM_NAME_SPACE::DataEntry *>(d) )
         {              
            copy->GetFile()->InsertEntryBinArea( de->GetBinArea(),de->GetLength(),
                                                 de->GetGroup(),de->GetElement(),
                                                 de->GetVR() );
         }
         else
         {
          // We skip GDCM_NAME_SPACE::SeqEntries
         }
      }
      d = f->GetNextEntry();
   }

   int imageSize = fh->GetImageDataSize();
// Black up all 'grey' pixels
   int i;
   int n = 0;
   for (i = 0; i<imageSize/3; i++)
   {
      if ( ((rgb8_t *)imageData)[i].r == ((rgb8_t *)imageData)[i].g
         &&
           ((rgb8_t *)imageData)[i].r == ((rgb8_t *)imageData)[i].b )
      {
         n++;
         ((rgb8_t *)imageData)[i].r = (unsigned char)background;
         ((rgb8_t *)imageData)[i].g = (unsigned char)background;
         ((rgb8_t *)imageData)[i].b = (unsigned char)background;
      }
   }
   
   std::cout << n << " points put to black (within " 
             << imageSize/3 << ")" << std::endl;

   n = 0;
   for (i = 0; i<imageSize/3; i++)
   {
      if ( ((rgb8_t *)imageData)[i].r < threshold
         &&
         ((rgb8_t *)imageData)[i].g < threshold
         &&
         ((rgb8_t *)imageData)[i].b < threshold )
      {
         n++;
        ((rgb8_t *)imageData)[i].r = (unsigned char)background;
        ((rgb8_t *)imageData)[i].g = (unsigned char)background;
        ((rgb8_t *)imageData)[i].b = (unsigned char)background;  
      }
   }
   
   std::cout << n << " points put to black (within " 
             << imageSize/3 << ")" << std::endl; 
   // User knows the image is a 'color' one -RGB, YBR, Palette Color-
   // and wants to write it as RGB
   copy->SetImageData(imageData, imageSize);
   copy->SetWriteModeToRGB();

   copy->WriteDcmExplVR( output );

   f->Delete();
   fh->Delete();
   copy->Delete();

   exit (0);
}
