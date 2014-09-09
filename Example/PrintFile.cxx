/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: PrintFile.cxx,v $
  Language:  C++
  Date:      $Date: 2009/05/28 15:44:34 $
  Version:   $Revision: 1.93 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmFile.h"
#include "gdcmDocument.h"
#include "gdcmSeqEntry.h"
#include "gdcmSQItem.h"
#include "gdcmDataEntry.h"

#include "gdcmFileHelper.h"
#include "gdcmDebug.h"
#include "gdcmDirList.h"
#include "gdcmGlobal.h"
#include "gdcmDictSet.h"
#include "gdcmArgMgr.h"
#include "gdcmOrientation.h"
#include <iostream>

/// \todo : code factorization, for 'single file' an 'whole directory' processing

void ShowLutData(GDCM_NAME_SPACE::File *f);

     // Nothing is written yet to get LUT Data user friendly
     // The following is to be moved into a PixelReadConvert method
     // Let here, waiting for a clever idea on the way to do it.

void ShowLutData(GDCM_NAME_SPACE::File *f)
{  
   GDCM_NAME_SPACE::SeqEntry *modLutSeq = f->GetSeqEntry(0x0028,0x3000);
   if ( modLutSeq !=0 )
   {
      GDCM_NAME_SPACE::SQItem *sqi= modLutSeq->GetFirstSQItem();
      if ( sqi != 0 )
      {
         std::string lutDescriptor = sqi->GetEntryString(0x0028,0x3002);
         if (   /*lutDescriptor   == GDCM_UNFOUND*/ 0 )
         {
            std::cout << "LUT Descriptor is missing" << std::endl;
            return;
         }
         int length;   // LUT length in Bytes
         int deb;      // Subscript of the first Lut Value
         int nbits;    // Lut item size (in Bits)

         int nbRead;    // nb of items in LUT descriptor (must be = 3)

         nbRead = sscanf( lutDescriptor.c_str(),
                           "%d\\%d\\%d",
                              &length, &deb, &nbits );
         std::cout << "length " << length 
                  << " deb " << deb 
                  << " nbits " << nbits
                  << std::endl;
         if ( nbRead != 3 )
         {
            std::cout << "Wrong LUT descriptor" << std::endl;
         }
         //LUT Data (CTX dependent)    
         GDCM_NAME_SPACE::DataEntry *b = sqi->GetDataEntry(0x0028,0x3006); 
         if ( b != 0 )
         { 
            int BitsAllocated = f->GetBitsAllocated();
            if ( BitsAllocated <= 8 )
            { 
               int mult;
               if ( ( nbits == 16 ) && ( BitsAllocated == 8 ) )
               {
               // when LUT item size is different than pixel size
                  mult = 2; // high byte must be = low byte
               }
               else
               {
               // See PS 3.3-2003 C.11.1.1.2 p 619
                  mult = 1;
               }
               uint8_t *lut = b->GetBinArea();
               for( int i=0; i < length; ++i )
               {
                  std::cout << i+deb << " : \t"
                              << (int) (lut[i*mult + 1]) << std::endl;
               }
            }
            else
            {
               uint16_t *lut = (uint16_t *)(b->GetBinArea());  
               for( int i=0; i < length; ++i )
               {
                  std::cout << i+deb << " : \t"
                              << (int) (((uint16_t *)lut)[i])
                              << std::endl;
               }             
            }
         }  
         else
            std::cout << "No LUT Data DataEntry (0x0028,0x3006) found?!? " 
                        << std::endl;
      }
      else
         std::cout << "No First SQ Item within (0x0028,0x3000) ?!? " 
                     << std::endl;      
   }
   else
      std::cout << "No LUT Data SeqEntry (0x0028,0x3000) found " 
                  << std::endl;
}

int main(int argc, char *argv[])
{

   START_USAGE(usage)
   " \n PrintFile : \n                                                        ",
   " Display the header of a ACR-NEMA/PAPYRUS/DICOM File                      ",
   " usage: PrintFile {filein=inputFileName|dirin=inputDirectoryName}[level=n]",
   "                       [forceload=listOfElementsToForceLoad] [rec] [noex] ",
   "                       [4DLoc= ][dict= privateDirectory]                  ",
   "                       [ { [noshadowseq] | [noshadow][noseq] } ] [load]   ",
   "                       [debug] [warning]                                  ",
   "      level = 0,1,2 : depending on the amount of details user wants to see",
   "      rec : user wants to parse recursively the directory                 ",
   "      load : user wants to load the pixels, as well (to see warning info) ",
   "      noex : user doesn't want extra 'user friendly' info                 ",   
   "      4DLoc: group-elem(in hexa, no space) of the DataEntry holdind 4thDim",
   "      listOfElementsToForceLoad : group-elem,g2-e2,... (in hexa, no space)",
   "                                of Elements to load whatever their length ",
   "      privateDirectory : source file full path name of Shadow Group elems ",
   "      noshadowseq: user doesn't want to load Private Sequences            ",
   "      noshadow   : user doesn't want to load Private groups (odd number)  ",
   "      noseq      : user doesn't want to load Sequences                    ",
   "      debug      : user wants to run the program in 'debug mode'          ",
   "      warning    : user wants to be warned about any oddity in the File   ",
   "      showlut :user wants to display the Palette Color (as an int array)  ",
   FINISH_USAGE

   // Initialize Arguments Manager   
   GDCM_NAME_SPACE::ArgMgr *am= new GDCM_NAME_SPACE::ArgMgr(argc, argv);
  
   if (argc == 1 || am->ArgMgrDefined("usage") )
   {
      am->ArgMgrUsage(usage); // Display 'usage'
      delete am;
      return 1;
   }

   const char *fileName = am->ArgMgrGetString("filein");
   const char *dirName  = am->ArgMgrGetString("dirin");

   if ( (fileName == 0 && dirName == 0) ||
        (fileName != 0 && dirName != 0) )
   {
      std::cerr << std::endl
        << "Either 'filein=' or 'dirin=' must be present;" 
        << std::endl << "Not both" << std::endl;
      am->ArgMgrUsage(usage); // Display 'usage'  
      delete am;
      return 1;
   }   
      
   bool noex = ( 0 != am->ArgMgrDefined("noex") );
   bool rec  = ( 0 != am->ArgMgrDefined("rec") );   
        
   if (am->ArgMgrDefined("debug"))
      GDCM_NAME_SPACE::Debug::DebugOn();

   if (am->ArgMgrDefined("warning"))
      GDCM_NAME_SPACE::Debug::WarningOn();
       
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

   int level = am->ArgMgrGetInt("level", 1);

   int forceLoadNb;
   uint16_t *elemsToForceLoad 
                           = am->ArgMgrGetXInt16Enum("forceload", &forceLoadNb);

   int nbP =0;
   uint16_t *FourthDimLoc;
   if ( am->ArgMgrDefined("4DLoc") )
   {
      FourthDimLoc = am->ArgMgrGetXInt16Enum("4DLoc", &nbP);
   
      if (nbP != 1) 
      {   
         std::cout << "4DLoc must have 2 and only 2 components!" << std::endl;
         delete am;
         return 1;      
      }
   }

   bool load = ( 0 != am->ArgMgrDefined("load") );
   
   bool showlut = ( 0 != am->ArgMgrDefined("SHOWLUT") );

   bool ddict = am->ArgMgrDefined("dict") ? true : false;
   const char *dict = 0;

   if (ddict)
   {
     dict = am->ArgMgrGetString("dict",0);
   }

   /* if unused Param we give up */
   if ( am->ArgMgrPrintUnusedLabels() )
   {
      am->ArgMgrUsage(usage);
      delete am;
      return 1;
   } 

   delete am;  // we don't need Argument Manager any longer

   // ----------- End Arguments Manager ---------


   if (ddict)
   {
      GDCM_NAME_SPACE::Global::GetDicts()->GetDefaultPubDict()->AddDict(dict);
   }

   if ( fileName != 0 ) // ====== Deal with a single file ======
   {
      GDCM_NAME_SPACE::File *f = GDCM_NAME_SPACE::File::New();
      f->SetLoadMode(loadMode);
      f->SetFileName( fileName );
   f->SetMaxSizeLoadEntry(0xffff);

      for (int ri=0; ri<forceLoadNb; ri++)
      {
         f->AddForceLoadElement((uint32_t)elemsToForceLoad[2*ri], 
                                (uint32_t)elemsToForceLoad[2*ri+1] );
      }
// TODO : find why such a polution
// To avoid polluting the output with messages
// 'Last system error was : No such file or directory'

errno = 0; 


      bool res = false;
      try
        {
        res = f->Load();
        }
      catch(std::exception &ex)
        {
        std::cerr << "sorry an exception was thrown: " << ex.what() << std::endl;
        }
      // GDCM_NAME_SPACE::File::IsReadable() is no usable here, because we deal with
      // any kind of gdcm-Parsable *document*
      // not only GDCM_NAME_SPACE::File (as opposed to GDCM_NAME_SPACE::DicomDir)
      if ( !res )
      {
         std::cout << "Cannot process file [" << fileName << "]" << std::endl;
         std::cout << "Either it doesn't exist, or it's read protected "
                   << std::endl;
         std::cout << "or it's not a Dicom File, or its 'header' is bugged"
                   << std::endl;
         std::cout << "use 'PrintFile filein=... debug' to try to guess the pb"
                   << std::endl;
         f->Delete();
         return 0;
      }

      if (nbP == 1)
         f->SetFourthDimensionLocation(FourthDimLoc[0],FourthDimLoc[1]);


      GDCM_NAME_SPACE::FileHelper *fh = GDCM_NAME_SPACE::FileHelper::New(f);
      fh->SetPrintLevel( level );

      fh->Print();

      std::cout << "\n\n" << std::endl;

      std::cout <<std::endl;
      std::cout <<" dataSize    " << fh->GetImageDataSize()    << std::endl;
      std::cout <<" dataSizeRaw " << fh->GetImageDataRawSize() << std::endl;
if (!noex)
{
      int nX,nY,nZ,nT,sPP,planarConfig;
      std::string pixelType;
      nX=f->GetXSize();
      nY=f->GetYSize();
      nZ=f->GetZSize();
      nT=f->GetTSize();
      std::cout << " DIMX=" << nX << " DIMY=" << nY 
                << " DIMZ=" << nZ << " DIMT=" << nT
                << std::endl;

      pixelType    = f->GetPixelType();
      sPP          = f->GetSamplesPerPixel();
      std::cout << " pixelType= ["            << pixelType 
                << "] SamplesPerPixel= ["     << sPP
                << "] ";

      if (sPP == 3)
      {
         planarConfig = f->GetPlanarConfiguration();
         std::cout << " PlanarConfiguration= [" << planarConfig 
                << "] "<< std::endl;
      } 
      std::cout << " PhotometricInterpretation= [" 
                << f->GetEntryString(0x0028,0x0004)
                << "] "<< std::endl;

      int numberOfScalarComponents=f->GetNumberOfScalarComponents();
      std::cout << " NumberOfScalarComponents = " << numberOfScalarComponents 
                <<std::endl
                << " LUT = " << (f->HasLUT() ? "TRUE" : "FALSE")
                << std::endl;

      if ( f->GetDataEntry(0x0002,0x0010) )
         if ( f->GetDataEntry(0x0002,0x0010)->IsNotLoaded() ) 
         {
            std::cout << "Transfer Syntax not loaded. " << std::endl
                     << "Better you increase MAX_SIZE_LOAD_ELEMENT_VALUE"
                  << std::endl;
            f->Delete();
            return 0;
         }
  
      std::string transferSyntaxName = f->GetTransferSyntaxName();
      std::cout << " TransferSyntaxName= [" << transferSyntaxName << "]" 
                << std::endl;
      std::cout << " SwapCode= " << f->GetSwapCode() << std::endl;
      std::cout << " ------" << std::endl;


      std::cout << "\n\n" << std::endl; 
      std::cout << "X spacing " << f->GetXSpacing() << std::endl;
      std::cout << "Y spacing " << f->GetYSpacing() << std::endl;
      std::cout << "Z spacing " << f->GetZSpacing() << std::endl;
   
//------------------------------





      // Let's get and print some usefull fields about 'Orientation'
      // ------------------------------------------------------------

      std::string strPatientPosition = 
                                      f->GetEntryString(0x0018,0x5100);
      if ( strPatientPosition != GDCM_NAME_SPACE::GDCM_UNFOUND 
        && strPatientPosition != "" )  
            std::cout << "PatientPosition (0x0010,0x5100)= [" 
                      << strPatientPosition << "]" << std::endl;
 
      std::string strViewPosition = 
                                      f->GetEntryString(0x0018,0x5101);
      if ( strViewPosition != GDCM_NAME_SPACE::GDCM_UNFOUND 
        && strViewPosition != "" )  
            std::cout << "View Position (0x0018,0x5101)= [" 
                      << strViewPosition << "]" << std::endl;
      
     std::string strPatientOrientation = 
                                      f->GetEntryString(0x0020,0x0020);
      if ( strPatientOrientation != GDCM_NAME_SPACE::GDCM_UNFOUND
        && strPatientOrientation != "")  
         std::cout << "PatientOrientation (0x0020,0x0020)= [" 
                   << strPatientOrientation << "]" << std::endl;

      std::string strImageOrientationPatient = 
                                      f->GetEntryString(0x0020,0x0037);  
      if ( strImageOrientationPatient != GDCM_NAME_SPACE::GDCM_UNFOUND
        && strImageOrientationPatient != "" )  
         std::cout << "ImageOrientationPatient (0x0020,0x0037)= [" 
                   << strImageOrientationPatient << "]" << std::endl;

      std::string strImageOrientationRET = 
                                      f->GetEntryString(0x0020,0x0035);
      if ( strImageOrientationRET != GDCM_NAME_SPACE::GDCM_UNFOUND
        && strImageOrientationRET != "" )  
         std::cout << "ImageOrientationRET (0x0020,0x0035)= [" 
                   << strImageOrientationRET << "]" << std::endl;

      std::string strImagePositionPatient = 
                                      f->GetEntryString(0x0020,0x0032);  
      if ( strImagePositionPatient != GDCM_NAME_SPACE::GDCM_UNFOUND
        && strImagePositionPatient != "" )  
         std::cout << "ImagePositionPatient (0x0020,0x0032)= [" 
                   << strImagePositionPatient << "]" << std::endl;

      std::string strImagePositionPatientRET = 
                                      f->GetEntryString(0x0020,0x0030);
      if ( strImagePositionPatientRET != GDCM_NAME_SPACE::GDCM_UNFOUND
        && strImagePositionPatientRET != "" )  
         std::cout << "ImagePositionPatientRET (0x0020,0x0030)= [" 
                   << strImagePositionPatientRET << "]" << std::endl;
  
     float iop[6];
     //bool riop = 
        f->GetImageOrientationPatient(iop);  
     float ipp[3];
     //bool ripp = 
        f->GetImagePositionPatient(ipp);

     std::cout << "Image Position (0x0020,0x0032|0x0030) : "
               << ipp[0] << " , " << ipp[1] << " , "<< ipp[2]
               << std::endl;
     std::cout << "Image Orientation (0x0020,0x0037|0x0035) : "
               << iop[0] << " , " << iop[1] << " , "<< iop[2] << " , "
               << iop[3] << " , " << iop[4] << " , "<< iop[5]
               << std::endl; 


      // Let's compute 'user friendly' results about 'Orientation'
      // ---------------------------------------------------------
 
      GDCM_NAME_SPACE::Orientation *o = GDCM_NAME_SPACE::Orientation::New();

      if ( strImageOrientationPatient != GDCM_NAME_SPACE::GDCM_UNFOUND ||
           strImageOrientationRET     != GDCM_NAME_SPACE::GDCM_UNFOUND )
      {
  
         GDCM_NAME_SPACE::OrientationType orient = o->GetOrientationType( f );
 
         std::cout << "TypeOrientation = " << orient << " (-> " 
                   << o->GetOrientationTypeString(orient) << " )" << std::endl;
      }

      std::string ori = o->GetOrientation ( f );
      if (ori != "\\" )
         std::cout << "Orientation [" << ori << "]" << std::endl;
      o->Delete();
      
      
/*      
std::vector <double> valueVector; 
GDCM_NAME_SPACE::DataEntry *e_0018_5212 = f->GetDataEntry(0x0018, 0x5212);
bool resJP = e_0018_5212->GetDSValue(valueVector);
if (resJP) {
   double test;
   for ( int i=0; i < 3; i++ ) {
       test = valueVector[i];
       std::cout << " test " << test << std::endl;
    }
}
//e_0018_5212->Delete();
*/       


/* -----------------------------

// Try :
std::cout << std::endl << std::endl << "===========Try Get Numerical ======="
          << std::endl;
GDCM_NAME_SPACE::DataEntry *e;
bool res;
std::vector<double> vd;

// Transfert Syntax
e=f->GetDataEntry(0x0002,0x0010);
if (e){
  res=e->GetNumerical(vd);
  if (!res){
    std::cout << "0x0002,0x0010 not numerical, size =" << vd.size() << std::endl;  
  }
}
// Columns
e=f->GetDataEntry(0x0028,0x0011);
if (e){
  res=e->GetNumerical(vd);
  if (!res){
    std::cout << "0x0028,0x0011 not numerical, size =" << vd.size() << std::endl;  
  } else {
    std::cout << "0x0028,0x0011 numerical, size =" << vd.size() << std::endl;
    std::cout << vd[0]<< std::endl;
  }
}
// Im Orient (Pat)
e=f->GetDataEntry(0x0020,0x0032);
if (e){
  res=e->GetNumerical(vd);
  if (!res){
    std::cout << "0x0020,0x0032 not numerical, size =" << vd.size() << std::endl;  
  } else {
    std::cout << "0x0020,0x0032 numerical, size =" << vd.size() << std::endl;
    for(int l=0; l<vd.size(); l++)
      std::cout << "vd[" << l << "]=" << vd[l]<< std::endl;
  }
}

// Pixel Spacing

e=f->GetDataEntry(0x0028,0x0030);
if (e){
  res=e->GetNumerical(vd);
  if (!res){
    std::cout << "0x0028,0x0030 not numerical, size =" << vd.size() << std::endl;  
  } else {
    std::cout << "0x0028,0x0030 numerical, size =" << vd.size() << std::endl;
    for(int l=0; l<vd.size(); l++)
      std::cout << "vd[" << l << "]=" << vd[l]<< std::endl;
  }
}

----------------------------------------------*/  
    
}  
//------------------------------


      // Display the LUT as an int array (for debugging purpose)
      if ( f->HasLUT() && showlut )
      {
         uint8_t* lutrgba = fh->GetLutRGBA();
         if ( lutrgba == 0 )
         {
            std::cout << "Lut RGBA (Palette Color) not built " << std::endl;
 
           // Nothing is written yet to get LUT Data user friendly
           // The following is to be moved into a PixelRedaConvert method
  
            GDCM_NAME_SPACE::SeqEntry *modLutSeq = f->GetSeqEntry(0x0028,0x3000);
            if ( modLutSeq !=0 )
            {
               GDCM_NAME_SPACE::SQItem *sqi= modLutSeq->GetFirstSQItem();
               if ( !sqi )
               {
                  std::string lutDescriptor = sqi->GetEntryString(0x0028,0x3002);
                  int length;   // LUT length in Bytes
                  int deb;      // Subscript of the first Lut Value
                  int nbits;    // Lut item size (in Bits)
                  int nbRead;   // nb of items in LUT descriptor (must be = 3)

                  nbRead = sscanf( lutDescriptor.c_str(),
                                    "%d\\%d\\%d",
                                     &length, &deb, &nbits );
                  if ( nbRead != 3 )
                  {
                      std::cout << "Wrong LUT descriptor" << std::endl;
                  }                                                  
                  GDCM_NAME_SPACE::DataEntry *b = sqi->GetDataEntry(0x0028,0x3006);
                  if ( b != 0 )
                  {
                     if ( b->GetLength() != 0 )
                     {
                        std::cout << "---------------------------------------"
                               << " We should never reach this point      "
                               << std::endl;
                        //LoadEntryBinArea(b);    //LUT Data (CTX dependent)
                     }   
                 }
              }      
            }
            else
               std::cout << "No LUT Data (0x0028,0x3000) found " << std::endl;
        }
        /*
         else
         {
            if ( fh->GetLutItemSize() == 8 )
            {
               for (int i=0;i<fh->GetLutItemNumber();i++)
                  std::cout << i << " : \t"
                         << (int)(lutrgba[i*4])   << " "
                         << (int)(lutrgba[i*4+1]) << " "
                         << (int)(lutrgba[i*4+2]) << std::endl;
            }
            else // LutItemSize assumed to be = 16
            {
               uint16_t* lutrgba16 = (uint16_t*)lutrgba;
               for (int i=0;i<fh->GetLutItemNumber();i++)
                  std::cout << i << " : \t"
                         << (int)(lutrgba16[i*4])   << " "
                         << (int)(lutrgba16[i*4+1]) << " "
                         << (int)(lutrgba16[i*4+2]) << std::endl;
            }
         }
         */
      }
      else if (showlut)
      {
         std::cout << "Try LUT Data "<< std::endl;
         ShowLutData(f);
      }

      // Parsability of the GDCM_NAME_SPACE::Document already checked, after Load() !
      
      if ( f->IsReadable() )
      {
         std::cout <<std::endl<<fileName<<" is Readable"<<std::endl;
      }
      else if ( f->GetSeqEntry(0x0041,0x1010) )
      {
         std::cout <<std::endl<<fileName<<" looks like a 'PAPYRUS image' file"
                   <<std::endl;
      }
      else if ( f->GetSeqEntry(0x0004,0x1220) )
      {
         std::cout <<std::endl<<fileName<<" looks like a 'DICOMDIR file'"
                   <<std::endl;
      }
      else 
      {
         std::cout <<std::endl<<fileName<<" doesn't look like an image file "
             <<std::endl; 
      }
 
      std::cout<<std::flush;
      f->Delete();
      fh->Delete();
   }
         // ===========================================================================
   else  // =============================== Deal with a Directory =====================
   {     // ===========================================================================
      std::cout << "dirName [" << dirName << "]" << std::endl;
      
      GDCM_NAME_SPACE::DirList dirList(dirName,rec); // gets recursively (or not) the file list
      GDCM_NAME_SPACE::DirListType fileList = dirList.GetFilenames();
      GDCM_NAME_SPACE::File *f;
      bool res;

      if (fileList.size() == 0)
      {
         std::cout << "No file found in : [" << dirName << "]" << std::endl;
      }
      
      for( GDCM_NAME_SPACE::DirListType::iterator it  = fileList.begin();
                                 it != fileList.end();
                                 ++it )
      {
         std::cout << std::endl<<" Start processing :[" << it->c_str() << "]"
                   << std::endl;
         f = GDCM_NAME_SPACE::File::New();
         f->SetLoadMode(loadMode);
         f->SetFileName( it->c_str() );

         for (int ri=0; ri<forceLoadNb; ri++)
         {
            printf("%04x,%04x\n",elemsToForceLoad[2*ri], 
                                 elemsToForceLoad[2*ri+1]);
            f->AddForceLoadElement((uint32_t)elemsToForceLoad[2*ri], 
                                   (uint32_t)elemsToForceLoad[2*ri+1]); 
         }
         res = f->Load();

         if ( !res )
         {
            std::cout << "Cannot process file [" << it->c_str() << "]" 
                      << std::endl;
            std::cout << "Either it doesn't exist, or it's read protected " 
                      << std::endl;
            std::cout << "or it's not a Dicom File, or its 'header' is bugged" 
                      << std::endl;
            std::cout << "use 'PrintFile filein=... debug' "
                      << "to try to guess the pb"
                      << std::endl;
            f->Delete();
            continue;
         }

         GDCM_NAME_SPACE::FileHelper *fh = GDCM_NAME_SPACE::FileHelper::New(f);
         fh->SetPrintLevel( level );
         fh->Print();

//------------------------------
if (!noex)
{
         // Lets's get and print some usefull fields about 'Orientation'
         // ------------------------------------------------------------

         std::string strPatientPosition = 
                                       f->GetEntryString(0x0018,0x5100);
         if ( strPatientPosition != GDCM_NAME_SPACE::GDCM_UNFOUND 
         && strPatientPosition != "" )  
               std::cout << "PatientPosition (0x0010,0x5100)= [" 
                        << strPatientPosition << "]" << std::endl;
    
         std::string strViewPosition = 
                                       f->GetEntryString(0x0018,0x5101);
         if ( strViewPosition != GDCM_NAME_SPACE::GDCM_UNFOUND 
         && strViewPosition != "" )  
               std::cout << "strViewPosition (0x0010,0x5101)= [" 
                        << strViewPosition << "]" << std::endl;
         
         std::string strPatientOrientation = 
                                       f->GetEntryString(0x0020,0x0020);
         if ( strPatientOrientation != GDCM_NAME_SPACE::GDCM_UNFOUND
         && strPatientOrientation != "")  
            std::cout << "PatientOrientation (0x0020,0x0020)= [" 
                      << strPatientOrientation << "]" << std::endl;

         std::string strImageOrientationPatient = 
                                       f->GetEntryString(0x0020,0x0037);  
         if ( strImageOrientationPatient != GDCM_NAME_SPACE::GDCM_UNFOUND
         && strImageOrientationPatient != "" )  
            std::cout << "ImageOrientationPatient (0x0020,0x0037)= [" 
                     << strImageOrientationPatient << "]" << std::endl;

         std::string strImageOrientationRET = 
                                       f->GetEntryString(0x0020,0x0035);
         if ( strImageOrientationRET != GDCM_NAME_SPACE::GDCM_UNFOUND
         && strImageOrientationRET != "" )
         {
            std::cout << "ImageOrientationRET (0x0020,0x0035)= [" 
                     << strImageOrientationRET << "]" << std::endl;
         }

         // Let's compute 'user friendly' results about 'Orientation'
         // ---------------------------------------------------------
    
         GDCM_NAME_SPACE::Orientation *o = GDCM_NAME_SPACE::Orientation::New(); 


         if ( strImageOrientationPatient != GDCM_NAME_SPACE::GDCM_UNFOUND ||
            strImageOrientationRET     != GDCM_NAME_SPACE::GDCM_UNFOUND )
         {
     
            GDCM_NAME_SPACE::OrientationType orient = o->GetOrientationType( f );
    
            std::cout << "TypeOrientation = " << orient << " (-> " 
                     << o->GetOrientationTypeString(orient) << " )" << std::endl;
         }

         std::string ori = o->GetOrientation ( f );
         if (ori != "\\" )
            std::cout << "Orientation [" << ori << "]" << std::endl;
         o->Delete();
}
//------------------------------- 
        
         if (f->IsReadable())
         {
            if (load)  // just to see warning messages at load time !
            {
               uint8_t *pixels = fh->GetImageData(); (void)pixels;
               uint32_t lgth   = fh->GetImageDataSize(); (void)lgth;
            }         

            std::cout <<std::endl<<it->c_str()<<" is Readable"<<std::endl;
         }
         else
            std::cout <<std::endl<<it->c_str()<<" is NOT Readable"<<std::endl;
         std::cout << "\n\n" << std::endl;
         


         f->Delete();
         fh->Delete();
      }
      std::cout<<std::flush;
   }
   return 0;
}
