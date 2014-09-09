/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: AnonymizeNoLoad.cxx,v $
  Language:  C++
  Date:      $Date: 2007/06/06 13:03:57 $
  Version:   $Revision: 1.21 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmCommon.h"
#include "gdcmDebug.h"
#include "gdcmDirList.h"

#include "gdcmArgMgr.h"

#include <iostream>

int main(int argc, char *argv[])
{
   START_USAGE(usage)
   "\n AnonymizeNoLoad :\n                                                    ",
   "Anonymize a gdcm-parsable Dicom image even if pixels aren't gdcm readable ",
   "         Warning : the image is OVERWRITTEN                               ",
   "                   to preserve image integrity, use a copy.               ",
   "usage: AnonymizeNoLoad {filein=inputFileName|dirin=inputDirectoryName}    ",
   "                       [rubout=listOfElementsToRubOut]                    ",
   "                       [ { [noshadowseq] | [noshadow][noseq] } ] [debug]  ",
   "       inputFileName : Name of the (single) file user wants to anonymize  ",
   "       inputDirectoryName : user wants to anonymize *all* the files       ",
   "                            within the (single Patient!) directory        ",
   "       listOfElementsToRubOut : group-elem,g2-e2,... (in hexa, no space)  ",
   "                                of extra Elements to rub out              ",
   "       noshadowseq: user doesn't want to load Private Sequences           ",
   "       noshadow   : user doesn't want to load Private groups (odd number) ",
   "       noseq      : user doesn't want to load Sequences                   ",
   "       debug      : developper wants to run the program in 'debug mode'   ",
   FINISH_USAGE

   // ----- Initialize Arguments Manager ------
  
   GDCM_NAME_SPACE::ArgMgr *am = new GDCM_NAME_SPACE::ArgMgr(argc, argv);
  
   if (am->ArgMgrDefined("usage") || argc == 1) 
   {
      am->ArgMgrUsage(usage); // Display 'usage'
      delete am;
      return 0;
   }

   if (am->ArgMgrDefined("debug"))
      GDCM_NAME_SPACE::Debug::DebugOn();

   const char *fileName = am->ArgMgrGetString("filein");
   const char *dirName  = am->ArgMgrGetString("dirin");

   if ( (fileName == 0 && dirName == 0)
        ||
        (fileName != 0 && dirName != 0) )
   {
       std::cout <<std::endl
                 << "Either 'filein=' or 'dirin=' must be present;" 
                 << std::endl << "Not both" << std::endl;
       am->ArgMgrUsage(usage); // Display 'usage'  
       delete am;
       return 0;
 }
 
   int loadMode = GDCM_NAME_SPACE::LD_ALL;
   if ( am->ArgMgrDefined("noshadowseq") )
      loadMode |= GDCM_NAME_SPACE::LD_NOSHADOWSEQ;
   else 
   {
      if ( am->ArgMgrDefined("noshadow") )
         loadMode |= GDCM_NAME_SPACE::LD_NOSHADOW;
      if ( am->ArgMgrDefined("noseq") )
         loadMode |= GDCM_NAME_SPACE::LD_NOSEQ;
   }

   int rubOutNb;
   uint16_t *elemsToRubOut = am->ArgMgrGetXInt16Enum("rubout", &rubOutNb);

   /* if unused Param we give up */
   if ( am->ArgMgrPrintUnusedLabels() )
   {
      am->ArgMgrUsage(usage);
      delete am;
      return 0;
   } 
 
   delete am;  // ------ we don't need Arguments Manager any longer ------

   GDCM_NAME_SPACE::File *f;
   if ( fileName != 0 ) // ====== Deal with a single file ======
   {

   // 
   //   Parse the input file.
   // 
      f = GDCM_NAME_SPACE::File::New( );
      f->SetLoadMode(loadMode);
      f->SetFileName( fileName );

      // GDCM_NAME_SPACE::File::IsReadable() is no usable here, because we deal with
      // any kind of GDCM_NAME_SPACE::Readable *document*
      // not only GDCM_NAME_SPACE::File (as opposed to GDCM_NAME_SPACE::DicomDir)
      if ( !f->Load() ) 
      {
          std::cout <<std::endl
              << "Sorry, " << fileName <<"  not a gdcm-readable "
              << "DICOM / ACR Document"
              << std::endl;
           f->Delete();
           return 1;
      }
      std::cout << fileName << " is readable " << std::endl;

      // 
      //      No need to load the pixels in memory.
      //      File will be overwritten
      // 

      // 
      //  Choose the fields to anonymize.
      // 

      // Institution name 
      f->AddAnonymizeElement( 0x0008, 0x0080, "gdcm-Xanadoo Hospital" ); 
      // Patient's name 
      f->AddAnonymizeElement( 0x0010, 0x0010, "gdcm^Fantomas" );      
      // Patient's ID
      f->AddAnonymizeElement( 0x0010, 0x0020,"1515" );
      // Patient's Birthdate
      f->AddAnonymizeElement( 0x0010, 0x0030,"11111111" );
      // Patient's Adress
      f->AddAnonymizeElement( 0x0010, 0x1040,"gdcm at Sing-sing.org" );
      // Patient's Mother's Birth Name
      f->AddAnonymizeElement( 0x0010, 0x1060,"gdcm^Vampirella" );      
      // Study Instance UID
      f->AddAnonymizeElement( 0x0020, 0x000d, "9.99.999.9999" );
      // Telephone
      f->AddAnonymizeElement(0x0010, 0x2154, "3615" );

      for (int ri=0; ri<rubOutNb; ri++)
      {
         printf("%04x,%04x\n",elemsToRubOut[2*ri], elemsToRubOut[2*ri+1]);
         f->AddAnonymizeElement((uint32_t)elemsToRubOut[2*ri], 
                                (uint32_t)elemsToRubOut[2*ri+1],"*" ); 
      }

      // Aware use will add new fields here

      // 
      //      Overwrite the file
      // 

      std::cout <<"Let's AnonymizeNoLoad " << std::endl;

      // The GDCM_NAME_SPACE::File remains untouched in memory

      f->AnonymizeNoLoad();

      // No need to write the file : modif were done on disc !
      //  ( The Dicom file is overwritten )
      std::cout <<"End AnonymizeNoLoad" << std::endl;

      // 
      //      Remove the Anonymize list
      //   
      f->ClearAnonymizeList();
 
      f->Delete();
      return 0;

   }
   else  // ====== Deal with a (single Patient) Directory ======
   {
      std::cout << "dirName [" << dirName << "]" << std::endl;
      GDCM_NAME_SPACE::DirList dirList(dirName,1); // gets recursively the file list
      GDCM_NAME_SPACE::DirListType fileList = dirList.GetFilenames();
      for( GDCM_NAME_SPACE::DirListType::iterator it  = fileList.begin();
                                 it != fileList.end();
                                 ++it )
      {
         f = GDCM_NAME_SPACE::File::New( );
         f->SetLoadMode(loadMode);
         f->SetFileName( it->c_str() );

         if ( !f->Load() )
         {
            f->Delete();
            continue;
         }
         // 
         //  Choose the fields to anonymize.
         // 
 
         // Institution name 
         f->AddAnonymizeElement( 0x0008, 0x0080, "gdcm-Xanadoo Hospital" ); 
         // Patient's name 
         f->AddAnonymizeElement( 0x0010, 0x0010, "gdcm^Fantomas" );   
         // Patient's ID
         f->AddAnonymizeElement( 0x0010, 0x0020,"1515" );
         // Patient's Birthdate
         f->AddAnonymizeElement( 0x0010, 0x0030,"11111111" );
         // Patient's Adress
         f->AddAnonymizeElement( 0x0010, 0x1040,"gdcm at Sing-sing.org" );
         // Patient's Mother's Birth Name
         f->AddAnonymizeElement( 0x0010, 0x1060,"gdcm^Vampirella" );   
         // Study Instance UID
         // we may not brutaly overwrite it
         //f->AddAnonymizeElement( 0x0020, 0x000d, "9.99.999.9999" );
         // Telephone
         f->AddAnonymizeElement(0x0010, 0x2154, "3615" );

         // deal with user defined Elements set

         for (int ri=0; ri<rubOutNb; ri++)
         {
            f->AddAnonymizeElement((uint32_t)elemsToRubOut[2*ri], 
                                   (uint32_t)elemsToRubOut[2*ri+1],"*" ); 
         }        
         std::cout <<"Let's AnonymizeNoLoad " << it->c_str() << std::endl;

         // The GDCM_NAME_SPACE::File remains untouched in memory
         // The Dicom file is overwritten on disc

         f->AnonymizeNoLoad();

         // 
         //   Remove the Anonymize list
         //

         f->ClearAnonymizeList();
    
         f->Delete();
      }
   }
   return 0;
}

