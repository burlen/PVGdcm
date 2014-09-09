/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: exReadWriteFile.cxx,v $
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


std::cout << " --- WARNING --- WARNING --- WARNING --- WARNING ---" <<std::endl;
std::cout << " "                                                    <<std::endl; 
std::cout << " This source program is NOT intendend to be run as is"<<std::endl;
std::cout << " "                                                    <<std::endl;
std::cout << " It just shows a set of possible uses."               <<std::endl;
std::cout << "User MUST read it, "                                  <<std::endl;
std::cout << "          comment out the useless parts "             <<std::endl;
std::cout << "          invoke it with an ad hoc image(*) "         <<std::endl;
std::cout << "           check the resulting image   "              <<std::endl; 
std::cout << "  "                                                   <<std::endl;
std::cout << " (*) For samples, user can refer to gdcmData"         <<std::endl;
std::cout << "         and read README.txt file "                   <<std::endl;
std::cout << " "                                                    <<std::endl;
std::cout << "This source program will be splitted into smaller elementary" 
          <<  " programs"                                           <<std::endl;
std::cout << " "                                                    <<std::endl;
std::cout << " --- WARNING --- WARNING --- WARNING --- WARNING ---" <<std::endl;

   if (argc < 3)
   {
      std::cerr << "Usage :" << std::endl << 
      argv[0] << " input_dicom output_dicom" << std::endl;
      return 1;
   }

   std::string filename = argv[1];
   std::string output   = argv[2];

   // First, let's create a GDCM_NAME_SPACE::File
   // that will contain all the Dicom fields but the Pixels Element

   GDCM_NAME_SPACE::File *f1= GDCM_NAME_SPACE::File::New( );
   f1->SetFileName( filename );
   f1->Load();


   // Ask content to be printed
   std::cout << std::endl
             << "--- Standard Print -------------------------------------------"
             << std::endl;
   f1->SetPrintLevel(2);   // to have a nice output
   //f1->SetPrintLevel(1); 
   f1->Print();            // user may comment out if too much verbose


  // User asks for field by field Printing
  
   std::cout << std::endl
             << "--- Display only human readable values -----------------------"
             << std::endl;

   GDCM_NAME_SPACE::DataEntry *dataEntry;
   std::string value;
   GDCM_NAME_SPACE::VRKey vr;   // value representation
   std::string vm;   // value multiplicity
   std::string name; // held in the Dicom Dictionary


   GDCM_NAME_SPACE::DocEntry *d = f1->GetFirstEntry();
   while( d )
   {
      // We skip SeqEntries, since user cannot do much with them
      if ( !(dynamic_cast<GDCM_NAME_SPACE::SeqEntry*>(d))
      // We skip Shadow Groups, since nobody knows what they mean
           && !( d->GetGroup()%2 ) )
     {      
         // If user just 'wants to see'
         //d->Print();
         //std::cout << std::endl;

         // If user wants to get info about the entry
         // (he is sure, here that DocEntry is a DataEntry)
         dataEntry = dynamic_cast<GDCM_NAME_SPACE::DataEntry *>(d);
         // Let's be carefull -maybe he commented out some previous line-
         if (!dataEntry)
            continue;

         value  = dataEntry->GetString();
         vr     = dataEntry->GetVR();
         // user wants really to know everything about entry!
         vm     = dataEntry->GetVM();
         name   = dataEntry->GetName();

         std::cout //<< std::hex << group << "," << elem 
             << dataEntry->GetKey()
             << "     VR :[" << vr    << "] VM : [" << vm 
             << "] name : [" << name  << "]"
             << " value : [" << value << "]" 
             << std::endl;
      }
      d = f1->GetNextEntry();
   }

   std::cout << std::endl
             << "--- Use pre-defined acessors ---------------------------------"
             << std::endl;
 
   // ------ some pre-defined acessors may supply usefull informations ----

   // about Image
   int linesNumber   = f1->GetYSize();
   int rawsNumber    = f1->GetXSize();
   int framesNumber  = f1->GetYSize();// defaulted to 1 if not found

   std::cout << "lines : "   << linesNumber  << " columns : " << rawsNumber
             << " frames : " << framesNumber << std::endl;
 
   // about Pixels
   int pixelSize         = f1->GetPixelSize(); 
   std::string pixelType = f1->GetPixelType();
   bool isSigned         = f1->IsSignedPixelData();
  
   std::cout << "pixelSize : "   << pixelSize  << " pixelType : " << pixelType
             << " signed : "     << isSigned   << std::endl;
 
   // about pixels, too.
   // Better you forget these ones
  
   std::cout << "GetBitsStored()"      << f1->GetBitsStored()      << std::endl;
   std::cout << "GetBitsAllocated()"   << f1->GetBitsAllocated()   << std::endl;
   std::cout << "GetHighBitPosition()" << f1->GetHighBitPosition() << std::endl;

   std::cout << "GetSamplesPerPixel()"     
          << f1->GetSamplesPerPixel()     << std::endl;
   std::cout << "GetPlanarConfiguration()" 
          << f1->GetPlanarConfiguration() << std::endl; 
 
   // about 'image geography'
 
   float xs = f1->GetXSpacing();
   float ys = f1->GetYSpacing();
   float zs = f1->GetZSpacing();  // defaulted to 1.0 if not found

   float xo = f1->GetXOrigin();
   float yo = f1->GetYOrigin();
   float zo = f1->GetZOrigin();

   std::cout << "GetXSpacing()"     << xs      << std::endl;
   std::cout << "GetYSpacing()"     << ys      << std::endl;
   std::cout << "GetXSpacing()"     << zs      << std::endl;

   std::cout << "GetXOrigin()"      << xo      << std::endl;
   std::cout << "GetYOrigin()"      << yo      << std::endl;
   std::cout << "GetZOrigin()"      << zo      << std::endl;

   // about its way to store colors (if user is aware)

   // checks Photometric Interpretation
   std::cout << "IsMonochrome()"   << f1->IsMonochrome()     << std::endl;
   std::cout << "IsYBRFull()"      << f1->IsYBRFull()        << std::endl;
   std::cout << "IsPaletteColor()" << f1->IsPaletteColor()   << std::endl;
   // checks if LUT are found
   std::cout << "HasLUT()"         << f1->HasLUT()           << std::endl;

   std::cout << "GetNumberOfScalarComponents()"    
          << f1->GetNumberOfScalarComponents()<< std::endl;
   std::cout << "GetNumberOfScalarComponentsRaw()" 
          << f1->GetNumberOfScalarComponentsRaw()<< std::endl;
  

   std::cout << std::endl
             << "--- Get values on request ------------------------------------"
             << std::endl;
   // ------ User is aware, and wants to get fields with no accesor --------

   std::cout << "Manufacturer :["     << f1->GetEntryString(0x0008,0x0070)
             << "]" << std::endl; 
   std::cout << "Institution :["      << f1->GetEntryString(0x0008,0x0080)
             << "]" << std::endl;
   std::cout << "Patient's name :["   << f1->GetEntryString(0x0010,0x0010)
             << "]" << std::endl;
   std::cout << "Physician's name :[" << f1->GetEntryString(0x0008,0x0090)
             << "]" << std::endl; 
   std::cout << "Study Date :["       << f1->GetEntryString(0x0008,0x0020)
             << "]" << std::endl; 
   std::cout << "Study inst UID :["   << f1->GetEntryString(0x0020,0x000d)
             << "]" << std::endl;
   std::cout << "Serie inst UID :["   << f1->GetEntryString(0x0020,0x000e)
             << "]" << std::endl;
   std::cout << "Frame ref UID :["   << f1->GetEntryString(0x0020,0x0052)
             << "]" << std::endl;
 
   // User wants to get info about the 'real world' vs image


   // ------ User wants to load the pixels---------------------------------
   
   // Hope now he knows enought about the image ;-)

   // First, create a GDCM_NAME_SPACE::FileHelper
   GDCM_NAME_SPACE::FileHelper *fh1 = GDCM_NAME_SPACE::FileHelper::New(f1);

   // Load the pixels, transforms LUT (if any) into RGB Pixels 
   uint8_t *imageData = fh1->GetImageData();
   // Get the image data size
   size_t dataSize    = fh1->GetImageDataSize();

   // Probabely, a straigh user won't load both ...

   // Load the pixels, DO NOT transform LUT (if any) into RGB Pixels 
   uint8_t *imageDataRaw = fh1->GetImageDataRaw();
   // Get the image data size
   size_t dataRawSize    = fh1->GetImageDataRawSize();

   // TODO : Newbee user would appreciate any comment !
 
   std::cout << "GetImageDataSize()"    
          << fh1->GetImageDataSize()    << std::endl;
   std::cout << "GetImageDataRawSize()" 
          << fh1->GetImageDataRawSize() << std::endl;
   // User Data
   std::cout << "GetRGBDataSize()"      
          << fh1->GetRGBDataSize()      << std::endl;
   std::cout << "GetRawDataSize()"      
          << fh1->GetRawDataSize()      << std::endl;
   std::cout << "GetUserDataSize()"     
          << fh1->GetUserDataSize()     << std::endl;
 

   std::cout << std::endl
             << "--- write a new image(1) -------------------------------------"
             << std::endl;
 
   // ------ User wants write a new image without shadow groups -------------

   GDCM_NAME_SPACE::FileHelper *copy = GDCM_NAME_SPACE::FileHelper::New( );
   copy->SetFileName( output );
   copy->Load();
 
   d = f1->GetFirstEntry();
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
      d = f1->GetNextEntry();
   }

   std::cout << std::endl
             << "--- write a new image(2) -------------------------------------"
             << std::endl;
 
   // User knows the image is a 'color' one -RGB, YBR, Palette Color-
   // and wants to write it as RGB
   copy->SetImageData(imageData, dataSize);
   copy->SetWriteModeToRGB();
   copy->WriteDcmExplVR( output );

   // User wants to see if there is any difference before and after writting

   std::cout << "GetImageDataSize()"    
          << fh1->GetImageDataSize()    << std::endl;
   std::cout << "GetImageDataRawSize()" 
          << fh1->GetImageDataRawSize() << std::endl;
   // User Data
   std::cout << "GetRGBDataSize()"      
          << fh1->GetRGBDataSize()      << std::endl;
   std::cout << "GetRawDataSize()"      
          << fh1->GetRawDataSize()      << std::endl;
   std::cout << "GetUserDataSize()"     
          << fh1->GetUserDataSize()     << std::endl;
   // User wants to keep the Palette Color -if any- 
   // and write the image as it was
   copy->SetImageData(imageDataRaw, dataRawSize);
   copy->SetWriteModeToRaw();
   copy->WriteDcmExplVR( output );


   std::cout << std::endl
             << "------------------------------------------------------------"
             << std::endl;
   // User is in a fancy mood and wants to forge a bomb image
   // just to see how other Dicom viewers act


   // TODO : finish it 


   std::cout << std::endl
             << "------------------------------------------------------------"
             << std::endl;
   f1->Delete();
   fh1->Delete();
   copy->Delete();

   exit (0);
}

