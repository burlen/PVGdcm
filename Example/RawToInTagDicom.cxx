/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: RawToInTagDicom.cxx,v $
  Language:  C++
  Date:      $Date: 2009/01/22 16:30:39 $
  Version:   $Revision: 1.2 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                      
=========================================================================*/

/**
 * Writes an old style Bruker Dicom file, InTag compliant, from a Raw File
 * User has to supply parameters. 
 * 
 */
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmDebug.h"
#include "gdcmUtil.h"
#include "gdcmDirList.h"

#include "gdcmArgMgr.h"

#include <iostream>
#include <sstream>


/**
  * \brief   
  *          - explores  the given directory
  *          - orders the Raw Files
  *            according to their names
  *          - fills a single level Directory with *all* the files,
  *            converted into a Brucker-like Dicom, Intags compliant
  */  
  
typedef char               * PS8;
typedef unsigned char      * PU8;
typedef short int          * PS16;
typedef unsigned short int * PU16;
typedef int                * PS32;
typedef unsigned int       * PU32;
typedef float              * PF32;
typedef double             * PD64;

#define CRR(t1,t2)   { for(int l=0;l<nX*nY*/* nZ* */samplesPerPixel;l++)           \
                                  {*((t2)planePixelsOut + l) = *(((t1)pixels)+ l);}\
                     }
    
#define CFR(PPt)  switch ( pixelTypeOutCode ) {     \
          case -8   : CRR(PPt,PS8);  break;         \
          case 8    : CRR(PPt,PU8);  break;         \
          case -16  : CRR(PPt,PS16); break;         \
          case 16   : CRR(PPt,PU16); break;         \
          case -32  : CRR(PPt,PS32); break;         \
          case 32   : CRR(PPt,PU32); break;         \
          case 33   : CRR(PPt,PF32); break;         \
          case 64   : CRR(PPt,PD64); break;         \
          }


void ConvertSwapZone(int pixelSize, void *Raw, size_t RawSize);

void ConvertSwapZone(int pixelSize, void *Raw, size_t RawSize)
{
   unsigned int i;    
   if ( pixelSize == 2 )
   {
      uint16_t *im16 = (uint16_t*)Raw;
      for( i = 0; i < RawSize / 2; i++ )
      {
         im16[i]= (im16[i] >> 8) | (im16[i] << 8 );
      }
   }
   else if ( pixelSize == 4 )
   {
      uint32_t s32;
      uint16_t high;
      uint16_t low;
      uint32_t *im32 = (uint32_t*)Raw;

      for( i = 0; i < RawSize / 4; i++ )
      {
         low     = im32[i] & 0x0000ffff; // 3412
         high    = im32[i] >> 16;
         s32     = low;
         im32[i] = ( s32 << 16 ) | high;
      }
      
   }
}

int main(int argc, char *argv[])
{
   START_USAGE(usage)
   " \n RawToDicom : \n                                                       ",
   " - explores recursively the given directory,                              ",
   " - orders the Raw Files according to their names                          ",
   " - fills a single level (*) Directory with *all* the files,               ",
   "           converted into a Brucker-like Dicom, InTags compliant          ",
   " usage: RawToDicom dirin=rootDirectoryName                                ",
   "                   dirout=outputDirectoryName                             ",
   "                   rows=nb of Rows                                        ",
   "                   lines=nb of Lines,                                     ",
   "                   [frames = nb of Frames] //defaulted to 1               ",
   "                   pixeltype={8U|8S|16U|16S|32U|32S|32F|64D}              ",
   "                   pixeltypeout={8U|8S|16U|16S|32U|32S}                   ",
   "                   [{b|l}] b:BigEndian,l:LittleEndian default : l         ",
   "                   [samples = {1|3}}       //(1:Gray,3:RGB) defaulted to 1",
   "                   [monochrome1]                                          ",
   "                   [studyid = ] [patientname = Patient's name]            ",
   "                   [debug] [verbose] [listonly]                           ",
   "                                                                          ",
   "  monochrome1 = user wants MONOCHROME1 photom. interp. (0=white)          ",
   "  studyUID   : *aware* user wants to add the serie                        ",
   "                                             to an already existing study ",
   "  debug      : developper wants to run the program in 'debug mode'        ",
   FINISH_USAGE
   

   // ------------ Initialize Arguments Manager ----------------  
   GDCM_NAME_SPACE::ArgMgr *am= new GDCM_NAME_SPACE::ArgMgr(argc, argv);
  
   if (argc == 1 || am->ArgMgrDefined("usage") )
   {
      am->ArgMgrUsage(usage); // Display 'usage'
      delete am;
      return 1;
   }

   const char *dirNamein;   
   dirNamein  = am->ArgMgrGetString("dirin",".");

   const char *dirNameout;   
   dirNameout  = am->ArgMgrGetString("dirout",".");
   
   const char *patientName = am->ArgMgrGetString("patientname", "g^Fantomas");
   
   int nX = am->ArgMgrWantInt("rows", usage);
   int nY = am->ArgMgrWantInt("lines", usage);
   int nZ = am->ArgMgrGetInt("frames", 1);
   int samplesPerPixel = am->ArgMgrGetInt("samples", 1);
   
   int b = am->ArgMgrDefined("b");
   int l = am->ArgMgrDefined("l");
      
   char *pixelType = am->ArgMgrWantString("pixeltype", usage);
   const char *pixelTypeOut = am->ArgMgrGetString("pixeltypeout", pixelType);   

   bool monochrome1 = ( 0 != am->ArgMgrDefined("monochrome1") );
   
   if (am->ArgMgrDefined("debug"))
      GDCM_NAME_SPACE::Debug::DebugOn();

   int verbose  = am->ArgMgrDefined("verbose");      
   int listonly = am->ArgMgrDefined("listonly");
   
   bool userDefinedStudy = ( 0 != am->ArgMgrDefined("studyUID") );
   const char *studyUID;
   if (userDefinedStudy)
      studyUID  = am->ArgMgrGetString("studyUID");  

   // not described *on purpose* in the Usage !
   bool userDefinedSerie = ( 0 != am->ArgMgrDefined("serieUID") );       
   const char *serieUID;
   if(userDefinedSerie)
      serieUID = am->ArgMgrGetString("serieUID");

   /* if unused Param we give up */
   if ( am->ArgMgrPrintUnusedLabels() )
   {
      am->ArgMgrUsage(usage);
      delete am;
      return 1;
   } 

   delete am;  // we don't need Argument Manager any longer

   // ----------- End Arguments Manager ---------
   

   // ----- Begin Processing -----


   bool bigEndian = GDCM_NAME_SPACE::Util::IsCurrentProcessorBigEndian();

   std::string strPixelType(pixelType);
   int pixelSign;
   int pixelSize;
   int pixelTypeCode; // for the switch case
   
   if (strPixelType == "8S")
   {
      pixelSize = 1;
      pixelSign = 1;
      pixelTypeCode = -8;
   }
   else if (strPixelType == "8U")
   {
      pixelSize = 1;
      pixelSign = 0;
      pixelTypeCode = 8;
   }
   else if (strPixelType == "16S")
   {
      pixelSize = 2;
      pixelSign = 1;
      pixelTypeCode = -16;
   }   
   else if (strPixelType == "16U")
   {
      pixelSize = 2;
      pixelSign = 0;
      pixelTypeCode = 16;
   }      
   else if (strPixelType == "32S")
   {
      pixelSize = 4;
      pixelSign = 1;
      pixelTypeCode = -32;
   }   
   else if (strPixelType == "32U")
   {
      pixelSize = 4;
      pixelSign = 0;
      pixelTypeCode = 32;
   }
   else if (strPixelType == "32F")
   {
      pixelSize = 4;
      pixelSign = 0;
      pixelTypeCode = 33;
   }

   else if (strPixelType == "64D")
   {
      pixelSize = 8;
      pixelSign = 0;
      pixelTypeCode = 64;
   }
   else
   {
      std::cout << "Wrong 'pixeltype' (" << strPixelType << ")" << std::endl;
      return 1;
   }

   std::string strPixelTypeOut(pixelTypeOut);
   int pixelSignOut;
   int pixelSizeOut;
   int pixelTypeOutCode; // for the switch case
    
   if (strPixelTypeOut == "8S")
   {
      pixelSizeOut = 1;
      pixelSignOut = 1;
      pixelTypeOutCode = -8;
   }
   else if (strPixelTypeOut == "8U")
   {
      pixelSizeOut = 1;
      pixelSignOut = 0;
      pixelTypeOutCode = 8;
   }
   else if (strPixelTypeOut == "16S")
   {
      pixelSizeOut = 2;
      pixelSignOut = 1; 
      pixelTypeOutCode = -16;
   }   
   else if (strPixelTypeOut == "16U")
   {
      pixelSizeOut = 2;
      pixelSignOut = 0;
      pixelTypeOutCode = 16;
   }      
   else if (strPixelTypeOut == "32S")
   {
      pixelSizeOut = 4;
      pixelSignOut = 1;
      pixelTypeOutCode = -32;
   }   
   else if (strPixelTypeOut == "32U")
   {
      pixelSizeOut = 4;
      pixelSignOut = 0;
      pixelTypeOutCode = 32;
   }
   else
   {
      std::cout << "Wrong 'pixeltypeout' (" << strPixelTypeOut << ")" << std::endl;
      return 1;
   }
 
 
 
   if ( ! GDCM_NAME_SPACE::DirList::IsDirectory(dirNamein) )
   {
      std::cout << "KO : [" << dirNamein << "] is not a Directory." << std::endl;
      return 0;

   }
   else
   {
      std::cout << "OK : [" << dirNamein << "] is a Directory." << std::endl;
   }
 
   std::string systemCommand;
   
   std::cout << "Check for output directory :[" << dirNameout << "]."
             <<std::endl;
   if ( ! GDCM_NAME_SPACE::DirList::IsDirectory(dirNameout) )    // dirout not found
   {
      std::string strDirNameout(dirNameout);          // to please gcc 4
      systemCommand = "mkdir " +strDirNameout;        // create it!
      if (verbose)
         std::cout << systemCommand << std::endl;
      system (systemCommand.c_str());
      if ( ! GDCM_NAME_SPACE::DirList::IsDirectory(dirNameout) ) // be sure it worked
      {
          std::cout << "KO : not a dir : [" << dirNameout << "] (creation failure ?)" << std::endl;
      return 0;

      }
      else
      {
        std::cout << "Directory [" << dirNameout << "] created." << std::endl;
      }
   }
   else
   {
       std::cout << "Output Directory [" << dirNameout << "] already exists; Used as is." << std::endl;
   }

   std::string strDirNamein(dirNamein);
   GDCM_NAME_SPACE::DirList dirList(strDirNamein, false); // DON'T get recursively the list of files
   std::string strDirNameout(dirNameout);   
   
   if (listonly)
   {
      std::cout << "------------List of found files ------------" << std::endl;
      dirList.Print();
      std::cout << std::endl;
      return 1;
   }

   std::string strStudyUID;
   std::string strSerieUID;

   if (userDefinedStudy)
      strSerieUID = studyUID;
   else
      strStudyUID = GDCM_NAME_SPACE::Util::CreateUniqueUID();
   
   if (userDefinedSerie)
     strSerieUID = serieUID;
   else
      strSerieUID = GDCM_NAME_SPACE::Util::CreateUniqueUID();

   GDCM_NAME_SPACE::DirListType fileNames;
   fileNames = dirList.GetFilenames();

   GDCM_NAME_SPACE::File *f;
   GDCM_NAME_SPACE::FileHelper *fh;

   std::string outputFileName;

   int sliceIndex  = 1;  // IMPORTANT : must start with non zero value
   int frameIndex  = 1;  // IMPORTANT : must start with non zero value
   std::string chSessionIndex("1");

   // -----------------------------------------------------
   // Iterate fo ALL the files found in the input directory
   // -----------------------------------------------------

   for (GDCM_NAME_SPACE::DirListType::iterator it = fileNames.begin();
                                    it != fileNames.end();
                                  ++it)
   {

 // Open the Raw file
   std::ifstream *Fp = new std::ifstream((*it).c_str(), std::ios::in | std::ios::binary);
   if ( ! *Fp )
   {
      std::cout << "Cannot open file: [" << *it << "]" << std::endl;
      delete Fp;
      Fp = 0;
      return 0;
   }
   if (verbose)
      std::cout << "Success to open file: [" << *it << "]" << std::endl;

   // Read the pixels

   int singlePlaneDataSize =  nX*nY*samplesPerPixel*pixelSizeOut;
   int dataSizeIn          =  nX*nY*samplesPerPixel*pixelSize*nZ;

   uint8_t *pixels         = new uint8_t[dataSizeIn];
   uint8_t *planePixelsOut = new uint8_t[singlePlaneDataSize];

   Fp->read((char*)pixels, (size_t)dataSizeIn);

   if ( pixelSize !=1 && ( (l && bigEndian) || (b && ! bigEndian) ) )
   {  
      ConvertSwapZone(pixelSize, pixels, dataSizeIn);
   }

if (verbose)
   std::cout << "After ConvertSwapZone" << std::endl;

// Copy (and convert) pixels of a single plane

     switch ( pixelTypeCode )
     {
       case 8    : CFR(PU8);  break;
       case -8   : CFR(PS8);  break;
       case -16  : CFR(PU16); break;
       case 16   : CFR(PS16); break;
       case -32  : CFR(PS32); break;
       case 32   : CFR(PU32); break;
       case 33   : CFR(PF32); break;
       case 64   : CFR(PD64); break;
     }

if (verbose)
   std::cout << "After CFR" << std::endl;
   
// Create an empty FileHelper

   GDCM_NAME_SPACE::FileHelper *fileH = GDCM_NAME_SPACE::FileHelper::New();
 
 // Get the (empty) image header.
   GDCM_NAME_SPACE::File *fileToBuild = fileH->GetFile();

   // 'Study Instance UID'
   // The user is allowed to create his own Study, 
   //          keeping the same 'Study Instance UID' for various images
   // The user may add images to a 'Manufacturer Study',
   //          adding new Series to an already existing Study

   fileToBuild->InsertEntryString(strStudyUID,0x0020,0x000d,"UI");  //  Study UID   

   // 'Serie Instance UID'
   // The user is allowed to create his own Series, 
   // keeping the same 'Serie Instance UID' for various images
   // The user shouldn't add any image to a 'Manufacturer Serie'
   // but there is no way no to prevent him for doing that
   
   fileToBuild->InsertEntryString(strSerieUID,0x0020,0x000e,"UI");  //  Serie UID
   
   std::ostringstream str;

   // Set the image size
   str.str("");
   str << nX;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0011, "US"); // Columns
   str.str("");
   str << nY;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0010, "US"); // Rows
   
       if (verbose)
         std::cout << "before  debut des choses serieuses2"<< std::endl;      
  
//   str.str("");
//   str << nZ;
//   fileToBuild->InsertEntryString(str.str(),0x0028,0x0008, "IS"); // Number of Frames

   // Set the pixel type
   
   str.str("");
   str << pixelSizeOut*8;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0100, "US"); // Bits Allocated

   str.str("");
   str << pixelSizeOut*8;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0101, "US"); // Bits Stored

   str.str("");
   str << ( pixelSizeOut*8 - 1 );
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0102, "US"); // High Bit

   str.str("");
   str << pixelSign;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0103, "US"); // Pixel Representation

// If you deal with a Serie of images, as slices of a volume,
// it's up to you to tell gdcm, for each image, what are the values of :
// 
// 0020 0032 DS 3 Image Position (Patient)
// 0020 0037 DS 6 Image Orientation (Patient)

   str.str("");
   str << "0.0\\0.0\\0.0";

// take Frame Index as position (we don't know anything else) NO!

   fileToBuild->InsertEntryString(str.str(),0x0020,0x0032, "DS");

   fileToBuild->InsertEntryString("1.0\\0.0\\0.0\\0.0\\1.0\\0.0",0x0020,0x0037, "DS"); //[1\0\0\0\1\0] : Axial
   
   
   str.str("");
   str << samplesPerPixel;
   fileToBuild->InsertEntryString(str.str(),0x0028,0x0002, "US"); // Samples per Pixel

   if (strlen(patientName) != 0)
      fileToBuild->InsertEntryString(patientName,0x0010,0x0010, "PN"); // Patient's Name
    
   //  0=white  
   if(monochrome1)
      fileH->SetPhotometricInterpretationToMonochrome1();

// Special InTag, now.

      fileH->InsertEntryString(chSessionIndex, 0x0020, 0x0012, "IS");

      // Deal with  0x0021, 0x1020 : 'SLICE INDEX'
      // will stay to 0, since the stuff deals with single slice directories
      char chSliceIndex[5];
      sprintf(chSliceIndex, "%04d", sliceIndex);
      std::string strChSliceIndex(chSliceIndex);

      // Deal with  0x0021, 0x1040 : 'FRAME INDEX'

      str.str("");
      str << frameIndex;
      frameIndex++;  // be ready for next one 

      fileH->InsertEntryString(strChSliceIndex, 0x0021, 0x1020, "IS");
      fileH->InsertEntryString(str.str(),    0x0021, 0x1040, "IS");

      // Pixel Size
      /// \TODO Ask user to supply 'Pixel Size' value
      float pxSzX = 1.0;
      float pxSzY = 1.0;

      // Field of view
      char fov[64];
      sprintf(fov, "%f\\%f",nX*pxSzX, nY*pxSzY);
      fileH->InsertEntryString(fov, 0x0019, 0x1000, "DS");


// We want to create MR-like image !
      fileH->InsertEntryString("MR", 0x0008, 0x0060, "CS");
      fileH->InsertEntryString("1.2.840.10008.5.1.4.1.1.4" , 0x0002, 0x0002, "UI");
      fileH->InsertEntryString("1.2.840.10008.5.1.4.1.1.4" , 0x0008, 0x0016, "UI");

      fileH->SetContentType(GDCM_NAME_SPACE::UNMODIFIED_PIXELS_IMAGE);
      
      

// Set the image Pixel Data
   fileH->SetImageData(planePixelsOut,singlePlaneDataSize);

// Set the writting mode and write the image
   fileH->SetWriteModeToRaw();
   
   

 // Write a DICOM Explicit VR file
   fileH->SetWriteTypeToDcmExplVR();


   outputFileName = strDirNameout +  GDCM_NAME_SPACE::GDCM_FILESEPARATOR + GDCM_NAME_SPACE::Util::GetName(*it) + "_ForInTag.dcm";
   if( !fileH->Write(outputFileName ) )
   {
      std::cout << "Failed for [" << outputFileName << "]\n"
                << "           File is unwrittable\n";
   }

// End of :    for (GDCM_NAME_SPACE::DirListType::iterator it = fileNames.begin() ... 


  
   fileH->Delete();

   delete[] pixels;
   delete[] planePixelsOut;

  }
   return 1;
 } // end of : for (GDCM_NAME_SPACE::DirListType::iterator it
