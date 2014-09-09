/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestInline.cxx,v $
  Language:  C++
  Date:      $Date: 2008/09/19 09:33:17 $
  Version:   $Revision: 1.21 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
// This test is expected to 'show' the actual effect on an 'inline' function.
// We exchange 2 numbers
// - with a macro : this is the quicker (any doubt ?)
// - with a function, passing the params by pointer
// - with a function, passing the params by reference (exactly same time)
// - with an inline function described in the .cxx
//                                                   absolutely NO effect ?!?
// - with a function, described in the .h
//                                                   absolutely NO effect ?!?
// - with an inline function, described in the .h
//                                                   absolutely NO effect ?!?
//
// Which CXX_FLAGS, LINKER_FLAGS, ...,  must we set to see the difference?

#include "gdcmUtil.h"
#include <string.h>  // Needed under Suse?!? 
#include <stdlib.h> // atoi
#if defined(__BORLANDC__)  || defined (_MSC_VER)
#include <time.h>
   #if defined(__BORLANDC__)
   #include <stdio.h>
   #endif
#define GET_TIME(a) a=clock()
#define HOW_LONG(b,a)                             \
   std::cout  << (double) (b-a)  << std::endl 

#else
#include <sys/times.h>
#define GET_TIME(a)  times(&a)
#define HOW_LONG(b,a)                             \
   std::cout                                      \
        << (long) ((b.tms_utime) - (a.tms_utime)) \
        << std::endl  
#endif

#include <iostream>

void        frswap (double &a, double &b);
void        fpswap (double *a, double *b);
inline void ifrswap(double &a, double &b);
inline void ifpswap(double *a, double *b);

uint8_t     passDirect8(uint8_t a,  uint8_t b);
uint8_t     passRef8(uint8_t &a, uint8_t &b);
uint8_t     passPtr8(uint8_t *a, uint8_t *b);

uint16_t     passDirect16(uint16_t a,  uint16_t b);
uint16_t     passRef16(uint16_t &a, uint16_t &b);
uint16_t     passPtr16(uint16_t *a, uint16_t *b);

uint32_t     passDirect32(uint32_t a,  uint32_t b);
uint32_t     passRef32(uint32_t &a, uint32_t &b);
uint32_t     passPtr32(uint32_t *a, uint32_t *b);

double     passDirect(double a,  double b);
double     passRef(double &a, double &b);
double     passPtr(double *a, double *b);

#define           \
mswap(a, b)       \
{                 \
   double tmp = a;\
   a   = b;       \
   b   = tmp;     \
}

//  ============= no inline

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

//  ============= inline

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


uint32_t passRef32(uint32_t &a, uint32_t &b)
{
   return a + b;
} 
uint32_t passPtr32(uint32_t *a, uint32_t *b)
{
   return *a + *b;
} 
uint32_t passDirect32(uint32_t a, uint32_t b)
{
   return a + b;
} 


uint16_t passRef16(uint16_t &a, uint16_t &b)
{
   return a + b;
} 
uint16_t passPtr16(uint16_t *a, uint16_t *b)
{
   return *a + *b;
} 
uint16_t passDirect16(uint16_t a, uint16_t b)
{
   return a + b;
} 

uint8_t passRef8(uint8_t &a, uint8_t &b)
{
   return a + b;
} 
uint8_t passPtr8(uint8_t *a, uint8_t *b)
{
   return *a + *b;
} 
uint8_t passDirect8(uint8_t a, uint8_t b)
{
   return a + b;
} 

float passRefFloat(float &a, float &b)
{
   return a + b;
} 
float passPtrFloat(float *a, float *b)
{
   return *a + *b;
} 
float passDirectFloat(float a, float b)
{
   return a + b;
} 

double passRefDouble(double &a, double &b)
{
   return a + b;
} 
double passPtrDouble(double *a, double *b)
{
   return *a + *b;
} 
double passDirectDouble(double a, double b)
{
   return a + b;
} 

int TestInline(int argc, char *argv[])
{

#if defined(__BORLANDC__) || defined (_MSC_VER)
   clock_t tms1, tms2;
#else
   struct tms tms1, tms2;
#endif

// ====================================================================

   std::cout << std::endl << std::endl
             << "Just to be sure : sizes of native types" << std::endl
             << "======================================="      
             << std::endl << std::endl;
   // just to know, on every proc
   std::cout << "Size of char      " << sizeof(char)      << std::endl;   
   std::cout << "Size of short int " << sizeof(short int) << std::endl;
   std::cout << "Size of int       " << sizeof(int)       << std::endl;
   std::cout << "Size of long      " << sizeof(long)      << std::endl;
   std::cout << "Size of float     " << sizeof(float)     << std::endl;
   std::cout << "Size of double    " << sizeof(double)    << std::endl;
   std::cout << "Size of bool      " << sizeof(bool)    << std::endl;   
   std::cout << std::endl;
   std::cout << "Size of char*     " << sizeof(char*)     << std::endl;
   std::cout << "Size of short int*" << sizeof(short int*)<< std::endl;
   std::cout << "Size of int*      " << sizeof(int*)      << std::endl;
   std::cout << "Size of double*   " << sizeof(double*)   << std::endl;
   std::cout <<  "-----------------" << std::endl;   
   
 // ====================================================================
    
   unsigned int nbLoop; 
   unsigned int i;
      
   if (argc > 1)
      nbLoop = atoi(argv[1]);
   else
      nbLoop = 100000000;

   uint8_t  x8 =1, y8 =2;    
   uint16_t x16=1, y16=2;    
   uint32_t x32=1, y32=2;    
   float  fx =1.0f, fy=1.0f;
   double dx =1.0 , dy=1.0;
   double a = 1, b = 2;
   
 // ====================================================================
 
   std::cout << std::endl << std::endl
             << "Check different ways of passing scalars to a function "<< nbLoop << " times"  << std::endl
             << "=====================================================" 
             << std::endl << std::endl; 
    
   std::cout << "Pass uint_8 param directly"
             << std::endl;

   GET_TIME(tms1);    
   for(i = 0 ; i< nbLoop ; i++)
   {
      passDirect8 (x8, y8);  
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);

 // ----------------------------------------
 
   std::cout << "Pass uint_8 param as ref"
             << std::endl;

   GET_TIME(tms1);  
   for(i = 0 ; i< nbLoop ; i++)
   {
      passRef8 (x8, y8);  
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);

 // ----------------------------------------
 
   std::cout << "Pass uint_8 param as ptr"
             << std::endl;

   GET_TIME(tms1);  
   for(i = 0 ; i< nbLoop ; i++)
   {
      passPtr8 (&x8, &y8);  
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);

 // ----------------------------------------
   std::cout << std::endl;
   std::cout << "Pass uint_16 param directly"
             << std::endl;

   GET_TIME(tms1);   
   for(i = 0 ; i< nbLoop ; i++)
   {
      passDirect16 (x16, y16);  
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);

 // ----------------------------------------
 
   std::cout << "Pass uint_16 param as ref"
             << std::endl;

   GET_TIME(tms1);   
   for(i = 0 ; i< nbLoop ; i++)
   {
      passRef16 (x16, y16);  
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);

 // ----------------------------------------
 
   std::cout << "Pass uint_16 param as ptr"
             << std::endl;

   GET_TIME(tms1);   
   for(i = 0 ; i< nbLoop ; i++)
   {
      passPtr16 (&x16, &y16);  
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);

 // ----------------------------------------
   std::cout << std::endl;
   std::cout << "Pass uint_32 param directly"
             << std::endl;

   GET_TIME(tms1);  
   for(i = 0 ; i< nbLoop ; i++)
   {
      passDirect32 (x32, y32);  
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);

 // ----------------------------------------
 
   std::cout << "Pass uint32_t param as ref"
             << std::endl;

   GET_TIME(tms1);    
   for(i = 0 ; i< nbLoop ; i++)
   {
      passRef32 (x32, y32 );  
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);

 // ----------------------------------------
 
   std::cout << "Pass uint_32 param as ptr"
             << std::endl;

   GET_TIME(tms1);    
   for(i = 0 ; i< nbLoop ; i++)
   {
      passPtr32 (&x32, &y32);  
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);

 // ----------------------------------------
   std::cout << std::endl; 
   std::cout << "Pass float param directly"
             << std::endl;

   GET_TIME(tms1);   
   for(i = 0 ; i< nbLoop ; i++)
   {
      passDirectFloat (fx, fy);  
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);

 // ----------------------------------------
 
   std::cout << "Pass float param as ref"
             << std::endl;

   GET_TIME(tms1);    
   for(i = 0 ; i< nbLoop ; i++)
   {
      passRefFloat (fx, fy);  
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);

 // ----------------------------------------
 
   std::cout << "Pass float param as ptr"
             << std::endl;

   GET_TIME(tms1);   
   for(i = 0 ; i< nbLoop ; i++)
   {
      passPtrFloat (&fx, &fy);  
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);

 // ----------------------------------------
   std::cout << std::endl; 
   std::cout << "Pass double param directly"
             << std::endl;

   GET_TIME(tms1);   
   for(i = 0 ; i< nbLoop ; i++)
   {
      passDirectDouble (dx, dy);  
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);

 // ----------------------------------------
 
   std::cout << "Pass double param as ref"
             << std::endl;

   GET_TIME(tms1);  
   for(i = 0 ; i< nbLoop ; i++)
   {
      passRefDouble (dx, dy);  
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);
 // ----------------------------------------
 
   std::cout << "Pass double param as ptr"
             << std::endl;

   GET_TIME(tms1);  
   for(i = 0 ; i< nbLoop ; i++)
   {
      passPtrDouble (&dx, &dy);  
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);

 
// ====================================================================
  
   std::cout << std::endl;
   std::cout << "Exchange 2 scalars " << nbLoop << " times" << std::endl
             << "==================="
             << std::endl << std::endl;
    
 // ----------------------------------------
 
   std::cout << "Direct "<< std::endl;

   GET_TIME(tms1);   
   for(i = 0 ; i< nbLoop ; i++)
   {
      double tmp;
      tmp=a;
      a=b;
      b=tmp;
   }
   GET_TIME(tms2);
   HOW_LONG(tms2,tms1);  

   
 // ----------------------------------------
 
   std::cout << "Use a macro "<< std::endl;

   GET_TIME(tms1);   
   for(i = 0 ; i< nbLoop ; i++)
   {
      mswap (a,b);  
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);  
   
 // ----------------------------------------
 
   std::cout << std::endl;  
   std::cout << "Use a function, param passed by reference" << std::endl;

   GET_TIME(tms1);    
   for(i = 0 ; i< nbLoop ; i++)
   {
      frswap (a,b);  
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1); 
   
 // ----------------------------------------
  
   std::cout << "Use a function, param passed by pointer" << std::endl;

   GET_TIME(tms1);  ;   
   for(i = 0 ; i< nbLoop ; i++)
   {
      fpswap (&a, &b);  
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1); 
   
 // ----------------------------------------
 
   std::cout << std::endl;
 
   std::cout << "Use inline, .cxx-defined function, param passed by reference" << std::endl;

   GET_TIME(tms1);  
   for(i = 0 ; i< nbLoop ; i++)
   {
      ifrswap (a, b);  
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);
   
 // ----------------------------------------
 
   std::cout << "Use inline, .cxx-defined function, param passed by pointer" << std::endl;

   GET_TIME(tms1);     
   for(i = 0 ; i< nbLoop ; i++)
   {
      ifpswap (&a, &b);  
   }
   GET_TIME(tms2);
   HOW_LONG(tms2,tms1);

 // ----------------------------------------

   std::cout << std::endl;
     
//To check the 2 following cases, we just put the two 'static' functions
//hifpswap and  hNoifpswap in gdcmUtil.h
    
   std::cout << "Use inline, .h defined, WITH inline keyword, param passed by pointer"
             << std::endl;

   GDCM_NAME_SPACE::Util util;

   GET_TIME(tms1);    
   for(i = 0 ; i< nbLoop ; i++)
   {
      util.hifpswap (&a, &b);
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);

   
 // ----------------------------------------

   std::cout << "Use inline, .h defined, NO inline keyword, param passed by pointer"
             << std::endl;

   GET_TIME(tms1);     
   for(i = 0 ; i< nbLoop ; i++)
   {
      util.hNoifpswap (&a, &b);
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);

 // ----------------------------------------
   std::cout << std::endl;
   
   std::cout << "Use inline, .h defined, WITH inline keyword, param passed by pointer STATIC function"
             << std::endl;

   GET_TIME(tms1);   
   for(i = 0 ; i< nbLoop ; i++)
   {
      GDCM_NAME_SPACE::Util::sthifpswap (&a, &b);
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);
   
 // ----------------------------------------

   std::cout << "Use inline, .h defined, NO inline keyword, param passed by pointer STATIC function"
             << std::endl;

   GET_TIME(tms1);    
   for(i = 0 ; i< nbLoop ; i++)
   {
      GDCM_NAME_SPACE::Util::sthNoifpswap (&a, &b);
   }
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);
 
 // ----------------------------------------
 
 // Just to point out that playing with pointers doesn't save so much time ...
 
  std::cout << "Play with arrays\n================" << std::endl; 
 
   nbLoop=1000;

   std::cout << "Copy 2 arrays [i][j]"
             << std::endl; 

     
   unsigned short int  z1[128][3118], z2[128][3118];       
   GET_TIME(tms1);  
   for(i = 0 ; i< nbLoop ; i++)
   {
   unsigned short int *pv1=&z1[0][0], *pv2=&z2[0][0];     
   for (int j=0;j<3118;j++)
      for(int i=0; i<128;i++)
         z2[i][j] = z1[i][j];
   }      
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);

   std::cout << "Copy 2 arrays ([i][j], pointer)"
             << std::endl;
       
   GET_TIME(tms1); 
   for(i = 0 ; i< nbLoop ; i++)
   {
   unsigned short int *pv1=&z1[0][0], *pv2=&z2[0][0];
   for (int j=0;j<3118;j++)
      for(int i=0; i<128;i++)
         z2[i][j] = *pv1++;
   }      
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);
   
   std::cout << "Copy 2 arrays (2 pointers)"    << std::endl;      
   GET_TIME(tms1);     
  // unsigned short int  w1[3118*128], w2[3118][128];
   for(i = 0 ; i< nbLoop ; i++)
   {
   unsigned short int *pw1=&z1[0][0], *pw2=&z2[0][0];  
   for (int j=0;j<3118;j++)
      for(int i=0; i<128;i++)
         *pw2++ = *pw1++;
   }      
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);  


   std::cout << "Copy 2 arrays (memcpy)"    << std::endl;   
   GET_TIME(tms1);    
   for(i = 0 ; i< nbLoop ; i++)
   {
    unsigned short int *pw1=&z1[0][0], *pw2=&z2[0][0];  
    memcpy(pw2,pw1,3118*128*sizeof(unsigned short int));
   }      
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);   
   
   
         
   std::cout << "Transpose 2 arrays [i][j]"
             << std::endl; 
     
   unsigned short int  t1[3118][128], t2[128][3118];       
   GET_TIME(tms1);  
   for(i = 0 ; i< nbLoop ; i++)
   {
   unsigned short int *pv1=&t1[0][0], *pv2=&t2[0][0];     
   for (int j=0;j<3118;j++)
      for(int i=0; i<128;i++)
         t2[i][j] = t1[j][i];
   }      
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);


   std::cout << "Transpose 2 arrays ([i][j], pointer)"
             << std::endl;
     
   unsigned short int  w1[3118*128], w2[3118][128];      
   GET_TIME(tms1); 
   for(i = 0 ; i< nbLoop ; i++)
   {
   unsigned short int *pw1=w1, *pw2=&w2[0][0];  
   for (int j=0;j<3118;j++)
      for(int i=0; i<128;i++)
         w2[i][j] = *pw1++;
   }      
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);
   
      
   std::cout << "Transpose 2 arrays (2 pointers)"
             << std::endl; 
     
   unsigned short int  v1[3118*128], v2[128*3118];       
   GET_TIME(tms1);  
   for(i = 0 ; i< nbLoop ; i++)
   {
   unsigned short int *pv1=v1, *pv2=v2;  
   for (int j=0;j<3118;j++)
      for(int i=0; i<128;i++)
         *(pv2+i*128+j) = *pv1++;
   }      
   GET_TIME(tms2);   
   HOW_LONG(tms2,tms1);


   
   //return 1; // will generate an error, 
             // just to allow us to see the full log in the dashboard
   return 0;
}
