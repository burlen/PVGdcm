/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmJpeg12.cxx,v $
  Language:  C++
  Date:      $Date: 2005/01/31 04:00:04 $
  Version:   $Revision: 1.32 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmFileHelper.h"
#include <stdio.h>

extern "C" {
#include "gdcmjpeg/12/jconfig.h"
#include "gdcmjpeg/12/jpeglib.h"
#include "gdcmjpeg/12/jinclude.h"
#include "gdcmjpeg/12/jerror.h"
}

#define gdcm_write_JPEG_file  gdcm_write_JPEG_file12
#define ReadJPEGFile   ReadJPEGFile12
#define SampBuffer SampBuffer12

#include "gdcmJpeg.cxx"

