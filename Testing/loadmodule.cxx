/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: loadmodule.cxx,v $
  Language:  C++
  Date:      $Date: 2008/09/15 15:49:21 $
  Version:   $Revision: 1.4 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
/* For some bizarre reason global symbol, mainly std::string
 * are creating seg fault when loading a shared lib, build on top
 * of a static gdcm lib */

#include "gdcmFile.h"
#include <dlfcn.h>

int main(int, char* [])
{
  //typedef itk::DynamicLoader dl;
  typedef void* lh;
  
  // the type of function stored in the dynamic lib 
  typedef void (*fnptr)(void);
  
  //This function gives a segmentation fault.
  //std::cerr << "Last error before opening: " << dl::LastError() << std::endl;

  //Fill in here  <path_to_lib>/libadll.so"
  std::string libname = GDCM_LIBRARY_OUTPUT_PATH "/libdynmodule.so";

  //open the dynamic lib
  //lh handle =  dl::OpenLibrary( libname.c_str() );
  lh handle =  dlopen(libname.c_str(), RTLD_LAZY);
  std::cerr << "address of handle: " << handle << std::endl;
  if (!handle)
    {
    std::cerr << "Ooops cannot open lib:" << libname << std::endl;
    return 1;
    }
 
  //std::cerr << "Error after opening: " << dl::LastError() << std::endl;

  
  std::cerr << "Getting function address from lib" << std::endl;
  void * aptr = dlsym(handle, "afunc");
  
  if (aptr == 0)
  {
    std::cerr << "error in finding function" << std::endl;
    //const char *er = dlerror();
    //std::cerr << "dl says: " << dl::LastError() << std::endl;

    std::cerr << "Closing libs" << std::endl;
    //dl::CloseLibrary(handle);
    dlclose(handle);
    //std::cerr << "dl says: " << dl::LastError() << std::endl;

    std::cerr << "Exiting" << std::endl;
    return 1;
  }

  
  std::cerr << "Casting function address from lib" << std::endl;
  fnptr afuncptr = (fnptr)(aptr);

  std::cerr << "Calling function from lib" << std::endl;
  afuncptr(); 

  // Create an instance of the GDCMImageIO class. This makes sure
  // that the problematic gdcm-library is linked.
  //itk::GDCMImageIO::Pointer gdcmio = itk::GDCMImageIO::New();
  GDCM_NAME_SPACE::File *file = GDCM_NAME_SPACE::File::New();
  file->Print(); // to avoid warning
  
  std::cerr << "Closing libs" << std::endl;
  //dl::CloseLibrary(handle);
  dlclose(handle);

  //std::cerr << "dl says: " << dl::LastError() << std::endl;
  
  std::cerr << "Exiting" << std::endl;
  return 0;
  
}


