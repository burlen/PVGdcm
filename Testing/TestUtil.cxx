/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestUtil.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:06 $
  Version:   $Revision: 1.21 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
// This test should test everything in Util, since I didn't know any other 
// way to test this class.

#include "gdcmUtil.h"
#include "gdcmDebug.h"
#include <iostream>

int TestUtil(int , char *[])
{
   unsigned int i;

   // Seeing at a glance HOW int16, int32, float, double, 
   // are implanted in memory
   uint16_t u16 = 0x0102;
   uint32_t u32 = 0x01020304;
   float flt;
   double dbl=1.0;

   std::cout << "This is gdcm version: " << GDCM_NAME_SPACE::Util::GetVersion() << std::endl;

   std::cout << "---------- uint16 : " << std::dec << u16 << " = 0x" 
             << std::hex << u16 << std::endl;
   for (i=0;i<sizeof(uint16_t);i++) 
   {
      std::cout << std::hex <<"[" <<(uint16_t)((uint8_t*)&u16)[i] << "] " ;
     // printf("[%0x]\n",((uint8_t*)&u16)[i]);
   }
   std::cout << std::endl;

   std::cout << "---------- unit32 : "<< std::dec << u32 << " = 0x" 
             << std::hex << u32 << std::endl;
   for (i=0;i<sizeof(uint32_t);i++) 
   {
      std::cout << std::hex <<"[" <<(uint32_t)((uint8_t*)&u32)[i] << "] " ;
   }
   std::cout << std::endl;
   std::cout << std::endl;
   std::cout << "------------------- float -----------------" << std::endl;
   std::cout << std::endl;
   flt = -5;
   int k;   
   for (k=0; k<11;k++)
   {
      std::cout << "--- " << flt 
             // << " = 0x" 
             // << std::hex << (float *)&flt << std::endl;   
             // << std::hex << (int)(float *)&flt << std::endl;
             // << std::hex << flt 
                << std::endl;
      for (i=0;i<sizeof(float);i++) 
      {
         std::cout << std::hex <<"[" <<(uint16_t)((uint8_t*)&flt)[i] << "] " ;
      }
      flt += 1.0;
      std::cout << std::endl;
   }

   std::cout << std::endl;
   std::cout << std::endl;
   std::cout << "--------------------- double ----------------" << std::endl;
   std::cout << std::endl;

   dbl = -5.0;
   for (k=0; k<11; k++)
   {
      std::cout << "---  " << std::dec << dbl
                << std::endl;
      for (i=0;i<sizeof(double);i++) 
      {
         std::cout << std::hex <<"[" <<(uint16_t)((uint8_t*)&dbl)[i] << "] " ;
      }
      std::cout << std::endl;
      dbl += 1.0;
   }


   std::cout << "Size of short int  " << sizeof(short int) << std::endl;
   std::cout << "Size of int  "       << sizeof(int)       << std::endl;
   std::cout << "Size of int* "       << sizeof(int*)      << std::endl;
   std::cout << "Size of long "       << sizeof(long)      << std::endl;
   std::cout << "Size of float"       << sizeof(float)     << std::endl;
   std::cout << "Size of double"      << sizeof(double)    << std::endl;

   // CreateCleanString
   std::string x = "a#@-bc\tdef";
   std::string y = GDCM_NAME_SPACE::Util::CreateCleanString(x);
   std::cout << "Clean : [" << x <<"] --> [" << y <<"]" << std::endl;


   // CountSubstring : substring id "#@-"
   x = "abcd#@-wyz*@-lmn#@-uvw-#@ijk";
   std::cout << "in [" << x << "] " << std::endl; 
   std::cout << " - count '#@-' : " 
             << GDCM_NAME_SPACE::Util::CountSubstring(x, "#@-") << std::endl;

   // Tokenize : tokens are '#', '@', '-'
   std::vector<std::string> tokens;
   std::cout << " - use tokens '#@-' :" << std::endl;
   GDCM_NAME_SPACE::Util::Tokenize (x, tokens, "#@-");
   for (unsigned int ui=0; ui<tokens.size();ui++)
   {
      std::cout << "[" << tokens[ui] << "]" << std::endl;
   }
   tokens.clear();

   // Time 
   std::cout << "Time:" << GDCM_NAME_SPACE::Util::GetCurrentDateTime() << std::endl;

   // Processor ID
   unsigned int processorID;;
   processorID = GDCM_NAME_SPACE::Util::GetCurrentProcessID();
   std::cout << "Current Processor ID " <<  processorID << std::endl;

   // MAC Adress
   std::cout << "Mac Address:" << GDCM_NAME_SPACE::Util::GetMACAddress() << std::endl;

   // Unique UID test
   std::string gdcmUid;
   for (i=0; i<10; i++)
   {
      gdcmUid = GDCM_NAME_SPACE::Util::CreateUniqueUID();
      std::cout << "Current UID for gdcm " <<  gdcmUid << std::endl;
   }
   
   // DicomString test
   const char ref[] = "MONOCHROME1";
   std::string a = "MONOCHROME1";
   a += '\0';
   std::string b = "MONOCHROME1 ";
   std::string c = GDCM_NAME_SPACE::Util::DicomString("MONOCHROME1");
   std::string d = "MONOCHROME1";

   if( !GDCM_NAME_SPACE::Util::DicomStringEqual(a,ref) ) 
      return 1;
   if( !GDCM_NAME_SPACE::Util::DicomStringEqual(b,ref) ) 
      return 1;
   if( !GDCM_NAME_SPACE::Util::DicomStringEqual(c,ref) ) 
      return 1;
   if(  GDCM_NAME_SPACE::Util::DicomStringEqual(d,ref) ) 
      return 1;

// ----------------------------------------------------------
// Let's test GDCM_NAME_SPACE::Debug, now.
    std::cout << "GetDebugFlag : " << GDCM_NAME_SPACE::Debug::GetDebugFlag() <<std::endl;
    GDCM_NAME_SPACE::Debug::SetOutputFileName ("DummyFileNameToWriteTo.txt");
    std::cout << "We set a Debug file"   <<std::endl;
    if ( !GDCM_NAME_SPACE::Debug::GetDebugFlag() )
    {
       std::cout << "Debug Flag should be TRUE... " << std::endl;
       return 1;
    }
    std::cout << "GetDebugFlag : " << GDCM_NAME_SPACE::Debug::GetDebugFlag()<<std::endl;
    GDCM_NAME_SPACE::Debug::SetDebugFlag ( false );
    std::cout << "GetDebugFlag : " << GDCM_NAME_SPACE::Debug::GetDebugFlag()<<std::endl;
    GDCM_NAME_SPACE::Debug::SetOutputFileName ("DummyFileNameToWriteTo2.txt");    

    std::string s1;
    s1="azertyuiop";
    std::cout << s1 << " --> MD5 : " << GDCM_NAME_SPACE::Util::ConvertToMD5(s1)
              <<std::endl;
    s1="azertyuiom";    
    std::cout <<  s1 << " --> MD5 : " << GDCM_NAME_SPACE::Util::ConvertToMD5(s1)
              <<std::endl;
    s1="azertyuipo";
    std::cout <<  s1 << " --> MD5 : " << GDCM_NAME_SPACE::Util::ConvertToMD5(s1)
              <<std::endl;
    s1="qzertyuiop";
    std::cout <<  s1 << " --> MD5 : " << GDCM_NAME_SPACE::Util::ConvertToMD5(s1)
              <<std::endl; 
    s1="zaertyuiop";
    std::cout <<  s1 << " --> MD5 : " << GDCM_NAME_SPACE::Util::ConvertToMD5(s1)
              <<std::endl;         
   return 0;
}
