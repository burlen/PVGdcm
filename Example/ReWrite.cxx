/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: ReWrite.cxx,v $
  Language:  C++
  Date:      $Date: 2008/05/16 10:40:23 $
  Version:   $Revision: 1.35 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmDebug.h"

#include "gdcmArgMgr.h"

#include <string.h> // for memcpy
#include <iostream>

int main(int argc, char *argv[])
{
   START_USAGE(usage)
   " \n ReWrite :\n",
   " Re write a full gdcm-readable Dicom image                              ",
   "     (usefull when the file header is not very straight).               ",
   "                                                                        ",
   " usage: ReWrite filein=inputFileName fileout=outputFileName             ",
   "       [keepoverlays] [mode=write mode] [monochrome1]                   ",
   "       [noshadow] [noseq][debug]                                        ",
   "  --> The following line to 'rubout' a burnt-in Patient name            ",
   "       [rubout=xBegin,xEnd,yBegin,yEnd [ruboutvalue=n (<255)] ]         ",
   "  --> The 2 following lines, to extract a sub image within some frames  ",
   "       [ROI=xBegin,xEnd,yBegin,yEnd]                                    ",
   "       [firstframe=beg] [lastframe=end]                                 ", 
   "                                                                        ",
   "        mode = a (ACR), x (Explicit VR Dicom), r (RAW : only pixels)    ",
   "               j (jpeg lossless), 2 (jpeg2000)                          ",
   "               NOTE : JPEG encapsulated in a Dicom File, not JPEG File !",
   "        keepoverlays : user wants to keep ACR-NEMA-like overlays        ",
   "        monochrome1 = user wants MONOCHROME1 photom. interp. (0=white)  ",
   "        noshadowseq: user doesn't want to load Private Sequences        ",
   "        noshadow : user doesn't want to load Private groups (odd number)",
   "        noseq    : user doesn't want to load Sequences                  ",
   "        rgb      : user wants to transform LUT (if any) to RGB pixels   ",
   "        warning  : developper wants to run the program in 'warning mode'",
   "        debug    : developper wants to run the program in 'debug mode'  ",
   FINISH_USAGE

   // ----- Initialize Arguments Manager ------  
    
   GDCM_NAME_SPACE::ArgMgr *am = new GDCM_NAME_SPACE::ArgMgr(argc, argv);
  
   if (argc == 1 || am->ArgMgrDefined("usage")) 
   {
      am->ArgMgrUsage(usage); // Display 'usage'
      delete am;
      return 1;
   }
   char *fileName = am->ArgMgrWantString("filein",usage);
   if ( fileName == NULL )
   {
      std::cout << "'filein= ...' is mandatory" << std::endl;
      delete am;
      return 1;
   }

   char *outputFileName = am->ArgMgrWantString("fileout",usage);
   if ( outputFileName == NULL )
   {
      std::cout << "'fileout= ...' is mandatory" << std::endl;
      delete am;
      return 1;
   }

   const char *mode = am->ArgMgrGetString("mode","X");

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

   bool rgb          = ( 0 != am->ArgMgrDefined("RGB") );
   bool keepoverlays = ( 0 != am->ArgMgrDefined("keepoverlays") );   
   bool monochrome1  = ( 0 != am->ArgMgrDefined("monochrome1") );
   
   if (am->ArgMgrDefined("debug"))
      GDCM_NAME_SPACE::Debug::DebugOn();

   if (am->ArgMgrDefined("warning"))
      GDCM_NAME_SPACE::Debug::WarningOn();
            
   bool fail = false;
   int *boundVal;
   int ruboutVal;
   bool rubout = false; 
   if (am->ArgMgrDefined("rubout"))
   {
      int nbBound;
      boundVal = am->ArgMgrGetListOfInt("rubout", &nbBound);

      if (nbBound !=4)
      {
         std::cout << "Illegal number of 'rubout' boundary values (expected : 4, found:" 
                   << nbBound << "); 'rubout' ignored" << std::endl;
         fail = true;
      }
            
      ruboutVal = am->ArgMgrGetInt("ruboutvalue", 0);
      rubout = true;   
   }

   int *roiBoundVal;
   bool roi = false; 
   if (am->ArgMgrDefined("roi"))
   {
      int nbRoiBound;
      roiBoundVal = am->ArgMgrGetListOfInt("roi", &nbRoiBound);

      if (nbRoiBound !=4)
      {
        std::cout << "Illegal number of 'ROI' boundary values (expected : 4, found:" 
                  << nbRoiBound << "); 'ROI' ignored" << std::endl;
        fail = true;
      }
      else
        roi = true;   
   }
  
   int beg = am->ArgMgrGetInt("firstFrame",-1);
   int end = am->ArgMgrGetInt("lastFrame",-1);
 
   // if unused Params we give up
   if ( am->ArgMgrPrintUnusedLabels() )
   { 
      am->ArgMgrUsage(usage);
      delete am;
      return 0;
   }

   delete am;  // we don't need Argument Manager any longer

   // ----------- End Arguments Manager ---------
   GDCM_NAME_SPACE::File *f = GDCM_NAME_SPACE::File::New();
   
   f->SetMaxSizeLoadEntry(0x7fffffff);
   f->SetLoadMode( loadMode );
   f->SetFileName( fileName );

   bool res = f->Load();  
      if ( !res )
   {
      f->Delete();
      return 1;
   }

   if (!f->IsReadable())
   {
       std::cerr << "Sorry, not a Readable DICOM / ACR File"  <<std::endl;
       f->Delete();
       return 1;
   }
   
   GDCM_NAME_SPACE::FileHelper *fh = GDCM_NAME_SPACE::FileHelper::New(f);
   uint8_t *imageData; 
   int dataSize;
 
   int nX,nY,nZ,sPP,planarConfig;
   std::string pixelType, transferSyntaxName;
   nX=f->GetXSize();
   nY=f->GetYSize();
   nZ=f->GetZSize();
   
   std::cout << " DIMX=" << nX << " DIMY=" << nY << " DIMZ=" << nZ << std::endl;

   pixelType    = f->GetPixelType();
   sPP          = f->GetSamplesPerPixel();
   planarConfig = f->GetPlanarConfiguration();
   
   std::cout << " pixelType="           << pixelType 
             << " SampleserPixel="      << sPP
             << " PlanarConfiguration=" << planarConfig 
             << " PhotometricInterpretation=" 
             << f->GetEntryString(0x0028,0x0004) 
             << std::endl;

   int numberOfScalarComponents=f->GetNumberOfScalarComponents();
   std::cout << "NumberOfScalarComponents " << numberOfScalarComponents 
             <<std::endl;
   transferSyntaxName = f->GetTransferSyntaxName();
   std::cout << " TransferSyntaxName= [" << transferSyntaxName << "]" 
             << std::endl;
 
   fh->SetKeepOverlays( keepoverlays );
    
   if(monochrome1)
      fh->SetPhotometricInterpretationToMonochrome1();
   
   if (rgb)
   {
      dataSize  = fh->GetImageDataSize();
      imageData = fh->GetImageData(); // somewhat important : Loads the Pixels in memory !
      fh->SetWriteModeToRGB();
   }
   else
   {
      dataSize  = fh->GetImageDataRawSize();
      imageData = fh->GetImageDataRaw();// somewhat important : Loads the Pixels in memory !
      fh->SetWriteModeToRaw();
   }

   if ( imageData == 0 ) // to avoid warning
   {
      std::cout << "Was unable to read pixels " << std::endl;
   }
   printf(" dataSize %d imageData %p\n",dataSize, imageData);

   // Since we just ReWrite the image, we know no modification 
   // was performed on the pixels.
   // We don't want this image appears as a 'Secondary Captured image'
   fh->SetContentType(GDCM_NAME_SPACE::UNMODIFIED_PIXELS_IMAGE);
   

   /// \todo : think about rubbing out a part of a *multiframe* image!
   if (rubout)
   {     
      if (boundVal[0]<0 || boundVal[0]>nX)
      { 
         std::cout << "xBegin out of bounds; 'rubout' ignored" << std::endl;
         fail = true;      
      }
      if (boundVal[1]<0 || boundVal[1]>nX)
      { 
         std::cout << "xEnd out of bounds; 'rubout' ignored" << std::endl;
         fail = true;      
      }
      if (boundVal[0] > boundVal[1])
      { 
         std::cout << "xBegin greater than xEnd; 'rubout' ignored" << std::endl;
         fail = true;      
      }       
      if (boundVal[2]<0 || boundVal[2]>nY)
      { 
         std::cout << "yBegin out of bounds; 'rubout' ignored" << std::endl;
         fail = true;      
      }
      if (boundVal[3]<0 || boundVal[3]>nY)
      { 
         std::cout << "yEnd out of bounds; 'rubout' ignored" << std::endl;
         fail = true;      
      }
      if (boundVal[2] > boundVal[3])
      { 
         std::cout << "yBegin greater than yEnd; 'rubout' ignored" << std::endl;
         fail = true;      
      }  
      if (!fail)
      {
         int pixelLength = f->GetBitsAllocated()/8;
         int lineLength = nX * sPP * pixelLength;
         size_t lengthToRubout = (boundVal[1]-boundVal[0])*sPP*pixelLength;
         int offsetToBeginOfRubout = boundVal[0]*sPP*pixelLength+lineLength*boundVal[2];
      
         for(int rbl=boundVal[2]; rbl<boundVal[3];rbl++)
         {
            memset((char *)imageData+offsetToBeginOfRubout, ruboutVal, lengthToRubout);
            offsetToBeginOfRubout += lineLength; 
         }
      }   
   } 


//------------------------------ Set the Writing mode ---------------------------------

   switch (mode[0])
   {
      case 'A' :
      case 'a' :
      // Writting an ACR file
      // from a full gdcm readable File
         std::cout << "WriteACR" << std::endl;
         fh->SetWriteTypeToAcr();
         break;

      case 'D' : // Not documented in the 'usage', because the method 
      case 'd' : //                             is known to be bugged. 
      // Writting a DICOM Implicit VR file
      // from a full gdcm readable File
         std::cout << "WriteDCM Implicit VR" << std::endl;
         fh->SetWriteTypeToDcmImplVR(); 
         break;

      case 'X' :
      case 'x' :
      // writting a DICOM Explicit VR 
      // from a full gdcm readable File
         std::cout << "WriteDCM Explicit VR" << std::endl;
         // fh->WriteDcmExplVR(outputFileName);
         // Try this one :
         fh->SetWriteTypeToDcmExplVR();

         break;

      case 'R' :
      case 'r' :
      //  Writting a Raw File,
         std::cout << "WriteRaw" << std::endl;
         fh->WriteRawData(outputFileName);
         break;
 
      case 'J' :
      case 'j' :
      // writting a DICOM Jpeg Lossless
      // from a full gdcm readable File
         std::cout << "WriteDCM Jpeg Lossless" << std::endl;
         fh->SetWriteTypeToJPEG();
         break;

      case '2' :
      // writting a DICOM Jpeg 2000
      // from a full gdcm readable File
         std::cout << "WriteDCM Jpeg 2000" << std::endl;
         fh->SetWriteTypeToJPEG2000();
         break;

 // Just for fun :
 // Write a 'Video inverse' version of the file.
 // *Not* described, on purpose,  in the USAGE
      case 'V' :
      case 'v' :
         if ( fh->GetFile()->GetBitsAllocated() == 8)
         {
            std::cout << "videoinv for 8 bits" << std::endl;
            for (int i=0; i<dataSize; i++)
            {
               ((uint8_t*)imageData)[i] = 255 - ((uint8_t*)imageData)[i];
            }
         }
         else
         {
            std::cout << "videoinv for 16 bits" << std::endl;
            for (int i=0; i<dataSize/2; i++)
            {
               ((uint16_t*)imageData)[i] =  65535 - ((uint16_t*)imageData)[i];
            }
         }
         std::cout << "WriteDCM Explicit VR + VideoInv" << std::endl;
         fh->SetWriteTypeToDcmExplVR();
         break;
   }

//
// user wants to keep only a part of the image (ROI, and/or some frames)
// ---------------------------------------------------------------------
// (==> this is no longer really 'ReWrite' !)

    int subImDimX = nX;
    int subImDimY = nY;

    if (roi)
    {
      if (roiBoundVal[0]<0 || roiBoundVal[0]>=nX)
      { 
         std::cout << "xBegin out of bounds; 'roi' ignored" << std::endl;
         fail = true;      
      }
      if (roiBoundVal[1]<0 || roiBoundVal[1]>=nX)
      { 
         std::cout << "xEnd out of bounds; 'roi' ignored" << std::endl;
         fail = true;      
      }
      if (roiBoundVal[0] > roiBoundVal[1])
      { 
         std::cout << "xBegin greater than xEnd; 'roi' ignored" << std::endl;
         fail = true;
      }

      if (roiBoundVal[2]<0 || roiBoundVal[2]>=nY)
      {
         std::cout << "yBegin out of bounds; 'roi' ignored" << std::endl;
         fail = true;
      }
      if (roiBoundVal[3]<0 || roiBoundVal[3]>=nY)
      {
         std::cout << "yEnd out of bounds; 'roi' ignored" << std::endl;
         fail = true;
      }
      if (roiBoundVal[2] > roiBoundVal[3])
      {
         std::cout << "yBegin greater than yEnd; 'roi' ignored" << std::endl;
         fail = true;
      }
   }
   else
   {
     roiBoundVal = new int[4];
     roiBoundVal[0] = 0;
     roiBoundVal[1] = nX-1;
     roiBoundVal[2] = 0;
     roiBoundVal[3] = nY-1;  
  }

   subImDimX = roiBoundVal[1]-roiBoundVal[0]+1;  
   subImDimY = roiBoundVal[3]-roiBoundVal[2]+1;

  if (roi || beg != -1 || end != -1)
  {
     if (beg == -1)
        beg = 0;  
     if (end == -1)
        end = nZ-1;

     std::ostringstream str;

    // Set the data that will be *actually* written.

     int pixelSize = fh->GetFile()->GetPixelSize();
     size_t lgrSubLine  = subImDimX* pixelSize * numberOfScalarComponents;
     size_t lgrSubFrame = subImDimY*lgrSubLine;

     int lgrSubImage = (end-beg+1) * lgrSubFrame;

     uint8_t * subImage = new uint8_t[lgrSubImage];

     uint8_t * srcCopy = (uint8_t *) imageData;
     uint8_t * destCopy = subImage;
     int lineSize = nX*pixelSize*numberOfScalarComponents;
     int frameSize = nY*lineSize;
 
     int lineOffset = roiBoundVal[0]*pixelSize * numberOfScalarComponents;
     
     for (int frameNb=beg, frameCount=0; frameNb<=end; frameNb++, frameCount++)
     { 
        for (int lineNb=roiBoundVal[2], lineCount=0; lineNb<=roiBoundVal[3]; lineNb++, lineCount++)
        {  
            /// \todo : increment data pointer, don't multiply so much!
            memcpy( (void *)(destCopy + frameCount*lgrSubFrame + lineCount*lgrSubLine), 
                    (void *)(srcCopy  + frameNb*frameSize + lineNb*lineSize + lineOffset ), 
                    lgrSubLine);
        }
     }
 
    // Set the image size
     str.str("");
     str << subImDimX ;
     fh->InsertEntryString(str.str(),0x0028,0x0011,"US"); // Columns

     str.str("");
     str << subImDimY;
     fh->InsertEntryString(str.str(),0x0028,0x0010,"US"); // Rows
     str.str("");
     str << end-beg+1; 
     fh->InsertEntryString(str.str(),0x0028,0x0008, "IS"); // Number of Frames 
      
     //fh->SetImageData(subImage,lgrSubImage);
      fh->SetUserData(subImage,lgrSubImage);   // ensures the compression (if any)    
  }
  else
  {         
      fh->SetUserData(imageData,dataSize); // ensures the compression (if any) 
  }



//----------------------------------- Write, now! ---------------------------------

   if (mode[0] != 'R' && mode[0] != 'r')
      res = fh->Write(outputFileName);
      
   if(!res)
      std::cout <<"Fail to write [" << outputFileName << "]" <<std::endl;    

   f->Delete();
   fh->Delete();
   return 0;
}

 
