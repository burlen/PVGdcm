/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestPrintAllDocument.cxx,v $
  Language:  C++
  Date:      $Date: 2008/09/15 15:49:21 $
  Version:   $Revision: 1.18 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

// TODO : check what's *actually* usefull

#include "gdcmDictEntry.h"
#include "gdcmDict.h"
#include "gdcmDictSet.h"
#include "gdcmFile.h"
#include "gdcmUtil.h"
#include "gdcmCommon.h"
#include "gdcmDocEntry.h" 
#include "gdcmDocEntrySet.h"           
#include "gdcmDocument.h"          
#include "gdcmElementSet.h"        
#include "gdcmSeqEntry.h" 
#include "gdcmSQItem.h" 
#include "gdcmOrientation.h"
#include <fstream>
#include <iostream>
#include <iomanip> // for std::ios::left, ...

//Generated file:
#include "gdcmDataImages.h"

int TestPrintAllDocument(int argc, char *argv[])
{
   //std::ostringstream s;
   int i = 0;
   int swapC;
   unsigned int j;
   std::string pixelType, photomInterp;
   unsigned int l;
   l = strlen("PALETTE COLOR ");
   
   //GDCM_NAME_SPACE::Debug::DebugOn();
   
   while( gdcmDataImages[i] != 0 )
   {
      std::string filename;      
      if (argc ==2)
      {
         filename = argv[1];
      }
      else
      {
         filename = GDCM_DATA_ROOT;
         filename += "/";  //doh!      
         filename += gdcmDataImages[i];
      }
      
      std::cout << " ----------------------------------------------"
                << "Begin with " << filename << std::endl;

      GDCM_NAME_SPACE::File *f= GDCM_NAME_SPACE::File::New( );
      f->SetFileName( filename );
      f->Load();

      f->SetPrintLevel(2);
      f->Print();
      
      //s.setf(std::ios::left);
      //s << std::setw(60-filename.length()) << " ";
      //std::cout << s.str() << gdcmDataImages[i];

      std::cout << gdcmDataImages[i];

      unsigned int nbSpaces;
      if (strlen(gdcmDataImages[i]) <= 60)
         nbSpaces = 60-strlen(gdcmDataImages[i]);
      else
         nbSpaces = 0;
      for (j=0; j<nbSpaces; j++)
         std::cout << " ";    

      pixelType = f->GetPixelType();
      std::cout << " pixelType="            << pixelType;
      if ( pixelType == "8U" || pixelType == "8S" )
         std::cout << " ";
      std::cout << " Smpl.P.Pix.="          << f->GetSamplesPerPixel()
                << " Plan.Config.="         << f->GetPlanarConfiguration();

      photomInterp =  f->GetEntryString(0x0028,0x0004);               
      std::cout << " Photom.Interp.="       << photomInterp << " l : " << l <<"length : " << photomInterp.length()<< std::endl;
      
      if (l > photomInterp.length())
        for (j=0; j<l-photomInterp.length(); j++)
           std::cout << " ";
 
      std::cout << " TransferSyntaxName= [" << f->GetTransferSyntaxName() << "]" ;

      swapC = f->GetSwapCode();
      if ( swapC != 1234 )
          std::cout << " SwapCode = "       << f->GetSwapCode(); 
      if ( f->CheckIfEntryExist(0x0088,0x0200) )
          std::cout << " Icon Image Sequence";

      std::cout << std::endl;

      std::string strImageOrientationPatient = 
                                          f->GetEntryString(0x0020,0x0037);
      if ( strImageOrientationPatient != GDCM_NAME_SPACE::GDCM_UNFOUND )
      {
         GDCM_NAME_SPACE::Orientation *o = GDCM_NAME_SPACE::Orientation::New();
 
         GDCM_NAME_SPACE::OrientationType orient = o->GetOrientationType( f );
         std::cout << " ---------------------- Orientation " << orient
                   << std::endl;
         o->Delete(); 
      }

      if( f->IsReadable() )
      {
         std::cout <<filename << " is Readable" 
                   << std::endl << std::endl;
      }
      else
      {
         std::cout << filename << " is NOT Readable" 
                   << std::endl << std::endl;
         f->Delete();
         return 1;
      }
      f->Delete();
      
      if (argc ==2)
         break; // user asked to check a single file.      
      
      i++;
   }
   return 0;
}
