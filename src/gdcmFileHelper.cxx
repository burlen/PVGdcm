/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmFileHelper.cxx,v $
  Language:  C++

  Date:      $Date: 2009/05/19 15:07:58 $
  Version:   $Revision: 1.139 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#include "gdcmFileHelper.h"
#include "gdcmGlobal.h"
#include "gdcmTS.h"
#include "gdcmDocument.h"
#include "gdcmDebug.h"
#include "gdcmUtil.h"
#include "gdcmSeqEntry.h"
#include "gdcmSQItem.h"
#include "gdcmDataEntry.h"
#include "gdcmDocEntry.h"
#include "gdcmFile.h"
#include "gdcmPixelReadConvert.h"
#include "gdcmPixelWriteConvert.h"
#include "gdcmDocEntryArchive.h"
#include "gdcmDictSet.h"
#include "gdcmOrientation.h"



#include <algorithm>  // for transform?

#if defined(__BORLANDC__)
   #include <mem.h>   // for memset
   #include <ctype.h> //for toupper
   #include <math.h>
#endif 

#include <fstream>

/*
// ----------------------------- WARNING -------------------------

These lines will be moved to the document-to-be 'User's Guide'

// To read an image, user needs a GDCM_NAME_SPACE::File
GDCM_NAME_SPACE::File *f = new GDCM_NAME_SPACE::File(fileName);
// or (advanced) :
// user may also decide he doesn't want to load some parts of the header
GDCM_NAME_SPACE::File *f = new GDCM_NAME_SPACE::File();
f->SetFileName(fileName);
   f->SetLoadMode(LD_NOSEQ);               // or      
   f->SetLoadMode(LD_NOSHADOW);            // or
   f->SetLoadMode(LD_NOSEQ | LD_NOSHADOW); // or
   f->SetLoadMode(LD_NOSHADOWSEQ);
f->Load();

// To decide whether it's an 'image of interest for him, or not,
// user can now check some values
std::string v = f->GetEntryValue(groupNb,ElementNb);

// to get the pixels, user needs a GDCM_NAME_SPACE::FileHelper
GDCM_NAME_SPACE::FileHelper *fh = new GDCM_NAME_SPACE::FileHelper(f);

// user may ask not to convert Palette (if any) to RGB
uint8_t *pixels = fh->GetImageDataRaw();
int imageLength = fh->GetImageDataRawSize();

// He can now use the pixels, create a new image, ...
uint8_t *userPixels = ...

//To re-write the image, user re-uses the GDCM_NAME_SPACE::FileHelper
GDCM_NAME_SPACE::File *fh = new GDCM_NAME_SPACE::FileHelper();

fh->SetTypeToRaw(); // Even if it was possible to convert Palette to RGB
                    // (WriteMode is set)

// If user wants to write the file as MONOCHROME1 (0=white)
fh->SetPhotometricInterpretationToMonochrome1();

fh->SetWriteTypeToDcmExpl();  // he wants Explicit Value Representation
                              // Little Endian is the default,
                              // bigendian not supported for writting
                                (-->SetWriteType(ExplicitVR);)
                                   -->WriteType = ExplicitVR;
fh->SetWriteTypeToJPEG();     // lossless compression   
fh->SetWriteTypeToJPEG2000(); // lossless compression   

fh->SetImageData( userPixels, userPixelsLength);
or
fh->SetUserData( userPixels, userPixelsLength); // this one performs compression, when required
   
fh->Write(newFileName);      // overwrites the file, if any




These lines will be moved to the document-to-be 'Developer's Guide'

WriteMode : WMODE_RAW / WMODE_RGB
WriteType : ImplicitVR, ExplicitVR, ACR, ACR_LIBIDO
PhotometricInterpretation : MONOCHROME2 (0=black), MONOCHROME2 (0=white)

fh->SetImageData( userPixels, userPixelsLength);
or
fh->SetUserData( userPixels, userPixelsLength);
   PixelWriteConverter->SetUserData(inData, expectedSize);
   
   
fh->SetWriteMode(WMODE_RAW / WMODE_RGB)

fh->SetWriteType( ImplicitVR/ExplicitVR/ACR/ACR_LIBIDO/JPEG/JPEG2000)
      
fh->Write(newFileName);
   CheckMandatoryElements(); // Checks existing ones / Add missing ones
   Fix VR if unknown elements
   SetWriteFileTypeToImplicitVR() / SetWriteFileTypeToExplicitVR(); /
   SetWriteFileTypeToACR() / SetWriteFileTypeToJPEG() / SetWriteFileTypeToJ2K()
      (Modifies TransferSyntax if any; Pushes to the Archives old one)
   SetWriteToRaw(); / SetWriteToRGB();
      (Modifies and pushes to the Archive, when necessary : photochr. interp., 
       samples per pixel, Planar configuration, 
       bits allocated, bits stored, high bit -ACR 24 bits-
       Pixels element VR, pushes out the LUT )
          SetWriteToRaw()
             Sets Photometric Interpretation
             DataEntry *pixel =CopyDataEntry(7fe0,0010,VR)
             Sets VR, BinArea, Length for PixelData
             if MONOCHROME1
                ConvertFixGreyLevels
             Archive->Push(photInt);
             Archive->Push(pixel);
             photInt->Delete();
             pixel->Delete();
        SetWriteToRGB()
           if NumberOfScalarComponents==1
              SetWriteToRaw(); return;
           PixelReadConverter->BuildRGBImage()
           DataEntry *pixel =CopyDataEntry(7fe0,0010,VR)
           Archives spp, planConfig,photInt, pixel
           Pushes out any LUT               
   CheckWriteIntegrity();
      (checks user given pixels length)
   FileInternal->Write(fileName,WriteType)
      fp = opens file(fileName); // out|binary
      ComputeGroup0002Length( );
      Document::WriteContent(fp, writetype);
         writes Dicom File Preamble not ACR-NEMA
         ElementSet::WriteContent(fp, writetype);
            writes recursively all DataElements    
   RestoreWrite();
         (moves back to the GDCM_NAME_SPACE::File all the archived elements)
*/




namespace GDCM_NAME_SPACE 
{
typedef std::map<uint16_t, int> GroupHT;    //  Hash Table
//-------------------------------------------------------------------------
// Constructor / Destructor
/**
 * \brief Constructor dedicated to deal with the *pixels* area of a ACR/DICOMV3
 *        file (GDCM_NAME_SPACE::File only deals with the ... header)
 *        Opens (in read only and when possible) an existing file and checks
 *        for DICOM compliance. Returns NULL on failure.
 *        It will be up to the user to load the pixels into memory
 *        ( GetImageDataSize() + GetImageData() methods)
 * \note  the in-memory representation of all available tags found in
 *        the DICOM header is post-poned to first header information access.
 *        This avoid a double parsing of public part of the header when
 *        one sets an a posteriori shadow dictionary (efficiency can be
 *        seen as a side effect).   
 */
FileHelper::FileHelper( )
{ 
   FileInternal = File::New( );
   Initialize();
}

/**
 * \brief Constructor dedicated to deal with the *pixels* area of a ACR/DICOMV3
 *        file (File only deals with the ... header)
 *        Opens (in read only and when possible) an existing file and checks
 *        for DICOM compliance. Returns NULL on failure.
 *        It will be up to the user to load the pixels into memory
 *        ( GetImageDataSize() + GetImageData() methods)
 * \note  the in-memory representation of all available tags found in
 *        the DICOM header is post-poned to first header information access.
 *        This avoid a double parsing of public part of the header when
 *        user sets an a posteriori shadow dictionary (efficiency can be
 *        seen as a side effect).   
 * @param header already built Header
 */
FileHelper::FileHelper(File *header)
{
   gdcmAssertMacro(header);

   FileInternal = header;
   FileInternal->Register();
   Initialize();
   if ( FileInternal->IsReadable() )
   {
      PixelReadConverter->GrabInformationsFromFile( FileInternal, this );
   }
}

/**
 * \brief canonical destructor
 * \note  If the header (GDCM_NAME_SPACE::File) was created by the FileHelper constructor,
 *        it is destroyed by the FileHelper
 */
FileHelper::~FileHelper()
{ 
   if ( PixelReadConverter )
   {
      delete PixelReadConverter;
   }
   if ( PixelWriteConverter )
   {
      delete PixelWriteConverter;
   }
   if ( Archive )
   {
      delete Archive;
   }

   FileInternal->Unregister();
}

//-----------------------------------------------------------------------------
// Public

/**
 * \brief Sets the LoadMode of the internal GDCM_NAME_SPACE::File as a boolean string. 
 *        NO_SEQ, NO_SHADOW, NO_SHADOWSEQ ... (nothing more, right now)
 *        WARNING : before using NO_SHADOW, be sure *all* your files
 *        contain accurate values in the 0x0000 element (if any) 
 *        of *each* Shadow Group. The parser will fail if the size is wrong !
 * @param   loadMode Load mode to be used    
 */
void FileHelper::SetLoadMode(int loadMode) 
{ 
   GetFile()->SetLoadMode( loadMode ); 
}
/**
 * \brief Sets the LoadMode of the internal GDCM_NAME_SPACE::File
 * @param  fileName name of the file to be open  
 */
void FileHelper::SetFileName(std::string const &fileName)
{
   FileInternal->SetFileName( fileName );
}

/**
 * \brief   Loader  
 * @return false if file cannot be open or no swap info was found,
 *         or no tag was found.
 */
bool FileHelper::Load()
{ 
   if ( !FileInternal->Load() )
      return false;

   PixelReadConverter->GrabInformationsFromFile( FileInternal, this );
   return true;
}

/**
 * \brief   Accesses an existing DataEntry through its (group, element) 
 *          and modifies its content with the given value.
 * @param   content new value (string) to substitute with
 * @param   group  group number of the Dicom Element to modify
 * @param   elem element number of the Dicom Element to modify
 * \return  false if DataEntry not found
 */
bool FileHelper::SetEntryString(std::string const &content,
                                uint16_t group, uint16_t elem)
{ 
   return FileInternal->SetEntryString(content, group, elem);
}


/**
 * \brief   Accesses an existing DataEntry through its (group, element) 
 *          and modifies its content with the given value.
 * @param   content new value (void*  -> uint8_t*) to substitute with
 * @param   lgth new value length
 * @param   group  group number of the Dicom Element to modify
 * @param   elem element number of the Dicom Element to modify
 * \return  false if DataEntry not found
 */
bool FileHelper::SetEntryBinArea(uint8_t *content, int lgth,
                                 uint16_t group, uint16_t elem)
{
   return FileInternal->SetEntryBinArea(content, lgth, group, elem);
}

/**
 * \brief   Modifies the value of a given DataEntry when it exists.
 *          Creates it with the given value when unexistant.
 * @param   content (string) value to be set
 * @param   group   Group number of the Entry 
 * @param   elem  Element number of the Entry
 * @param   vr  Value Representation of the DataElement to be inserted
 * \return  pointer to the modified/created DataEntry (NULL when creation
 *          failed).
 */ 
DataEntry *FileHelper::InsertEntryString(std::string const &content,
                                         uint16_t group, uint16_t elem,
                                         VRKey const &vr )
{
   return FileInternal->InsertEntryString(content, group, elem, vr);
}

/**
 * \brief   Modifies the value of a given DataEntry when it exists.
 *          Creates it with the given value when unexistant.
 *          A copy of the binArea is made to be kept in the Document.
 * @param   binArea (binary) value to be set
 * @param   lgth new value length
 * @param   group   Group number of the Entry 
 * @param   elem  Element number of the Entry
 * @param   vr  Value Representation of the DataElement to be inserted 
 * \return  pointer to the modified/created DataEntry (NULL when creation
 *          failed).
 */
DataEntry *FileHelper::InsertEntryBinArea(uint8_t *binArea, int lgth,
                                          uint16_t group, uint16_t elem,
                                          VRKey const &vr )
{
   return FileInternal->InsertEntryBinArea(binArea, lgth, group, elem, vr);
}

/**
 * \brief   Adds an empty SeqEntry 
 *          (remove any existing entry with same group,elem)
 * @param   group   Group number of the Entry 
 * @param   elem  Element number of the Entry
 * \return  pointer to the created SeqEntry (NULL when creation
 *          failed).
 */
SeqEntry *FileHelper::InsertSeqEntry(uint16_t group, uint16_t elem)
{
   return FileInternal->InsertSeqEntry(group, elem);
}

/**
 * \brief   Get the size of the image data
 *          If the image can be RGB (with a lut or by default), the size 
 *          corresponds to the RGB image
 *         (use GetImageDataRawSize if you want to be sure to get *only*
 *          the size of the pixels)
 * @return  The image size
 */
size_t FileHelper::GetImageDataSize()
{
   if ( PixelWriteConverter->GetUserData() )
   {
      return PixelWriteConverter->GetUserDataSize();
   }
   return PixelReadConverter->GetRGBSize();
}

/**
 * \brief   Get the size of the image data.
 *          If the image could be converted to RGB using a LUT, 
 *          this transformation is not taken into account by GetImageDataRawSize
 *          (use GetImageDataSize if you wish)
 * @return  The raw image size
 */
size_t FileHelper::GetImageDataRawSize()
{
   if ( PixelWriteConverter->GetUserData() )
   {
      return PixelWriteConverter->GetUserDataSize();
   }
   return PixelReadConverter->GetRawSize();
}

/**
 * \brief brings pixels into memory :  
 *          - Allocates necessary memory,
 *          - Reads the pixels from disk (uncompress if necessary),
 *          - Transforms YBR pixels, if any, into RGB pixels,
 *          - Transforms 3 planes R, G, B, if any, into a single RGB Plane
 *          - Transforms single Grey plane + 3 Palettes into a RGB Plane
 *          - Copies the pixel data (image[s]/volume[s]) to newly allocated zone.
 * @return  Pointer to newly allocated pixel data.
 *          (uint8_t is just for prototyping. feel free to cast)
 *          NULL if alloc fails 
 */
uint8_t *FileHelper::GetImageData()
{
   if ( PixelWriteConverter->GetUserData() )
   {
      return PixelWriteConverter->GetUserData();
   }

   if ( ! GetRaw() )
   {
      // If the decompression failed nothing can be done.
      return 0;
   }

   if ( FileInternal->HasLUT() && PixelReadConverter->BuildRGBImage() )
   {
      return PixelReadConverter->GetRGB();
   }
   else
   {
      // When no LUT or LUT conversion fails, return the Raw
      return PixelReadConverter->GetRaw();
   }
}

/**
 * \brief brings pixels into memory :  
 *          - Allocates necessary memory, 
 *          - Transforms YBR pixels (if any) into RGB pixels
 *          - Transforms 3 planes R, G, B  (if any) into a single RGB Plane
 *          - Copies the pixel data (image[s]/volume[s]) to newly allocated zone. 
 *          - DOES NOT transform Grey plane + 3 Palettes into a RGB Plane
 * @return  Pointer to newly allocated pixel data.
 *          (uint8_t is just for prototyping. feel free to cast)
 *          NULL if alloc fails
 */
uint8_t *FileHelper::GetImageDataRaw ()
{
   return GetRaw();
}

//#ifndef GDCM_LEGACY_REMOVE
/*
 * \brief   Useless function, since PixelReadConverter forces us 
 *          copy the Pixels anyway.  
 *          Reads the pixels from disk (uncompress if necessary),
 *          Transforms YBR pixels, if any, into RGB pixels
 *          Transforms 3 planes R, G, B, if any, into a single RGB Plane
 *          Transforms single Grey plane + 3 Palettes into a RGB Plane   
 *          Copies at most MaxSize bytes of pixel data to caller allocated
 *          memory space.
 * \warning This function allows people that want to build a volume
 *          from an image stack *not to* have, first to get the image pixels, 
 *          and then move them to the volume area.
 *          It's absolutely useless for any VTK user since vtk chooses 
 *          to invert the lines of an image, that is the last line comes first
 *          (for some axis related reasons?). Hence he will have 
 *          to load the image line by line, starting from the end.
 *          VTK users have to call GetImageData
 *     
 * @param   destination Address (in caller's memory space) at which the
 *          pixel data should be copied
 * @param   maxSize Maximum number of bytes to be copied. When MaxSize
 *          is not sufficient to hold the pixel data the copy is not
 *          executed (i.e. no partial copy).
 * @return  On success, the number of bytes actually copied. Zero on
 *          failure e.g. MaxSize is lower than necessary.
 */
 /*
size_t FileHelper::GetImageDataIntoVector (void *destination, size_t maxSize)
{
   if ( ! GetRaw() )
   {
      // If the decompression failed nothing can be done.
      return 0;
   }

   if ( FileInternal->HasLUT() && PixelReadConverter->BuildRGBImage() )
   {
      if ( PixelReadConverter->GetRGBSize() > maxSize )
      {
         gdcmWarningMacro( "Pixel data bigger than caller's expected MaxSize");
         return 0;
      }
      memcpy( destination,
              (void*)PixelReadConverter->GetRGB(),
              PixelReadConverter->GetRGBSize() );
      return PixelReadConverter->GetRGBSize();
   }

   // Either no LUT conversion necessary or LUT conversion failed
   if ( PixelReadConverter->GetRawSize() > maxSize )
   {
      gdcmWarningMacro( "Pixel data bigger than caller's expected MaxSize");
      return 0;
   }
   memcpy( destination,
           (void *)PixelReadConverter->GetRaw(),
           PixelReadConverter->GetRawSize() );
   return PixelReadConverter->GetRawSize();
}
*/
//#endif

/**
 * \brief   Points the internal pointer to the callers inData
 *          image representation, BUT WITHOUT COPYING THE DATA.
 *          'image' Pixels are presented as C-like 2D arrays : line per line.
 *          'volume'Pixels are presented as C-like 3D arrays : plane per plane 
 * \warning Since the pixels are not copied, it is the caller's responsability
 *          not to deallocate its data before gdcm uses them (e.g. with
 *          the Write() method )
 * @param inData user supplied pixel area (uint8_t* is just for the compiler.
 *               user is allowed to pass any kind of pixels since the size is
 *               given in bytes) 
 * @param expectedSize total image size, *in Bytes*
 */
void FileHelper::SetImageData(uint8_t *inData, size_t expectedSize)
{
   PixelWriteConverter->SetUserData(inData, expectedSize);
   /// \todo : shouldn't we call SetCompressJPEGUserData/SetCompressJPEG2000UserData
   ///         here, too?
}

/**
 * \brief   Set the image data defined by the user
 * \warning When writting the file, this data are get as default data to write
 * @param inData user supplied pixel area (uint8_t* is just for the compiler.
 *               user is allowed to pass any kind of pixels since the size is
 *               given in bytes) 
 * @param expectedSize total image size, *in Bytes* 
 */
void FileHelper::SetUserData(uint8_t *inData, size_t expectedSize)
{
  // Shouldn't we move theese lines to FileHelper::Write()?
/*  
   if( WriteType == JPEG2000 )
   {
      PixelWriteConverter->SetCompressJPEG2000UserData(inData, expectedSize, FileInternal);
   }
   else if( WriteType == JPEG )
   {
      PixelWriteConverter->SetCompressJPEGUserData(inData, expectedSize, FileInternal);
   }
   else
   {
      PixelWriteConverter->SetUserData(inData, expectedSize);
   }
   */
   // Just try!
   PixelWriteConverter->SetUserData(inData, expectedSize);
}

/**
 * \brief   Get the image data defined by the user
 * \warning When writting the file, this data are get as default data to write
 */
uint8_t *FileHelper::GetUserData()
{
   return PixelWriteConverter->GetUserData();
}

/**
 * \brief   Get the image data size defined by the user
 * \warning When writting the file, this data are get as default data to write
 */
size_t FileHelper::GetUserDataSize()
{
   return PixelWriteConverter->GetUserDataSize();
}

/**
 * \brief   Get the image data from the file.
 *          If a LUT is found, the data are expanded to be RGB
 */
uint8_t *FileHelper::GetRGBData()
{
   return PixelReadConverter->GetRGB();
}

/**
 * \brief   Get the image data size from the file.
 *          If a LUT is found, the data are expanded to be RGB
 */
size_t FileHelper::GetRGBDataSize()
{
   return PixelReadConverter->GetRGBSize();
}

/**
 * \brief   Get the image data from the file.
 *          Even when a LUT is found, the data are not expanded to RGB!
 */
uint8_t *FileHelper::GetRawData()
{
   return PixelReadConverter->GetRaw();
}

/**
 * \brief   Get the image data size from the file.
 *          Even when a LUT is found, the data are not expanded to RGB!
 */
size_t FileHelper::GetRawDataSize()
{
   return PixelReadConverter->GetRawSize();
}

/**
 * \brief Access to the underlying PixelReadConverter RGBA LUT
 */
uint8_t* FileHelper::GetLutRGBA()
{
   if ( PixelReadConverter->GetLutRGBA() ==0 )
      PixelReadConverter->BuildLUTRGBA();
   return PixelReadConverter->GetLutRGBA();
}

/**
 * \brief Access to the underlying PixelReadConverter RGBA LUT Item Number
 */
int FileHelper::GetLutItemNumber()
{
   return PixelReadConverter->GetLutItemNumber();
}

/**
 * \brief Access to the underlying PixelReadConverter RGBA LUT Item Size
 */
int FileHelper::GetLutItemSize()
{
   return PixelReadConverter->GetLutItemSize();
}

/**
 * \brief Writes on disk A SINGLE Dicom file
 *        NO test is performed on  processor "Endiannity".
 *        It's up to the user to call his Reader properly
 * @param fileName name of the file to be created
 *                 (any already existing file is over written)
 * @return false if write fails
 */
bool FileHelper::WriteRawData(std::string const &fileName)
{
   std::ofstream fp1(fileName.c_str(), std::ios::out | std::ios::binary );
   if (!fp1)
   {
      gdcmWarningMacro( "Fail to open (write) file:" << fileName.c_str());
      return false;
   }

   if ( PixelWriteConverter->GetUserData() )
   {
      fp1.write( (char *)PixelWriteConverter->GetUserData(), 
                 PixelWriteConverter->GetUserDataSize() );
   }
   else if ( PixelReadConverter->GetRGB() )
   {
      fp1.write( (char *)PixelReadConverter->GetRGB(), 
                 PixelReadConverter->GetRGBSize());
   }
   else if ( PixelReadConverter->GetRaw() )
   {
      fp1.write( (char *)PixelReadConverter->GetRaw(), 
                 PixelReadConverter->GetRawSize());
   }
   else
   {
      gdcmErrorMacro( "Nothing written." );
   }

   fp1.close();

   return true;
}

/**
 * \brief Writes on disk A SINGLE Dicom file, 
 *        using the Implicit Value Representation convention
 *        NO test is performed on  processor "Endianity".
 * @param fileName name of the file to be created
 *                 (any already existing file is overwritten)
 * @return false if write fails
 */

bool FileHelper::WriteDcmImplVR (std::string const &fileName)
{
   SetWriteTypeToDcmImplVR();
   return Write(fileName);
}

/**
* \brief Writes on disk A SINGLE Dicom file, 
 *        using the Explicit Value Representation convention
 *        NO test is performed on  processor "Endiannity". 
 * @param fileName name of the file to be created
 *                 (any already existing file is overwritten)
 * @return false if write fails
 */

bool FileHelper::WriteDcmExplVR (std::string const &fileName)
{
   SetWriteTypeToDcmExplVR();
   return Write(fileName);
}

/**
 * \brief Writes on disk A SINGLE Dicom file, 
 *        using the ACR-NEMA convention
 *        NO test is performed on  processor "Endiannity".
 *        (a l'attention des logiciels cliniques 
 *        qui ne prennent en entrée QUE des images ACR ...
 * \warning if a DICOM_V3 header is supplied,
 *         groups < 0x0008 and shadow groups are ignored
 * \warning NO TEST is performed on processor "Endiannity".
 * @param fileName name of the file to be created
 *                 (any already existing file is overwritten)
 * @return false if write fails
 */

bool FileHelper::WriteAcr (std::string const &fileName)
{
   SetWriteTypeToAcr();
   return Write(fileName);
}

/**
 * \brief Writes on disk A SINGLE Dicom file, 
 * @param fileName name of the file to be created
 *                 (any already existing file is overwritten)
 * @return false if write fails
 */
bool FileHelper::Write(std::string const &fileName)
{ 
   CheckMandatoryElements(); //called once, here !
   
   switch(WriteType)
   {
      case ImplicitVR:
         SetWriteFileTypeToImplicitVR();
         break;
 
      case Unknown:  // should never happen; ExplicitVR is the default value
      case ExplicitVR:
   
   // We let DocEntry::WriteContent to put vr=UN for undocumented Shadow Groups !
         SetWriteFileTypeToExplicitVR();

  break;
      case ACR:
      case ACR_LIBIDO:
      // NOTHING is done here just for LibIDO.
      // Just to avoid further trouble if user creates a file ex-nihilo,
      // wants to write it as an ACR-NEMA file,
      // and forgets to create any Entry belonging to group 0008
      // (shame on him !)
      // We add Recognition Code (RET)
        if ( ! FileInternal->GetDataEntry(0x0008, 0x0010) )
            FileInternal->InsertEntryString("ACR-NEMA V1.0 ", 
                                             0x0008, 0x0010, "LO");
         SetWriteFileTypeToACR();
        // SetWriteFileTypeToImplicitVR(); // ACR IS implicit VR !
         break;
 
      /// \todo FIXME : JPEG/JPEG2000 may be either ExplicitVR or ImplicitVR      
      case JPEG:
         SetWriteFileTypeToJPEG();
         // was :
         //PixelWriteConverter->SetCompressJPEGUserData(
         //   inData, expectedSize, FileInternal);
         PixelWriteConverter->SetCompressJPEGUserData(
                 PixelWriteConverter->GetUserData(),
                 PixelWriteConverter->GetUserDataSize(),FileInternal);
         break;

      case JPEG2000:
         /// \todo Maybe we should consider doing the compression here !
         // PixelWriteConverter->SetCompressJPEG2000UserData(inData, expectedSize, FileInternal);

         SetWriteFileTypeToJPEG2000();
         PixelWriteConverter->SetCompressJPEG2000UserData(
            PixelWriteConverter->GetUserData(),
            PixelWriteConverter->GetUserDataSize(),
            FileInternal);
         break;
   }

   // --------------------------------------------------------------
   // Special Patch to allow gdcm to re-write ACR-LibIDO formated images
   //
   // if recognition code tells us we dealt with a LibIDO image
   // we reproduce on disk the switch between lineNumber and columnNumber
   // just before writting ...
   /// \todo the best trick would be *change* the recognition code
   ///       but pb expected if user deals with, e.g. COMPLEX images
   
   if ( WriteType == ACR_LIBIDO )
   {
      SetWriteToLibido();
   }
   else
   {
      SetWriteToNoLibido();
   }
   // ----------------- End of Special Patch ----------------
  
   switch(WriteMode)
   {
      case WMODE_RAW :
         SetWriteToRaw(); // modifies and pushes to the archive, when necessary
         break;
      case WMODE_RGB :
         SetWriteToRGB(); // modifies and pushes to the archive, when necessary
         break;
   }

   bool check;
   if (WriteType == JPEG || WriteType == JPEG2000)
      check = true;
   else
      check = CheckWriteIntegrity(); // verifies length

   if (check)
   {
      check = FileInternal->Write(fileName,WriteType);
   }

   RestoreWrite();
  // RestoreWriteFileType();
  // RestoreWriteMandatory();

   // --------------------------------------------------------------
   // Special Patch to allow gdcm to re-write ACR-LibIDO formated images
   // 
   // ...and we restore the header to be Dicom Compliant again 
   // just after writting
   RestoreWriteOfLibido();
   // ----------------- End of Special Patch ----------------

   return check;
}

//-----------------------------------------------------------------------------
// Protected
/**
 * \brief Verifies the size of the user given PixelData
 * @return true if check is successfull
 */
bool FileHelper::CheckWriteIntegrity()
{
   if ( PixelWriteConverter->GetUserData() )
   {
      int numberBitsAllocated = FileInternal->GetBitsAllocated();
      if ( numberBitsAllocated == 0 || numberBitsAllocated == 12 )
      {
         gdcmWarningMacro( "numberBitsAllocated changed from "
                          << numberBitsAllocated << " to 16 "
                          << " for consistency purpose" );
         numberBitsAllocated = 16;
      }

      size_t decSize = FileInternal->GetXSize()
                     * FileInternal->GetYSize()
                     * FileInternal->GetZSize()
                     * FileInternal->GetTSize()     
                     * FileInternal->GetSamplesPerPixel()
                     * ( numberBitsAllocated / 8 );
      size_t rgbSize = decSize;
      if ( FileInternal->HasLUT() )
         rgbSize = decSize * 3;

      size_t userDataSize = PixelWriteConverter->GetUserDataSize();
      switch(WriteMode)
      {
         case WMODE_RAW :
            if ( abs((long)(decSize-userDataSize))>1) // ignore padding zero
            {
               gdcmWarningMacro( "Data size (Raw) is incorrect. Should be " 
                           << decSize << "(" 
                           << FileInternal->GetXSize() << " * "
                           << FileInternal->GetYSize() << " * "
                           << FileInternal->GetZSize() << " * "
                           << FileInternal->GetTSize() << " * "   
                           << FileInternal->GetSamplesPerPixel() << " * "
                           << numberBitsAllocated / 8   
                           << ") / Found :" 
                           << userDataSize );
               return false;
            }
            break;
         case WMODE_RGB :
            if ( abs((long)(rgbSize-userDataSize))>1) // ignore padding zero
            {
               gdcmWarningMacro( "Data size (RGB) is incorrect. Should be " 
                          << rgbSize << " / Found " 
                          << userDataSize );
               return false;
            }
            break;
      }
   }
   return true;
}

/**
 * \brief Updates the File to write RAW data (as opposed to RGB data)
 *       (modifies, when necessary, photochromatic interpretation, 
 *       bits allocated, Pixels element VR)
 *       WARNING : if SetPhotometricInterpretationToMonochrome1() was called
 *                 before Pixel Elements is modified :-( 
 */ 
void FileHelper::SetWriteToRaw()
{
   if ( FileInternal->GetNumberOfScalarComponents() == 3 
    && !FileInternal->HasLUT() )
   {
      SetWriteToRGB();
   } 
   else
   {
      // 0x0028,0x0004 : Photometric Interpretation
      DataEntry *photInt = CopyDataEntry(0x0028,0x0004,"CS");
      if (FileInternal->HasLUT() )
      {
         photInt->SetString("PALETTE COLOR ");
      }
      else
      {
         if (GetPhotometricInterpretation() == 2)
            photInt->SetString("MONOCHROME2 ");  // 0 = Black
         else
            photInt->SetString("MONOCHROME1 ");  // 0 = White !
      }

      PixelWriteConverter->SetReadData(PixelReadConverter->GetRaw(),
                                       PixelReadConverter->GetRawSize());

      std::string vr = "OB";
      if ( FileInternal->GetBitsAllocated()>8 )
         vr = "OW";
      if ( FileInternal->GetBitsAllocated()==24 ) // For RGB ACR files 
         vr = "OB";
       // For non RAW data. Mainly JPEG/JPEG2000
      if( WriteType == JPEG || WriteType == JPEG2000)
      {
         vr = "OW";
      }

      DataEntry *pixel = 
         CopyDataEntry(GetFile()->GetGrPixel(),GetFile()->GetNumPixel(),vr);
      pixel->SetFlag(DataEntry::FLAG_PIXELDATA);
      pixel->SetBinArea(PixelWriteConverter->GetData(),false);
      pixel->SetLength(
         static_cast< uint32_t >(PixelWriteConverter->GetDataSize()) );

      if (!FileInternal->HasLUT() && GetPhotometricInterpretation() == 1)
      {
          ConvertFixGreyLevels( pixel->GetBinArea(), pixel->GetLength() );
      }

      Archive->Push(photInt);
      Archive->Push(pixel);

      photInt->Delete();
      pixel->Delete();
   }
}

/**
 * \brief Updates the File to write RGB data (as opposed to RAW data)
 *       (modifies, when necessary, photochromatic interpretation, 
 *       samples per pixel, Planar configuration, 
 *       bits allocated, bits stored, high bit -ACR 24 bits-
 *       Pixels element VR, pushes out the LUT, )
 */ 
void FileHelper::SetWriteToRGB()
{
   if ( FileInternal->GetNumberOfScalarComponents()==3 )
   {
      PixelReadConverter->BuildRGBImage();
      
      DataEntry *spp = CopyDataEntry(0x0028,0x0002,"US");
      spp->SetString("3 ");  // Don't drop trailing space

      DataEntry *planConfig = CopyDataEntry(0x0028,0x0006,"US");
      planConfig->SetString("0 "); // Don't drop trailing space

      DataEntry *photInt = CopyDataEntry(0x0028,0x0004,"CS");
      photInt->SetString("RGB "); // Don't drop trailing space

      if ( PixelReadConverter->GetRGB() )
      {
         PixelWriteConverter->SetReadData(PixelReadConverter->GetRGB(),
                                          PixelReadConverter->GetRGBSize());
      }
      else // Raw data
      {
         PixelWriteConverter->SetReadData(PixelReadConverter->GetRaw(),
                                          PixelReadConverter->GetRawSize());
      }

      std::string vr = "OB";
      if ( FileInternal->GetBitsAllocated()>8 )
         vr = "OW";
      if ( FileInternal->GetBitsAllocated()==24 ) // For RGB ACR files 
         vr = "OB";
      DataEntry *pixel = 
         CopyDataEntry(GetFile()->GetGrPixel(),GetFile()->GetNumPixel(),vr);
      pixel->SetFlag(DataEntry::FLAG_PIXELDATA);
      pixel->SetBinArea(PixelWriteConverter->GetData(),false);
      pixel->SetLength(PixelWriteConverter->GetDataSize());

      Archive->Push(spp);
      Archive->Push(planConfig);
      Archive->Push(photInt);
      Archive->Push(pixel);

      spp->Delete();
      planConfig->Delete();
      photInt->Delete();
      pixel->Delete();

      // Remove any LUT
      Archive->Push(0x0028,0x1101);
      Archive->Push(0x0028,0x1102);
      Archive->Push(0x0028,0x1103);
      Archive->Push(0x0028,0x1201);
      Archive->Push(0x0028,0x1202);
      Archive->Push(0x0028,0x1203);

      // push out Palette Color Lookup Table UID, if any
      Archive->Push(0x0028,0x1199);

      // For old '24 Bits' ACR-NEMA
      // Thus, we have a RGB image and the bits allocated = 24 and 
      // samples per pixels = 1 (in the read file)
      if ( FileInternal->GetBitsAllocated()==24 ) 
      {
         DataEntry *bitsAlloc = CopyDataEntry(0x0028,0x0100,"US");
         bitsAlloc->SetString("8 ");

         DataEntry *bitsStored = CopyDataEntry(0x0028,0x0101,"US");
         bitsStored->SetString("8 ");

         DataEntry *highBit = CopyDataEntry(0x0028,0x0102,"US");
         highBit->SetString("7 ");

         Archive->Push(bitsAlloc);
         Archive->Push(bitsStored);
         Archive->Push(highBit);

         bitsAlloc->Delete();
         bitsStored->Delete();
         highBit->Delete();
      }
   }
   else
   {
      SetWriteToRaw();
   }
}

/**
 * \brief Restore the File write mode  
 */ 
void FileHelper::RestoreWrite()
{
   Archive->Restore(0x0028,0x0002);
   Archive->Restore(0x0028,0x0004);
   
   Archive->Restore(0x0028,0x0006);
   Archive->Restore(GetFile()->GetGrPixel(),GetFile()->GetNumPixel());

   // For old ACR-NEMA (24 bits problem)
   Archive->Restore(0x0028,0x0100);
   Archive->Restore(0x0028,0x0101);
   Archive->Restore(0x0028,0x0102);

   // For the LUT
   Archive->Restore(0x0028,0x1101);
   Archive->Restore(0x0028,0x1102);
   Archive->Restore(0x0028,0x1103);
   Archive->Restore(0x0028,0x1201);
   Archive->Restore(0x0028,0x1202);
   Archive->Restore(0x0028,0x1203);

   // For the Palette Color Lookup Table UID
   Archive->Restore(0x0028,0x1203); 

   // group 0002 may be pushed out for ACR-NEMA writting purposes 
   Archive->Restore(0x0002,0x0000);
   Archive->Restore(0x0002,0x0001);
   Archive->Restore(0x0002,0x0002);
   Archive->Restore(0x0002,0x0003);
   Archive->Restore(0x0002,0x0010);
   Archive->Restore(0x0002,0x0012);
   Archive->Restore(0x0002,0x0013);
   Archive->Restore(0x0002,0x0016);
   Archive->Restore(0x0002,0x0100);
   Archive->Restore(0x0002,0x0102);

}

/**
 * \brief Pushes out the whole group 0002
 *        FIXME : better, set a flag to tell the writer not to write it ...
 *        FIXME : method should probably have an other name !
 *                SetWriteFileTypeToACR is NOT opposed to 
 *                SetWriteFileTypeToExplicitVR and SetWriteFileTypeToImplicitVR
 */ 
void FileHelper::SetWriteFileTypeToACR()
{
   Archive->Push(0x0002,0x0000);
   Archive->Push(0x0002,0x0001);
   Archive->Push(0x0002,0x0002);
   Archive->Push(0x0002,0x0003);
   Archive->Push(0x0002,0x0010);
   Archive->Push(0x0002,0x0012);
   Archive->Push(0x0002,0x0013);
   Archive->Push(0x0002,0x0016);
   Archive->Push(0x0002,0x0100);
   Archive->Push(0x0002,0x0102);
}

/**
 * \brief Sets in the File the TransferSyntax to 'JPEG2000'
 */
void FileHelper::SetWriteFileTypeToJPEG2000()
{
   std::string ts = Util::DicomString(
   Global::GetTS()->GetSpecialTransferSyntax(TS::JPEG2000Lossless) );

   DataEntry *tss = CopyDataEntry(0x0002,0x0010,"UI");
   tss->SetString(ts);

   Archive->Push(tss);
   tss->Delete();   
}

/**
 * \brief Sets in the File the TransferSyntax to 'JPEG'
 */
void FileHelper::SetWriteFileTypeToJPEG()
{
   std::string ts = Util::DicomString(
      Global::GetTS()->GetSpecialTransferSyntax(TS::JPEGLosslessProcess14_1) );

   DataEntry *tss = CopyDataEntry(0x0002,0x0010,"UI");
   tss->SetString(ts);

   Archive->Push(tss);
   tss->Delete();
}

/**
 * \brief Sets in the File the TransferSyntax to 'Explicit VR Little Endian"   
 */ 
void FileHelper::SetWriteFileTypeToExplicitVR()
{
   std::string ts = Util::DicomString( 
      Global::GetTS()->GetSpecialTransferSyntax(TS::ExplicitVRLittleEndian) );

   DataEntry *tss = CopyDataEntry(0x0002,0x0010,"UI");
   tss->SetString(ts);
   Archive->Push(tss);
   tss->Delete();
}

/**
 * \brief Sets in the File the TransferSyntax to 'Implicit VR Little Endian"   
 */ 
void FileHelper::SetWriteFileTypeToImplicitVR()
{
   std::string ts = Util::DicomString(
      Global::GetTS()->GetSpecialTransferSyntax(TS::ImplicitVRLittleEndian) );

   DataEntry *tss = CopyDataEntry(0x0002,0x0010,"UI");
   tss->SetString(ts);
   Archive->Push(tss);
   tss->Delete();
}

/**
 * \brief Set the Write not to Libido format
 */ 
void FileHelper::SetWriteToLibido()
{
   DataEntry *oldRow = FileInternal->GetDataEntry(0x0028, 0x0010);
   DataEntry *oldCol = FileInternal->GetDataEntry(0x0028, 0x0011);
   
   if ( oldRow && oldCol )
   {
      std::string rows, columns; 

      DataEntry *newRow=DataEntry::New(0x0028, 0x0010, "US");
      DataEntry *newCol=DataEntry::New(0x0028, 0x0011, "US");

      newRow->Copy(oldCol);
      newCol->Copy(oldRow);

      newRow->SetString(oldCol->GetString());
      newCol->SetString(oldRow->GetString());

      Archive->Push(newRow);
      Archive->Push(newCol);

      newRow->Delete();
      newCol->Delete();
   }

   DataEntry *libidoCode = CopyDataEntry(0x0008,0x0010,"LO");
   libidoCode->SetString("ACRNEMA_LIBIDO_1.1");
   Archive->Push(libidoCode);
   libidoCode->Delete();
}

/**
 * \brief Set the Write not to No Libido format
 */ 
void FileHelper::SetWriteToNoLibido()
{
   DataEntry *recCode = FileInternal->GetDataEntry(0x0008,0x0010);
   if ( recCode )
   {
      if ( recCode->GetString() == "ACRNEMA_LIBIDO_1.1" )
      {
         DataEntry *libidoCode = CopyDataEntry(0x0008,0x0010,"LO");
         libidoCode->SetString("");
         Archive->Push(libidoCode);
         libidoCode->Delete();
      }
   }
}

/**
 * \brief Restore the Write format
 */ 
void FileHelper::RestoreWriteOfLibido()
{
   Archive->Restore(0x0028,0x0010);
   Archive->Restore(0x0028,0x0011);
   Archive->Restore(0x0008,0x0010);

   // Restore 'LibIDO-special' entries, if any
   Archive->Restore(0x0028,0x0015);
   Archive->Restore(0x0028,0x0016);
   Archive->Restore(0x0028,0x0017);
   Archive->Restore(0x0028,0x00199);
}

/**
 * \brief   Duplicates a DataEntry or creates it.
 * @param   group   Group number of the Entry 
 * @param   elem  Element number of the Entry
 * @param   vr  Value Representation of the Entry
 * \return  pointer to the new Bin Entry (NULL when creation failed).
 */ 
DataEntry *FileHelper::CopyDataEntry(uint16_t group, uint16_t elem,
                                   const VRKey &vr)
{
   DocEntry *oldE = FileInternal->GetDocEntry(group, elem);
   DataEntry *newE;

   if ( oldE && vr != GDCM_VRUNKNOWN ) 
      if ( oldE->GetVR() != vr )
         oldE = NULL;

   if ( oldE )
   {
      newE = DataEntry::New(group, elem, vr);
      newE->Copy(oldE);
   }
   else
   {
      newE = GetFile()->NewDataEntry(group, elem, vr);
   }

   return newE;
}

/**
 * \brief   This method is called automatically, just before writting
 *         in order to produce a 'True Dicom V3' image.
 *
 *         We cannot know *how* the user made the File :
 *         (reading an old ACR-NEMA file or a not very clean DICOM file ...) 
 *          Just before writting :
 *             - we check the Entries
 *             - we create the mandatory entries if they are missing
 *             - we modify the values if necessary
 *             - we push the sensitive entries to the Archive
 *          The writing process will restore the entries as they where before 
 *          entering FileHelper::CheckMandatoryElements, so the user will always
 *          see the entries just as they were before he decided to write.
 *
 * \note
 *       -  Entries whose type is 1 are mandatory, with a mandatory value
 *       -  Entries whose type is 1c are mandatory-inside-a-Sequence,
 *                             with a mandatory value
 *       -  Entries whose type is 2 are mandatory, with an optional value
 *       -  Entries whose type is 2c are mandatory-inside-a-Sequence,
 *                             with an optional value
 *       -  Entries whose type is 3 are optional
 * 
 * \todo 
 *         - warn the user if we had to add some entries :
 *         even if a mandatory entry is missing, we add it, with a default value
 *         (we don't want to give up the writting process if user forgot to
 *         specify Lena's Patient ID, for instance ...)
 *         - read the whole PS 3.3 Part of DICOM  (890 pages)
 *         and write a *full* checker (probably one method per Modality ...)
 *         Any contribution is welcome. 
 *         - write a user callable full checker, to allow post reading
 *         and/or pre writting image consistency check.           
 */ 

/* -------------------------------------------------------------------------------------
To be moved to User's guide / WIKI  ?

We have to deal with 4 *very* different cases :
-1) user created ex nihilo his own image and wants to write it as a Dicom image.
    USER_OWN_IMAGE
-2) user modified the pixels of an existing image.
   FILTERED_IMAGE
-3) user created a new image, using a set of existing images (eg MIP, MPR, cartography image)
   CREATED_IMAGE
-4) user modified/added some tags *without processing* the pixels (anonymization...)
   UNMODIFIED_PIXELS_IMAGE
-Probabely some more to be added.
 --> Set it with FileHelper::SetContentType(int);
 
GDCM_NAME_SPACE::FileHelper::CheckMandatoryElements() deals automatically with these cases.

1)2)3)4)
0008 0012 Instance Creation Date
0008 0013 Instance Creation Time
0008 0018 SOP Instance UID
are *always* created with the current values; user has *no* possible intervention on
them.

'Serie Instance UID'(0x0020,0x000e)
'Study Instance UID'(0x0020,0x000d) are kept as is if already exist,
                                    created  if it doesn't.
 The user is allowed to create his own Series/Studies, 
     keeping the same 'Serie Instance UID' / 'Study Instance UID' for various images
 Warning :     
 The user shouldn't add any image to a 'Manufacturer Serie'
     but there is no way no to allow him to do that
     
 None of the 'shadow elements' are droped out.
     

1)
'Conversion Type (0x0008,0x0064) is forced to 'SYN' (Synthetic Image).
 
1)3)
'Media Storage SOP Class UID' (0x0002,0x0002)
'SOP Class UID'               (0x0008,0x0016) are set to 
                                               [Secondary Capture Image Storage]
'Image Type'                  (0x0008,0x0008) is forced to  "DERIVED\PRIMARY"
Conversion Type               (0x0008,0x0064) is forced to 'SYN' (Synthetic Image)

2)4)
If 'SOP Class UID' exists in the native image  ('true DICOM' image)
    we create the 'Source Image Sequence' SeqEntry (0x0008, 0x2112)    
    --> 'Referenced SOP Class UID' (0x0008, 0x1150)
         whose value is the original 'SOP Class UID'
    --> 'Referenced SOP Instance UID' (0x0008, 0x1155)
         whose value is the original 'SOP Class UID'

3) TO DO : find a trick to allow user to pass to the writter the list of the Dicom images 
          or the Series, (or the Study ?) he used to created his image 
          (MIP, MPR, cartography image, ...)
           These info should be stored (?)
          0008 1110 SQ 1 Referenced Study Sequence
          0008 1115 SQ 1 Referenced Series Sequence
          0008 1140 SQ 1 Referenced Image Sequence
       
4) When user *knows* he didn't modified the pixels, we keep some informations unchanged :
'Media Storage SOP Class UID' (0x0002,0x0002)
'SOP Class UID'               (0x0008,0x0016)
'Image Type'                  (0x0008,0x0008)
'Conversion Type'             (0x0008,0x0064)


Bellow follows the full description (hope so !) of the consistency checks performed 
by GDCM_NAME_SPACE::FileHelper::CheckMandatoryElements()


-->'Media Storage SOP Class UID' (0x0002,0x0002)
-->'SOP Class UID'               (0x0008,0x0016) are defaulted to 
                                               [Secondary Capture Image Storage]
--> 'Image Type'  (0x0008,0x0008)
     is forced to  "DERIVED\PRIMARY"
     (The written image is no longer an 'ORIGINAL' one)
  Except if user knows he didn't modify the image (e.g. : he just anonymized the file)
   
 -->  Conversion Type (0x0008,0x0064)
     is defaulted to 'SYN' (Synthetic Image)
  when *he* knows he created his own image ex nihilo
            
--> 'Modality' (0x0008,0x0060)   
    is defaulted to "OT" (other) if missing.   
    (a fully user created image belongs to *no* modality)
      
--> 'Media Storage SOP Instance UID' (0x0002,0x0003)
--> 'Implementation Class UID'       (0x0002,0x0012)
    are automatically generated; no user intervention possible

--> 'Serie Instance UID'(0x0020,0x000e)
--> 'Study Instance UID'(0x0020,0x000d) are kept as is if already exist
                                             created  if it doesn't.
     The user is allowed to create his own Series/Studies, 
     keeping the same 'Serie Instance UID' / 'Study Instance UID' 
     for various images
     Warning :     
     The user shouldn't add any image to a 'Manufacturer Serie'
     but there is no way no to allowed him to do that 
             
--> If 'SOP Class UID' exists in the native image  ('true DICOM' image)
    we create the 'Source Image Sequence' SeqEntry (0x0008, 0x2112)
    
    --> 'Referenced SOP Class UID' (0x0008, 0x1150)
         whose value is the original 'SOP Class UID'
    --> 'Referenced SOP Instance UID' (0x0008, 0x1155)
         whose value is the original 'SOP Class UID'
    
--> Bits Stored, Bits Allocated, Hight Bit Position are checked for consistency
--> Pixel Spacing     (0x0028,0x0030) is defaulted to "1.0\1.0"
--> Samples Per Pixel (0x0028,0x0002) is defaulted to 1 (grayscale)

--> Imager Pixel Spacing (0x0018,0x1164) : defaulted to Pixel Spacing value

--> Instance Creation Date, Instance Creation Time are forced to current Date and Time

--> Study Date, Study Time are defaulted to current Date and Time
   (they remain unchanged if they exist)

--> Patient Orientation : (0x0020,0x0020), if not present, is deduced from 
    Image Orientation (Patient) : (0020|0037) or from
    Image Orientation (RET)     : (0020 0035)
   
--> Study ID, Series Number, Instance Number, Patient Orientation (Type 2)
    are created, with empty value if there are missing.

--> Manufacturer, Institution Name, Patient's Name, (Type 2)
    are defaulted with a 'gdcm' value.
    
--> Patient ID, Patient's Birth Date, Patient's Sex, (Type 2)
--> Referring Physician's Name  (Type 2)
    are created, with empty value if there are missing.

 -------------------------------------------------------------------------------------*/

void FileHelper::CheckMandatoryElements()
{
   std::string sop =  Util::CreateUniqueUID();

   // --------------------- For Meta Elements ---------------------
   // just to remember : 'official' 0002 group
   if ( WriteType != ACR && WriteType != ACR_LIBIDO )
   {
     // Group 000002 (Meta Elements) already pushed out
  
   //0002 0000 UL 1 Meta Group Length
   //0002 0001 OB 1 File Meta Information Version
   //0002 0002 UI 1 Media Storage SOP Class UID
   //0002 0003 UI 1 Media Storage SOP Instance UID
   //0002 0010 UI 1 Transfer Syntax UID
   //0002 0012 UI 1 Implementation Class UID
   //0002 0013 SH 1 Implementation Version Name
   //0002 0016 AE 1 Source Application Entity Title
   //0002 0100 UI 1 Private Information Creator
   //0002 0102 OB 1 Private Information

   // Push out 'ACR-NEMA-special' entries, if any
      Archive->Push(0x0008,0x0001); // Length to End
      Archive->Push(0x0008,0x0010); // Recognition Code
      Archive->Push(0x0028,0x0005); // Image Dimension

   // Create them if not found
   // Always modify the value
   // Push the entries to the archive.
      CopyMandatoryEntry(0x0002,0x0000,"0","UL");

      DataEntry *e_0002_0001 = CopyDataEntry(0x0002,0x0001, "OB");
      e_0002_0001->SetBinArea((uint8_t*)Util::GetFileMetaInformationVersion(),
                               false);
      e_0002_0001->SetLength(2);
      Archive->Push(e_0002_0001);
      e_0002_0001->Delete(); 

      if ( ContentType == FILTERED_IMAGE || ContentType == UNMODIFIED_PIXELS_IMAGE)
      {      
   // we keep the original 'Media Storage SOP Class UID', we default it if missing
         CheckMandatoryEntry(0x0002,0x0002,"1.2.840.10008.5.1.4.1.1.7","UI"); 
      }
      else
      {
   // It's *not* an image comming straight from a source. We force
   // 'Media Storage SOP Class UID'  --> [Secondary Capture Image Storage]
         CopyMandatoryEntry(0x0002,0x0002,"1.2.840.10008.5.1.4.1.1.7","UI");
      }

   // 'Media Storage SOP Instance UID'
      CopyMandatoryEntry(0x0002,0x0003,sop,"UI");

   // 'Implementation Class UID'
   // FIXME : in all examples we have, 0x0002,0x0012 is not so long :
   //         seems to be Root UID + 4 digits (?)
      CopyMandatoryEntry(0x0002,0x0012,Util::CreateUniqueUID(),"UI");

   // 'Implementation Version Name'
      std::string version = "GDCM ";
      version += Util::GetVersion();
      CopyMandatoryEntry(0x0002,0x0013,version,"SH");
   }

   // --------------------- For DataSet ---------------------

   // check whether 0018|0015 [CS] [Body Part Examined] value is UPPER CASE
   //      (avoid dciodvfy to complain!)
   DataEntry *e_0018_0015 = FileInternal->GetDataEntry(0x0018, 0x0015);  
   if ( e_0018_0015)
   {
      std::string bodyPartExamined = e_0018_0015->GetString();
      std::transform(bodyPartExamined.begin(), bodyPartExamined.end(), bodyPartExamined.begin(), 
                    (int(*)(int)) toupper);
      CopyMandatoryEntry(0x0018,0x0015,bodyPartExamined,"CS");       
   }

   if ( ContentType != USER_OWN_IMAGE) // when it's not a user made image
   { 
   // If 'SOP Class UID' and 'SOP Instance UID' exist ('true DICOM' image)
   // we create the 'Source Image Sequence' SeqEntry
   // to hold informations about the Source Image
 
      // 'SOP Instance UID' 
      DataEntry *e_0008_0016 = FileInternal->GetDataEntry(0x0008, 0x0016);
      //
      DataEntry *e_0008_0018 = FileInternal->GetDataEntry(0x0008, 0x0018);
      if ( e_0008_0016 && e_0008_0018)
      {
         // Create 'Source Image Sequence' SeqEntry
         SeqEntry *sis = SeqEntry::New (0x0008, 0x2112);
         SQItem *sqi = SQItem::New(1);
      
         // create 'Referenced SOP Class UID' from 'SOP Class UID'

         DataEntry *e_0008_1150 = DataEntry::New(0x0008, 0x1150, "UI");
         e_0008_1150->SetString( e_0008_0016->GetString());
         sqi->AddEntry(e_0008_1150);
         e_0008_1150->Delete();
      
         // create 'Referenced SOP Instance UID' from 'SOP Instance UID'
        // DataEntry *e_0008_0018 = FileInternal->GetDataEntry(0x0008, 0x0018);
         
         DataEntry *e_0008_1155 = DataEntry::New(0x0008, 0x1155, "UI"); 
         e_0008_1155->SetString( e_0008_0018->GetString());
         sqi->AddEntry(e_0008_1155);
         e_0008_1155->Delete();
      
         sis->AddSQItem(sqi,1);
         sqi->Delete();

         // temporarily replaces any previous 'Source Image Sequence' 
         Archive->Push(sis);
         sis->Delete();
         // FIXME : is 'Image Type' *really* depending on the presence of 'SOP Class UID'?
         
         if ( ContentType == FILTERED_IMAGE) // the user *knows* he just modified the pixels
         { 
            DataEntry *e_0008_0008 = FileInternal->GetDataEntry(0x0008, 0x0008);  
            if ( e_0008_0008)
            {
               std::string imageType = e_0008_0008->GetString();
               std::string::size_type p = imageType.find("ORIGINAL");
               if (p == 0) // image is ORIGINAL one
               {            
                 // the image is no longer an 'Original' one
                 CopyMandatoryEntry(0x0008,0x0008,"DERIVED\\PRIMARY","CS");
               }
               // if Image Type was not ORIGINAL\..., we keep it.
             }
             else // 0008_0008 was missing, wee add it.
             {
                 CopyMandatoryEntry(0x0008,0x0008,"DERIVED\\PRIMARY","CS");             
             }  
         }    
      }
   }
      
   if ( ContentType == FILTERED_IMAGE || ContentType == UNMODIFIED_PIXELS_IMAGE)
   {      
   // we keep the original 'Media Storage SOP Class UID', we default it if missing (it should be present !)
         CheckMandatoryEntry(0x0008,0x0016,"1.2.840.10008.5.1.4.1.1.7","UI");      
   }
   else
   {
   // It's *not* an image comming straight from a source. We force
   // 'Media Storage SOP Class UID'  --> [Secondary Capture Image Storage]
         CopyMandatoryEntry(0x0008,0x0016,"1.2.840.10008.5.1.4.1.1.7", "UI");      
   }
     
   Archive->Push(0x0028,0x005); // [Image Dimensions (RET)
   // Push out 'LibIDO-special' entries, if any
   Archive->Push(0x0028,0x0015);
   Archive->Push(0x0028,0x0016);
   Archive->Push(0x0028,0x0017);
   Archive->Push(0x0028,0x0198);  // very old versions
   Archive->Push(0x0028,0x0199);

   // Replace deprecated 0028 0012 US Planes   
   // by new             0028 0008 IS Number of Frames

  ///\todo : find if there is a rule!
   DataEntry *e_0028_0012 = FileInternal->GetDataEntry(0x0028, 0x0012);
   if ( e_0028_0012 )
   {
      CopyMandatoryEntry(0x0028, 0x0008,e_0028_0012->GetString(),"IS");
      Archive->Push(0x0028,0x0012);      
   }

   // Deal with the pb of (Bits Stored = 12)
   // - we're gonna write the image as Bits Stored = 16
   if ( FileInternal->GetEntryString(0x0028,0x0100) ==  "12")
   {
      CopyMandatoryEntry(0x0028,0x0100,"16","US");
   }

   // Check if user wasn't drunk ;-)

   std::ostringstream s;
   // check 'Bits Allocated' vs decent values
   int nbBitsAllocated = FileInternal->GetBitsAllocated();

   // We allow now to deal with 'non standard' 64 bits 'real' values
 
   if ( (nbBitsAllocated == 0 || nbBitsAllocated > 64) // was 32
     || ( nbBitsAllocated > 8 && nbBitsAllocated <16) )
   {
      CopyMandatoryEntry(0x0028,0x0100,"16","US");
      gdcmWarningMacro("(0028,0100) changed from "
         << nbBitsAllocated << " to 16 for consistency purpose");
      nbBitsAllocated = 16; 
   }
   // check 'Bits Stored' vs 'Bits Allocated'   
   int nbBitsStored = FileInternal->GetBitsStored();
   if ( nbBitsStored == 0 || nbBitsStored > nbBitsAllocated )
   {
      s.str("");
      s << nbBitsAllocated;
      CopyMandatoryEntry(0x0028,0x0101,s.str(),"US");
      gdcmWarningMacro("(0028,0101) changed from "
                       << nbBitsStored << " to " << nbBitsAllocated
                       << " for consistency purpose" );
      nbBitsStored = nbBitsAllocated; 
    }
   // check 'Hight Bit Position' vs 'Bits Allocated' and 'Bits Stored'
   int highBitPosition = FileInternal->GetHighBitPosition();
   if ( highBitPosition == 0 || 
        highBitPosition > nbBitsAllocated-1 ||
        highBitPosition < nbBitsStored-1  )
   {
      s.str("");
      s << nbBitsStored - 1; 
      CopyMandatoryEntry(0x0028,0x0102,s.str(),"US");
      gdcmWarningMacro("(0028,0102) changed from "
                       << highBitPosition << " to " << nbBitsAllocated-1
                       << " for consistency purpose");
   }

   // check Pixel Representation (default it as 0 -unsigned-)

   DataEntry *e_0028_0103 = FileInternal->GetDataEntry(0x0028, 0x0103);
   if ( !e_0028_0103 )
   {
      gdcmWarningMacro("PixelRepresentation (0028,0103) is supposed to be mandatory");
      CopyMandatoryEntry(0x0028, 0x0103,"0","US"); 
   }
   else
   {
      int sign = (int)e_0028_0103->GetValue(0);
      if (sign !=1 && sign !=0)
      {
         gdcmWarningMacro("PixelRepresentation (0028,0103) is supposed to be =1 or =0");
         CopyMandatoryEntry(0x0028, 0x0103,"0","US");
      }
   }

   std::string pixelAspectRatio = FileInternal->GetEntryString(0x0028,0x0034);
   if ( pixelAspectRatio == GDCM_UNFOUND ) // avoid conflict with pixelSpacing !
   {
      std::string pixelSpacing = FileInternal->GetEntryString(0x0028,0x0030);
      if ( pixelSpacing == GDCM_UNFOUND )
      {
         pixelSpacing = "1.0\\1.0";
          // if missing, Pixel Spacing forced to "1.0\1.0"
         CopyMandatoryEntry(0x0028,0x0030,pixelSpacing,"DS");
      }
  
      // 'Imager Pixel Spacing' : defaulted to 'Pixel Spacing'
      // --> This one is the *legal* one !
      if ( ContentType != USER_OWN_IMAGE)
      //  we write it only when we are *sure* the image comes from
      //         an imager (see also 0008,0x0064)
         CheckMandatoryEntry(0x0018,0x1164,pixelSpacing,"DS");
   } 
/*
///Exact meaning of RETired fields

// See page 73 of ACR-NEMA_300-1988.pdf !

// 0020,0020 : Patient Orientation :
Patient direction of the first row and
column of the images. The first entry id the direction of the raws, given by the
direction of the last pixel in the first row from the first pixel in tha row.
the second entry is the direction of the columns, given by the direction of the
last pixel in the first column from the first pixel in that column.
L : Left, F : Feet, A : Anterior, P : Posterior.
Up to 3 letters can be used in combination to indicate oblique planes.

//0020,0030 Image Position (RET)
x,y,z coordinates im mm of the first pixel in the image

// 0020,0035 Image Orientation (RET)
Direction cosines of the R axis of the image system with respect to the
equipment coordinate axes x,y,z, followed by direction cosines of the C axis of
the image system with respect to the same axes

//0020,0050 Location
An image location reference, standard for the modality (such as CT bed position),
used to indicate position. Calculation of position for other purposes
is only from (0020,0030) and (0020,0035)
*/

/*
// if imagePositionPatient    not found, default it with imagePositionRet,    if any
// if imageOrientationPatient not found, default it with imageOrientationRet, if any

   std::string imagePositionRet        = FileInternal->GetEntryString(0x0020,0x0030);
   std::string imageOrientationRet     = FileInternal->GetEntryString(0x0020,0x0035);
   std::string imagePositionPatient    = FileInternal->GetEntryString(0x0020,0x0032);
   std::string imageOrientationPatient = FileInternal->GetEntryString(0x0020,0x0037);

   if(  imagePositionPatient == GDCM_UNFOUND && imageOrientationPatient == GDCM_UNFOUND
     && imagePositionRet     != GDCM_UNFOUND && imageOrientationRet     != GDCM_UNFOUND)
   {
      CopyMandatoryEntry(0x0020, 0x0032,imagePositionRet,"DS");
      Archive->Push(0x0020,0x0030); 
      CopyMandatoryEntry(0x0020, 0x0037,imageOrientationRet,"DS");
      Archive->Push(0x0020,0x0035);
   }
*/

   // Samples Per Pixel (type 1) : default to grayscale
   CheckMandatoryEntry(0x0028,0x0002,"1","US");

   // --- Check UID-related Entries ---
 
   // At the end, not to overwrite the original ones,
   // needed by 'Referenced SOP Instance UID', 'Referenced SOP Class UID'
   // 'SOP Instance UID'  
   CopyMandatoryEntry(0x0008,0x0018,sop,"UI");

   if ( ContentType == USER_OWN_IMAGE)
   {
      gdcmDebugMacro( "USER_OWN_IMAGE (2)");
       // Conversion Type.
       // Other possible values are :
       // See PS 3.3, Page 408

       // DV = Digitized Video
       // DI = Digital Interface 
       // DF = Digitized Film
       // WSD = Workstation
       // SD = Scanned Document
       // SI = Scanned Image
       // DRW = Drawing
       // SYN = Synthetic Image

      CheckMandatoryEntry(0x0008,0x0064,"SYN","CS"); // Why not?
   } 
/*
   if ( ContentType == CREATED_IMAGE)
   {
   /// \todo : find a trick to pass the Media Storage SOP Instance UID of the images used to create the current image
   
   }
*/

   // ---- The user will never have to take any action on the following ----

   // new value for 'SOP Instance UID'
   //SetMandatoryEntry(0x0008,0x0018,Util::CreateUniqueUID());

   // Instance Creation Date
   const std::string &date = Util::GetCurrentDate();
   CopyMandatoryEntry(0x0008,0x0012,date,"DA");

   // Instance Creation Time
   const std::string &time = Util::GetCurrentTime();
   CopyMandatoryEntry(0x0008,0x0013,time,"TM");

   // Study Date
   CheckMandatoryEntry(0x0008,0x0020,date,"DA");
   // Study Time
   CheckMandatoryEntry(0x0008,0x0030,time,"TM");

   // Accession Number
   //CopyMandatoryEntry(0x0008,0x0050,"");
   CheckMandatoryEntry(0x0008,0x0050,"","SH");
   

   // ----- Add Mandatory Entries if missing ---
   // Entries whose type is 1 are mandatory, with a mandatory value
   // Entries whose type is 1c are mandatory-inside-a-Sequence,
   //                          with a mandatory value
   // Entries whose type is 2 are mandatory, with an optional value
   // Entries whose type is 2c are mandatory-inside-a-Sequence,
   //                          with an optional value
   // Entries whose type is 3 are optional

   // 'Study Instance UID'
   // Keep the value if exists
   // The user is allowed to create his own Study, 
   //          keeping the same 'Study Instance UID' for various images
   // The user may add images to a 'Manufacturer Study',
   //          adding new Series to an already existing Study 
   CheckMandatoryEntry(0x0020,0x000d,Util::CreateUniqueUID(),"UI");

   // 'Serie Instance UID'
   // Keep the value if exists
   // The user is allowed to create his own Series, 
   // keeping the same 'Serie Instance UID' for various images
   // The user shouldn't add any image to a 'Manufacturer Serie'
   // but there is no way no to prevent him for doing that 
   CheckMandatoryEntry(0x0020,0x000e,Util::CreateUniqueUID(),"UI");

   // Study ID
   CheckMandatoryEntry(0x0020,0x0010,"","SH");

   // Series Number
   CheckMandatoryEntry(0x0020,0x0011,"","IS");

   // Instance Number
   CheckMandatoryEntry(0x0020,0x0013,"","IS");

   // Patient Orientation
   // Can be computed from (0020|0037) :  Image Orientation (Patient)
   GDCM_NAME_SPACE::Orientation *o = GDCM_NAME_SPACE::Orientation::New();
   std::string ori = o->GetOrientation ( FileInternal );
   o->Delete();
   if (ori != "\\" && ori != GDCM_UNFOUND)
      CheckMandatoryEntry(0x0020,0x0020,ori,"CS");
   else
      CheckMandatoryEntry(0x0020,0x0020,"","CS");

   // Default Patient Position to HFS
   CheckMandatoryEntry(0x0018,0x5100,"HFS","CS");

   // Modality : if missing we set it to 'OTher'
   CheckMandatoryEntry(0x0008,0x0060,"OT","CS");

   // Manufacturer : if missing we set it to 'GDCM Factory'
   CheckMandatoryEntry(0x0008,0x0070,"GDCM Factory","LO");

   // Institution Name : if missing we set it to 'GDCM Hospital'
   CheckMandatoryEntry(0x0008,0x0080,"GDCM Hospital","LO");

   // Patient's Name : if missing, we set it to 'GDCM^Patient'
   CheckMandatoryEntry(0x0010,0x0010,"GDCM^Patient","PN");

   // Patient ID : some clinical softwares *demand* it although it's a 'type 2' entry.
   CheckMandatoryEntry(0x0010,0x0020,"gdcm ID","LO");

   // Patient's Birth Date : 'type 2' entry -> must exist, value not mandatory
   CheckMandatoryEntry(0x0010,0x0030,"","DA");

   // Patient's Sex :'type 2' entry -> must exist, value not mandatory
   CheckMandatoryEntry(0x0010,0x0040,"","CS");

   // Referring Physician's Name :'type 2' entry -> must exist, value not mandatory
   CheckMandatoryEntry(0x0008,0x0090,"","PN");

 /*
   // Deal with element 0x0000 (group length) of each group.
   // First stage : get all the different Groups

  GroupHT grHT;
  DocEntry *d = FileInternal->GetFirstEntry();
  while(d)
  {
    grHT[d->GetGroup()] = 0;
    d=FileInternal->GetNextEntry();
  }
  // Second stage : add the missing ones (if any)
  for (GroupHT::iterator it = grHT.begin(); it != grHT.end(); ++it)  
  {
      CheckMandatoryEntry(it->first, 0x0000, "0"); 
  }    
  // Third stage : update all 'zero level' groups length
*/


   if (PhotometricInterpretation == 1)
   {
   }
} 

void FileHelper::CheckMandatoryEntry(uint16_t group,uint16_t elem,std::string value,const VRKey &vr )
{
   DataEntry *entry = FileInternal->GetDataEntry(group,elem);
   if ( !entry )
   {
      //entry = DataEntry::New(Global::GetDicts()->GetDefaultPubDict()->GetEntry(group,elem));
      entry = DataEntry::New(group,elem,vr);
      entry->SetString(value);
      Archive->Push(entry);
      entry->Delete();
   }    
}

/// \todo : what is it used for ? (FileHelper::SetMandatoryEntry)
void FileHelper::SetMandatoryEntry(uint16_t group,uint16_t elem,std::string value,const VRKey &vr)
{
   //DataEntry *entry = DataEntry::New(Global::GetDicts()->GetDefaultPubDict()->GetEntry(group,elem));
   DataEntry *entry = DataEntry::New(group,elem,vr);
   entry->SetString(value);
   Archive->Push(entry);
   entry->Delete();
}

void FileHelper::CopyMandatoryEntry(uint16_t group,uint16_t elem,std::string value,const VRKey &vr)
{
   DataEntry *entry = CopyDataEntry(group,elem,vr);
   entry->SetString(value);
   Archive->Push(entry);
   entry->Delete();
}

/**
 * \brief Restore in the File the initial group 0002
 */
void FileHelper::RestoreWriteMandatory()
{
   // group 0002 may be pushed out for ACR-NEMA writting purposes 
   Archive->Restore(0x0002,0x0000);
   Archive->Restore(0x0002,0x0001);
   Archive->Restore(0x0002,0x0002);
   Archive->Restore(0x0002,0x0003);
   Archive->Restore(0x0002,0x0010);
   Archive->Restore(0x0002,0x0012);
   Archive->Restore(0x0002,0x0013);
   Archive->Restore(0x0002,0x0016);
   Archive->Restore(0x0002,0x0100);
   Archive->Restore(0x0002,0x0102);

   // FIXME : Check if none is missing !
   
   Archive->Restore(0x0008,0x0012);
   Archive->Restore(0x0008,0x0013);
   Archive->Restore(0x0008,0x0016);
   Archive->Restore(0x0008,0x0018);
   Archive->Restore(0x0008,0x0060);
   Archive->Restore(0x0008,0x0070);
   Archive->Restore(0x0008,0x0080);
   Archive->Restore(0x0008,0x0090);
   Archive->Restore(0x0008,0x2112);

   Archive->Restore(0x0010,0x0010);
   Archive->Restore(0x0010,0x0030);
   Archive->Restore(0x0010,0x0040);

   Archive->Restore(0x0020,0x000d);
   Archive->Restore(0x0020,0x000e);
}

/**
 * \brief   CallStartMethod
 */
void FileHelper::CallStartMethod()
{
   Progress = 0.0f;
   Abort    = false;
   CommandManager::ExecuteCommand(this,CMD_STARTPROGRESS);
}

/**
 * \brief   CallProgressMethod
 */
void FileHelper::CallProgressMethod()
{
   CommandManager::ExecuteCommand(this,CMD_PROGRESS);
}

/**
 * \brief   CallEndMethod
 */
void FileHelper::CallEndMethod()
{
   Progress = 1.0f;
   CommandManager::ExecuteCommand(this,CMD_ENDPROGRESS);
}

//-----------------------------------------------------------------------------
// Private
/**
 * \brief Factorization for various forms of constructors.
 */
void FileHelper::Initialize()
{
   UserFunction = 0;
   ContentType = USER_OWN_IMAGE;

   WriteMode = WMODE_RAW;
   WriteType = ExplicitVR;
   
   PhotometricInterpretation = 2; // Black = 0

   PixelReadConverter  = new PixelReadConvert;
   PixelWriteConverter = new PixelWriteConvert;
   Archive = new DocEntryArchive( FileInternal );
   
   KeepOverlays = false;
}

/**
 * \brief Reads/[decompresses] the pixels, 
 *        *without* making RGB from Palette Colors 
 * @return the pixels area, whatever its type 
 *         (uint8_t is just for prototyping : feel free to Cast it) 
 */ 
uint8_t *FileHelper::GetRaw()
{
   PixelReadConverter->SetUserFunction( UserFunction );

   uint8_t *raw = PixelReadConverter->GetRaw();
   if ( ! raw )
   {
      // The Raw image migth not be loaded yet:
      std::ifstream *fp = FileInternal->OpenFile();
      PixelReadConverter->ReadAndDecompressPixelData( fp );
      if ( fp ) 
         FileInternal->CloseFile();

      raw = PixelReadConverter->GetRaw();
      if ( ! raw )
      {
         gdcmWarningMacro( "Read/decompress of pixel data apparently went wrong.");
         return 0;
      }
   }
   return raw;
}

/**
 * \brief Deal with Grey levels i.e. re-arange them
 *        to have low values = dark, high values = bright
 */
void FileHelper::ConvertFixGreyLevels(uint8_t *raw, size_t rawSize)
{
   uint32_t i; // to please M$VC6
   int16_t j;

   // Number of Bits Allocated for storing a Pixel is defaulted to 16
   // when absent from the file.
   int bitsAllocated = FileInternal->GetBitsAllocated();
   if ( bitsAllocated == 0 )
   {
      bitsAllocated = 16;
   }

   else if (bitsAllocated > 8 && bitsAllocated < 16 && bitsAllocated != 12)
   {
      bitsAllocated = 16;
   }   
   // Number of "Bits Stored", defaulted to number of "Bits Allocated"
   // when absent from the file.
   int bitsStored = FileInternal->GetBitsStored();
   if ( bitsStored == 0 )
   {
      bitsStored = bitsAllocated;
   }

   if (!FileInternal->IsSignedPixelData())
   {
      if ( bitsAllocated == 8 )
      {
         uint8_t *deb = (uint8_t *)raw;
         for (i=0; i<rawSize; i++)      
         {
            *deb = 255 - *deb;
            deb++;
         }
         return;
      }

      if ( bitsAllocated == 16 )
      {
         uint16_t mask =1;
         for (j=0; j<bitsStored-1; j++)
         {
            mask = (mask << 1) +1; // will be fff when BitsStored=12
         }

         uint16_t *deb = (uint16_t *)raw;
         for (i=0; i<rawSize/2; i++)      
         {
            *deb = mask - *deb;
            deb++;
         }
         return;
       }
   }
   else
   {
      if ( bitsAllocated == 8 )
      {
         uint8_t smask8 = 255;
         uint8_t *deb = (uint8_t *)raw;
         for (i=0; i<rawSize; i++)      
         {
            *deb = smask8 - *deb;
            deb++;
         }
         return;
      }
      if ( bitsAllocated == 16 )
      {
         uint16_t smask16 = 65535;
         uint16_t *deb = (uint16_t *)raw;
         for (i=0; i<rawSize/2; i++)      
         {
            *deb = smask16 - *deb;
            deb++;
         }
         return;
      }
   }
}

//-----------------------------------------------------------------------------
/**
 * \brief   Prints the FileInternal + info on PixelReadConvertor
 * @param   os ostream we want to print in
 * @param indent (unused)
 */
void FileHelper::Print(std::ostream &os, std::string const &)
{
   FileInternal->SetPrintLevel(PrintLevel);
   FileInternal->Print(os);

   if ( FileInternal->IsReadable() )
   {
      PixelReadConverter->SetPrintLevel(PrintLevel);
      PixelReadConverter->Print(os);
   }
}

//-----------------------------------------------------------------------------
} // end namespace gdcm


/* Probabely something to be added to use Rescale Slope/Intercept
Have a look at ITK code !

// Internal function to rescale pixel according to Rescale Slope/Intercept
template<class TBuffer, class TSource>
void RescaleFunction(TBuffer* buffer, TSource *source,
                     double slope, double intercept, size_t size)
{
  size /= sizeof(TSource);

  if (slope != 1.0 && intercept != 0.0)
    {
    // Duff's device.  Instead of this code:
    //
    //   for(unsigned int i=0; i<size; i++)
    //    {
    //    buffer[i] = (TBuffer)(source[i]*slope + intercept);
    //    }
    //
    // use Duff's device which exploits "fall through"
    register size_t n = (size + 7) / 8;
    switch ( size % 8)
      {
      case 0: do { *buffer++ = (TBuffer)((*source++)*slope + intercept);
      case 7:      *buffer++ = (TBuffer)((*source++)*slope + intercept);
      case 6:      *buffer++ = (TBuffer)((*source++)*slope + intercept);
      case 5:      *buffer++ = (TBuffer)((*source++)*slope + intercept);
      case 4:      *buffer++ = (TBuffer)((*source++)*slope + intercept);
      case 3:      *buffer++ = (TBuffer)((*source++)*slope + intercept);
      case 2:      *buffer++ = (TBuffer)((*source++)*slope + intercept);
      case 1:      *buffer++ = (TBuffer)((*source++)*slope + intercept);
                 }  while (--n > 0);
      }
    }
  else if (slope == 1.0 && intercept != 0.0)
    {
    // Duff's device.  Instead of this code:
    //
    //   for(unsigned int i=0; i<size; i++)
    //    {
    //    buffer[i] = (TBuffer)(source[i] + intercept);
    //    }
    //
    // use Duff's device which exploits "fall through"
    register size_t n = (size + 7) / 8;
    switch ( size % 8)
      {
      case 0: do { *buffer++ = (TBuffer)(*source++ + intercept);
      case 7:      *buffer++ = (TBuffer)(*source++ + intercept);
      case 6:      *buffer++ = (TBuffer)(*source++ + intercept);
      case 5:      *buffer++ = (TBuffer)(*source++ + intercept);
      case 4:      *buffer++ = (TBuffer)(*source++ + intercept);
      case 3:      *buffer++ = (TBuffer)(*source++ + intercept);
      case 2:      *buffer++ = (TBuffer)(*source++ + intercept);
      case 1:      *buffer++ = (TBuffer)(*source++ + intercept);
                 }  while (--n > 0);
      }
    }
  else if (slope != 1.0 && intercept == 0.0)
    {
    // Duff's device.  Instead of this code:
    //
    //   for(unsigned int i=0; i<size; i++)
    //    {
    //    buffer[i] = (TBuffer)(source[i]*slope);
    //    }
    //
    // use Duff's device which exploits "fall through"
    register size_t n = (size + 7) / 8;
    switch ( size % 8)
      {
      case 0: do { *buffer++ = (TBuffer)((*source++)*slope);
      case 7:      *buffer++ = (TBuffer)((*source++)*slope);
      case 6:      *buffer++ = (TBuffer)((*source++)*slope);
      case 5:      *buffer++ = (TBuffer)((*source++)*slope);
      case 4:      *buffer++ = (TBuffer)((*source++)*slope);
      case 3:      *buffer++ = (TBuffer)((*source++)*slope);
      case 2:      *buffer++ = (TBuffer)((*source++)*slope);
      case 1:      *buffer++ = (TBuffer)((*source++)*slope);
                 }  while (--n > 0);
      }
    }
  else
    {
    // Duff's device.  Instead of this code:
    //
    //   for(unsigned int i=0; i<size; i++)
    //    {
    //    buffer[i] = (TBuffer)(source[i]);
    //    }
    //
    // use Duff's device which exploits "fall through"
    register size_t n = (size + 7) / 8;
    switch ( size % 8)
      {
      case 0: do { *buffer++ = (TBuffer)(*source++);
      case 7:      *buffer++ = (TBuffer)(*source++);
      case 6:      *buffer++ = (TBuffer)(*source++);
      case 5:      *buffer++ = (TBuffer)(*source++);
      case 4:      *buffer++ = (TBuffer)(*source++);
      case 3:      *buffer++ = (TBuffer)(*source++);
      case 2:      *buffer++ = (TBuffer)(*source++);
      case 1:      *buffer++ = (TBuffer)(*source++);
                 }  while (--n > 0);
      }
   }   
}


template<class TSource>
void RescaleFunction(ImageIOBase::IOComponentType bufferType,
                     void* buffer, TSource *source,
                     double slope, double intercept, size_t size)
{
  switch (bufferType)
    {
    case ImageIOBase::UCHAR:
      RescaleFunction( (unsigned char *)buffer, source, slope, intercept, size);
      break;
    case ImageIOBase::CHAR:
      RescaleFunction( (char *)buffer, source, slope, intercept, size);
      break;
    case ImageIOBase::USHORT:
      RescaleFunction( (unsigned short *)buffer, source, slope, intercept,size);
      break;
    case ImageIOBase::SHORT:
      RescaleFunction( (short *)buffer, source, slope, intercept, size);
      break;
    case ImageIOBase::UINT:
      RescaleFunction( (unsigned int *)buffer, source, slope, intercept, size);
      break;
    case ImageIOBase::INT:
      RescaleFunction( (int *)buffer, source, slope, intercept, size);
      break;
    case ImageIOBase::FLOAT:
      RescaleFunction( (float *)buffer, source, slope, intercept, size);
      break;
    case ImageIOBase::DOUBLE:
      RescaleFunction( (double *)buffer, source, slope, intercept, size);
      break;
    default:
      ::itk::OStringStream message;
      message << "itk::ERROR: GDCMImageIO: Unknown component type : " << bufferType;
      ::itk::ExceptionObject e(__FILE__, __LINE__, message.str().c_str(),ITK_LOCATION);
      throw e;
    }
}
*/
