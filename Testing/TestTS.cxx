/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestTS.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:06 $
  Version:   $Revision: 1.11 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmTS.h"
#include "gdcmGlobal.h"

int TestTS(int , char *[])
{
   GDCM_NAME_SPACE::TS *ts = GDCM_NAME_SPACE::TS::New();
   // There should be ~150 entries
   ts->Print( std::cout );

   // Implicit VR Little Endian
   std::cout << ts->IsTransferSyntax( "1.2.840.10008.1.2" ) << std::endl;
   // Implicit VR Big Endian DLX (G.E Private)
   std::cout << ts->IsTransferSyntax( "1.2.840.113619.5.2" ) << std::endl;
   // Explicit VR Little Endian
   std::cout << ts->IsTransferSyntax( "1.2.840.10008.1.2.1" ) << std::endl;
   // Deflated Explicit VR Little Endian
   std::cout << ts->IsTransferSyntax( "1.2.840.10008.1.2.1.99" ) << std::endl;
   // Explicit VR Big Endian
   std::cout << ts->IsTransferSyntax( "1.2.840.10008.1.2.2" ) << std::endl;
   // JPEG Baseline (Process 1)
   std::cout << ts->IsTransferSyntax( "1.2.840.10008.1.2.4.50" ) << std::endl;
   // JPEG Extended (Process 2 & 4)
   std::cout << ts->IsTransferSyntax( "1.2.840.10008.1.2.4.51" ) << std::endl;
   // JPEG Extended (Process 3 & 5)
   std::cout << ts->IsTransferSyntax( "1.2.840.10008.1.2.4.52" ) << std::endl;
   // JPEG Spectral Selection, Non-Hierarchical (Process 6 & 8)
   std::cout << ts->IsTransferSyntax( "1.2.840.10008.1.2.4.53" ) << std::endl;
   // JPEG Full Progression, Non-Hierarchical (Process 10 & 12)
   std::cout << ts->IsTransferSyntax( "1.2.840.10008.1.2.4.55" ) << std::endl;
   // JPEG Lossless, Non-Hierarchical (Process 14)
   std::cout << ts->IsTransferSyntax( "1.2.840.10008.1.2.4.57" ) << std::endl;
   // JPEG Lossless, Hierarchical, First-Order Prediction (Process 14, 
   //               [Selection Value 1])
   std::cout << ts->IsTransferSyntax( "1.2.840.10008.1.2.4.70" ) << std::endl;
   // JPEG 2000 Lossless
   std::cout << ts->IsTransferSyntax( "1.2.840.10008.1.2.4.90" ) << std::endl;
   // JPEG 2000
   std::cout << ts->IsTransferSyntax( "1.2.840.10008.1.2.4.91" ) << std::endl;
   // RLE Lossless
   std::cout << ts->IsTransferSyntax( "1.2.840.10008.1.2.5" ) << std::endl;
   // Unknown
   std::cout << ts->IsTransferSyntax( "Unknown Transfer Syntax" ) << std::endl;

   // Test JPEG test:
   std::cout << "Test TS:" << std::endl;
   std::cout << ts->IsJPEGLossless( "1.2.840.10008.1.2.4.55") << std::endl;
//if ( key == "1.2.840.10008.1.2.4.55"
//  || key == "1.2.840.10008.1.2.4.57"
//  || key == "1.2.840.10008.1.2.4.70" )
   std::cout << ts->IsRLELossless( "1.2.840.10008.1.2.5") << std::endl;
   std::cout << ts->IsJPEGLossless( "1.2.840.10008.1.2.5") << std::endl;
   std::cout << ts->IsJPEG2000( "1.2.840.10008.1.2.5") << std::endl;
   std::cout << ts->IsJPEG( "1.2.840.10008.1.2.5") << std::endl;
   std::cout << ts->GetSpecialTransferSyntax( 
                ts->GetSpecialTransferSyntax( "1.2.840.10008.1.2.5")) << std::endl;
   std::cout << GDCM_NAME_SPACE::Global::GetTS()->IsRLELossless( "1.2.840.10008.1.2.5" )  
             << std::endl;

   bool ret = ts->GetValue( "" ) != GDCM_NAME_SPACE::GDCM_UNFOUND;

   ts->Delete();

   return ret;
}
