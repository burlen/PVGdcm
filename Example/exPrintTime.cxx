/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: exPrintTime.cxx,v $
  Language:  C++
  Date:      $Date: 2007/06/21 15:01:00 $
  Version:   $Revision: 1.2 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

// TODO : check what's *actually* usefull

#include "gdcmFile.h"
#include "gdcmUtil.h"
#include "gdcmCommon.h"

#include <iostream>
#include <time.h>
#include <sys/times.h>

#include <iomanip> // for std::ios::left, ...

// Where is it?
//#define CLK_TCK 1  ///usr/include/bits/time.h:41: 
//                      warning: this is the location of the previous definition


//Generated file:
#include "gdcmDataImages.h"
int main(int, char *[])
{
   clock_t r1, r2, r3, r4;
   struct tms tms1, tms2, tms3, tms4;
   GDCM_NAME_SPACE::File *e1;

   //std::ostream os = std::cout;
   //std::ostringstream s;

   int i = 0;
   while( gdcmDataImages[i] != 0 )
   {
      std::string filename = GDCM_DATA_ROOT;
      filename += "/";  //doh!
      filename += gdcmDataImages[i];

      e1= new GDCM_NAME_SPACE::File( );
      r1 = times(&tms1);
      e1->SetFileName( filename );
      e1->Load( );
      r2 = times(&tms2);
      if (!e1->IsReadable())
      std::cout << "-----------Not Readable " << std::endl;
      delete e1;
      e1= new GDCM_NAME_SPACE::File( );
      e1->SetLoadMode( GDCM_NAME_SPACE::LD_NOSEQ | GDCM_NAME_SPACE::LD_NOSHADOW );
      r3 = times(&tms3);
      e1->SetFileName( filename );
      e1->Load( );
      
      r4 = times(&tms4);
      if (!e1->IsReadable())
         std::cout << "-----------Not Readable " << std::endl;
      delete e1;
 
      std::cout 
        << std::setw(60-strlen(gdcmDataImages[i])) << " "
        << gdcmDataImages[i] << "   " 
        << (long) ((tms2.tms_utime)  - (tms1.tms_utime)) 
        << " vs  "
        << (long) ((tms4.tms_utime)  - (tms3.tms_utime));
                
      std::cout << "\twith clock time : " << r2-r1 << " vs " << r4-r3 
          << std::endl;

      i++;
   }

   std::cout << "---------------------------------------------------" << std::endl;

   r1 = times(&tms1);
   for(int k=0;k<1; k++) // run many tiles to increase time ...
   {
      i=0;
      while( gdcmDataImages[i] != 0 )
      {
         std::string filename = GDCM_DATA_ROOT;
         filename += "/";  //doh!
         filename += gdcmDataImages[i];

         e1= new GDCM_NAME_SPACE::File( );
         e1->SetFileName( filename );
         e1->Load( ); 
         if (!e1->IsReadable())
            std::cout << "-----------Not Readable " << std::endl;
         delete e1;

        i++;
    }
 
 }
   r2 = times(&tms2);

// ---------------------------
   r3 = times(&tms3);
   for(int k=0;k<1; k++)
   {

     i=0;
     while( gdcmDataImages[i] != 0 )
     {
        std::string filename = GDCM_DATA_ROOT;
        filename += "/";  //doh!
        filename += gdcmDataImages[i];

        e1= new GDCM_NAME_SPACE::File( );
        e1->SetLoadMode( GDCM_NAME_SPACE::LD_NOSEQ | GDCM_NAME_SPACE::LD_NOSHADOW );
        e1->SetFileName( filename );
        e1->Load( );
        if (!e1->IsReadable())
          std::cout << "-----------Not Readable " << std::endl;
        delete e1;
        i++;
      }
   }
    r4 = times(&tms4);

    std::cout  
          << (long) ((tms2.tms_utime)  - (tms1.tms_utime)) 
          << "\t\t "
          << (long) ((tms4.tms_utime)  - (tms3.tms_utime)) 
          << std::endl;
          std::cout << "---------------------------------------------------" << std::endl;

   return 0;
}
