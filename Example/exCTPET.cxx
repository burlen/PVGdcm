/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: exCTPET.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:05 $
  Version:   $Revision: 1.3 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmFile.h"
#include "gdcmSerieHelper.h"
#include "gdcmDirList.h"
#include "gdcmUtil.h"
#include "gdcmDataEntry.h"

int main(int argc, char *argv[])
{
  if(argc < 3 )
    {
    std::cerr << argv[0] << " reference directory" << std::endl;
    return 1;
    }

  // Get the reference & directory name
  const char *reference = argv[1];
  const char *directory = argv[2];

  // Open a file A
  // Open another file B
  // Same Serie/Study ?
  // No -> return
  // Yes -> continue
  // Same Frame of Reference
  // No -> Return
  // Yes -> continue
  // A is CT and B is PT (PET)
  // No -> return
  // Yes -> continue
  // Same Image Position (no string comparison !!)
  // Even if floating point comparison are dangerous, string comp will not work in general case
  // No -> Return
  // Yes: We found a match

  GDCM_NAME_SPACE::File *fileRef = GDCM_NAME_SPACE::File::New();
  fileRef->SetFileName( reference );
  fileRef->SetLoadMode(GDCM_NAME_SPACE::LD_NOSHADOW | GDCM_NAME_SPACE::LD_NOSEQ);
  fileRef->Load();
  // 0008 0060 CS 1 Modality
  std::string modalityRef = fileRef->GetEntryString(0x0008,0x0060);
  if( modalityRef == GDCM_NAME_SPACE::GDCM_UNFOUND ) return 1;
  if ( !GDCM_NAME_SPACE::Util::DicomStringEqual(modalityRef, "CT") ) return 1;
  // 0020 000d UI 1 Study Instance UID
  // 0020 000e UI REL Series Instance UID
  std::string series_uid_ref = fileRef->GetEntryString(0x0020, 0x000e);
  // 0020 0052 UI 1 Frame of Reference UID
  std::string frame_uid_ref = fileRef->GetEntryString(0x0020, 0x0052);
  // 0020 0032 DS 3 Image Position (Patient)
  GDCM_NAME_SPACE::DataEntry *imagePosRef = fileRef->GetDataEntry(0x0020,0x0032);
  assert( imagePosRef->GetValueCount() == 3 );

  GDCM_NAME_SPACE::DirList dirList( directory, true );
  const GDCM_NAME_SPACE::DirListType filenames = dirList.GetFilenames();
  GDCM_NAME_SPACE::DirListType::const_iterator it = filenames.begin();
  GDCM_NAME_SPACE::File *file = GDCM_NAME_SPACE::File::New();
  file->SetLoadMode(GDCM_NAME_SPACE::LD_NOSHADOW | GDCM_NAME_SPACE::LD_NOSEQ);
  for( ; it != filenames.end(); ++it)
    {
    file->SetFileName( *it );
    file->Load();
    std::string modality   = file->GetEntryString(0x0008,0x0060);
    // This is a dual modality: modality should be *different*
    if( modality == modalityRef ) continue;
    if ( !GDCM_NAME_SPACE::Util::DicomStringEqual(modality, "PT") ) continue;
    std::string series_uid = file->GetEntryString(0x0020, 0x000e);
    // Not same series !
    if( series_uid == series_uid_ref ) continue;
    std::string frame_uid = file->GetEntryString(0x0020, 0x0052);
    if( frame_uid_ref != frame_uid ) continue;
    GDCM_NAME_SPACE::DataEntry *imagePos = file->GetDataEntry(0x0020,0x0032);
    assert( imagePos->GetValueCount() == 3 );
    if( imagePos->GetValue(0) == imagePosRef->GetValue(0)
     && imagePos->GetValue(1) == imagePosRef->GetValue(1)
     && imagePos->GetValue(2) == imagePosRef->GetValue(2) )
      {
      std::cerr << "We found a match: " << *it << std::endl;
      }
    }
  return 0;
}

