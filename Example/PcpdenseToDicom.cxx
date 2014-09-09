/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: PcpdenseToDicom.cxx,v $
  Language:  C++
  Date:      $Date: 2010/08/26 12:46:12 $
  Version:   $Revision: 1.5 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#include <fstream>
#include <iostream>
//#include <values.h>

#if defined(__BORLANDC__)
#include <ctype.h>
#endif

#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmDebug.h"
#include "gdcmDirList.h"
#include "gdcmUtil.h"
#include "gdcmArgMgr.h"

/**
  * \brief   
  *  Converts the "pcpdense" ".txt" (2008 version)  files into 16 bits Dicom Files,
  * Hope they don't change soon!
  */  

void MakeDicomImage(unsigned short int *tabVal, int X, int Y, std::string dcmImageName, const char * patientName, int nbFrames,
                    std::string studyUID, std::string serieUID, std::string SerieDescr, int imgNum, bool m );

void LoadImage(std::ifstream &from,  unsigned short int * );

void LoadImageX2(std::ifstream &from, unsigned short int * );      
bool verbose;

int main(int argc, char *argv[])
{
   START_USAGE(usage)
   " \n pcpdenseToDicom :\n                                                  ",
   "        Converts the '.txt' files into 16 bits Dicom Files,               ",
   " usage:                                                                   ",
   " pcpdenseToDicom rootfilename=...                                        ",
   "                 (e.g.. :   meas_MID380_DENSE_stacked_slices_aif_FID81637)",
   "                 numberOfSlices =  (default : 3)                          ",
   "                 X2 : multiply x 2 image size                             ",
   "                 m :create multiframe files instead of image stacks       ", 
   "                 [patientname = Patient's name]                           ",
   "                 [verbose] [debug]                                        ",
   "                                                                          ",
   " verbose  : user wants to run the program in 'verbose mode'               ",
   " debug    : *developer*  wants to run the program in 'debug mode'         ",
   FINISH_USAGE

   // ----- Initialize Arguments Manager ------
      
   GDCM_NAME_SPACE::ArgMgr *am = new GDCM_NAME_SPACE::ArgMgr(argc, argv);
  
   if (argc == 1 || am->ArgMgrDefined("usage")) 
   {
      am->ArgMgrUsage(usage); // Display 'usage'
      delete am;
      return 0;
   }
   // Seems that ArgMgrWantString doesn't work on MacOS   
   if(!am->ArgMgrDefined("rootfilename"))
   {
      std::cout << "'rootfilename' is mandatory" << std::endl;
      exit(0);   
   }
 
   const char *rootfilename = am->ArgMgrWantString("rootfilename",usage);
   int numberOfSlices       = am->ArgMgrGetInt("numberOfSlices",3);
   const char *patientName  = am->ArgMgrGetString("patientname", "Patient^Name");
         
   if (am->ArgMgrDefined("debug"))
      GDCM_NAME_SPACE::Debug::DebugOn();

   verbose         = ( 0 != am->ArgMgrDefined("verbose") );    
   bool X2         = ( 0 != am->ArgMgrDefined("X2") );
   bool multiframe = ( 0 != am->ArgMgrDefined("m") );

   // if unused Param we give up
   if ( am->ArgMgrPrintUnusedLabels() )
   { 
      am->ArgMgrUsage(usage);
      delete am;
      return 0;
   }
   delete am;  // we don't need Argument Manager any longer

   // ----- Begin Processing -----
   
   std::ostringstream Ecc;
   std::ostringstream perf;
   std::ostringstream WashoutTc;

   std::string strSerieUID; 
   std::string strStudyUID =  GDCM_NAME_SPACE::Util::CreateUniqueUID();
   std::string dcmImageName, textFileName, patientname,/* studyUID, serieUID, */ serieDescr ;
   std::string deb(rootfilename);
   
   unsigned short int *image;
   
   int NX, NY;
     
  // Get some info
  // -------------

     {
        Ecc.str(rootfilename); 
        Ecc   << Ecc.str() << "_s0" << "_Ecc.txt";

        std::ifstream fromEcc( Ecc.str().c_str() );             
        if ( !fromEcc )
        {
           std::cout << "Can't open file [" << Ecc.str() << "]" << std::endl;
           exit(0);
        }
        std::string str1;

         fromEcc >> str1;
         fromEcc >> str1;

         fromEcc >> NX;
         fromEcc >> NY;
         std::cout << "NX, NY : " << NX << ", " << NY << std::endl; 
   }         

   int mult;
   if (X2)
      mult=4;
   else
      mult=1;
      
   if (multiframe)
      image = new unsigned short int[NX*NY*mult*numberOfSlices];
   else
      image = new unsigned short int[NX*NY*mult];        
   
   

   // === Ecc ===
   
   strSerieUID =  GDCM_NAME_SPACE::Util::CreateUniqueUID();
   
   serieDescr = "Ecc";
   
   if (!multiframe) {
             
     for (int i=0; i<numberOfSlices; i++)
     {  
        Ecc.str(rootfilename); 
        Ecc   << Ecc.str()    << "_s" << i << "_Ecc.txt";
      
        std::ifstream fromEcc( Ecc.str().c_str() );             
        if ( !fromEcc )
        {
           std::cout << "Can't open file [" << Ecc.str() << "]" << std::endl;
           exit(0);
        }
        std::cout << "Open file [" << Ecc.str() << "] : OK" << std::endl;

        if (X2)
           LoadImageX2(fromEcc, image);
        else
           LoadImage(fromEcc, image);      
        fromEcc.close();

        dcmImageName = Ecc.str() + ".dcm";

        MakeDicomImage(image, NX, NY, dcmImageName, patientName, 1, strStudyUID, strSerieUID, serieDescr, i, multiframe ); 
   } // end : for (int i=0; i<numberOfSlices
   delete []image;
}

      
   if (multiframe) {
     
     for (int i=0; i<numberOfSlices; i++)
     {  
        Ecc.str(rootfilename);
        Ecc   << Ecc.str()    << "_s" << i << "_Ecc.txt";
      
        std::ifstream fromEcc( Ecc.str().c_str() );             
        if ( !fromEcc )
        {
           std::cout << "Can't open file [" << Ecc.str() << "]" << std::endl;
           exit(0);
        }

        std::cout << "Open file [" << Ecc.str() << "] : OK" << std::endl;
        if (X2)
          LoadImageX2(fromEcc ,&image[NX*NY*i] );
        else
          LoadImage(fromEcc, &image[NX*NY*i] );
        
        fromEcc.close();
     } // end : for (int i=0; i<numberOfSlices
     
     dcmImageName = deb + "_Ecc.dcm";
     MakeDicomImage(image, NX, NY, dcmImageName, patientName, numberOfSlices, strStudyUID, strSerieUID, serieDescr, 0, multiframe ); 
  
   }  // end : if (multiframe) 



// === perf ===

   strSerieUID =  GDCM_NAME_SPACE::Util::CreateUniqueUID();
   
   serieDescr = "perf";
   
   if (!multiframe) {    
     for (int i=0; i<numberOfSlices; i++)
     {      
        perf.str(rootfilename);
        perf  << perf.str()    << "_s" << i << "_perf.txt";

        std::ifstream fromperf( perf.str().c_str() );             
        if ( !fromperf )
        {
           std::cout << "Can't open file [" << perf.str() << "]" << std::endl;
           exit(0);
        }
        std::cout << "Open file [" << perf.str() << "] : OK" << std::endl;

        if (X2)
          LoadImageX2(fromperf ,image );
        else
          LoadImage(fromperf, image );
      
        fromperf.close();
        
        dcmImageName = Ecc.str() + ".dcm";
        MakeDicomImage(image, NX, NY, dcmImageName, patientName, 1, strStudyUID, strSerieUID, serieDescr, i, multiframe ); 

     } // end : for (int i=0; i<numberOfSlices
   }


   if (multiframe) {
     for (int i=0; i<numberOfSlices; i++)
     {      
        perf.str(rootfilename);
        perf  << perf.str()    << "_s" << i << "_perf.txt";

        std::ifstream fromperf( perf.str().c_str() );             
        if ( !fromperf )
        {
           std::cout << "Can't open file [" << perf.str() << "]" << std::endl;
           exit(0);
        }
        std::cout << "Open file [" << perf.str() << "] : OK" << std::endl;

      if (X2)
         LoadImageX2(fromperf, &image[NX*NY*i]);
      else
         LoadImage(fromperf, &image[NX*NY*i] );
       
      fromperf.close();  
   } // end : for (int i=0; i<numberOfSlices
   
     dcmImageName = deb + "_perf.dcm";
     MakeDicomImage(image, NX, NY, dcmImageName, patientName, numberOfSlices, strStudyUID, strSerieUID, serieDescr, numberOfSlices, multiframe ); 
 }
 


// === WashoutTc ===


   strSerieUID =  GDCM_NAME_SPACE::Util::CreateUniqueUID();
   
   serieDescr = "WashoutTc";   
   
   if (multiframe) {    
     for (int i=0; i<numberOfSlices; i++)
     {     
        WashoutTc.str(rootfilename);
        WashoutTc <<  WashoutTc.str() << "_s" << i << "_WashoutTc.txt";

        std::ifstream fromWashoutTc( WashoutTc.str().c_str() );             
        if ( !fromWashoutTc )
        {
           std::cout << "Can't open file [" << WashoutTc.str() << "]" << std::endl;
           exit(0);
        }
        std::cout << "Open file [" << WashoutTc.str() << "] : OK" << std::endl;

        if (X2)      
          LoadImageX2(fromWashoutTc,  &image[NX*NY*i]);
        else
          LoadImage(fromWashoutTc, &image[NX*NY*i] );
        fromWashoutTc.close();  

     } // end : for (int i=0; i<numberOfSlices
          
     dcmImageName = deb + "_WashoutTc.dcm";
     MakeDicomImage(image, NX, NY, dcmImageName, patientName, numberOfSlices, strStudyUID, strSerieUID, serieDescr, numberOfSlices, multiframe );      
 } 


   if (!multiframe) {
    
   for (int i1=0; i1<numberOfSlices; i1++)
   {     
      WashoutTc.str(rootfilename);
      WashoutTc <<  WashoutTc.str() << "_s" << i1 << "_WashoutTc.txt";
 
      std::ifstream fromWashoutTc( WashoutTc.str().c_str() );             
      if ( !fromWashoutTc )
      {
         std::cout << "Can't open file [" << WashoutTc.str() << "]" << std::endl;
         exit(0);
      }

      std::cout << "Open file [" << WashoutTc.str() << "] : OK" << std::endl;
      if (X2)      
         LoadImageX2(fromWashoutTc,image );
       else
         LoadImage(fromWashoutTc,image );
      fromWashoutTc.close();
       
      dcmImageName = Ecc.str() + ".dcm";
      MakeDicomImage(image, NX, NY, dcmImageName, patientName, 1, strStudyUID, strSerieUID, serieDescr, i1, multiframe ); 
    } // end : for (int i=0; i<numberOfSlices

  }

   delete []image;
   return 1;            
}

// =====================================================================================================================

void LoadImage(std::ifstream &from,  unsigned short int *image)
{
// in any file ".txt" :

/*
XY Dimensions           47          50
     0.000000     0.000000     0.000000     0.000000     0.000000     0.000000
     ...
*/  

   if (!from)
      return;

   std::string str1;

    from >> str1;
    from >> str1;
    
    int NX, NY;
    from >> NX;
    from >> NY;
    std::cout << "NX, NY : " << NX << ", " << NY << std::endl; 

    float pixelValue;
     
     int i, j;
     for( i=0;i<NY;i++)
        for(j=0;j<NX;j++) {
           from >> pixelValue;
           image[i*NX+j] = (unsigned short int)(pixelValue * 1000.); // Why do we multiply by 1000? // JPR
        }
}


// =====================================================================================================================

void LoadImageX2(std::ifstream &from,  unsigned short int *image )
{
// in any file ".txt" :

/*
XY Dimensions           47          50
     0.000000     0.000000     0.000000     0.000000     0.000000     0.000000
     ...
*/  

   if (!from)
      return;

   std::string str1;

    from >> str1;
    from >> str1;
    
    int NX, NY;
    from >> NX;
    from >> NY;
    std::cout << "NX, NY : " << NX << ", " << NY << std::endl; 

    float pixelValue;
     
     int i, j;
     for( i=0;i<NY;i++) {
           for(j=0;j<NX;j++) {
              from >> pixelValue;
              pixelValue*=1000.;  // Why do we multiply by 1000? // JPR
              image[i*4*NX + j*2] = image[i*4*NX + j*2+1] =  image[(i*4+2)*NX + j*2] =  image[(i*4+2)*NX + j*2+1] = (unsigned short int)(pixelValue);  
   }
        }

}
// =====================================================================================================================================


void MakeDicomImage(unsigned short int *tabVal, int X, int Y, std::string dcmImageName, const char * patientName, int nbFrames, std::string studyUID, std::string serieUID, std::string SerieDescr, int imgNum, bool m)
{

 // GDCM_NAME_SPACE::Debug::DebugOn();
  
   std::ostringstream str;

   GDCM_NAME_SPACE::File *file;
   file = GDCM_NAME_SPACE::File::New();       
      
  // Set the image size
   str.str(""); 
   str << X;
   file->InsertEntryString(str.str(),0x0028,0x0011,"US"); // Columns
   str.str("");
   str << Y;
   file->InsertEntryString(str.str(),0x0028,0x0010,"US"); // Rows

  // Set the pixel type
  //      16; //8, 16, 32
   file->InsertEntryString("16",0x0028,0x0100,"US"); // Bits Allocated
   
   str.str("");
   str << 16; // may be 12 or 16 if componentSize =16
   file->InsertEntryString("16",0x0028,0x0101,"US"); // Bits Stored
   file->InsertEntryString("15",0x0028,0x0102,"US"); // High Bit

  // Set the pixel representation // 0/1 , 0=unsigned
   file->InsertEntryString("1",0x0028,0x0103, "US"); // Pixel Representation
   
  // Set the samples per pixel // 1:Grey level, 3:RGB
   file->InsertEntryString("1",0x0028,0x0002, "US"); // Samples per Pixel


   if (nbFrames != 1)
   {
      str.str("");
      str << nbFrames;
      file->InsertEntryString(str.str(),0x0028,0x0008,"IS"); // Number of Frames  
   }
  
   if (strlen(patientName) != 0)
      file->InsertEntryString(patientName,0x0010,0x0010, "PN"); // Patient's Name

   file->InsertEntryString(studyUID, 0x0020, 0x000d, "UI");
   file->InsertEntryString(serieUID, 0x0020, 0x000e, "UI");
 
   file->InsertEntryString(SerieDescr,0x0008,0x103e, "LO");  // Series Description 

// 0020 0037 DS 6 Image Orientation (Patient)   
   file->InsertEntryString("1.0\\0.0\\0.0\\0.0\\1.0\\0.0",0x0020,0x0037, "DS"); //[1\0\0\0\1\0] : Axial   (Tant pis!)
   
// 0020 0032 DS 3 Image Position (Patient)   
   char charImagePosition[256];
   sprintf(charImagePosition,"0.0\\0.0\\%f",(float)imgNum);
   file->InsertEntryString(charImagePosition,0x0020,0x0032, "DS");  //0020 0032 DS 3 Image Position (Patient)        

// 0020 0x1041 DS 1 Slice Location 
        sprintf(charImagePosition,"%f",float(imgNum));
        file->InsertEntryString(charImagePosition,0x0020,0x1041, "DS");   
/*
  // Set Rescale Intercept
        str.str("");
        str << div;  
        file->InsertEntryString(str.str(),0x0028,0x1052,"DS");

  // Set Rescale Slope
        str.str("");
        str << mini;  
        file->InsertEntryString(str.str(),0x0028,0x1053,"DS");
*/

   GDCM_NAME_SPACE::FileHelper *fileH;
   fileH = GDCM_NAME_SPACE::FileHelper::New(file);
   // cast is just to avoid warnings (*no* conversion is performed)
   //fileH->SetImageData((uint8_t *)img,int(maxX*maxY)*sizeof(uint16_t)); // troubles when maxX, mayY are *actually* float!
   
   fileH->SetImageData((uint8_t *)tabVal,X*Y*nbFrames*sizeof(uint16_t));
   fileH->SetWriteModeToRaw(); 
   fileH->SetWriteTypeToDcmExplVR();
        
   if( !fileH->Write(dcmImageName))
      std::cout << "Failed for [" << dcmImageName << "]\n"
                << "           File is unwrittable" << std::endl;

  // file->Print();
           
  // delete img;
   file->Delete();
   fileH->Delete();  
}



// =====================================================================================================================

