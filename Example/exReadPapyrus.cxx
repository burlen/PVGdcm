/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: exReadPapyrus.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:05 $
  Version:   $Revision: 1.8 $
                                                                                
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
#include "gdcmSQItem.h"
#include "gdcmDebug.h"
#include "gdcmUtil.h"

#include "gdcmArgMgr.h"
#include <iostream>

//#include <fstream>

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

// ----------------------------------------------------------------------
// Here we load a supposed to be Papyrus File (GDCM_NAME_SPACE::File compliant)
// and then try to get the pixels, using low-level SeqEntry accessors.
// Since it's not a general purpose Papyrus related program
// (just a light example) we suppose *everything* is clean
// and we don't perform any integrity check
// ----------------------------------------------------------------------

int main(int argc, char *argv[])
{

   START_USAGE(usage)
   " \n exReadPapyrus :\n                                       ",
   " Reads a Papyrus V3 File, Writes a Multiframe Dicom V3 File ",
   "     (just to show gdcm can do it ...)                      ", 
   "     ==> Well ... It does, but not too much! ",
   "     ==> This code assumes that Pixel Data is uncompressed; no check is performed :-(",
   "",
   " usage: exReadPapyrus filein=inputPapyrusFileName fileout=outputDicomFileName", 
   "                      [debug]  ", 
   "        debug    : developper wants to run the program in 'debug mode'        ",
   FINISH_USAGE


   // ----- Initialize Arguments Manager ------
   
   GDCM_NAME_SPACE::ArgMgr *am = new GDCM_NAME_SPACE::ArgMgr(argc, argv);
  
   if (am->ArgMgrDefined("usage")) 
   {
      am->ArgMgrUsage(usage); // Display 'usage'
      delete am;
      return 0;
   }
   char *fileName = am->ArgMgrWantString("filein",usage);
   if ( fileName == NULL )
   {
      delete am;
      return 0;
   }

   char *outputFileName = am->ArgMgrWantString("fileout",usage);
   if ( outputFileName == NULL )
   {
      delete am;
      return 0;
   }

   if (am->ArgMgrDefined("debug"))
      GDCM_NAME_SPACE::Debug::DebugOn();
 
   // if unused Params we give up
   if ( am->ArgMgrPrintUnusedLabels() )
   { 
      am->ArgMgrUsage(usage);
      delete am;
      return 0;
   }

   delete am;  // we don't need Argument Manager any longer

   // ----------- End Arguments Manager ---------

   if( FileExists( outputFileName ) )
   {
      if( !RemoveFile( outputFileName ) )
      {
         std::cerr << "Ouch, the file exist, but I cannot remove it" << std::endl;
         return 1;
      }
   }

   int loadMode = 0x0; // load everything
   GDCM_NAME_SPACE::File *f = GDCM_NAME_SPACE::File::New();
   f->SetLoadMode( loadMode );
   f->SetFileName( fileName );
   bool res = f->Load();  

   if ( !res )
   {
      f->Delete();
      return 0;
   }

   // Look for private Papyrus Sequence
   GDCM_NAME_SPACE::SeqEntry *seqPapyrus= f->GetSeqEntry(0x0041, 0x1050);
   if (!seqPapyrus)
   {
      std::cout << "NOT a Papyrus File : " << fileName <<std::endl;
      f->Delete();
      return 1;
   }

//   GDCM_NAME_SPACE::FileHelper *original = new GDCM_NAME_SPACE::FileHelper( fileName );
//   GDCM_NAME_SPACE::File *h = original->GetFile();

   //GDCM_NAME_SPACE::FileHelper *f1 = new GDCM_NAME_SPACE::FileHelper(f);
   GDCM_NAME_SPACE::SQItem *sqi = seqPapyrus->GetFirstSQItem();
   if (sqi == 0)
   {
      std::cout << "NO SQItem found within private Papyrus Sequence"
          << std::endl;
      f->Delete();
      return 1;
   }
      
   std::string TransferSyntax;
   std::string StudyDate;
   std::string StudyTime;
   std::string Modality;
   std::string PatientName;
   std::string MediaStSOPinstUID;

// Get informations on the file : 
//  Modality, Transfer Syntax, Study Date, Study Time
// Patient Name, Media Storage SOP Instance UID, etc

   MediaStSOPinstUID   =  f->GetEntryString(0x0002,0x0002);
   TransferSyntax      =  f->GetEntryString(0x0002,0x0010);
   StudyDate           = sqi->GetEntryString(0x0008,0x0020);
   StudyTime           = sqi->GetEntryString(0x0008,0x0030);
   Modality            = sqi->GetEntryString(0x0008,0x0060);
   PatientName         = sqi->GetEntryString(0x0010,0x0010);

   std::cout << "TransferSyntax " << TransferSyntax << std::endl;

   std::string Rows;
   std::string Columns;
   std::string SamplesPerPixel;
   std::string BitsAllocated;
   std::string BitsStored;
   std::string HighBit;
   std::string PixelRepresentation;
   

   // we brutally suppose all the images within a Papyrus file
   // have the same caracteristics.
   // if you're aware they have not, just move the GetEntryString
   // inside the loop

   // Get caracteristics of the first image
   SamplesPerPixel     = sqi->GetEntryString(0x0028,0x0002);
   Rows                = sqi->GetEntryString(0x0028,0x0010);
   Columns             = sqi->GetEntryString(0x0028,0x0011);
   BitsAllocated       = sqi->GetEntryString(0x0028,0x0100);
   BitsStored          = sqi->GetEntryString(0x0028,0x0101);
   HighBit             = sqi->GetEntryString(0x0028,0x0102);
   PixelRepresentation = sqi->GetEntryString(0x0028,0x0103);

   // just convert those needed to compute PixelArea length
   int iRows            = (uint32_t) atoi( Rows.c_str() );
   int iColumns         = (uint32_t) atoi( Columns.c_str() );
   int iSamplesPerPixel = (uint32_t) atoi( SamplesPerPixel.c_str() );
   int iBitsAllocated   = (uint32_t) atoi( BitsAllocated.c_str() );

   int lgrImage = iRows*iColumns * iSamplesPerPixel * (iBitsAllocated/8);

   // compute number of images
   int nbImages = seqPapyrus->GetNumberOfSQItems();
   std::cout <<"Number of frames :" << nbImages << std::endl;  

   //  allocate enough room to get the pixels of all images.
   uint8_t *PixelArea = new uint8_t[lgrImage*nbImages];
   uint8_t *currentPosition = PixelArea;
   GDCM_NAME_SPACE::DataEntry *pixels;

   // declare and open the file
   std::ifstream *Fp;
   Fp = new std::ifstream(fileName, std::ios::in | std::ios::binary);
   if( ! *Fp )
   {
      std::cout <<  "Cannot open file: " << fileName << std::endl;
      delete Fp;
      Fp = 0;
      return 0;
   }
   // to be sure to be at the beginning
   Fp->seekg(0, std::ios::end);

   uint32_t offset;
   std::string previousRows = Rows;
   sqi = seqPapyrus->GetFirstSQItem();
   while (sqi)
   {
      std::cout << "One more image read. Keep waiting" << std::endl;
      Rows = sqi->GetEntryString(0x0028,0x0010);
      // minimum integrity check
      if (Rows != previousRows)
      {
         std::cout << "Consistency check failed " << std::endl;
         return 1;
      }
     
      // get the images pixels
      
      // Oops! The following code assumes Pixel Data is uncompressed :-(
      
      pixels = sqi->GetDataEntry(0x7fe0,0x0010);
      offset = pixels->GetOffset();
      // perform a fseek, on offset length on the 'right' length
      Fp->seekg(offset, std::ios::beg);
      // perform a fread into the right place
      Fp->read((char *)currentPosition, (size_t)lgrImage);
      currentPosition +=lgrImage;

      std::string previousRowNb = Rows;

      sqi =  seqPapyrus->GetNextSQItem();
   }

   // build up a new File, with file info + images info + global pixel area.

   std::string NumberOfFrames = GDCM_NAME_SPACE::Util::Format("%d", nbImages); 

   GDCM_NAME_SPACE::File *n = GDCM_NAME_SPACE::File::New();

   n->InsertEntryString(MediaStSOPinstUID,  0x0002,0x0002);
  // Whe keep default gdcm Transfer Syntax (Explicit VR Little Endian)
  // since using Papyrus one (Implicit VR Little Endian) is a mess
   //n->InsertEntryString(TransferSyntax,     0x0002,0x0010);
   n->InsertEntryString(StudyDate,          0x0008,0x0020);
   n->InsertEntryString(StudyTime,          0x0008,0x0030);
   n->InsertEntryString(Modality,           0x0008,0x0060);
   n->InsertEntryString(PatientName,        0x0010,0x0010);

   n->InsertEntryString(SamplesPerPixel,    0x0028,0x0002);
   n->InsertEntryString(NumberOfFrames,     0x0028,0x0008);
   n->InsertEntryString(Rows,               0x0028,0x0010);
   n->InsertEntryString(Columns,            0x0028,0x0011);
   n->InsertEntryString(BitsAllocated,      0x0028,0x0100);
   n->InsertEntryString(BitsStored,         0x0028,0x0101);
   n->InsertEntryString(HighBit,            0x0028,0x0102);
   n->InsertEntryString(PixelRepresentation,0x0028,0x0103);

   // create the file
   GDCM_NAME_SPACE::FileHelper *file = GDCM_NAME_SPACE::FileHelper::New(n);

   file->SetImageData(PixelArea,lgrImage*nbImages);
   file->SetWriteTypeToDcmExplVR();

   //file->SetPrintLevel(2);
   n->Print();

   // Write the file
   file->Write(outputFileName); 
   if (!file)
   {
      std::cout <<"Fail to open (write) file:[" << outputFileName << "]" << std::endl;;
      n->Delete();
      file->Delete();
      return 1;  
   }
   n->Delete();
   file->Delete();
   return 0;
}
