/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestException.cxx,v $
  Language:  C++
  Date:      $Date: 2005/07/08 13:39:57 $
  Version:   $Revision: 1.2 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#include "gdcmException.h"
#include "gdcmDebug.h"
#include "gdcmGlobal.h"
#include "gdcmCommon.h"

struct myException
{
   std::string error;

   myException( std::string ErrorMessage )
   {
      error = ErrorMessage;
   }
   void Print() { std::cerr << "Pfff!" << error << std::endl; }
};



void functionForException()  throw ( myException );
  
void functionForException() 
     throw ( myException )
{
      throw myException("in functionForException : ");
}



int TestException(int , char **)
{  
   try
   {
      functionForException();
   }
   catch (myException)
   {
      std::cout << "Exception 'myException' received" << std::endl;
   }

return 0;
}
