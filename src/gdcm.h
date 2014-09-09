/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcm.h,v $
  Language:  C++
  Date:      $Date: 2005/11/18 14:40:36 $
  Version:   $Revision: 1.56 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef GDCM_H
#define GDCM_H

// General purpose include file. This file should be included by
// external users of gdcm. It exposes the necessary API.

// No user should use any ".h" file that doesn't appear here!

#include "gdcmCommon.h"
#include "gdcmUtil.h"
#include "gdcmDebug.h"

#include "gdcmDictEntry.h"
#include "gdcmDict.h"
#include "gdcmDictSet.h"

#include "gdcmDataEntry.h"
#include "gdcmSeqEntry.h"
#include "gdcmSQItem.h"

#include "gdcmDocument.h"
#include "gdcmFile.h"

#include "gdcmFileHelper.h"
#include "gdcmSerieHelper.h"
#include "gdcmOrientation.h"

#include "gdcmDicomDir.h"
#include "gdcmDicomDirPatient.h"
#include "gdcmDicomDirStudy.h"
#include "gdcmDicomDirSerie.h"
#include "gdcmDicomDirImage.h"
#include "gdcmDicomDirVisit.h"

#include "gdcmValidator.h"

#endif
