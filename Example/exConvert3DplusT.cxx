/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: exConvert3DplusT.cxx,v $
  Language:  C++
  Date:      $Date: 2008/02/13 19:02:39 $
  Version:   $Revision: 1.6 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
//
// This program is used to convert huge amounts of functionnal (3D + T) MR images
// into 'image tiles' (one title per volume) processable by clinical softwares
//
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmCommon.h"
#include "gdcmDebug.h"
#include "gdcmDirList.h"
#include "gdcmUtil.h"

#include "gdcmArgMgr.h"

#include <iostream>
#include <sstream>
#include <string.h> // for memset

int main(int argc, char *argv[])
{
   START_USAGE(usage)
   "\n exConvert3plusT :\n                                                    ",
   " Converts the Dicom files inside a single-level Directory                 ",
   " into a '3D + T' image set                                                ",
   " usage: exConvert3plusT                                                   ",
   "              dirin=inputDirectoryName                                    ",
   "              dirout=outputDirectoryName                                  ",
   "              [studyUID = ] [patName = ]                                  ",
   "              [ { [noshadowseq] | [noshadow][noseq] } ]  [debug] [verbose]",
   "                                                                          ",
   "       dirin : single-level Directory containing the images               ",
   "                                        (no recursive parsing)            ",
   "       dirout : will be created if doesn't exist                          ",
   "                                                                          ",
   "       [imdimx = ] [imdimy = ] [imgline = ] [imgcol = ]                   ",
   "       [pixelsize = ] [imagesinvolume = ]                                 ",
   "                                                                          ",
   "       studyUID   : *aware* user wants to add the serie                   ",
   "                                             to an already existing study ",
   "       imdimx,imdimy : 'elementary image' size (default : 64)             ",
   "                        used to reject erroneous images                   ",
   "       imgline, imgcol : sizes of the 'image tile' defaulted as 6x6       ",
   "       pixelsize : in bytes; defaulted as 2 (uint16_t)                    ",
   "                        used to reject erroneous images                   ",
   "       imagesinvolume : we have no way to guess it!                       ",
   "                        defaulted as imgline*imgcol                       ",
   "       noshadowseq: user doesn't want to load Private Sequences           ",
   "       noshadow   : user doesn't want to load Private groups (odd number) ",
   "       noseq      : user doesn't want to load Sequences                   ",
   "       verbose    : user wants to run the program in 'verbose mode'       ",
   "       debug      : developper wants to run the program in 'debug mode'   ",
   FINISH_USAGE
     // ----- Initialize Arguments Manager ------
   GDCM_NAME_SPACE::ArgMgr *am = new GDCM_NAME_SPACE::ArgMgr(argc, argv);
   if (am->ArgMgrDefined("usage") || argc == 1) 
   {
      am->ArgMgrUsage(usage); // Display 'usage'
      delete am;
      return 0;
   }

   if (am->ArgMgrDefined("debug"))
      GDCM_NAME_SPACE::Debug::DebugOn(); 
   int verbose  = am->ArgMgrDefined("verbose");
   int oververbose  = am->ArgMgrDefined("oververbose");
   std::string patName = am->ArgMgrGetString("patname", "g^PatientName");
   //float zSpacing = am->ArgMgrGetFloat("zSpacing", 1.0);

   const char *dirIn   = am->ArgMgrGetString("dirin");
   const char *dirOut  = am->ArgMgrGetString("dirout");

   bool userDefinedStudy = am->ArgMgrDefined("studyUID");
   const char *studyUID  = am->ArgMgrGetString("studyUID");

// not described *on purpose* in the Usage !
   bool userDefinedSerie = am->ArgMgrDefined("serieUID");
   const char *serieUID  = am->ArgMgrGetString("serieUID");
   

   int imageDimX          = am->ArgMgrGetInt("imdimx",64);
   int imageDimY          = am->ArgMgrGetInt("imdimy",64); 
   int imagePixelSize     = am->ArgMgrGetInt("pixelsize", 2);
   int imagetteLineNumber = am->ArgMgrGetInt("imgline",6);
   int imagetteRowNumber  = am->ArgMgrGetInt("imgcol",6);
   int nbOfImagesInVolume = am->ArgMgrGetInt("imagesinvolume",
                                         imagetteLineNumber*imagetteRowNumber);
 
   int loadMode = GDCM_NAME_SPACE::LD_ALL;
   if ( am->ArgMgrDefined("noshadowseq") )
      loadMode |= GDCM_NAME_SPACE::LD_NOSHADOWSEQ;
   else
   {
   if ( am->ArgMgrDefined("noshadow") )
         loadMode |= GDCM_NAME_SPACE::LD_NOSHADOW;
      if ( am->ArgMgrDefined("noseq") )
         loadMode |= GDCM_NAME_SPACE::LD_NOSEQ;
   }

   /* if unused Param we give up */
   if ( am->ArgMgrPrintUnusedLabels() )
   {
      am->ArgMgrUsage(usage);
      delete am;
      return 0;
   }

   delete am;  // ------ we don't need Arguments Manager any longer ------
 // ====== Deal with a (single level, single Patient) Directory ======
   //std::cout << "dirIn [" << dirIn << "]" << std::endl;
   if ( ! GDCM_NAME_SPACE::DirList::IsDirectory(dirIn) )
   {
      std::cout << "KO : [" << dirIn << "] is not a Directory." << std::endl;
      return 0;
   }
   else
   {
      if (verbose)     
         std::cout << "OK : [" << dirIn << "] is a Directory." << std::endl;
   }

   std::string systemCommand;
   std::string strDirNameout(dirOut);          // to please gcc 4
   if (verbose)
      std::cout << "Check for output directory :[" << dirOut << "]."
             <<std::endl;
   if ( ! GDCM_NAME_SPACE::DirList::IsDirectory(dirOut) )    // dirout not found
   {
      systemCommand = "mkdir " +strDirNameout;        // create it!
      if (verbose)
         std::cout << systemCommand << std::endl;
      system (systemCommand.c_str());
      if ( ! GDCM_NAME_SPACE::DirList::IsDirectory(dirOut) ) // be sure it worked
      {
          std::cout << "KO : not a dir : [" << dirOut 
                    << "] (creation failure ?)" << std::endl;
         return 0;
      }
      else
      {
        if (verbose)
           std::cout << "Directory [" << dirOut << "] created." << std::endl;
      }
   }
   else
   {
      if (verbose)
         std::cout << "Output Directory [" << dirOut
                   << "] already exists; Used as is." << std::endl;
   }

   GDCM_NAME_SPACE::DirList dirList(dirIn,false); // gets (at single level) the file list
   GDCM_NAME_SPACE::DirListType fileList = dirList.GetFilenames();
   // hope sorting on the filename is enough!
   // anyway, *no* filed is available to perform anything more clever.
   std::sort(fileList.begin(), fileList.end() );

   // 'Study Instance UID'
   // The user is allowed to create his own Study, 
   //          keeping the same 'Study Instance UID' for various images
   // The user may add images to a 'Manufacturer Study',
   //          adding new Series to an already existing Study
   std::string strStudyUID;
   if ( !userDefinedStudy)
      strStudyUID =  GDCM_NAME_SPACE::Util::CreateUniqueUID();
   else
      strStudyUID = studyUID;

   // 'Serie Instance UID'
   // The user is allowed to create his own Series,
   // keeping the same 'Serie Instance UID' for various images
   // The user shouldn't add any image to a 'Manufacturer Serie'
   // but there is no way no to prevent him for doing that
   std::string strSerieUID;
   if ( !userDefinedSerie)
      strSerieUID =  GDCM_NAME_SPACE::Util::CreateUniqueUID();
   else
      strSerieUID = serieUID;

   std::ostringstream str;
   size_t totalNumberOfPixels =  imageDimX*imageDimY * imagetteLineNumber*imagetteRowNumber;
   int16_t *imageTable = new int16_t[totalNumberOfPixels];

   memset(imageTable, 0, totalNumberOfPixels * imagePixelSize);

   int16_t **tabImageData = new int16_t *[nbOfImagesInVolume];
   GDCM_NAME_SPACE::File **f         = new GDCM_NAME_SPACE::File *[nbOfImagesInVolume];
   GDCM_NAME_SPACE::FileHelper **fh  = new GDCM_NAME_SPACE::FileHelper *[nbOfImagesInVolume];

   std::string fullFilename, lastFilename;
   float zPositionComponent = 0.0;
   int imageNumber = 0;
   for( GDCM_NAME_SPACE::DirListType::iterator it  = fileList.begin();
                                 it != fileList.end();
                                 ++it )
   {
      fullFilename = *it;
      f[imageNumber] = GDCM_NAME_SPACE::File::New( );
      f[imageNumber]->SetLoadMode(loadMode);
      f[imageNumber]->SetFileName( it->c_str() );

      if (verbose)
         std::cout << "file [" << it->c_str() << "], as imageNumber : " << imageNumber << std::endl;

      if ( !f[imageNumber]->Load() )
      {
         if (verbose)
            std::cout << "fail to load [" << it->c_str() << "]" << std::endl;
         f[imageNumber]->Delete();
         continue;
      }

      // Load the pixels in RAM.

      fh[imageNumber] = GDCM_NAME_SPACE::FileHelper::New(f[imageNumber]);
      // Don't convert (Gray Pixels + LUT) into (RGB pixels) ?!?
   
      tabImageData[imageNumber] = (int16_t *)fh[imageNumber]->GetImageDataRaw();
      if (!tabImageData[imageNumber]) 
      {
         std::cout << "fail to read [" << it->c_str() << std::endl;
         continue;
      }
      int16_t mini=32000;
      int16_t maxi=-32000;
      if (imageNumber == nbOfImagesInVolume-1)
      {
         for(imageNumber=0; imageNumber < nbOfImagesInVolume; imageNumber++)
         {
            int debMove = (imageNumber%imagetteRowNumber) * imageDimX 
                + (imageNumber/imagetteRowNumber) *imageDimX*imageDimY*imagetteRowNumber;

            if (verbose)
               std::cout << "imageNumber " << imageNumber << " debMove " << debMove << std::endl;
            for(int i=0; i<imageDimY; i++)
            {
               int debLigne = debMove + i*imagetteRowNumber*imageDimX;
               //if (oververbose)
               //   std::cout << "numLigne " << i << " debLigne " << debLigne 
               //   << ": " << debMove << " + " << i << " * " << imagetteRowNumber*imageDimX << std::endl;
               for (int j=0; j<imageDimX; j++)
               {
                  //std::cout << "j " << j << std::endl;
                  imageTable[debLigne + j] = *(tabImageData[imageNumber] + i*imageDimY + j);
                  if (imageTable[debLigne + j] < 0) imageTable[debLigne + j]=0;
                     //std::cout << debLigne + j << " : " << imageTable[debLigne + j] << std::endl;
                     if (*(tabImageData[imageNumber] + i*imageDimY + j) < mini)
                                        mini=*(tabImageData[imageNumber] + i*imageDimY + j);
                     else if (*(tabImageData[imageNumber] + i*imageDimY + j) > maxi) 
                                        maxi=*(tabImageData[imageNumber] + i*imageDimY + j); 
             }
          }
      }
  //   if (oververbose)
  //    std::cout << " mini = " << mini << " maxi = " << maxi << std::endl;
  // just to check (actually, it's useless)
  /*
     int16_t mini=32000;
     int16_t maxi=-32000;
     for (int k=0; k < totalNumberOfPixels; k++)
     {
        if (imageTable[k] < mini) mini=imageTable[k];
        else if (imageTable[k] > maxi) maxi=imageTable[k];
    }

  // if (oververbose)
  //    std::cout << " mini = " << mini << " maxi = " << maxi << std::endl;
         /// \todo : FIXME what do I do ? Smallest/Largest Image Pixel Value is vr=US, Pixels are signed ?!?

         str.str("");
         str << mini;
         fh->InsertEntryString(str.str(),0x0028,0x0106, "US"); // Smallest Image Pixel Value

         str.str("");
         str << maxi;
         fh->InsertEntryString(str.str(),0x0028,0x0107, "US"); // Largest Image Pixel Value 
*/
         imageNumber = 0;

    // write the imagette
 
         // Set the image size
         str.str("");
         str << imageDimX*imagetteRowNumber;
         fh[imageNumber]->InsertEntryString(str.str(),0x0028,0x0011, "US"); // Columns

         str.str("");
         str << imageDimY*imagetteLineNumber;
         fh[imageNumber]->InsertEntryString(str.str(),0x0028,0x0010, "US"); // Rows

         fh[imageNumber]->InsertEntryString(strStudyUID,0x0020,0x000d,"UI");
         fh[imageNumber]->InsertEntryString(strSerieUID,0x0020,0x000e,"UI");
         fh[imageNumber]->InsertEntryString(patName,0x0010,0x0010, "PN");   // Patient's Name

         fh[imageNumber]->SetImageData((uint8_t *)imageTable, totalNumberOfPixels * imagePixelSize);

// ==================================================================================================

// This is a dirty heuristics, but no other way :-(

// if Image Orientation (Patient) is not present
//    I create one, (as Axial)
//    if Image Position (Patient) is not present
//       I create one, incrementing  zPositionComponent up by user supplied zSpacing
//    if Slice Location is not present
//       I create one, as zPositionComponent
//
// Aware user is free to supply his own one !

/*    
         if (! f[imageNumber]->CheckIfEntryExist(0x0020,0x0037) ) // 0020 0037 DS 6 Image Orientation (Patient)
         {
            fh[imageNumber]->InsertEntryString("1.0\\0.0\\0.0\\0.0\\1.0\\0.0",0x0020,0x0037, "DS"); //[1\0\0\0\1\0] : Axial

            char charImagePosition[256];
            sprintf(charImagePosition,"%f\\0.0\\0.0",zPositionComponent);
            zPositionComponent += zSpacing;
            if (! f[imageNumber]->CheckIfEntryExist(0x0020,0x0032) ) //0020 0032 DS 3 Image Position (Patient)
               fh[imageNumber]->InsertEntryString(charImagePosition,0x0020,0x0032, "DS");
            if (! f[imageNumber]->CheckIfEntryExist(0x0020,0x1041) ) // 0020 0x1041 DS 1 Slice Location
            {
               sprintf(charImagePosition,"%f",zPositionComponent);
               fh[imageNumber]->InsertEntryString(charImagePosition,0x0020,0x1041, "DS");
            }
         }
*/

// ==================================================================================================

         fh[imageNumber]->SetWriteTypeToDcmExplVR();
         fh[imageNumber]->SetContentType(GDCM_NAME_SPACE::UNMODIFIED_PIXELS_IMAGE);

         lastFilename =  GDCM_NAME_SPACE::Util::GetName( fullFilename );
         std::string fullWriteFilename = strDirNameout + GDCM_NAME_SPACE::GDCM_FILESEPARATOR 
                                       + lastFilename;
         if (verbose)
            std::cout << "Write : [" << fullWriteFilename << "]" << std::endl;
         if (!fh[imageNumber]->Write(fullWriteFilename))
         {
            std::cout << "Fail to write :[" << fullWriteFilename << "]"
                      << std::endl;
         }
 
         for(int k=0; k < nbOfImagesInVolume; k++)
         {
            fh[k]->Delete();
            f[k]->Delete();
         }

      }  // end : 'write the imagette'
      else // start a new 'volume'
      {
         imageNumber++;
      }

   }
}

