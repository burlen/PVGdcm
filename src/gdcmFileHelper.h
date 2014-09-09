/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmFileHelper.h,v $
  Language:  C++
  Date:      $Date: 2007/09/17 12:16:01 $
  Version:   $Revision: 1.55 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMFILEHELPER_H_
#define _GDCMFILEHELPER_H_

#include "gdcmDebug.h"
#include "gdcmRefCounter.h"
#include "gdcmVRKey.h"
#include "gdcmFile.h"

namespace GDCM_NAME_SPACE
{
//class File;
class DataEntry;
class SeqEntry;
class PixelReadConvert;
class PixelWriteConvert;
class DocEntryArchive;

typedef void (*VOID_FUNCTION_PUINT8_PFILE_POINTER)(uint8_t *, File *);

//-----------------------------------------------------------------------------
/**
 * \brief In addition to Dicom header exploration, this class is designed
 * for accessing the image/volume content. One can also use it to
 * write Dicom/ACR-NEMA/RAW files.
 */
class GDCM_EXPORT FileHelper : public RefCounter
{
   gdcmTypeMacro(FileHelper);

public:
   enum FileMode
   {
      WMODE_RAW,
      WMODE_RGB
   };

/// \brief Constructs a FileHelper with a RefCounter
   static FileHelper *New() {return new FileHelper();}
/// \brief Constructs a FileHelper with a RefCounter from a fileHelper  
   static FileHelper *New(File *header) {return new FileHelper(header);}

   void Print(std::ostream &os = std::cout, std::string const &indent = ""); 

   /// Accessor to File
   File *GetFile() { return FileInternal; }
   
   /// \brief Tells gdcm wether we want to keep ACR-NEMA-like overlays or not.  
   void SetKeepOverlays(bool k) { KeepOverlays =k; }
   bool GetKeepOverlays( )      { return KeepOverlays; }
     
   void SetLoadMode(int loadMode);
   void SetFileName(std::string const &fileName);
   bool Load();
   /// to allow user to modify pixel order (e.g. Mirror, UpsideDown,...)
   void SetUserFunction( VOID_FUNCTION_PUINT8_PFILE_POINTER userFunc ) 
                        { UserFunction = userFunc; }   
   // File methods
   bool SetEntryString(std::string const &content,
                           uint16_t group, uint16_t elem);
   bool SetEntryBinArea(uint8_t *content, int lgth,
                            uint16_t group, uint16_t elem);

   DataEntry *InsertEntryString(std::string const &content,
                                uint16_t group, uint16_t elem, const VRKey &vr);
   DataEntry *InsertEntryBinArea(uint8_t *binArea, int lgth,
                                uint16_t group, uint16_t elem, const VRKey &vr);
   SeqEntry *InsertSeqEntry(uint16_t group, uint16_t elem);

   // File helpers
   size_t GetImageDataSize();
   size_t GetImageDataRawSize();

   uint8_t *GetImageData();
   uint8_t *GetImageDataRaw();

// GDCM_LEGACY(size_t GetImageDataIntoVector(void *destination,size_t maxSize))

   void SetImageData(uint8_t *data, size_t expectedSize);

   // User data
   void SetUserData(uint8_t *data, size_t expectedSize);
   uint8_t *GetUserData();
   size_t GetUserDataSize();
   // RBG data (from file)
   uint8_t *GetRGBData();
   size_t GetRGBDataSize();
   // RAW data (from file)
   uint8_t *GetRawData();
   size_t GetRawDataSize();

   void ConvertFixGreyLevels(uint8_t *data, size_t size);

   // LUT
   uint8_t* GetLutRGBA();
   int GetLutItemNumber();
   int GetLutItemSize();

   // Write mode

   /// \brief Tells the writer we want to keep 'Grey pixels + Palettes color'
   ///        when possible (as opposed to convert 'Palettes color' to RGB)
   void SetWriteModeToRaw()           { SetWriteMode(WMODE_RAW);  }
   /// \brief Tells the writer we want to write RGB image when possible
   ///        (as opposed to 'Grey pixels + Palettes color')
   void SetWriteModeToRGB()           { SetWriteMode(WMODE_RGB);  }
   /// \brief Sets the Write Mode ( )
   void SetWriteMode(FileMode mode)   { WriteMode = mode;         }
   /// \brief Gets the Write Mode ( )
   FileMode GetWriteMode()            { return WriteMode;         }

   // Write format

   /// \brief Tells the writer we want to write as Implicit VR
   void SetWriteTypeToDcmImplVR()     { SetWriteType(ImplicitVR); }
   /// \brief Tells the writer we want to write as Explicit VR
   void SetWriteTypeToDcmExplVR()     { SetWriteType(ExplicitVR); }
   /// \brief Tells the writer we want to write as ACR-NEMA
   void SetWriteTypeToAcr()           { SetWriteType(ACR);        }
   /// \brief Tells the writer we want to write as LibIDO
   void SetWriteTypeToAcrLibido()     { SetWriteType(ACR_LIBIDO); }
   /// \brief Tells the writer we want to write as JPEG
   void SetWriteTypeToJPEG()          { SetWriteType(JPEG);       }
   /// \brief Tells the writer we want to write as JPEG2000
   void SetWriteTypeToJPEG2000()      { SetWriteType(JPEG2000);   }
   /// \brief Tells the writer which format we want to write
   /// (ImplicitVR, ExplicitVR, ACR, ACR_LIBIDO)
   void SetWriteType(FileType format) { WriteType = format;       }
   /// \brief Gets the format we talled the write we wanted to write
   /// (ImplicitVR, ExplicitVR, ACR, ACR_LIBIDO)
   FileType GetWriteType()            { return WriteType;         }
   /// \brief 1 : white=0, black=high value
   void SetPhotometricInterpretationToMonochrome1() {
                                    PhotometricInterpretation = 1;}
   /// \brief 2 : black=0, white=high value  (default)
   void SetPhotometricInterpretationToMonochrome2() {
                                    PhotometricInterpretation = 2;}
   /// \brief 1 : white=0, black=high value
   int GetPhotometricInterpretation() {
                                return PhotometricInterpretation; }

   // Write pixels of ONE image on hard drive
   // No test is made on processor "endianness"
   // The user must call his reader correctly
   bool WriteRawData  (std::string const &fileName);
   bool WriteDcmImplVR(std::string const &fileName);
   bool WriteDcmExplVR(std::string const &fileName);
   bool WriteAcr      (std::string const &fileName);
   bool Write         (std::string const &fileName);
   
/// \brief We have to deal with 4 *very* different cases :
/// -1) user created ex nihilo his own image and wants to write it as a Dicom image.
///    USER_OWN_IMAGE
/// -2) user modified the pixels of an existing image.
///    FILTERED_IMAGE
/// -3) user created a new image, using existing images (eg MIP, MPR, cartography image)
///   CREATED_IMAGE
/// -4) user modified/added some tags *without processing* the pixels (anonymization...
///   UNMODIFIED_PIXELS_IMAGE

   void SetContentType (ImageContentType c) { ContentType = c; }
   // no GetContentType() method, on purpose!
   
   void CallStartMethod();
   void CallProgressMethod();
   void CallEndMethod();
   
protected:
   FileHelper( );
   FileHelper( File *header );
   ~FileHelper();

   /// \todo move all those 'protected' methods to 'private'
   ///       since FileHelper is not derived in anything!
   bool CheckWriteIntegrity();

   void SetWriteToRaw();
   void SetWriteToRGB();
   void RestoreWrite();

   void SetWriteFileTypeToACR();
   void SetWriteFileTypeToJPEG();
   void SetWriteFileTypeToJPEG2000();
   void SetWriteFileTypeToExplicitVR();
   void SetWriteFileTypeToImplicitVR();
   void RestoreWriteFileType();

   void SetWriteToLibido();
   void SetWriteToNoLibido();
   void RestoreWriteOfLibido();

   DataEntry *CopyDataEntry(uint16_t group, uint16_t elem, 
                            const VRKey &vr = GDCM_VRUNKNOWN);
   void CheckMandatoryElements();
   void CheckMandatoryEntry(uint16_t group, uint16_t elem, std::string value,
                            const VRKey &vr = GDCM_VRUNKNOWN);
   void SetMandatoryEntry(uint16_t group, uint16_t elem, std::string value,
                          const VRKey &vr = GDCM_VRUNKNOWN);
   void CopyMandatoryEntry(uint16_t group, uint16_t elem, std::string value,
                           const VRKey &vr = GDCM_VRUNKNOWN);
   void RestoreWriteMandatory();

private:
   void Initialize();

   uint8_t *GetRaw();

// members variables:
protected:
   /// value of the ??? for any progress bar
   float Progress;
   mutable bool Abort;
   
private:

   /// gdcm::File to use to load the file
   File *FileInternal;

   /// Whether already parsed or not
   bool Parsed;

   // Utility pixel converter
   
   /// \brief Pointer to the PixelReadConverter
   PixelReadConvert *PixelReadConverter;
   
   /// \brief Pointer to the PixelWriteConverter
   PixelWriteConvert *PixelWriteConverter;

   // Utility header archive
   /// \brief Pointer to the DocEntryArchive (used while writting process)
   DocEntryArchive *Archive;

   // Write variables
   /// \brief (WMODE_RAW, WMODE_RGB)
   FileMode WriteMode;

   /// \brief (ImplicitVR, ExplicitVR, ACR, ACR_LIBIDO)
   FileType WriteType;

   /// \brief Pointer to a user supplied function to allow modification 
   /// of pixel order (e.g. : Mirror, UpsideDown, 90°Rotation, ...)
   /// use as : void userSuppliedFunction(uint8_t *im, gdcm::File *f)
   /// NB : the "uint8_t *" type of first param is just for prototyping.
   /// User will Cast it according what he founds with f->GetPixelType()
   /// See vtkgdcmSerieViewer for an example
   VOID_FUNCTION_PUINT8_PFILE_POINTER UserFunction;
   
   /// \brief only user knows what he did before asking the image to be written
   /// - he created ex nihilo his own image
   /// - he just applied a mathematical process on the pixels
   /// - he created a new image, using existing images (eg MIP, MPR,cartography)
   /// - he anonymized and image (*no* modif on the pixels)
   ImageContentType ContentType;
 
   /// \brief  1 : white=0, black=high value
   ///         2 : black=0, white=high value (default)
   int PhotometricInterpretation;
 
   /// \brief wether we want to keep ACR-NEMA-like overlays or not.
   bool KeepOverlays;   

};
} // end namespace gdcm

//-----------------------------------------------------------------------------
#endif
