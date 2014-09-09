/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: exMoveImagesToSingleSerieUID.cxx,v $
  Language:  C++
  Date:      $Date: 2007/10/01 09:33:20 $
  Version:   $Revision: 1.6 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
/*
Some old versions of VTK? ITK ? SeriesWriter weren't too much strict
 when they did the job.
 A lot of people has a lot of 'supposed to be Serie' directories
 that, actually, aren't at all.
 The following program converts them (hope so!)
 
 Le programme 'de convertion' s'utilise comme les 'filtres LibIDO'
(mots-clés et valeurs par defaut).

exMoveImagesToSingleSerieUID
                                     ",
Converts the a Dicom file inside a single-level Directory
 into a 'Single Study UID - Single SerieUID      
  usage:
exMoveImagesToSingleSerieUID
      dirin=inputDirectoryName                                   
      dirout=outputDirectoryName                                  
      [studyUID = ] [patName = ] [Zspacing = ]
      [ { [noshadowseq] | [noshadow][noseq] } ]  [debug]
      [verbose]",


  dirin : single-level Directory containing the images 
           (no recursive parsing)
  dirout : will be created if doesn't exist                
  studyUID   : *aware* user wants to add the serie  
                    to an already existing study 


dirin : directory (un seul niveau) contenant les images
dirout : directory de sortie (cree s'il n'existe pas); le nom des images
est conservé.
studyUID : si on veut *ajouter* la serie qui va etre creee a une study
qui existe deja (il faut connaitre son Study UID)
patName : nom du patient (ca peut servir pour y voir plus clair sur les
logiciels cliniques)
Zspacing : si c'est important pour la visu 3D. Ca servira a calculer le
Slice Location (s'il n'existe pas deja) et la composante Z de
ImagePositionPatient (s'il n'existe pas)

// if Image Orientation (Patient) is not present
//    I create one, (as Axial)
//    if Image Position (Patient) is not present
//       I create one, incrementing  zPositionComponent up by user
supplied zSpacing
//    if Slice Location is not present
//       I create one, as zPositionComponent
//
*/
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmCommon.h"
#include "gdcmDebug.h"
#include "gdcmDirList.h"
#include "gdcmUtil.h"

#include "gdcmArgMgr.h"

#include <iostream>

int main(int argc, char *argv[])
{   
   START_USAGE(usage)
   "\n exMoveImagesToSingleSerieUID :\n                                       ",
   " Converts the Dicom files inside a single-level Directory                 ",
   " into a 'Single Study UID - Single SerieUID' file set                     ",
   " usage: exMoveImagesToSingleSerieUID                                      ",
   "              dirin=inputDirectoryName                                    ",
   "              dirout=outputDirectoryName                                  ",
   "              [studyUID = ] [patName = ] [Zspacing = ]                    ",
   "              [ { [noshadowseq] | [noshadow][noseq] } ]  [debug] [verbose]",
   "                                                                          ",
   "       dirin : single-level Directory containing the images               ",
   "                                        (no recursive parsing)            ",   
   "       dirout : will be created if doesn't exist                          ",
   "                                                                          ",
   "       studyUID   : *aware* user wants to add the serie                   ",
   "                                             to an already existing study ",
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

   bool verbose = ( 0 != am->ArgMgrDefined("verbose") );
   
   std::string patName = am->ArgMgrGetString("patname", "g^PatientName");
   float zSpacing = am->ArgMgrGetFloat("zSpacing", 1.0);
      
   const char *dirIn   = am->ArgMgrGetString("dirin");
   const char *dirOut  = am->ArgMgrGetString("dirout");

   bool userDefinedStudy = ( 0 != am->ArgMgrDefined("userDefinedStudy") );
   const char *studyUID  = am->ArgMgrGetString("studyUID");

// not described *on purpose* in the Usage ! 
   bool userDefinedSerie = ( 0 != am->ArgMgrDefined("userDefinedSerie") );
  
   const char *serieUID  = am->ArgMgrGetString("serieUID");      
 
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
          std::cout << "KO : not a dir : [" << dirOut << "] (creation failure ?)" << std::endl;
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
      std::cout << "Output Directory [" << dirOut << "] already exists; Used as is." << std::endl;
   }  

   GDCM_NAME_SPACE::DirList dirList(dirIn,false); // gets (at single level) the file list
   GDCM_NAME_SPACE::DirListType fileList = dirList.GetFilenames();

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

   GDCM_NAME_SPACE::File *f;
   GDCM_NAME_SPACE::FileHelper *fh;
   std::string fullFilename, lastFilename;
   float zPositionComponent = 0.0;
   
   for( GDCM_NAME_SPACE::DirListType::iterator it  = fileList.begin();
                                 it != fileList.end();
                                 ++it )
   {
      fullFilename = *it;
      f = GDCM_NAME_SPACE::File::New( );
      f->SetLoadMode(loadMode);
      f->SetFileName( it->c_str() );

      if (verbose)
         std::cout << "file [" << it->c_str() << "]" << std::endl;
      if ( !f->Load() )
      {
         if (verbose)
            std::cout << "fail to load [" << it->c_str() << "]" << std::endl;   
         f->Delete();
         continue;
      }

      // Load the pixels in RAM.
   
      fh = GDCM_NAME_SPACE::FileHelper::New(f);
      fh->SetKeepOverlays( true );
      uint8_t *imageData = fh->GetImageDataRaw(); // Don't convert (Gray Pixels + LUT) into (RGB pixels) ?!?
      if (!imageData)
         std::cout << "fail to read [" << it->c_str() << std::endl;
      fh->SetWriteTypeToDcmExplVR();

      fh->InsertEntryString(strStudyUID,0x0020,0x000d,"UI");      
      fh->InsertEntryString(strSerieUID,0x0020,0x000e,"UI");
      fh->InsertEntryString(patName,0x0010,0x0010, "PN");   // Patient's Name

// ==================================================================================================

// This is a dirty heuristics, but no other way :-(

// if Image Orientation (Patient) is not present
//    I create one, (as Axial) 
//    if Image Position (Patient) is not present
//       I create one, incrementing  zPositionComponent up by user supplied zSpacing
//    if Slice Location is not present 
//       I create one, as zPositionComponent
//
// Aware use is free to supply his own one !    
    
      if (! f->CheckIfEntryExist(0x0020,0x0037) ) // 0020 0037 DS 6 Image Orientation (Patient)
      {
         fh->InsertEntryString("1.0\\0.0\\0.0\\0.0\\1.0\\0.0",0x0020,0x0037, "DS"); //[1\0\0\0\1\0] : Axial

         char charImagePosition[256];
     
         sprintf(charImagePosition,"0.0\\0.0\\%f",zPositionComponent);
         zPositionComponent += zSpacing;
 
         if (! f->CheckIfEntryExist(0x0020,0x0032) ) //0020 0032 DS 3 Image Position (Patient)
            fh->InsertEntryString(charImagePosition,0x0020,0x0032, "DS"); 
    
         if (! f->CheckIfEntryExist(0x0020,0x1041) ) // 0020 0x1041 DS 1 Slice Location
         {
            sprintf(charImagePosition,"%f",zPositionComponent);
            fh->InsertEntryString(charImagePosition,0x0020,0x1041, "DS");
          }    
      } 

// ==================================================================================================

              
      fh->SetContentType(GDCM_NAME_SPACE::UNMODIFIED_PIXELS_IMAGE);
      
      lastFilename =  GDCM_NAME_SPACE::Util::GetName( fullFilename );
      std::string fullWriteFilename = strDirNameout + GDCM_NAME_SPACE::GDCM_FILESEPARATOR 
                                        + lastFilename;
      if (verbose)
         std::cout << "Write : [" << fullWriteFilename << "]" << std::endl;

      if (!fh->Write(fullWriteFilename))
      {
         std::cout << "Fail to write :[" << fullWriteFilename << "]"
                   << std::endl;
      } 
 
      fh->Delete();
      f->Delete();
   }
   return 1;
}                         
