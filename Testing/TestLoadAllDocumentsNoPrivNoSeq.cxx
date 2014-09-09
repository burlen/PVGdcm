/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestLoadAllDocumentsNoPrivNoSeq.cxx,v $
  Language:  C++
  Date:      $Date: 2008/09/15 15:49:21 $
  Version:   $Revision: 1.6 $
                                                                                
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

int TestLoadAllDocumentsNoPrivNoSeq(int, char *[])
{
   //std::ostringstream s;
   int i = 0;
   //int swapC;
   //unsigned int j;
   std::string pixelType, photomInterp;
   //int l;
   //l = strlen("PALETTE COLOR ");
   while( gdcmDataImages[i] != 0 )
   {
      std::string filename = GDCM_DATA_ROOT;
      filename += "/";  //doh!
      filename += gdcmDataImages[i];

      GDCM_NAME_SPACE::File *f= GDCM_NAME_SPACE::File::New( );
      f->SetFileName( filename );
      // just to improve coverage
      f->SetLoadMode (GDCM_NAME_SPACE::LD_NOSEQ|GDCM_NAME_SPACE::LD_NOSHADOW);
      f->Load();

/*
      f->SetPrintLevel(2);
      f->Print();
      // just to be able to grep the display result, for some usefull info
 
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
 
      photomInterp =  f->GetEntryValue(0x0028,0x0004);               
      std::cout << " Photom.Interp.="       << photomInterp;
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
                                          f->GetEntryValue(0x0020,0x0037);
      if ( strImageOrientationPatient != GDCM_NAME_SPACE::GDCM_UNFOUND )
      {
         GDCM_NAME_SPACE::Orientation o;
         double orient = o.TypeOrientation( f );
         std::cout << " ---------------------- Orientation " << orient
                   << std::endl;
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
 */
      f->Delete();
      i++;
   }
   return 0;
}
