/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: exInline.cxx,v $
  Language:  C++
  Date:      $Date: 2007/06/21 15:01:00 $
  Version:   $Revision: 1.3 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
// This test is expected to 'show' the actual effect on an 'inline' function.
// We exchange 2 numbers
// - with a macro : this is the quicker (anny doubt ?)
// - with a function, passing the params by pointer
// - with a function, passing the params by reference (exactly same time)
// - with an inline function described in the main() : absolutely NO effect ?!?
// - with an inline function described in the .h     : absolutely NO effect ?!?
//
// Must we ask optimization to see the difference?

#include "gdcmUtil.h"
#include "gdcmDebug.h"
#include <iostream>

#include <time.h>
#include <sys/times.h>

void        frswap (double &a, double &b);
void        fpswap (double *a, double *b);
inline void ifrswap(double &a, double &b);
inline void ifpswap(double *a, double *b);


#define       \
mswap(a, b)   \
{             \
   tmp = a;   \
   a   = b;   \
   b   = tmp; \
}

void frswap(double &a, double &b)
{
   double tmp;
   tmp = a;
   a   = b;
   b   = tmp;

}

void fpswap(double *a, double *b)
{
   double tmp;
   tmp = *a;
   *a  = *b;
   *b  = tmp;

}

inline void ifpswap(double *a, double *b)
{
   double tmp;
   tmp = *a;
   *a  = *b;
   *b  = tmp;
}

inline void ifrswap(double &a, double &b)
{
   double tmp;
   tmp = a;
   a   = b;
   b   = tmp;
}

int main(int argc, char *argv[])
{

uint32_t a1 = 0xfedcba98;
uint64_t b1 = a1<<8;
std::cout<<  "sizeof(uint32_t) " << sizeof(uint32_t) 
         << " sizeof(uint64_t) " << sizeof(uint64_t) << std::endl;

std::cout<< std::hex <<a1 << " " << b1  << std::endl;
b1 = 0xfedcba98;
uint64_t b2= 0x76543210;
b1= b1<<32|b2;
std::cout<< std::hex <<b1 << " " << b2  << std::endl;

   int nbLoop;  
   if (argc > 1)
      nbLoop = atoi(argv[1]);
   else
      nbLoop = 100000000;      
   unsigned int i;
   clock_t r1, r2;
   struct tms tms1, tms2;
   
   double a = 1, b = 2;
   double tmp; 
 // ----------------------------------------
 
   std::cout << "Use a macro "<< std::endl;
   r1 = times(&tms1);   
   for(i = 0 ; i< nbLoop ; i++)
   {
      mswap (a,b);  
   }
   r2 = times(&tms2);
   std::cout 
        << (long) ((tms2.tms_utime)  - (tms1.tms_utime)) 
        << std::endl;
   
 // ----------------------------------------
 
   std::cout << "Use reference function" << std::endl;
   r1 = times(&tms1);         
   for(i = 0 ; i< nbLoop ; i++)
   {
      frswap (a,b);  
   }
   r2 = times(&tms2);
   std::cout 
        << (long) ((tms2.tms_utime)  - (tms1.tms_utime)) 
        << std::endl; 
   
 // ----------------------------------------
  
   std::cout << "Use pointer function" << std::endl;
   r1 = times(&tms1);     
   for(i = 0 ; i< nbLoop ; i++)
   {
      fpswap (&a, &b);  
   }
   r2 = times(&tms2);
   std::cout 
        << (long) ((tms2.tms_utime)  - (tms1.tms_utime)) 
        << std::endl;  
   
 // ----------------------------------------
 
   std::cout << "Use inline, main-defined reference function" << std::endl;
   r1 = times(&tms1);     
   for(i = 0 ; i< nbLoop ; i++)
   {
      ifrswap (a, b);  
   }
   r2 = times(&tms2);
   std::cout 
        << (long) ((tms2.tms_utime)  - (tms1.tms_utime)) 
        << std::endl;    
   
 // ----------------------------------------
 
   std::cout << "Use inline, main-defined pointer function" << std::endl;
   r1 = times(&tms1);     
   for(i = 0 ; i< nbLoop ; i++)
   {
      ifpswap (&a, &b);  
   }
   r2 = times(&tms2);
   std::cout 
        << (long) ((tms2.tms_utime)  - (tms1.tms_utime)) 
        << std::endl;

/*  
//To check the 2 following cases, just put the two 'static' functions
//hifpswap and  hNoifpswap in a .h

   static inline void hifpswap(double *a, double *b)    
   {
      double tmp;
      tmp = *a;
      *a = *b;
      *b = tmp;
   }

   static void hNoifpswap(double *a, double *b)    
   {
      double tmp;
      tmp = *a;
      *a = *b;
      *b = tmp;
   }
    
 // ----------------------------------------
    
   std::cout << "Use inline, .h defined, WITH inline keyword pointer function"
             << std::endl;
   r1 = times(&tms1);     
   for(i = 0 ; i< nbLoop ; i++)
   {
      GDCM_NAME_SPACE::Util::hifpswap (&a, &b);  
   }
   r2 = times(&tms2);
   std::cout 
        << (long) ((tms2.tms_utime)  - (tms1.tms_utime)) 
        << std::endl;  

   
 // ----------------------------------------

   std::cout << "Use inline, .h defined, NO inline keyword pointer function"
             << std::endl;
   r1 = times(&tms1);     
   for(i = 0 ; i< nbLoop ; i++)
   {
      GDCM_NAME_SPACE::Util::hNoifpswap (&a, &b);  
   }
   r2 = times(&tms2);
   std::cout 
        << (long) ((tms2.tms_utime)  - (tms1.tms_utime)) 
        << std::endl; 
*/ 

}
