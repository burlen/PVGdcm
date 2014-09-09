/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestAllReadCompareDicom.cxx,v $
  Language:  C++
  Date:      $Date: 2008/09/15 15:49:21 $
  Version:   $Revision: 1.62 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmDirList.h"
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmGlobal.h"
#include "gdcmTS.h"
#include "gdcmDebug.h"
#include "gdcmUtil.h"

#include <iostream>

//Generated file:
#include "gdcmDataImages.h"

//-->
//--> WARNING :
//-->          The .tst files *must* be generated on a Little Endian based computer.
//-->
/**
 * /brief   File Read/Writer specific for the TestAllReadCompareDicom test
 * /remarks The Test file format is (only in little endian) :
 *  - 4 bytes : 'gdcm'
 *  - 4 bytes : size X
 *  - 4 bytes : size Y
 *  - 4 bytes : size Z
 *  - 2 bytes : scalar size (8,16,32) --> ?!? 1 or 2 only in DICOM V3 !
 *  - 2 bytes : number of components per pixel (1,2,3) ---> 1 or 3 only in DICOMV3 !
 *  - n bytes : data
 */
class TestFile
{
public:
   TestFile();
   ~TestFile();

   bool IsReadable() {return readable;}
   
   int GetXSize() {return SizeX;}
   int GetYSize() {return SizeY;}
   int GetZSize() {return SizeZ;}
   
   void SetXSize(int size) {SizeX = size;}   
   void SetYSize(int size) {SizeY = size;}
   void SetZSize(int size) {SizeZ = size;}
   
   int GetScalarSize()                  {return ScalarSize;}
   void SetScalarSize(int size)         {ScalarSize = size;}
   
   int GetNumberOfComponents()          {return Components;}
   void SetNumberOfComponents(int size) {Components = size;}
   int GetSwapCode() {return SwapCode;}

   unsigned long GetDataSize() {return GetLineSize()*SizeY*SizeZ;}
   uint8_t *GetData() {return Data;}
   void SetData(const uint8_t *newData);

   void Load(const std::string &filename);
   void Write(const std::string &filename);

private:
   unsigned long GetLineSize() {return SizeX*ScalarSize*Components;}
   int ComputeSwapCode(uint32_t tag);

   void NewData();
   void DeleteData();

   void ReadFile();
   bool ReadFileHeader(std::ifstream *fp);
   bool ReadFileData(std::ifstream *fp);
   void WriteFile();
   bool WriteFileHeader(std::ofstream *fp);
   bool WriteFileData(std::ofstream *fp);

   uint8_t  ReadInt8 (std::ifstream *fp)
#if !(__GNUC__==2  && __GNUC_MINOR__<=96)
 throw( std::ios::failure );
#else
 ;
#endif
   uint16_t ReadInt16(std::ifstream *fp)
#if !(__GNUC__==2  && __GNUC_MINOR__<=96)
  throw( std::ios::failure );
#else
 ;
#endif
   uint32_t ReadInt32(std::ifstream *fp)
#if !(__GNUC__==2  && __GNUC_MINOR__<=96)
  throw( std::ios::failure );
#else
 ;
#endif
   void WriteInt8 (std::ofstream *fp,uint8_t  value);
   void WriteInt16(std::ofstream *fp,uint16_t value);
   void WriteInt32(std::ofstream *fp,uint32_t value);

   std::string fileName;
   bool readable;

   int SizeX;
   int SizeY;
   int SizeZ;
   uint16_t ScalarSize;
   uint16_t Components;
   uint8_t *Data;
   int SwapCode;

   static const unsigned int HEADER_SIZE;
};

const unsigned int MAX_NUMBER_OF_DIFFERENCE = 10;
const unsigned int TestFile::HEADER_SIZE = 20;

TestFile::TestFile()
{
   fileName = "";
   readable=false;

   SizeX = 0;
   SizeY = 0;
   SizeZ = 0;
   ScalarSize = 0;
   Components = 0;
   Data = NULL;

   //SwapCode = 1234;
}

TestFile::~TestFile()
{
   DeleteData();
}

void TestFile::SetData(const uint8_t *newData)
{
   DeleteData();
   NewData();
   if( Data )
      memcpy(Data,newData,GetDataSize());
}

void TestFile::Load(const std::string &filename)
{
   fileName = filename;
   ReadFile();
}

void TestFile::Write(const std::string &filename)
{
   fileName = filename;
   WriteFile();
}

int TestFile::ComputeSwapCode(uint32_t tag)
{
// FIXME : 100 % useless method !
// "gdcm" was written on disc byte per byte.
// when you fread it, you'll get *always* "gdcm"
// whatever the processor indianess is !

   int swap = 0;
   //std::cout << std::hex << "0x(" << tag << ")" << std::dec << std::endl;
   
   for(int i=0;i<4;i++)
   {
      switch(tag&0x000000FF)
      {
         case 'g':
            swap += (i+1)*1000;
            break;
         case 'd':
            swap += (i+1)*100;
            break;
         case 'c':
            swap += (i+1)*10;
            break;
         case 'm':
            swap += (i+1);
            break;
         default:
            return 0;
      }
      tag >>= 8;
   }
   //std::cout << std::hex << "0x(" << tag << ")" << std::dec << tag << std::endl;
   return swap;
}

void TestFile::NewData()
{
   DeleteData();
   if( GetDataSize() == 0 )
      return;
   Data = new uint8_t[GetDataSize()];
}

void TestFile::DeleteData()
{
   if( Data )
      delete[] Data;
   Data = NULL;
}

void TestFile::ReadFile()
{
   readable=true;
   std::ifstream fp(fileName.c_str(),std::ios::in | std::ios::binary);

   if(!fp)
   {
      readable=false;
      return;
   }

   try
   {
      readable=ReadFileHeader(&fp);
      if(!readable)
      {
         std::cout << "Problems when reading Header part" << std::endl;
         fp.close();
         return;
      }

      readable=ReadFileData(&fp);
      if(!readable)
      {
         std::cout << "Problems when reading data" << std::endl;
         fp.close();
         return;
      }
   }
   catch(...)
   {
      readable=false;
      fp.close();
      return;
   }

   fp.close();
}

bool TestFile::ReadFileHeader(std::ifstream *fp)
{
   uint32_t tag = ReadInt32(fp);
   SwapCode = ComputeSwapCode(tag);
   if( SwapCode == 0 )
   {
      // We shall *never* come here!
      std::cout << "TestFile: Bad tag - Must be 'gdcm'" << std::endl;
      return(false);
   }

   SizeX = ReadInt32(fp); // Size X
   SizeY = ReadInt32(fp); // Size Y
   SizeZ = ReadInt32(fp); // Size Z
   ScalarSize = ReadInt16(fp)/8; // bytes per scalar
   Components = ReadInt16(fp);   // Number of components

   return(true);
}

bool TestFile::ReadFileData(std::ifstream *fp)
{
   DeleteData();

   // Allocate data
   NewData();
   if( !Data )
      return(false);

   // Read data  Note : .tst images are *always* created 
   //           on little endian processor !
   fp->read((char *)Data,GetDataSize());

   // Track BigEndian troubles
   std::cout << " ScalarSize : " << GetScalarSize() 
          << " IsCurrentProcessorBigEndian:" 
          << GDCM_NAME_SPACE::Util::IsCurrentProcessorBigEndian()
          << std::endl;
        
   //if (GetScalarSize() == 1 || GetSwapCode() == 1234)  
   if (GetScalarSize() == 1 || !GDCM_NAME_SPACE::Util::IsCurrentProcessorBigEndian() )    
   {
      return true;
   }
   // We *know* the .tst files are written in 'Little Endian' format.
   // We *know* DataSize may be 1 or 2 !  
   uint16_t g;
   
   std::cout << " Let's swap Pixels" <<std::endl; 
     
   for (unsigned int i=0; i<GetDataSize()/2; i++)
   {
      g = ((uint16_t *)Data)[i];
      g = ( g << 8 |  g >> 8  );
      ((uint16_t *)Data)[i] = g;   
   }
   return(true);
}

void TestFile::WriteFile()
{
   std::ofstream fp(fileName.c_str(),std::ios::out | std::ios::binary);

   if(!fp)
   {
      readable=false;
      return;
   }

   WriteFileHeader(&fp);
   WriteFileData(&fp);

   fp.close();
}

bool TestFile::WriteFileHeader(std::ofstream *fp)
{
   WriteInt8(fp,'g'); // Bitmap tag - must be 'g'
   WriteInt8(fp,'d'); // Bitmap tag - must be 'd'
   WriteInt8(fp,'c'); // Bitmap tag - must be 'c'
   WriteInt8(fp,'m'); // Bitmap tag - must be 'm'
   
   // FIXME : Think of writting an int32, better !
   // (('g' << 8 + 'd') << 8 + 'c') + 'm'
   // if you want to use it to check the endianess.
   // (and upload again *all* the .tst files ...)
   WriteInt32(fp,SizeX); // Size X
   WriteInt32(fp,SizeY); // Size Y
   WriteInt32(fp,SizeZ); // Size Z
   WriteInt16(fp,ScalarSize*8); // bits per scalar
   WriteInt16(fp,Components);   // number of components

   return(true);
}

bool TestFile::WriteFileData(std::ofstream *fp)
{
   fp->write((char *)Data,GetDataSize());

   return(true);
}

uint8_t  TestFile::ReadInt8 (std::ifstream *fp)
#if !(__GNUC__==2  && __GNUC_MINOR__<=96)
   throw( std::ios::failure )
#endif
{
   uint8_t g;
   fp->read ((char*)&g, (size_t)1);
#if !(__GNUC__==2  && __GNUC_MINOR__<=96)
   if ( fp->fail() )
      throw std::ios::failure( "TestFile::ReadInt8() - file error." );
   if( fp->eof() )
      throw std::ios::failure( "TestFile::ReadInt8() - EOF." );
#endif
   return g;
}

uint16_t TestFile::ReadInt16(std::ifstream *fp)
#if !(__GNUC__==2  && __GNUC_MINOR__<=96)
   throw( std::ios::failure )
#endif
{
   uint16_t g;
   fp->read ((char*)&g, (size_t)2);
#if !(__GNUC__==2  && __GNUC_MINOR__<=96)
   if ( fp->fail() )
      throw std::ios::failure( "TestFile::ReadInt16() - file error." );
   if( fp->eof() )
      throw std::ios::failure( "TestFile::ReadInt16() - EOF." );
#endif

#if defined(GDCM_WORDS_BIGENDIAN)
   g = ( g << 8 |  g >> 8  );
#endif
   return g;
}

uint32_t TestFile::ReadInt32(std::ifstream *fp)
#if !(__GNUC__==2  && __GNUC_MINOR__<=96)
   throw( std::ios::failure )
#endif
{
   uint32_t g;
   fp->read ((char*)&g, (size_t)4);
#if !(__GNUC__==2  && __GNUC_MINOR__<=96)
   if ( fp->fail() )
      throw std::ios::failure( "TestFile::ReadInt32() - file error." );
   if( fp->eof() )
      throw std::ios::failure( "TestFile::ReadInt32() - EOF." );
#endif

#if defined(GDCM_WORDS_BIGENDIAN)
   g = (  (g<<24)               | ((g<<8)  & 0x00ff0000) | 
       (  (g>>8)  & 0x0000ff00) |  (g>>24)               );
#endif
   return g;
}

void TestFile::WriteInt8 (std::ofstream *fp,uint8_t value)
{
   fp->write((char*)&value, (size_t)1);
}

void TestFile::WriteInt16(std::ofstream *fp,uint16_t value)
{
#if defined(GDCM_WORDS_BIGENDIAN)
   value = ( value << 8 |  value >> 8  );
#endif
   fp->write((char*)&value, (size_t)2);
}

void TestFile::WriteInt32(std::ofstream *fp,uint32_t value)
{
#if defined(GDCM_WORDS_BIGENDIAN)
   value = (  (value<<24)               | ((value<<8)  & 0x00ff0000) | 
           (  (value>>8)  & 0x0000ff00) |  (value>>24)               );
#endif
   fp->write((char*)&value, (size_t)4);
}

int InternalTest(std::string const &filename, 
                 std::string const &referenceFileName )
{
      std::cout << "   Testing: " << filename << std::endl;
      std::cout << "      ";

      ////// Step 1:
      std::cout << "1...";

       // new style 
      GDCM_NAME_SPACE::File *f = GDCM_NAME_SPACE::File::New();
      f->SetLoadMode ( GDCM_NAME_SPACE::LD_ALL ); // Load everything
      f->SetFileName( filename );
      f->Load();
 
      if( !f->IsReadable() )
      {
        std::cout << " Failed" << std::endl
                   << "      Image not gdcm compatible:"
                  << filename << std::endl;
        f->Delete();
        return 1;
      }
      GDCM_NAME_SPACE::FileHelper *tested = GDCM_NAME_SPACE::FileHelper::New( f );
     
      ////// Step 2:
      ////// Check for existence of reference baseline dicom file:
      std::cout << "2...";

      TestFile *reference = new TestFile();
      std::ifstream refFile(referenceFileName.c_str(),
                            std::ios::binary|std::ios::in);
      if(!refFile)
      {
         std::cout << " Failed" << std::endl
                   << "      Image not found:"
                   << referenceFileName << std::endl;
         reference->SetXSize(tested->GetFile()->GetXSize());
         reference->SetYSize(tested->GetFile()->GetYSize());
         reference->SetZSize(tested->GetFile()->GetZSize());
         reference->SetScalarSize(tested->GetFile()->GetPixelSize());
         reference->SetNumberOfComponents(tested->GetFile()->GetNumberOfScalarComponents());
         reference->SetData(tested->GetImageData());
         reference->Write(referenceFileName);
      }
      else
         refFile.close();

      reference->Load(referenceFileName);
      if(!reference->IsReadable())
      {
        std::cout << " Failed" << std::endl
                   << "      Image not Testing compatible:"
                  << filename << std::endl;
         delete reference;
         tested->Delete();
         f->Delete();
         return 1;
      }

      ////// Step 3:
      std::string PixelType = tested->GetFile()->GetPixelType();
      std::cout << "3...";
      int testedDataSize    = tested->GetImageDataSize();
      uint8_t *testedImageData = tested->GetImageData();
    
      int    referenceDataSize = reference->GetDataSize();
      uint8_t *referenceImageData = reference->GetData();

      // Test the image size
      if (tested->GetFile()->GetXSize() != reference->GetXSize() ||
          tested->GetFile()->GetYSize() != reference->GetYSize() ||
          tested->GetFile()->GetZSize() != reference->GetZSize())
      {
         std::cout << "Failed" << std::endl
                   << "        Size differs: "
                   << "X: " << tested->GetFile()->GetXSize() << " # " 
                   << reference->GetXSize() << " | "
                   << "Y: " << tested->GetFile()->GetYSize() << " # " 
                   << reference->GetYSize() << " | "
                   << "Z: " << tested->GetFile()->GetZSize() << " # " 
                   << reference->GetZSize() << std::endl;
         delete reference;
         tested->Delete();
         f->Delete();
         return 1;
      }

      // Test the pixel size
      if (tested->GetFile()->GetPixelSize() != reference->GetScalarSize() ||
          tested->GetFile()->GetNumberOfScalarComponents() != reference->GetNumberOfComponents())
      {
         std::cout << "Failed" << std::endl
                   << "        Pixel size differs: " << std::endl
                   << "        Scalar size: " << tested->GetFile()->GetPixelSize() << " # " 
                   << reference->GetScalarSize() << std::endl
                   << "        Number of scalar: " << tested->GetFile()->GetNumberOfScalarComponents() << " # " 
                   << reference->GetNumberOfComponents() << std::endl
                   << "        Pixel type: " << tested->GetFile()->GetPixelType() << std::endl;
         delete reference;
         tested->Delete();
         f->Delete();
         return 1;
      }

      // Test the data size
      // *actual* image length may differ to 1 with Pixel Data Element length!
      if ((testedDataSize+testedDataSize%2) !=
                                      (referenceDataSize+referenceDataSize%2) )
      {
         std::cout << " Failed" << std::endl
                   << "        pixel ("
                   << PixelType
                   <<") areas lengths differ: "
                   << testedDataSize << " # " << referenceDataSize
                   << std::endl
                   << "        Image size: ("
                   << tested->GetFile()->GetXSize() << ","
                   << tested->GetFile()->GetYSize() << ","
                   << tested->GetFile()->GetZSize() << ") nb of scalar components "
                   << tested->GetFile()->GetNumberOfScalarComponents()
                   << std::endl;
         tested->Delete();
         delete reference;
         f->Delete();
         return 1;
      }

      // Test the data content
      int length = tested->GetFile()->GetXSize()*tested->GetFile()->GetYSize()*tested->GetFile()->GetZSize()
                  *reference->GetScalarSize()*tested->GetFile()->GetNumberOfScalarComponents();

      // *actual* image length may differ to 1 with Pixel Data Element length!
      if (length != testedDataSize)
         std::cout <<"--------------------length " << length << " != testedDataSize " << testedDataSize << std::endl;
      if ( memcmp(testedImageData, referenceImageData,
                           length/*testedDataSize*/) != 0 )
      {
         std::string ts  = tested->GetFile()->GetTransferSyntax();

         std::cout << " Failed" << std::endl
                   << "        pixel (" 
                   << PixelType
                   << ") differ (as expanded in memory)."
                   << std::endl
                   << "        compression : " 
                   << GDCM_NAME_SPACE::Global::GetTS()->GetValue(ts) << std::endl;

         std::cout << "        list of the first " << MAX_NUMBER_OF_DIFFERENCE
                   << " pixels differing (pos : test - ref) :" 
                   << std::endl;
         int i;
         unsigned int j;
         for(i=0, j=0;i<testedDataSize && j<MAX_NUMBER_OF_DIFFERENCE;i++)
         {
            if(testedImageData[i]!=referenceImageData[i])
              {
               std::cout << std::hex << "(" << i << " : " 
                         << std::hex << (int)(testedImageData[i]) << " - "
                         << std::hex << (int)(referenceImageData[i]) << ") "
                         << std::dec;
               ++j;
              }
         }
         std::cout << std::endl;

         tested->Delete();
         delete reference;
         f->Delete();
         return 1;
      }

      //////////////// Clean up:
      tested->Delete();
      delete reference;
      f->Delete();

      std::cout << "OK." << std::endl;
      
      return 0;
}

int TestAllReadCompareDicom(int argc, char *argv[]) 
{
// Temporarily added, to track BigEndian troubles
GDCM_NAME_SPACE::Debug::WarningOn();

   if (argc == 4)
      GDCM_NAME_SPACE::Debug::DebugOn();

   if ( argc >= 3 )
   {
      // The test is specified a specific filename, use it instead of looping
      // over all images
      const std::string input = argv[1];
      const std::string reference = argv[2];
      return InternalTest( input, reference );
   }
   else if ( argc > 4 || argc == 2 )
   {
      std::cerr << "   Usage: " << argv[0]
                << " (no arguments needed)." << std::endl;
      std::cerr << "or   Usage: " << argv[0]
                << " filename.dcm reference.dcm" << std::endl;
      return 1;
   }
   // else other cases:
   
   std::cout << "   Description (Test::TestAllReadCompareDicom): "
             << std::endl;
   std::cout << "   For all images in gdcmData (and not blacklisted in "
                "Test/CMakeLists.txt)"
             << std::endl;
   std::cout << "   apply the following to each filename.xxx: "
             << std::endl;
   std::cout << "   step 1: parse the image (as gdcmFile) and call"
             << " IsReadable(). "
             << std::endl;
   std::cout << "   step 2: find in GDCM_DATA_ROOT/BaselineDicom/filename.tst"
             << std::endl
             << "           special internal file format containing the"
             << std::endl
             << "           caracteristic of the image and the pixel data "
             << "(uncompressed). This file is written if it's not found."
             << std::endl;
   std::cout << "   step 3: compare the DICOM image with the reference image"
             << std::endl
             << "           (.tst file). The test is made on the caracteristics"
             << std::endl
             << "           of the image and the pixel data"
             << std::endl << std::endl;

   int i = 0;
   int result = 0;
   while( gdcmDataImages[i] != 0 )
   {
      ////// Check for existence of reference baseline directory

      std::string baseLineDir = GDCM_DATA_ROOT;
      baseLineDir += "/BaselineDicom";

      if( !GDCM_NAME_SPACE::DirList::IsDirectory(baseLineDir) )
      {
         std::cerr << "   The reference baseline directory " << std::endl
                   << "      "
                   << baseLineDir << std::endl
                   << "   couldn't be opened."
                   << std::endl;
         return 1;
      }

//if (gdcmDataImages[i] == "D_CLUNIE_CT2_RLE.dcm")
//   GDCM_NAME_SPACE::Debug::DebugOn(); // track pb on BigEndian Proc
//else 
   GDCM_NAME_SPACE::Debug::DebugOff();
   
      ////// Step 1 (see above description):
      std::string filename = GDCM_DATA_ROOT;
      filename += "/";
      filename += gdcmDataImages[i];
      
      baseLineDir += '/';
      std::string referenceFileName = baseLineDir + gdcmDataImages[i++];
      std::string::size_type slash_pos = referenceFileName.rfind( "." );
      if( slash_pos != std::string::npos )
      {
         referenceFileName.replace( slash_pos + 1, 3, "tst" );
      }

      if( InternalTest( filename, referenceFileName ) != 0 )
      {
         result++;
      }
   }

   return result;
}
