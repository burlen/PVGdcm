/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestImageSet.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:06 $
  Version:   $Revision: 1.8 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

/**
 * Write a dicom file from nothing
 * The written image is 256x256, 8 bits, unsigned char
 * The image content is a horizontal grayscale from 
 * 
 */
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmDataEntry.h"
#include "gdcmUtil.h"
#include "gdcmDebug.h"

#include <iostream>
#include <sstream>
#include <list>

typedef std::list<GDCM_NAME_SPACE::File *> FileList;

// If there is sameSerie, sameStudy is set to true
int CompareImages(FileList &list, bool sameSerie, bool sameStudy)
{
   GDCM_NAME_SPACE::Debug::DebugOn();

   if( sameSerie )
      sameStudy = true;

   GDCM_NAME_SPACE::DataEntry *entry;
   std::map<std::string, int> instUID;
   std::map<std::string, int> mediaUID;
   std::map<std::string, int> serieUID;
   std::map<std::string, int> studyUID;

   FileList::iterator it;
   for(it=list.begin();it!=list.end();++it)
   {
      // SOP Instance UID
      entry=(*it)->GetDataEntry(0x0008, 0x0018);
      if( entry )
         if( instUID.find(entry->GetString())!=instUID.end() )
            instUID[entry->GetString()]++;
         else
            instUID[entry->GetString()]=1;
      // Media Storage SOP Instance UID
      entry=(*it)->GetDataEntry(0x0002,0x0003);
      if( entry )
         if( mediaUID.find(entry->GetString())!=mediaUID.end() )
            mediaUID[entry->GetString()]++;
         else
            mediaUID[entry->GetString()]=1;
      // Series Instance UID
      entry=(*it)->GetDataEntry(0x0020,0x000e);
      if( entry )
         if( serieUID.find(entry->GetString())!=serieUID.end() )
            serieUID[entry->GetString()]++;
         else
            serieUID[entry->GetString()]=1;
      // Study Instance UID
      entry=(*it)->GetDataEntry(0x0020,0x000d);
      if( entry )
         if( studyUID.find(entry->GetString())!=studyUID.end() )
            studyUID[entry->GetString()]++;
         else
            studyUID[entry->GetString()]=1;
   }

   if( sameSerie )
   {
      if( serieUID.size()>1 )
      {
         std::cout << "Failed\n"
                   << "        Series UID not same (0x0020,0x000e)\n";
         return 1;
      }
   }
   else
   {
      if( serieUID.size()!=list.size() )
      {
         std::cout << "Failed\n"
                   << "        Some Series UID are same (0x0020,0x000e)\n";
         return 1;
      }
   }

   if( sameStudy )
   {
      if( studyUID.size()>1 )
      {
         std::cout << "Failed\n"
                   << "        Studies UID not same (0x0020,0x000d)\n";
         return 1;
      }
   }
   else
   {
      if( studyUID.size()!=list.size() )
      {
         std::cout << "Failed\n"
                   << "        Some Studies UID are same (0x0020,0x000d)\n";
         return 1;
      }
   }

   if( mediaUID.size()!=list.size() )
   {
      std::cout << "Failed\n"
                << "        Some Media UID are same (0x0002,0x0003)\n";
      return 1;
   }

   if( instUID.size()!=list.size() )
   {
      std::cout << "Failed\n"
                << "        Some Instance UID are same (0x0008,0x0018)\n";
      return 1;
   }

   return 0;
}

void ClearList(FileList &list)
{
   FileList::iterator it;
   for(it=list.begin();it!=list.end();++it)
   {
      (*it)->Delete();
   }
   list.clear();
}

GDCM_NAME_SPACE::File *WriteImage(GDCM_NAME_SPACE::File *file, const std::string &fileName)
{
   // Create a 256x256x1 image 8 bits, unsigned 
   std::ostringstream str;

   // Set the image size
   file->InsertEntryString("256",0x0028,0x0011,"US"); // Columns
   file->InsertEntryString("256",0x0028,0x0010,"US"); // Rows

   // Set the pixel type
   file->InsertEntryString("8",0x0028,0x0100,"US"); // Bits Allocated
   file->InsertEntryString("8",0x0028,0x0101,"US"); // Bits Stored
   file->InsertEntryString("7",0x0028,0x0102,"US"); // High Bit

   // Set the pixel representation
   file->InsertEntryString("0",0x0028,0x0103,"US"); // Pixel Representation

   // Set the samples per pixel
   file->InsertEntryString("1",0x0028,0x0002,"US"); // Samples per Pixel

   // The so called 'prepared image', built ex nihilo just before,
   // has NO Pixel Element yet.
   // therefore, it's NEVER 'file readable' ...
    
   //if( !file->IsReadable() )
   // {
   //   std::cout << "Failed\n"
   //             << "        Prepared image isn't readable\n";
   //  return NULL;
   //}

   size_t size = 256 * 256 * 1;
   unsigned char *imageData = new unsigned char[size];
   memset(imageData,0,size);

// Write the image
   GDCM_NAME_SPACE::FileHelper *hlp = GDCM_NAME_SPACE::FileHelper::New(file);
   hlp->SetImageData(imageData,size);
   hlp->SetWriteTypeToDcmExplVR();
   if( !hlp->Write(fileName) )
   {
      std::cout << "Failed\n"
                << "        File in unwrittable\n";

      hlp->Delete();
      delete[] imageData;
      return NULL;
   }
   delete[] imageData;
   hlp->Delete();

// Read the written image
   GDCM_NAME_SPACE::File *reread = GDCM_NAME_SPACE::File::New(  );
   reread->SetFileName( fileName );
   reread->Load();
   if( !reread->IsReadable() )
   {
     std::cerr << "Failed" << std::endl
               << "        Could not reread written image :" << fileName << std::endl;
     reread->Delete();
     return NULL;
   }

   return reread;
}

int TestImageSet(int argc, char *argv[])
{
   if (argc < 1) 
   {
      std::cerr << "usage: \n" 
                << argv[0] << " (without parameters) " << std::endl 
                << std::endl;
      return 1;
   }

   std::cout << "   Description (Test::TestSequence): " << std::endl;
   std::cout << "   Tests the creation of a 4 images Set" << std::endl;
   std::cout << "   with the following steps : "<< std::endl;
   std::cout << "   step 1: create images belonging" << std::endl
             << "           to different Study and Terie" << std::endl;
   std::cout << "   step 2: create images belonging" << std::endl
             << "           to the same Serie (therefore to the same Study)" << std::endl;
   std::cout << "   step 3: create images belonging" << std::endl
             << "           to different Series within the same Study" << std::endl;
   std::cout << std::endl << std::endl;

   GDCM_NAME_SPACE::File *file;
   GDCM_NAME_SPACE::File *newFile;
   FileList fileList;
   int i;

   std::cout<<"     step...";
   std::string studyUID;
   std::string serieUID;

   // Step 1 : All files have different UID 
   fileList.clear();
   for(i = 0;i < 4;i++)
   {
      std::ostringstream fileName;
      fileName << "FileSeq" << i << ".dcm";
      file = GDCM_NAME_SPACE::File::New();
      // It's up to the user to initialize Serie UID and Study UID
      // Study Instance UID
      studyUID = GDCM_NAME_SPACE::Util::CreateUniqueUID();
      file->InsertEntryString(studyUID, 0x0020, 0x000d, "UI");
      // Series Instance UID
      serieUID = GDCM_NAME_SPACE::Util::CreateUniqueUID();
      file->InsertEntryString(serieUID, 0x0020, 0x000e, "UI");

      newFile = WriteImage(file, fileName.str());
      if( !newFile )
      {
         file->Delete();
         return 1;
      }
      else
         fileList.push_back(newFile);

      file->Delete();
   }

   if( CompareImages(fileList, false, false) )
   {
      ClearList(fileList);
      return 1;
   }
   ClearList(fileList);

   std::cout<<"1...";

   // Step 2 : Same Serie & Study
   fileList.clear();
   studyUID = GDCM_NAME_SPACE::Util::CreateUniqueUID();
   serieUID = GDCM_NAME_SPACE::Util::CreateUniqueUID();
   for(i = 0;i < 4;i++)
   {
      std::ostringstream fileName;
      fileName << "FileSeq" << i << ".dcm";
      file = GDCM_NAME_SPACE::File::New();
      file->InsertEntryString(studyUID, 0x0020, 0x000d, "UI");
      file->InsertEntryString(serieUID, 0x0020, 0x000e, "UI");

      newFile = WriteImage(file, fileName.str());
      if( !newFile )
      {
         file->Delete();
         return(1);
      }
      else
         fileList.push_back(newFile);

      file->Delete();
   }

   if( CompareImages(fileList, true, true) )
   {
      ClearList(fileList);
      return 1;
   }
   ClearList(fileList);

   std::cout<<"2...";

   // Step 3 : Same Study
   fileList.clear();
   serieUID = GDCM_NAME_SPACE::Util::CreateUniqueUID();
   for(i = 0;i < 4;i++)
   {
      std::ostringstream fileName;
      fileName << "FileSeq" << i << ".dcm";
      file = GDCM_NAME_SPACE::File::New();
      file->InsertEntryString(studyUID, 0x0020, 0x000d, "UI");
      serieUID = GDCM_NAME_SPACE::Util::CreateUniqueUID();
      file->InsertEntryString(serieUID, 0x0020, 0x000e, "UI");
      newFile = WriteImage(file, fileName.str());
      if( !newFile )
      {
         file->Delete();
         return(1);
      }
      else
         fileList.push_back(newFile);

      file->Delete();
   }

   if( CompareImages(fileList, false, true) )
   {
      ClearList(fileList);
      return 1;
   }
   ClearList(fileList);

   std::cout<<"3...OK\n";

   return 0;
}
