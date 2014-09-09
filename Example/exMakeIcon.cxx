/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: exMakeIcon.cxx,v $
  Language:  C++
  Date:      $Date: 2007/08/29 16:26:05 $
  Version:   $Revision: 1.1 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmDebug.h"
#include "gdcmGlobal.h"
#include "gdcmCommon.h"
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmDataEntry.h"
#include "gdcmSeqEntry.h"
#include "gdcmSQItem.h"

// 0088 0200 SQ 1 Icon Image Sequence 

int main (int argc, char *argv[])
{
   // hard coded small image name
   std::string input = GDCM_DATA_ROOT;
   input += "/"; 
   input += "LIBIDO-8-ACR_NEMA-Lena_128_128.acr";

   std::string output = "/tmp/testIcon.dcm";

   GDCM_NAME_SPACE::Debug::DebugOn();

   if ( argc == 3 )
   {
      input  = argv[1];
      output = argv[2];
   }
   else if ( argc < 3  )
   {
      std::cout << "   Usage: " << argv[0]
                << " input filename.dcm output Filename.dcm" << std::endl;
   }

   GDCM_NAME_SPACE::File *f = GDCM_NAME_SPACE::File::New( );
   f->SetFileName( input );
   f->Load( );

   if ( ! f->IsReadable() )
   {
      std::cout << " Failed to Open/Parse file" << input << std::endl;
      f->Delete();
      return 1;
   }  
   GDCM_NAME_SPACE::FileHelper *fh = GDCM_NAME_SPACE::FileHelper::New(f); 
   uint8_t *pixels = fh->GetImageData();
   uint32_t lgth   = fh->GetImageDataSize();

   GDCM_NAME_SPACE::SeqEntry *icon = f->InsertSeqEntry(0x0088, 0x0200);
   GDCM_NAME_SPACE::SQItem *sqi = GDCM_NAME_SPACE::SQItem::New(1);
   icon->AddSQItem(sqi, 1);
   sqi->Delete();

   // icone is just defined like the image
   // The purpose is NOT to imagine an icon, 
   // just check the stuff works
 
   sqi->InsertEntryString( "MONOCHROME2", 0x0028,0x0004, "CS");
   sqi->InsertEntryString( "128", 0x0028,0x0010, "US");
   sqi->InsertEntryString( "8",   0x0028,0x0100, "US");
   sqi->InsertEntryString( "8",   0x0028,0x0101, "US");
   sqi->InsertEntryString( "7",   0x0028,0x0102, "US");
   sqi->InsertEntryString( "0",   0x0028,0x0103, "US");
   sqi->InsertEntryBinArea(  pixels, lgth, 0x7fe0,0x0010,"OB");
   // just to see if it's stored at the right place
   sqi->InsertEntryString( "128", 0x0028,0x0011, "US");
    
   fh->WriteDcmExplVR(output);

   f->Delete();
   fh->Delete();

   f = GDCM_NAME_SPACE::File::New();
   f->SetFileName(output);
   f->Load();
   f->Print();
   std::cout << "End of Print" << std::endl;


// ---------- Check everything is OK in the written image:

   icon = f->GetSeqEntry(0x0088, 0x0200);
   if (!icon)
   {
      std::cout << "Sequence 0088|0200 not found" << std::endl
                << "   ... Failed" << std::endl;
      f->Delete();
      return 1;
   }
   std::cout << "Sequence 0088|0200 found" << std::endl;
   
   sqi = icon->GetFirstSQItem();

   if ( !sqi )
   {
      std::cout << "Sequence 0088|0200 has no SQItem" << std::endl
                << "   ... Failed" << std::endl;
      f->Delete();
      return 1;
   }

   std::cout << "First Item found" << std::endl;

   // Test for entry 0028|0010
   if ( !sqi->GetDataEntry(0x0028,0x0010) )
   {
      std::cout << "GetDataEntry 0028|0010 not found" << std::endl
                << "   ... Failed" << std::endl;
      f->Delete();
      return 1;
   }
   std::cout << "First Item ->DataEntry 0028|0010 found" << std::endl;
   if ( sqi->GetDataEntry(0x0028,0x0010)->GetString() != "128" )
   {
      std::cout << "Value 0028|0010 don't match" << std::endl
                << "Read : " << sqi->GetDataEntry(0x0028,0x0010)->GetString()
                << " - Expected : 128" << std::endl
                << "   ... Failed" << std::endl;
      f->Delete();
      return 1;
   }

   // Test for entry 0028|0011
   if ( !sqi->GetDataEntry(0x0028,0x0011) )
   {
      std::cout << "GetDataEntry 0028|0011 not found" << std::endl
                << "   ... Failed" << std::endl;
      f->Delete();
      return 1;
   }
   std::cout << "First Item ->DataEntry 0028|0011 found" << std::endl;
   if ( sqi->GetDataEntry(0x0028,0x0011)->GetString() != "128" )
   {
      std::cout << "Value 0028|0011 don't match" << std::endl
                << "Read : " << sqi->GetDataEntry(0x0028,0x0011)->GetString()
                << " - Expected : 128" << std::endl
                << "   ... Failed" << std::endl;
      f->Delete();
      return 1;
   }

   // Test for entry 0028|0100
   if ( !sqi->GetDataEntry(0x0028,0x0100) )
   {
      std::cout << "GetDataEntry 0028|0100 not found" << std::endl
                << "   ... Failed" << std::endl;
      f->Delete();
      return 1;
   }
   std::cout << "First Item ->DataEntry 0028|0100 found" << std::endl;
   if ( sqi->GetDataEntry(0x0028,0x0100)->GetString() != "8" )
   {
      std::cout << "Value 0028|0100 don't match" << std::endl
                << "Read : " << sqi->GetDataEntry(0x0028,0x0100)->GetString()
                << " - Expected : 8" << std::endl
                << "   ... Failed" << std::endl;
      f->Delete();
      return 1;
   }

   // Test for entry 0028|0101
   if ( !sqi->GetDataEntry(0x0028,0x0101) )
   {
      std::cout << "GetDataEntry 0028|0101 not found" << std::endl
                << "   ... Failed" << std::endl;
      f->Delete();
      return 1;
   }
   std::cout << "First Item ->DataEntry 0028|0101 found" << std::endl;
   if ( sqi->GetDataEntry(0x0028,0x0101)->GetString() != "8" )
   {
      std::cout << "Value 0028|0101 don't match" << std::endl
                << "Read : " << sqi->GetDataEntry(0x0028,0x0101)->GetString()
                << " - Expected : 8" << std::endl
                << "   ... Failed" << std::endl;
      f->Delete();
      return 1;
   }

   // Test for entry 0028|0102
   if ( !sqi->GetDataEntry(0x0028,0x0102) )
   {
      std::cout << "DataEntry 0028|0102 not found" << std::endl
                << "   ... Failed" << std::endl;
      f->Delete();
      return 1;
   }
   std::cout << "First Item ->DataEntry 0028|0102 found" << std::endl;
   if ( sqi->GetDataEntry(0x0028,0x0102)->GetString() != "7" )
   {
      std::cout << "Value 0028|0102 don't match" << std::endl
                << "Read : " << sqi->GetDataEntry(0x0028,0x0102)->GetString()
                << " - Expected : 7" << std::endl
                << "   ... Failed" << std::endl;
      f->Delete();
      return 1;
   }

   // Test for entry 0028|0103
   if ( !sqi->GetDataEntry(0x0028,0x0103) )
   {
      std::cout << "GetDataEntry 0028|0010 not found" << std::endl
                << "   ... Failed" << std::endl;
      f->Delete();
      return 1;
   }
   std::cout << "First Item ->DataEntry 0028|0103 found" << std::endl;
   if ( sqi->GetDataEntry(0x0028,0x0103)->GetString() != "0" )
   {
      std::cout << "Value 0028|0103 don't match" << std::endl
                << "Read : " << sqi->GetDataEntry(0x0028,0x0103)->GetString()
                << " - Expected : 0" << std::endl
                << "   ... Failed" << std::endl;
      f->Delete();
      return 1;
   }

// ---------- End of checking

   f->Delete();

   return 0;
}
