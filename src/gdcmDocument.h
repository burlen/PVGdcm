/*=========================================================================
 
  Program:   gdcm
  Module:    $RCSfile: gdcmDocument.h,v $
  Language:  C++
  Date:      $Date: 2008/09/15 15:49:22 $
  Version:   $Revision: 1.154 $
 
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
 
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef _GDCMDOCUMENT_H_
#define _GDCMDOCUMENT_H_

#include "gdcmVR.h"
#include "gdcmDict.h"
#include "gdcmElementSet.h"
#include "gdcmException.h"
#include "gdcmDebug.h"  // for LEGACY
#include "gdcmCommandManager.h"

#include <map>
#include <list>
#include <fstream>

namespace GDCM_NAME_SPACE 
{
class SeqEntry;
class Dict;

//-----------------------------------------------------------------------------
/**
 * \brief Derived by both GDCM_NAME_SPACE::File and GDCM_NAME_SPACE::DicomDir
 */
class GDCM_EXPORT Document : public ElementSet
{
   gdcmTypeMacro(Document);

public:
   typedef std::list<DicomElement> ListElements;

// Loading
   //Deprecated : use SetFileName() + Load()
//#ifndef GDCM_LEGACY_REMOVE 
//   virtual bool Load( std::string const &filename ); 
//#endif
   virtual bool Load( ); 

// Dictionaries
   Dict *GetPubDict();
   Dict *GetShaDict();
   bool SetShaDict(Dict *dict);
   bool SetShaDict(DictKey const &dictName);

// Informations contained in the GDCM_NAME_SPACE::Document
   bool IsParsable();
   virtual bool IsReadable();
   bool IsDicomV3();
   bool IsPapyrus();
   FileType GetFileType();
   std::string GetTransferSyntax();
   /// Return the Transfer Syntax as a string
   std::string GetTransferSyntaxName();

// Swap code
   /// 'Swap code' accessor (see  SwapCode )
   int GetSwapCode() { return SwapCode; }
   
// File I/O
   /// Accessor to  Filename
   const std::string &GetFileName() const { return Filename; }
   /// Accessor to  Filename
   virtual void SetFileName(std::string const &fileName) 
                   { if (Filename != fileName)
                        Filename = fileName, IsDocumentModified = true; }

   std::ifstream *OpenFile();
   bool CloseFile();
   void WriteContent( std::ofstream *fp, FileType type, bool insideMetaElements, bool insideSequence );

// Data entries
   virtual void LoadEntryBinArea(uint16_t group, uint16_t elem);
   virtual void LoadEntryBinArea(DataEntry *entry);

   void SetMaxSizeLoadEntry(long);
   void AddForceLoadElement(uint16_t group, uint16_t elem);

// Ordering of Documents
   bool operator<(Document &document);

/**
 * \brief Sets the LoadMode as a boolean string.
 *        LD_NOSEQ, LD_NOSHADOW, LD_NOSHADOWSEQ
 ... (nothing more, right now)
 *        WARNING : before using NO_SHADOW, be sure *all* your files
 *        contain accurate values in the 0x0000 element (if any)
 *        of *each* Shadow Group. The parser will fail if the size is wrong !
 * @param   mode Load mode to be used
 */
   void SetLoadMode (int mode) { if (LoadMode != mode)
                                     LoadMode=mode, IsDocumentModified = true; }

protected:
// Methods
   // Constructor and destructor are protected to forbid end user
   // to instanciate from this class Document (only GDCM_NAME_SPACE::File and
   // GDCM_NAME_SPACE::DicomDir are meaningfull).
   Document();
   virtual ~Document();

   virtual void CallStartMethod();
   virtual void CallProgressMethod();
   virtual void CallEndMethod();
      
   uint16_t ReadInt16() throw ( FormatError );
   uint32_t ReadInt32() throw ( FormatError );

   /// \brief skips bytes inside the source file
   void     SkipBytes(uint32_t nBytes) { Fp->seekg((long)nBytes, std::ios::cur);}
   int ComputeGroup0002Length( );

// Variables
   /// Refering underlying filename.
   std::string Filename;

   /// \brief Swap code gives an information on the byte order of a 
   ///  supposed to be an int32, as it's read on disc 
   /// (depending on the image Transfer Syntax *and* on the processor endianess)
   /// as opposed as it should in memory to be dealt as an int32.
   /// For instance :
   /// - a 'Little Endian' image, read with a little endian processor
   /// will have a SwapCode= 1234 (the order is OK; nothing to do)
   /// - a 'Little Endian' image, read with a big endian procesor
   /// will have a SwapCode= 4321 (the order is wrong; int32 an int16 must be
   /// swapped)
   /// note : values 2143, 4321, 3412 remain for the ACR-NEMA time, and
   /// the well known 'Bad Big Endian' and 'Bad Little Endian' codes
   int SwapCode;

   ///\brief whether we already parsed group 0002 (Meta Elements)
   bool Group0002Parsed;

   ///\brief whether file has a DCM Preamble
   bool HasDCMPreamble;

   /// File Pointer, opened during Document parsing.
   std::ifstream *Fp;

   /// ACR, ACR_LIBIDO, ExplicitVR, ImplicitVR, Unknown
   FileType Filetype;  

   /// After opening the file, we read HEADER_LENGTH_TO_READ bytes.
   static const unsigned int HEADER_LENGTH_TO_READ; 
   /// \brief Elements whose value is longer than MAX_SIZE_LOAD_ELEMENT_VALUE
   /// are NOT loaded.
   static const unsigned int MAX_SIZE_LOAD_ELEMENT_VALUE;

   /// User supplied list of elements to Anonymize
   ListElements UserAnonymizeList;

   /// User supplied list of elements to force Load
   ListElements UserForceLoadList;

   /// \brief Bit string integer (each one considered as a boolean)
   ///        Bit 0 : Skip Sequences,    if possible
   ///        Bit 1 : Skip Shadow Groups if possible
   ///        Probabely, some more to add
   int LoadMode;
   
   /// \brief Whether the GDCM_NAME_SPACE::Document is already parsed/loaded :
   /// False from the creation of the GDCM_NAME_SPACE::Document untill 
   ///   GDCM_NAME_SPACE::Document:Load()
   bool IsDocumentAlreadyLoaded; // FIXME : probabely useless now

   /// Whether the GDCM_NAME_SPACE::Document was modified since the last Load()
   bool IsDocumentModified;

private:
// Methods
   void Initialize();
   bool DoTheLoadingDocumentJob();
     
      // System access (meaning endian related !?)
   void ReadBegBuffer(size_t l) throw ( FormatError );
   uint16_t SwapShort(uint16_t);
   uint32_t SwapLong(uint32_t);
   double SwapDouble(double);
   /// \brief  Unswaps back the bytes of 2-bytes long integer 
   ///         so they agree with the processor order.
   uint16_t UnswapShort(uint16_t a) { return SwapShort(a);}
   /// \brief  Unswaps back the bytes of 4-byte long integer 
   ///         so they agree with the processor order.
   uint32_t UnswapLong(uint32_t a) { return SwapLong(a);}
   
   // Read
   void ParseDES(DocEntrySet *set, long offset, long l_max, bool delim_mode);
   bool ParseSQ (SeqEntry *seq,    long offset, long l_max, bool delim_mode);

   void LoadDocEntry         (DocEntry *e, bool forceLoad = false);
   void FindDocEntryLength   (DocEntry *e) throw ( FormatError );
   uint32_t FindDocEntryLengthOBOrOW() throw( FormatUnexpected );
   VRKey FindDocEntryVR();
   bool CheckDocEntryVR      (const VRKey &k);

   void SkipDocEntry          (DocEntry *entry);
   void SkipToNextDocEntry    (DocEntry *entry);

   void FixDocEntryFoundLength(DocEntry *entry, uint32_t l);
   bool IsDocEntryAnInteger   (DocEntry *entry);

   bool CheckSwap();
   void SwitchByteSwapCode();

   // DocEntry related utilities
   DocEntry *ReadNextDocEntry();
   uint16_t GetInt16();
   uint32_t GetInt32();

   void HandleBrokenEndian  (uint16_t &group, uint16_t &elem);
   void HandleOutOfGroup0002(uint16_t &group, uint16_t &elem);
   DocEntry *Backtrack(DocEntry *docEntry, DocEntrySet *set);

// Variables
protected:
   /// value of the ??? for any progress bar
   float Progress;
   mutable bool Abort;
   
   /// Public dictionary used to parse this header
   Dict *RefPubDict;
   /// \brief Optional "shadow dictionary" (private elements) used to parse
   /// this header
   Dict *RefShaDict;

   /// \brief Size threshold above which an element value will NOT be loaded
   /// in memory (to avoid loading the image/volume itself). By default,
   /// this upper bound is fixed to 1024 bytes (which might look reasonable
   /// when one considers the definition of the various VR contents).
   uint32_t MaxSizeLoadEntry;

   /// \brief to allow any inner method to know current tag Group number 
   uint16_t CurrentGroup;
   /// \brief to allow any inner method to know current tag Element number 
   uint16_t CurrentElem; 
     
//  uint32_t GenerateFreeTagKeyInGroup(uint16_t group);
//  void BuildFlatHashTableRecurse( TagDocEntryHT &builtHT,
//                                  DocEntrySet *set );

private:
   /// \brief buffer to avoid some freads
   char BegBuffer[8];
   char *PtrBegBuffer;
   /// \brief to avoid time consuming ftellg
   size_t CurrentOffsetPosition;
   /// \brief to indicate if last supposed to be UN DataElement is not
   ///        (according to a private Dicom dictionary) 
   bool changeFromUN;
   /// \brief whether an unexpected EOF was encountered
   bool UnexpectedEOF;
   /// \brief to avoid infinite loop when illegal UN stands for OB
   size_t OffsetOfPreviousParseDES;
};

} // end namespace gdcm

//-----------------------------------------------------------------------------
#endif
