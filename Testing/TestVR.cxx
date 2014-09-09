/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestVR.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:06 $
  Version:   $Revision: 1.13 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmVR.h"
#include "gdcmDebug.h"

int TestVR(int , char *[])
{
   int error = 0;
   GDCM_NAME_SPACE::VR *vr = GDCM_NAME_SPACE::VR::New();
 
   GDCM_NAME_SPACE::Debug::DebugOn();

   // We should test the 27 entries ...
   vr->Print( std::cout );

   // Valid VR
   if( !vr->IsValidVR( "PN" ) )
   {
      std::cerr << "'PN' is not recognized as a valid VR" << std::endl;
      error++;
   }
   if( !vr->IsValidVR( "FD" ) )
   {
      std::cerr << "'FD' is not recognized as a valid VR" << std::endl;
      error++;
   }
   if( vr->IsValidVR( "" ) )
   {
      std::cerr << "'' is recognized as a valid VR" << std::endl;
      error++;
   }
   if( vr->IsValidVR( "  " ) )
   {
      std::cerr << "'  ' is recognized as a valid VR" << std::endl;
      error++;
   }
   if( vr->IsValidVR( "\000/" ) )
   {
      std::cerr << "' /' is recognized as a valid VR" << std::endl;
      error++;
   }
   if( vr->IsValidVR( GDCM_NAME_SPACE::GDCM_VRUNKNOWN ) )
   {
      std::cerr << "'  ' is recognized as a valid VR" << std::endl;
      error++;
   }

   // String representable
   //---------------------
   if( !vr->IsVROfStringRepresentable( "PN" ) )
   {
      std::cerr << "'PN' is not recognized as a string representable" << std::endl;
      error++;
   }
   if( vr->IsVROfStringRepresentable( "OB" ) )
   {
      std::cerr << "'OB' is recognized as a string representable" << std::endl;
      error++;
   }

   // Binary representable
   //---------------------
   if( !vr->IsVROfBinaryRepresentable( "OB" ) )
   {
      std::cerr << "OB is not recognized as a binary representable" << std::endl;
      error++;
   }
   if( vr->IsVROfBinaryRepresentable( "PN" ) )
   {
      std::cerr << "'PN' is a binary representable" << std::endl;
      error++;
   }

   // Sequence
   //---------
   if( vr->IsVROfSequence( "" ) )
   {
      std::cerr << "'' is a sequence" << std::endl;
      error++;
   }
   if( !vr->IsVROfSequence( "SQ" ) )
   {
      std::cerr << "'SQ' is not recognized as a Sequence" << std::endl;
      error++;
   }

   vr->Delete();
   return error;
}
