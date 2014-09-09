/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: exImageLighten.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:05 $
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
#include "gdcmDocument.h"
#include "gdcmDataEntry.h"
#include "gdcmSeqEntry.h"

#include <stdlib.h> // for exit

int main(int argc, char *argv[])
{ 
   std::cout << "-----------------------------------------------" << std::endl;
   std::cout << "Removes from a full gdcm-readable  Dicom image"  << std::endl;
   std::cout << " all the 'Shadow groups' and the 'Sequence' entries"
          << " Warning :  'Compressed images' are uncompressed" 
          << std::endl
          << "         :  'YBR images' are converted into 'RGB images'"
          << std::endl
          << "         :  'RGB planes' are converted into 'RGB pixels'"
          << std::endl
          << "         :  'Palette Color images' are kept 'as is'"
          << std::endl;
   std::cout << "-----------------------------------------------" << std::endl;

   if( argc < 3 )
    {
       std::cerr << "Usage " << argv[0]  << " Source image.dcm  " 
                 << " Output image.dcm " << std::endl;
       return 1;
    }

   std::string fileName       = argv[1];
   std::string output         = argv[2];

// ============================================================
//   Read the input image.
// ============================================================

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

// ============================================================
//   Read the Pixels
//
// ============================================================
 
   // Pixel Reading must be done here, to be sure 
   // to load the Palettes Color (if any)

   // First, create a GDCM_NAME_SPACE::FileHelper
   GDCM_NAME_SPACE::FileHelper *fh = GDCM_NAME_SPACE::FileHelper::New(f);

   // Load the pixels, DO NOT transform LUT (if any) into RGB Pixels 
   uint8_t *imageDataRaw = fh->GetImageDataRaw();
   // Get the image data size
   size_t dataRawSize    = fh->GetImageDataRawSize();

// ============================================================
//   Create a new GDCM_NAME_SPACE::Filehelper, to hold new image.
// ============================================================

   GDCM_NAME_SPACE::FileHelper *copy = GDCM_NAME_SPACE::FileHelper::New( );
   copy->SetFileName( output );
   copy->Load();

// ============================================================
//   Selective copy of the entries (including Pixel Element).
// ============================================================

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
   
   // User wants to keep the Palette Color -if any- 
   // and write the image as it was
   copy->SetImageData(imageDataRaw, dataRawSize);
   copy->SetWriteModeToRaw();
   copy->WriteDcmExplVR( output );

   std::cout << std::endl
             << "------------------------------------------------------------"
             << std::endl;

   f->Delete();
   fh->Delete();
   copy->Delete();

   exit (0);
}

