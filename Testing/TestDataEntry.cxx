/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestDataEntry.cxx,v $
  Language:  C++
  Date:      $Date: 2008/09/15 15:49:21 $
  Version:   $Revision: 1.13 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmDictEntry.h"
#include "gdcmDataEntry.h"
#include <math.h>

// ===============================================================

const char data[] = "1\\2\\3\\4\\5";
const char fdata[] = "1.1\\2.2\\3.3\\4.4\\5.5";

const int16_t svalue[]={1,2,3,4,5};
const int32_t lvalue[]={1,2,3,4,5};
const float fvalue[]={1.1f,2.2f,3.3f,4.4f,5.5f};
const double dvalue[]={1.1,2.2,3.3,4.4,5.5};

const unsigned long nbvalue = 5;
const double GDCM_EPS = 1e-6;

/**
  * \brief Test the DataEntry object
  */  
int TestDataEntry(int , char *[])
{
   /* Most of the tests are out of date! 
   (we don't use any longer DictEntry to build a DocEntry!
   
   unsigned int i;
   GDCM_NAME_SPACE::DictEntry *dict;
   GDCM_NAME_SPACE::DataEntry *entry;
      
   dict = GDCM_NAME_SPACE::DictEntry::New(0x0003,0x0004);
   // SetVR *before* making the DataEntry!
   dict->SetVR("US");   
   entry = GDCM_NAME_SPACE::DataEntry::New(dict);

   std::cout << "Test for VR = " << dict->GetVR() << "..." << std::endl;

   std::cout << "TagKey : [" << entry->GetKey() << "]" << std::endl;
   std::cout << "Group : [" << entry->GetGroup() << "]" << std::endl; 
   std::cout << "Element : [" << entry->GetElement() << "]" << std::endl; 
      
   entry->SetString("1");
   std::cout << "1: ";
   entry->Print(std::cout);
   std::cout << std::endl;
   if( entry->GetValueCount() != 1 )
   {
      std::cout << "   Failed" << std::endl
                << "   Number of content values is incorrect" << std::endl
                << "   Found: " << entry->GetValueCount() 
                << " - Must be: 1" << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }

   entry->SetString("1\\2");
   std::cout << "2: ";
   entry->Print(std::cout);
   std::cout << std::endl;
   if( entry->GetValueCount() != 2 )
   {
      std::cout << "   Failed" << std::endl
                << "   Number of content values is incorrect" << std::endl
                << "   Found: " << entry->GetValueCount() 
                << " - Must be: 2" << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }

   entry->SetString("");
   std::cout << "3: ";
   entry->Print(std::cout);
   std::cout << std::endl;
   if( entry->GetValueCount() != 0 )
   {
      std::cout << "   Failed" << std::endl
                << "   Number of content values is incorrect" << std::endl
                << "   Found: " << entry->GetValueCount() 
                << " - Must be: 0" << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }

   std::cout << std::endl;
   dict->Delete();
   entry->Delete();

   //------------------------------------------------------------------
   dict = GDCM_NAME_SPACE::DictEntry::New(0x0000,0x0000);
   // SetVR *before* making the DataEntry!   
   dict->SetVR("LT");
   entry = GDCM_NAME_SPACE::DataEntry::New(dict);

   std::cout << "Test for VR = " << dict->GetVR() << "..." << std::endl;
   entry->SetString(data);
   entry->Print(std::cout);
   std::cout << std::endl;

   if( entry->GetLength() != strlen(data) + strlen(data)%2 )
   {
      std::cout << "   Failed" << std::endl
                << "   Size of string is incorrect" << std::endl
                << "   Found: " << entry->GetLength() 
                << " - Must be: " << strlen(data) + strlen(data)%2 << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   if( entry->GetValueCount() != nbvalue )
   {
      std::cout << "   Failed" << std::endl
                << "   Number of content values is incorrect" << std::endl
                << "   Found: " << entry->GetValueCount() 
                << " - Must be: " << nbvalue << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   if( memcmp(entry->GetBinArea(),data,entry->GetLength()) != 0 )
   {
      std::cout << "   Failed" << std::endl
                << "   Content of bin area is incorrect" << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   if( memcmp(entry->GetString().c_str(),data,entry->GetLength()) != 0 )
   {
      std::cout << "   Failed" << std::endl
                << "   Content of string is incorrect" << std::endl
                << "   Found: " << entry->GetString().c_str()
                << " - Must be: " << data << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   for(i=0;i<entry->GetValueCount();i++)
   {
      if( entry->GetValue(i) != svalue[i] )
      {
         std::cout << "   Failed" << std::endl
                   << "   Content of entry's values is incorrect : id " << i << std::endl
                   << "   Found " << entry->GetValue(i)
                   << " - Must be " << svalue[i] << std::endl;
         dict->Delete();
         entry->Delete();
         return 1;
      }
   }

   std::cout << std::endl;
   dict->Delete();
   entry->Delete();

   //------------------------------------------------------------------
   dict = GDCM_NAME_SPACE::DictEntry::New(0x0000,0x0000);
   // SetVR *before* making the DataEntry! 
   dict->SetVR("US");
   entry = GDCM_NAME_SPACE::DataEntry::New(dict);


   std::cout << "Test for VR = " << dict->GetVR() << "..." << std::endl;
   entry->SetString(data);
   std::cout << "1: ";
   entry->Print(std::cout);
   std::cout << std::endl;

   if( entry->GetLength() != nbvalue*sizeof(uint16_t) )
   {
      std::cout << "   Failed" << std::endl
                << "   BinArea length is incorrect" << std::endl
                << "   Found: " << entry->GetLength()
                << " - Must be: " << nbvalue*sizeof(uint16_t) << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   if( memcmp(entry->GetString().c_str(),data,strlen(data)) != 0 )
   {
      std::cout << "   Failed" << std::endl
                << "   Content of string is incorrect" << std::endl
                << "   Found: " << entry->GetString().c_str()
                << " - Must be: " << data << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   if( entry->GetValueCount() != nbvalue )
   {
      std::cout << "   Failed" << std::endl
                << "   Number of content values is incorrect" << std::endl
                << "   Found: " << entry->GetValueCount()
                << " - Must be: " << nbvalue << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   for(i=0;i<entry->GetValueCount();i++)
   {
      if( entry->GetValue(i) != svalue[i] )
      {
         std::cout << "   Failed" << std::endl
                   << "   Content of entry's values is incorrect : id " << i << std::endl
                   << "   Found: " << entry->GetValue(i)
                   << " - Must be: " << svalue[i] << std::endl;
         dict->Delete();
         entry->Delete();
         return 1;
      }
   }

   entry->SetLength(nbvalue*sizeof(uint16_t));
   entry->SetBinArea((uint8_t *)svalue,false);
   std::cout << "2: ";
   entry->Print(std::cout);
   std::cout << std::endl;

   if( memcmp(entry->GetString().c_str(),data,strlen(data)) != 0 )
   {
      std::cout << "   Failed" << std::endl
                << "   Content of string is incorrect" << std::endl
                << "   Found: " << entry->GetString().c_str()
                << " - Must be: " << data << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   if( entry->GetValueCount() != nbvalue )
   {
      std::cout << "   Failed" << std::endl
                << "   Number of content values is incorrect" << std::endl
                << "   Found: " << entry->GetValueCount() 
                << " - Must be: " << nbvalue << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   for(i=0;i<entry->GetValueCount();i++)
   {
      if( entry->GetValue(i) != svalue[i] )
      {
         std::cout << "   Failed" << std::endl
                   << "   Content of entry's values is incorrect : id " << i << std::endl
                   << "   Found: " << entry->GetValue(i)
                   << " - Must be: " << svalue[i] << std::endl;
         dict->Delete();
         entry->Delete();
         return 1;
      }
   }

   std::cout << std::endl;
   dict->Delete();
   entry->Delete();

   //------------------------------------------------------------------
   dict = GDCM_NAME_SPACE::DictEntry::New(0x0000,0x0000);
   dict->SetVR("UL");
   entry = GDCM_NAME_SPACE::DataEntry::New(dict);

   std::cout << "Test for VR = " << dict->GetVR() << "..." << std::endl;
   entry->SetString(data);
   std::cout << "1: ";
   entry->Print(std::cout);
   std::cout << std::endl;

   if( entry->GetLength() != nbvalue*sizeof(uint32_t) )
   {
      std::cout << "   Failed" << std::endl
                << "   BinArea length is incorrect" << std::endl
                << "   Found: " << entry->GetLength()
                << " - Must be: " << nbvalue*sizeof(uint32_t) << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   if( memcmp(entry->GetString().c_str(),data,strlen(data)) != 0 )
   {
      std::cout << "   Failed" << std::endl
                << "   Content of string is incorrect" << std::endl
                << "   Found: " << entry->GetString().c_str()
                << " - Must be: " << data << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   if( entry->GetValueCount() != nbvalue )
   {
      std::cout << "   Failed" << std::endl
                << "   Number of content values is incorrect" << std::endl
                << "   Found: " << entry->GetValueCount() 
                << " - Must be: " << nbvalue << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   for(i=0;i<entry->GetValueCount();i++)
   {
      if( entry->GetValue(i) != lvalue[i] )
      {
         std::cout << "   Failed" << std::endl
                   << "   Content of entry's values is incorrect : id " << i << std::endl
                   << "   Found: " << entry->GetValue(i)
                   << " - Must be: " << lvalue[i] << std::endl;
         dict->Delete();
         entry->Delete();
         return 1;
      }
   }

   entry->SetLength(nbvalue*sizeof(uint32_t));
   entry->SetBinArea((uint8_t *)lvalue,false);
   std::cout << "2: ";
   entry->Print(std::cout);
   std::cout << std::endl;

   if( memcmp(entry->GetString().c_str(),data,strlen(data)) != 0 )
   {
      std::cout << "   Failed" << std::endl
                << "   Content of string is incorrect" << std::endl
                << "   Found: " << entry->GetString().c_str() 
                << " - Must be: " << data << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   if( entry->GetValueCount() != nbvalue )
   {
      std::cout << "   Failed" << std::endl
                << "   Number of content values is incorrect" << std::endl
                << "   Found: " << entry->GetValueCount() 
                << " - Must be: " << nbvalue << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   for(i=0;i<entry->GetValueCount();i++)
   {
      if( entry->GetValue(i) != lvalue[i] )
      {
         std::cout << "   Failed" << std::endl
                   << "   Content of entry's values is incorrect : id " << i << std::endl
                   << "   Found: " << entry->GetValue(i)
                   << " - Must be: " << lvalue[i] << std::endl;
         dict->Delete();
         entry->Delete();
         return 1;
      }
   }

   std::cout << std::endl;
   dict->Delete();
   entry->Delete();

   //------------------------------------------------------------------
   dict = GDCM_NAME_SPACE::DictEntry::New(0x0000,0x0000);
   dict->SetVR("FL");
   entry = GDCM_NAME_SPACE::DataEntry::New(dict);

   std::cout << "Test for VR = " << dict->GetVR() << "..." << std::endl;
   entry->SetString(fdata);
   std::cout << "1: ";
   entry->Print(std::cout);
   std::cout << std::endl;

   if( entry->GetLength() != nbvalue*sizeof(float) )
   {
      std::cout << "   Failed" << std::endl
                << "   BinArea length is incorrect" << std::endl
                << "   Found: " << entry->GetLength() 
                << " - Must be: " << nbvalue*sizeof(float) << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   if( memcmp(entry->GetString().c_str(),fdata,strlen(fdata)) != 0 )
   {
      std::cout << "   Failed" << std::endl
                << "   Content of string is incorrect" << std::endl
                << "   Found: " << entry->GetString().c_str()
                << " - Must be: " << fdata << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   if( entry->GetValueCount() != nbvalue )
   {
      std::cout << "   Failed" << std::endl
                << "   Number of content values is incorrect" << std::endl
                << "   Found: " << entry->GetValueCount() 
                << " - Must be: " << nbvalue << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   for(i=0;i<entry->GetValueCount();i++)
   {
      if( entry->GetValue(i) != fvalue[i] )
      {
         std::cout << "   Failed" << std::endl
                   << "   Content of entry's values is incorrect : id " << i << std::endl
                   << "   Found: " << entry->GetValue(i)
                   << " - Must be: " << fvalue[i] << std::endl;
         dict->Delete();
         entry->Delete();
         return 1;
      }
   }

   entry->SetLength(nbvalue*sizeof(float));
   entry->SetBinArea((uint8_t *)fvalue,false);
   std::cout << "2: ";
   entry->Print(std::cout);
   std::cout << std::endl;

   if( memcmp(entry->GetString().c_str(),fdata,strlen(fdata)) != 0 )
   {
      std::cout << "   Failed" << std::endl
                << "   Content of string is incorrect" << std::endl
                << "   Found: " << entry->GetString().c_str()
                << " - Must be: " << fdata << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   if( entry->GetValueCount() != nbvalue )
   {
      std::cout << "   Failed" << std::endl
                << "   Number of content values is incorrect" << std::endl
                << "   Found: " << entry->GetValueCount() 
                << " - Must be: " << nbvalue << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   for(i=0;i<entry->GetValueCount();i++)
   {
      if( entry->GetValue(i) != fvalue[i] )
      {
         std::cout << "   Failed" << std::endl
                   << "   Content of entry's values is incorrect : id " << i << std::endl
                   << "   Found: " << entry->GetValue(i)
                   << " - Must be: " << fvalue[i] << std::endl;
         dict->Delete();
         entry->Delete();
         return 1;
      }
   }

   std::cout << std::endl;
   dict->Delete();
   entry->Delete();

   //------------------------------------------------------------------
   dict = GDCM_NAME_SPACE::DictEntry::New(0x0000,0x0000);
   dict->SetVR("FD");
   entry = GDCM_NAME_SPACE::DataEntry::New(dict);

   std::cout << "Test for VR = " << dict->GetVR() << "..." << std::endl;
   entry->SetString(fdata);
   std::cout << "1: ";
   entry->Print(std::cout);
   std::cout << std::endl;

   if( entry->GetLength() != nbvalue*sizeof(double) )
   {
      std::cout << "   Failed" << std::endl
                << "   BinArea length is incorrect" << std::endl
                << "   Found: " << entry->GetLength()
                << " - Must be: " << nbvalue*sizeof(double) << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   if( memcmp(entry->GetString().c_str(),fdata,strlen(fdata)) != 0 )
   {
      std::cout << "   Failed" << std::endl
                << "   Content of string is incorrect" << std::endl
                << "   Found: " << entry->GetString().c_str()
                << " - Must be: " << fdata << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   if( entry->GetValueCount() != nbvalue )
   {
      std::cout << "   Failed" << std::endl
                << "   Number of content values is incorrect" << std::endl
                << "   Found: " << entry->GetValueCount() 
                << " - Must be: " << nbvalue << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   for(i=0;i<entry->GetValueCount();i++)
   {
      // Never compare floating point value...
      double dif = fabs(entry->GetValue(i) - dvalue[i]);
      if( dif > GDCM_EPS)
      {
         std::cout << "   Failed" << std::endl
                   << "   Content of entry's values is incorrect : id " << i << std::endl
                   << "   Found: " << entry->GetValue(i)
                   << " - Must be: " << dvalue[i] << std::endl;
         dict->Delete();
         entry->Delete();
         return 1;
      }
   }

   entry->SetLength(nbvalue*sizeof(double));
   entry->SetBinArea((uint8_t *)dvalue,false);
   std::cout << "2: ";
   entry->Print(std::cout);
   std::cout << std::endl;

   if( memcmp(entry->GetString().c_str(),fdata,strlen(fdata)) != 0 )
   {
      std::cout << "   Failed" << std::endl
                << "   Content of string is incorrect" << std::endl
                << "   Found: " << entry->GetString().c_str()
                << " - Must be: " << fdata << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   if( entry->GetValueCount() != nbvalue )
   {
      std::cout << "   Failed" << std::endl
                << "   Number of content values is incorrect" << std::endl
                << "   Found: " << entry->GetValueCount() 
                << " - Must be: " << nbvalue << std::endl;
      dict->Delete();
      entry->Delete();
      return 1;
   }
   for(i=0;i<entry->GetValueCount();i++)
   {
      if( entry->GetValue(i) != dvalue[i] )
      {
         std::cout << "   Failed" << std::endl
                   << "   Content of entry's values is incorrect : id " << i << std::endl
                   << "   Found: " << entry->GetValue(i)
                   << " - Must be: " << dvalue[i] << std::endl;
         dict->Delete();
         entry->Delete();
         return 1;
      }
   }

   std::cout << std::endl;
   dict->Delete();
   entry->Delete();

   //------------------------------------------------------------------
   std::cout<<std::flush;
   */
   return 0;
}
