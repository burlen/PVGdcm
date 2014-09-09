/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: DenseToDicom.cxx,v $
  Language:  C++
  Date:      $Date: 2007/06/21 15:06:13 $
  Version:   $Revision: 1.3 $
                                                                                
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

#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmDebug.h"
#include "gdcmDirList.h"

#include "gdcmArgMgr.h"

/**
  * \brief   
  *          - explores recursively the given directory
  *          - examines the ".txt" files
  *          - Converts the files into 16 bits Dicom Files,
  */  


void Load(std::ifstream &from, std::string imageName);
//std::ifstream& eatwhite(std::ifstream& is);

/*
window center (level) and width, are defined in the DICOM 
standard very precisely as follows (see PS 3.3 C.11.2.1.2):
>
>"These Attributes are applied according to the following pseudo-code, 
>where :
x is the input value, 
y is an output value with a range from ymin to ymax, 
c is Window Center (0028,1050)
w is Window Width (0028,1051):
>
>           if      (x  <= c - 0.5 - (w-1)/2), then y = ymin
>           else if (x > c - 0.5 + (w-1)/2), then y = ymax,
>           else    y = ((x - (c - 0.5)) / (w-1) + 0.5) * (ymax - ymin)+ ymin

*/
/*
From:   David Clunie - view profile
Date:   Thurs, Jun 1 2006 3:03 pm
Email:  David Clunie <dclu...@dclunie.com>
Groups: comp.protocols.dicom

The value of x is the output of the preceding step, the so-called
"modality LUT", which may either be:

- identity (no rescale values or Modality LUT, or the SOP Class is
  PET and rescale values are ignored), in which case x is the stored
  pixel value in (7FE0,0010)

- Rescale Slope and Intercept (as typically used in CT), in which
  case x is the value obtained from applying the rescale values to
  the stored pixel value

- an actual LUT, in which case x is the value stored in the LUT
  corresponding to the LUT index value that is the stored pixel
  value

The ymin and ymax are intended to represent the output range; for
example, if the hypothetical Presentation LUT step that follows
the VOI LUT (window) stage is an identity operation, then the
ymin and ymax represent P-Values, which might be the range of
digital driving levels for your display (calibrated to the GSDF),
in the 8-bit wide output case ranging from 0 to 255, for example.

The terms brightness and contrast are not used in radiology imaging 
- the window center and width are used instead. 
*/

int main(int argc, char *argv[])
{
   START_USAGE(usage)
   " \n DenseToDicom :\n                                                      ",
   " - explores recursively the given directory,                              ",
   "         - examines the '.txt' files                                      ",
   "         - Converts the files into 16 bits Dicom Files,                   ",
   " usage:                                                                   ",
   " DenseToDicom dirin=rootDirectoryName                                     ",
   "                [listonly] [verbose] [debug]                              ",
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

   const char *dirNamein;   
   dirNamein  = am->ArgMgrGetString("dirin","."); 

   if (am->ArgMgrDefined("debug"))
      GDCM_NAME_SPACE::Debug::DebugOn();
      
   int verbose  = am->ArgMgrDefined("verbose");      
   int listonly = am->ArgMgrDefined("listonly");
   
   // if unused Param we give up
   if ( am->ArgMgrPrintUnusedLabels() )
   { 
      am->ArgMgrUsage(usage);
      delete am;
      return 0;
   }
   delete am;  // we don't need Argument Manager any longer

   // ----- Begin Processing -----
   
   if ( ! GDCM_NAME_SPACE::DirList::IsDirectory(dirNamein) )
   {
      std::cout << "KO : [" << dirNamein << "] is not a Directory." << std::endl;
      return 0;
   }
   else
   {
      std::cout << "OK : [" << dirNamein << "] is a Directory." << std::endl;
   }

   std::string strDirNamein(dirNamein);
   GDCM_NAME_SPACE::DirList dirList(strDirNamein, true); // get recursively the list of files

   if (listonly)
   {
      std::cout << "------------List of found files ------------" << std::endl;
      dirList.Print();
      std::cout << std::endl;
      return 0;
    }
   
   std::string filenameout;
   
   GDCM_NAME_SPACE::DirListType fileNames;
   fileNames = dirList.GetFilenames();
   for (GDCM_NAME_SPACE::DirListType::iterator it = fileNames.begin();  
                                    it != fileNames.end();
                                  ++it)
   {
      std::ifstream from( (*it).c_str() );   
      if ( !from )
      {
         std::cout << "Can't open file" << *it << std::endl;
         //return 0;
      }
      else
      { 
         filenameout = *it + ".dcm";
         std::cout << "Success in open file" << *it << std::endl;
         Load(from, filenameout);
         //return 0;
      }   
   }
}


void Load(std::ifstream &from, std::string imageName)
{
   if (!from)
      return;
 /*     
   uint16_t group;
   uint16_t elem;
   VRKey vr;
   TagName vm;
   TagName name;
*/
   std::string str1;
   int nx, ny;
   // Get nx, ny   
   from >> str1;
   from >> str1;
   from >> str1;
   from >> nx;
   from >> str1;      
   from >> ny;
   
   std::cout << "nx " << nx << " ny " << ny << std::endl;
   
   // Skip 2 lines.
   std::getline(from, str1);
   std::cout << "[" << str1 << "]" << std::endl;
   std::getline(from, str1);
   std::cout << "[" << str1 << "]" << std::endl;
   std::getline(from, str1);
   std::cout << "[" << str1 << "]" << std::endl;
         
   //float *f = new float(nx*ny);
   float *f = (float *) malloc(nx*ny*sizeof(float));
  // float mini = FLT_MAX, maxi = FLT_MIN;
   float val;  
   for( int j=0; j<ny; j++)
   { 
      int l =0;   
      for (int i=0; i<nx; i++)
      {
         //eatwhite(from);
 
    char c;
    for (;;) {
      if (!from.get(c))
        break;
      if (!isspace(c)) {
        from.putback(c);
        break;
      }
    }  
         from >> str1;
         val = (float)atof(str1.c_str());
         *(f+j*nx+i) = val;
 
         // In our concern, just *100 all the values is enough!
         /*
        if (val < mini)
           mini = val;
        if (val > maxi)
            maxi = val;
         //std::cout << val << " ";
        */
        if(from.eof()) 
        {
            std::cout << "Missing values at [" << j <<"," << i << "]" 
                      << std::endl; 
           break;
         }
         l++;           
      }
      //std::cout << std::endl << " line nb : " << j 
      //          << " line length : " << l << std::endl;
         
    } 
    
   // std::cout << "mini : "<< mini  << " maxi : " << maxi << std::endl;
/*
// values are expressed as %.
// It's useless to rescale them as uint16_t : just *100
    uint16_t *img = new uint16_t[ny*nx];
    uint16_t *ptr = img;
    float *tmp = f;
    float div = maxi-mini;
    std::cout << "div : " << div << " mini : " << mini << std::endl;
    for( int k=0; k<ny*nx; k++)
    {
       *ptr = (uint16_t)((*tmp * 65535.0) / div);
       tmp ++;
       ptr++;
    }     
*/

    uint16_t *img = new uint16_t[ny*nx];
    uint16_t *ptr = img;
    float *tmp = f;
    for( int k=0; k<ny*nx; k++)
    {
       *ptr = (uint16_t)(*tmp *100);
       tmp ++;
       ptr++;
    }         
    
 /*
 Vtk/Python Theralys code for Rescale Slope, Rescale Intercept
 
mini,maxi=data.GetScalarRange()
                     minPrec=2
           maxPow=16-minPrec
           if mini<=-pow(2,maxPow):mini=-pow(2,maxPow)+1
           if maxi>pow(2,maxPow):maxi=pow(2,maxPow)
                     maxi=math.ceil(maxi)+1
           mini=math.ceil(mini-.5)
                     slope,intercept=(pow(2,16)-1)/(maxi-mini),-mini
           print mini,maxi,"->",slope,intercept
                     sl=math.ceil(1e6/slope)
           ok=1
           while(ok):
               val=1/(sl/1e6)
               print sl
               if val==int(val):ok=0
               else:sl+=1
                     print sl/1e6, 1/(sl/1e6)
           slope=sl/1e6
                     invSlope=1/slope
           interc=-math.ceil(intercept*1e6)/1e6
                     print invSlope,slope,interc
                               imageSC=vtkImageShiftScale()
           imageSC.SetOutputScalarTypeToUnsignedShort()
           imageSC.SetShift(-interc)
           imageSC.SetScale(invSlope)
           imageSC.SetInput(data)
           imageSC.Update()  
 */ 
 // GDCM_NAME_SPACE::Debug::DebugOn();
  
       std::ostringstream str; 
       GDCM_NAME_SPACE::File *file;
       file = GDCM_NAME_SPACE::File::New();       
              
  // Set the image size
        str.str("");
        str << nx;
        file->InsertEntryString(str.str(),0x0028,0x0011,"US"); // Columns
        str.str("");
        str << ny;
        file->InsertEntryString(str.str(),0x0028,0x0010,"US"); // Rows
  // Set the pixel type
  //      16; //8, 16, 32
        file->InsertEntryString("16",0x0028,0x0100,"US"); // Bits Allocated
        str.str("");
        str << 16; // may be 12 or 16 if componentSize =16
        file->InsertEntryString("16",0x0028,0x0101,"US"); // Bits Stored

        file->InsertEntryString("15",0x0028,0x0102,"US"); // High Bit
  // Set the pixel representation // 0/1

        file->InsertEntryString("0",0x0028,0x0103, "US"); // Pixel Representation
  // Set the samples per pixel // 1:Grey level, 3:RGB
        file->InsertEntryString("1",0x0028,0x0002, "US"); // Samples per Pixel

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
    
    file->Print();
    
    GDCM_NAME_SPACE::FileHelper *fileH;
    fileH = GDCM_NAME_SPACE::FileHelper::New(file);
    // cast is just to avoid warnings (*no* conversion)
    fileH->SetImageData((uint8_t *)img,nx*ny*sizeof(uint16_t));
    fileH->SetWriteModeToRaw(); 
    fileH->SetWriteTypeToDcmExplVR();
    
    fileH->Write(imageName);
    
      if( !fileH->Write(imageName))
             std::cout << "Failed for [" << imageName << "]\n"
                << "           File is unwrittable" << std::endl;
       
   delete img;  
   from.close();
}
