/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestHash.cxx,v $
  Language:  C++
  Date:      $Date: 2005/02/02 10:05:26 $
  Version:   $Revision: 1.17 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmCommon.h" //to shut up warnings
// Checks the basic functionalities of STL <map>.
#include <map>
#include <string>
#include <iostream>

int TestHash( int, char *[] )
{
   std::cout << "Test::TestHash : " << std::endl;
   std::cout << "   Checks that the basic STL <map> functionalities required "
             << std::endl
             << "   by gdcm are operational. " << std::endl;

   typedef std::map<std::string, std::string> dict;
   dict tb1;

   // Adding entries by key:

   dict::iterator im = tb1.find("00380010");
   tb1["00100010"] = "Patient Name";
   tb1["7fe00010"] = "Pixel Data";
   tb1["50000010"] = "Number of points";
   tb1["00380010"] = "Admission ID";

   // Travesing the dictionary:

   std::cout << "   Traversal of dictionary (note the proper ordering on key):"
             << std::endl;
   for ( im = tb1.begin(); im != tb1.end(); ++im )
      std::cout << "       \"" << im->first << "\" = " << im->second
                << std::endl;
   std::cout << "   End of dictionary." << std::endl;

   // Find request.

   std::cout << "   Find request on key 00380010" << std::endl;
   im = tb1.find("00380010");
   std::cout << "       \"" << im->first << "\" = " << im->second << std::endl;

   // The following should print in hexadecimal an in decimal the given
   // integer as stated by:
   //   http://www.developer.com/net/cplus/article.php/10919_2119781_3
   // Alas it doesn't work with g++ (at least)...
   int i = 0x0010;
   //std::cout.setf(std::ios::hex);
   std::cout << "Test::TestHash : hexdecimal output : " << std::hex << i << std::endl;
   //std::cout.setf(std::ios::dec);
   std::cout << "Test::TestHash : decimal output : "    << std::dec << i << std::endl;

   return 0;
}
