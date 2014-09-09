/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDocument.cxx,v $
  Language:  C++
  Date:      $Date: 2010/07/09 09:20:21 $
  Version:   $Revision: 1.385 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#include "gdcmDocument.h"
#include "gdcmSeqEntry.h"
#include "gdcmGlobal.h"
#include "gdcmUtil.h"
#include "gdcmDebug.h"
#include "gdcmTS.h"
#include "gdcmDictSet.h"
#include "gdcmDocEntrySet.h"
#include "gdcmSQItem.h"
#include "gdcmDataEntry.h"

#include <vector>
#include <iomanip>
#include <fstream>
#include <ctype.h>  // for isdigit
#include <stdlib.h> // for atoi

#if defined(__BORLANDC__)
   #include <mem.h> // for memset
#endif 

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------

// Refer to Document::SetMaxSizeLoadEntry()
const unsigned int Document::MAX_SIZE_LOAD_ELEMENT_VALUE = 0xfff; // 4096

//-----------------------------------------------------------------------------
// Constructor / Destructor
// Constructors and destructors are protected to avoid user to invoke directly

/**
 * \brief This default constructor neither loads nor parses the file. 
 *        You should then invoke Document::Load.
 *         
 */
Document::Document() 
         :ElementSet()
{
   Fp = 0;

   SetMaxSizeLoadEntry(MAX_SIZE_LOAD_ELEMENT_VALUE);
   Initialize();
   SwapCode = 1234;
   Filetype = ExplicitVR;
   CurrentOffsetPosition = 0;
   OffsetOfPreviousParseDES =0;
   // Load will set it to true if sucessfull
   Group0002Parsed = false;
   IsDocumentAlreadyLoaded = false;
   IsDocumentModified = true;
   LoadMode = LD_ALL; // default : load everything, later
   
   SetFileName("");
   changeFromUN=false;
   UnexpectedEOF=false;
}

/**
 * \brief   Canonical destructor.
 */
Document::~Document ()
{
   CloseFile();
}

//-----------------------------------------------------------------------------
// Public
/**
 * \brief   Loader. use SetLoadMode(), SetFileName() before ! 
 * @return false if file cannot be open or no swap info was found,
 *         or no tag was found.
 */
bool Document::Load(  ) 
{
   if ( GetFileName() == "" )
   {
      gdcmWarningMacro( "Use SetFileName, before !" );
      return false;
   }
   return DoTheLoadingDocumentJob( );
}


//#ifndef GDCM_LEGACY_REMOVE
/**
 * \brief   Loader. (DEPRECATED : not to break the API)   
 * @param   fileName 'Document' (File or DicomDir) to be open for parsing
 * @return false if file cannot be open or no swap info was found,
 *         or no tag was found.
 */
 /*
bool Document::Load( std::string const &fileName ) 
{
   Filename = fileName;
   return DoTheLoadingDocumentJob( );
}
*/
//#endif

/**
 * \brief   Performs the Loading Job (internal use only)  
 * @return false if file cannot be open or no swap info was found,
 *         or no tag was found.
 */
bool Document::DoTheLoadingDocumentJob(  ) 
{
   if ( ! IsDocumentModified ) // Nothing to do !
      return true;

   ClearEntry();

   Fp = 0;
   if ( !OpenFile() )
   {
      // warning already performed in OpenFile()
      Filetype = Unknown;
      return false;
   }

   Group0002Parsed = false;

   gdcmDebugMacro( "Starting parsing of file: " << Filename.c_str());

   // Computes the total length of the file
   Fp->seekg(0, std::ios::end);  // Once per Document !
   long lgt = Fp->tellg();       // Once per Document !   
   Fp->seekg(0, std::ios::beg);  // Once per Document !

   // CheckSwap returns a boolean 
   // (false if no swap info of any kind was found)
   if (! CheckSwap() )
   {
      gdcmWarningMacro( "Neither a DICOM V3 nor an ACR-NEMA file: " 
                   << Filename.c_str());
      CloseFile(); 
      return false;      
    }

   long beg = Fp->tellg();      // just after DICOM preamble (if any)

   lgt -= beg;                  // remaining length to parse    

   // Recursive call.
   // Loading is done during parsing
   OffsetOfPreviousParseDES = beg; 
   ParseDES( this, beg, lgt, false); // delim_mode is first defaulted to false

   if ( IsEmpty() )
   { 
      gdcmErrorMacro( "No tag in internal hash table for: "
                        << Filename.c_str());
      CloseFile(); 
      return false;
   }
   IsDocumentAlreadyLoaded = true;

   //Fp->seekg(0, std::ios::beg);  // Once per Document!
   
   // Load 'non string' values
      
   std::string PhotometricInterpretation = GetEntryString(0x0028,0x0004);   
   if ( PhotometricInterpretation == "PALETTE COLOR " )
   {
   // FIXME
   // Probabely this line should be outside the 'if'
   // Try to find an image sample holding a 'gray LUT'
      LoadEntryBinArea(0x0028,0x1200);  // gray LUT
   
      /// FIXME
      /// --> FIXME : The difference between BinEntry and DataEntry
      /// --> no longer exists, but the alteration of Dicom Dictionary remains.
      /// --> Old comment restored on purpose.
      /// --> New one (replacing both BinEntry and ValEntry by DataEntry)
      /// --> had absolutely no meaning.
      /// --> The whole comment will be removed when the stuff is cleaned !
      /// -->
      /// The tags refered by the three following lines used to be CORRECTLY
      /// defined as having an US Value Representation in the public
      /// dictionary. BUT the semantics implied by the three following
      /// lines state that the corresponding tag contents are in fact
      /// the ones of a BinEntry.
      /// In order to fix things "Quick and Dirty" the dictionary was
      /// altered on PURPOSE but now contains a WRONG value.
      /// In order to fix things and restore the dictionary to its
      /// correct value, one needs to decide of the semantics by deciding
      /// whether the following tags are either :
      /// - multivaluated US, and hence loaded as ValEntry, but afterwards
      ///   also used as BinEntry, which requires the proper conversion,
      /// - OW, and hence loaded as BinEntry, but afterwards also used
      ///   as ValEntry, which requires the proper conversion.
      
      // --> OB (byte aray) or OW (short int aray)
      // The actual VR has to be deduced from other entries.
      // Our way of loading them may fail in some cases :
      // We must or not SwapByte depending on other field values.
             
      LoadEntryBinArea(0x0028,0x1201);  // R    LUT
      LoadEntryBinArea(0x0028,0x1202);  // G    LUT
      LoadEntryBinArea(0x0028,0x1203);  // B    LUT
      
      // Segmented Red   Palette Color LUT Data
      LoadEntryBinArea(0x0028,0x1221);
      // Segmented Green Palette Color LUT Data
      LoadEntryBinArea(0x0028,0x1222);
      // Segmented Blue  Palette Color LUT Data
      LoadEntryBinArea(0x0028,0x1223);
   }
 
   //FIXME later : how to use it?
   SeqEntry *modLutSeq = GetSeqEntry(0x0028,0x3000); // Modality LUT Sequence
   if ( modLutSeq !=0 )
   {
      SQItem *sqi= modLutSeq->GetFirstSQItem();
      if ( sqi != 0 )
      {
         DataEntry *dataEntry = sqi->GetDataEntry(0x0028,0x3006); // LUT Data
         if ( dataEntry != 0 )
         {
            if ( dataEntry->GetLength() != 0 )
            {
               // FIXME : CTX dependent means : contexted dependant.
               //         see upper comment.
               LoadEntryBinArea(dataEntry);    //LUT Data (CTX dependent)
            }   
        }
     }      
   }

   // Force Loading some more elements if user asked to.

   GDCM_NAME_SPACE::DocEntry *d;
   for (ListElements::iterator it = UserForceLoadList.begin();  
                               it != UserForceLoadList.end();
                             ++it)
   {
      gdcmDebugMacro( "Force Load " << std::hex 
                       << (*it).Group << "|" <<(*it).Elem );
  
      d = GetDocEntry( (*it).Group, (*it).Elem);
  
      if ( d == NULL)
      {
         gdcmWarningMacro( "You asked to ForceLoad "  << std::hex
                          << (*it).Group <<"|"<< (*it).Elem
                          << " that doesn't exist" );
         continue;
      }

      LoadDocEntry(d, true);
   }

   CloseFile();
  
   // ----------------------------
   // Specific code to allow gdcm to read ACR-LibIDO formated images
   // Note: ACR-LibIDO is an extension of the ACR standard that was
   //       used at CREATIS. For the time being (say a couple of years)
   //       we keep this kludge to allow CREATIS users 
   //       reading their old images.
   //
   // if recognition code tells us we deal with a LibIDO image
   // we switch lineNumber and columnNumber
   //
   std::string RecCode;
   RecCode = GetEntryString(0x0008, 0x0010); // recognition code (RET)
   

   if(RecCode.find("ACRNEMA_LIBIDO") == 0 || // any version
      RecCode.find("CANRME_AILIBOD") == 0)   // for brain-damaged softwares
                                             // with "little-endian strings"
   {
   
         Filetype = ACR_LIBIDO; 
         std::string rows    = GetEntryString(0x0028, 0x0010);
         std::string columns = GetEntryString(0x0028, 0x0011);
         SetEntryString(columns, 0x0028, 0x0010);
         SetEntryString(rows   , 0x0028, 0x0011);
   }
   // --- End of ACR-LibIDO kludge --- 
   return true;
}


/**
 * \brief Adds a new element we want to load anyway
 * @param   group  Group number of the target tag.
 * @param   elem Element number of the target tag.
 */
void Document::AddForceLoadElement (uint16_t group, uint16_t elem) 
{ 
   DicomElement el;
   el.Group = group;
   el.Elem  = elem;
   UserForceLoadList.push_back(el); 
}
/**
 * \brief   Get the public dictionary used
 */
Dict *Document::GetPubDict()
{
   return RefPubDict;
}

/**
 * \brief   Get the shadow dictionary used
 */
Dict *Document::GetShaDict()
{
   return RefShaDict;
}

/**
 * \brief   Set the shadow dictionary used
 * @param   dict dictionary to use in shadow
 */
bool Document::SetShaDict(Dict *dict)
{
   RefShaDict = dict;
   return !RefShaDict;
}

/**
 * \brief   Set the shadow dictionary used
 * @param   dictName name of the dictionary to use in shadow
 */
bool Document::SetShaDict(DictKey const &dictName)
{
   RefShaDict = Global::GetDicts()->GetDict(dictName);
   return !RefShaDict;
}

/**
 * \brief  This predicate tells us whether or not the current Document 
 *         was properly parsed and contains at least *one* Dicom Element
 *         (and nothing more, sorry).
 * @return false when we're 150 % sure it's NOT a Dicom/Acr file,
 *         true otherwise. 
 */
bool Document::IsParsable()
{
   if ( Filetype == Unknown )
   {
      gdcmWarningMacro( "Wrong filetype for " << GetFileName());
      return false;
   }

   if ( IsEmpty() )
   { 
      gdcmWarningMacro( "No tag in internal hash table.");
      return false;
   }

   return true;
}
/**
 * \brief  This predicate tells us whether or not the current Document 
 *         was properly parsed and contains at least *one* Dicom Element
 *         (and nothing more, sorry).
 * @return false when we're 150 % sure it's NOT a Dicom/Acr file,
 *         true otherwise. 
 */
bool Document::IsReadable()
{
   return IsParsable();
}

/**
 * \brief   Predicate for dicom version 3 file.
 * @return  True when the file is a dicom version 3.
 */
bool Document::IsDicomV3()
{
   // Checking if Transfer Syntax exists is enough
   // Anyway, it's too late check if the 'Preamble' was found ...
   // And ... would it be a rich idea to check ?
   // (some 'no Preamble' DICOM images exist !)
   return GetDocEntry(0x0002, 0x0010) != NULL;
}

/**
 * \brief   Predicate for Papyrus file
 *          Dedicated to whomsoever it may concern
 * @return  True when the file is a Papyrus file.
 */
bool Document::IsPapyrus()
{
   // check for Papyrus private Sequence
   DocEntry *e = GetDocEntry(0x0041, 0x1050);
   if ( !e )
      return false;
   // check if it's actually a Sequence
   if ( !dynamic_cast<SeqEntry*>(e) )
      return  false;
   return true;
}

/**
 * \brief  returns the File Type 
 *         (ACR, ACR_LIBIDO, ExplicitVR, ImplicitVR, Unknown)
 * @return the FileType code
 */
FileType Document::GetFileType()
{
   return Filetype;
}

/**
 * \brief   Accessor to the Transfer Syntax (when present) of the
 *          current document (it internally handles reading the
 *          value from disk when only parsing occured).
 * @return  The encountered Transfer Syntax of the current document, if DICOM.
 *          GDCM_UNKNOWN for ACR-NEMA files (or broken headers ...)
 */
std::string Document::GetTransferSyntax()
{
   DocEntry *entry = GetDocEntry(0x0002, 0x0010);
   if ( !entry )
   {
      return GDCM_UNKNOWN;
   }

   // The entry might be present but not loaded (parsing and loading
   // happen at different stages): try loading and proceed with check...
   
   // Well ...
   // (parsing and loading happen at the very same stage!)
   //LoadDocEntrySafe(entry); //JPRx
   if (DataEntry *dataEntry = dynamic_cast<DataEntry *>(entry) )
   {
      std::string transfer = dataEntry->GetString();
      // The actual transfer (as read from disk) might be padded. We
      // first need to remove the potential padding. We can make the
      // weak assumption that padding was not executed with digits...
      if  ( transfer.length() == 0 )
      {
         // for brain damaged headers
         gdcmWarningMacro( "Transfer Syntax has length = 0.");
         return GDCM_UNKNOWN;
      }
      while ( !isdigit((unsigned char)transfer[transfer.length()-1]) )
      {
         transfer.erase(transfer.length()-1, 1);
         if  ( transfer.length() == 0 )
         {
            // for brain damaged headers
            gdcmWarningMacro( "Transfer Syntax contains no valid character.");
            return GDCM_UNKNOWN;
         }
      }
      return transfer;
   }
   return GDCM_UNKNOWN;
}

/**
 * \brief Accesses the info from 0002,0010 : Transfer Syntax and TS
 * @return The full Transfer Syntax Name (as opposed to Transfer Syntax UID)
 */
std::string Document::GetTransferSyntaxName()
{
   // use the TS (TS : Transfer Syntax)
   std::string transferSyntax = GetEntryString(0x0002,0x0010);

   if ( (transferSyntax.find(GDCM_NOTLOADED) < transferSyntax.length()) )
   {
      gdcmErrorMacro( "Transfer Syntax not loaded. " << std::endl
               << "Better you increase MAX_SIZE_LOAD_ELEMENT_VALUE" );
      return "Uncompressed ACR-NEMA";
   }
   if ( transferSyntax == GDCM_UNFOUND )
   {
      gdcmDebugMacro( "Unfound Transfer Syntax (0002,0010)");
      return "Uncompressed ACR-NEMA";
   }

   // we do it only when we need it
   const TSKey &tsName = Global::GetTS()->GetValue( transferSyntax );

   // Global::GetTS() is a global static you shall never try to delete it!
   return tsName;
}
//
// --------------- Swap Code ------------------
/**
 * \brief   Swaps the bytes so they agree with the processor order
 * @return  The properly swaped 16 bits integer.
 */
uint16_t Document::SwapShort(uint16_t a)
{
   if ( SwapCode == 4321 || SwapCode == 2143 )
   {
      //a = ((( a << 8 ) & 0xff00 ) | (( a >> 8 ) & 0x00ff ) );
      // Save CPU time
      a = ( a << 8 ) | ( a >> 8 );
   }
   return a;
}

/**
 * \brief   Swaps back the bytes of 4-byte long integer accordingly to
 *          processor order.
 * @return  The properly swaped 32 bits integer.
 */
uint32_t Document::SwapLong(uint32_t a)
{
   switch (SwapCode)
   {
      case 1234 :
         break;
      case 4321 :
//         a=( ((a<<24) & 0xff000000) | ((a<<8)  & 0x00ff0000) | 
//             ((a>>8)  & 0x0000ff00) | ((a>>24) & 0x000000ff) );
// save CPU time
         a=( ( a<<24)               | ((a<<8)  & 0x00ff0000) | 
             ((a>>8)  & 0x0000ff00) |  (a>>24)                );
         break;
      case 3412 :
//       a=( ((a<<16) & 0xffff0000) | ((a>>16) & 0x0000ffff) );
         a=( (a<<16)                | (a>>16)  );
         break;
      case 2143 :
         a=( ((a<< 8) & 0xff00ff00) | ((a>>8) & 0x00ff00ff)  );
      break;
      default :
         gdcmErrorMacro( "Unexpected swap code:" << SwapCode );
         a = 0;
   }
   return a;
}

/**
 * \brief   Swaps back the bytes of 8-byte long 'double' accordingly to
 *          processor order.
 * @return  The properly swaped 64 bits double.
 */
double Document::SwapDouble(double a)
{
   switch (SwapCode)
   {
      // There were no 'double' at ACR-NEMA time.
      // We just have to deal with 'straight Little Endian' and 
      // 'straight Big Endian'
      case 1234 :
         break;
      case 4321 :
         {
         char *beg = (char *)&a;
         char *end = beg + 7;
         char t;
         for (unsigned int i = 0; i<7; i++)
         {
            t    = *beg;
            *beg = *end;
            *end = t;
            beg++,
            end--;  
         }
         }
         break;   
      default :
         gdcmErrorMacro( "Unexpected swap code:" << SwapCode );
         a = 0.;
   }
   return a;
} 

//
// -----------------File I/O ---------------
/**
 * \brief  Tries to open the file Document::Filename and
 *         checks the preamble when existing,
 *         or if the file starts with an ACR-NEMA look-like element.
 * @return The FILE pointer on success, 0 on failure.
 */
std::ifstream *Document::OpenFile()
{
   HasDCMPreamble = false;
   if (Filename.length() == 0)
   {
      return 0;
   }

   if ( Fp )
   {
      gdcmDebugMacro( "File already open: " << Filename.c_str());
      CloseFile();
   }

   Fp = new std::ifstream(Filename.c_str(), std::ios::in | std::ios::binary);
   if ( ! *Fp )
   {
   // Don't user gdcmErrorMacro :
   // a spurious message will appear when you use, for instance
   // gdcm::FileHelper *fh = new gdcm::FileHelper( outputFileName );
   // to create outputFileName.

   // FIXME : if the upper comment is still usefull
   //         --> the constructor is not so good ...

      gdcmWarningMacro( "Cannot open file: " << Filename.c_str());
      delete Fp;
      Fp = 0;
      return 0;
      //exit(1); // No function is allowed to leave the application instead
                 // of warning the caller
   }

   uint16_t zero = 0;
   Fp->read((char*)&zero, (size_t)2);
   if ( Fp->eof() )
   {
      CloseFile();
      return 0;
   }

   //-- DICOM --
   Fp->seekg(126L, std::ios::cur);  // Once per Document
   char dicm[4]; // = {' ',' ',' ',' '};
   Fp->read(dicm,  (size_t)4);
   if ( Fp->eof() )
   {
      CloseFile();
      return 0;
   }

   if ( memcmp(dicm, "DICM", 4) == 0 )
   {
      HasDCMPreamble = true;
      return Fp;
   }

   //-- Broken ACR or DICOM (?) with no Preamble; may start with a Shadow Group --
   // FIXME : We cannot be sure the preable is only zeroes..
   //         (see ACUSON-24-YBR_FULL-RLE.dcm )
   if (
       zero == 0x0001 || zero == 0x0100 || zero == 0x0002 || zero == 0x0200 ||
       zero == 0x0003 || zero == 0x0300 || zero == 0x0004 || zero == 0x0400 ||
       zero == 0x0005 || zero == 0x0500 || zero == 0x0006 || zero == 0x0600 ||
       zero == 0x0007 || zero == 0x0700 || zero == 0x0008 || zero == 0x0800 ||
       zero == 0x0028 || 0x2800    // worse : some ACR-NEMA like files 
                                   // start 00028 group ?!?
       )
   {
      std::string msg = Util::Format(
        "ACR/DICOM starting by 0x(%04x) at the beginning of the file\n", zero);
      // FIXME : is it a Warning message, or a Debug message?
      gdcmWarningMacro( msg.c_str() );
      return Fp;
   }

   // -- Neither ACR/No Preamble Dicom nor DICOMV3 file
   CloseFile();
   // Don't user Warning nor Error, not to pollute the output
   // while directory recursive parsing ...
   gdcmDebugMacro( "Neither ACR/No Preamble Dicom nor DICOMV3 file: "
                      << Filename.c_str());
   return 0;
}

/**
 * \brief closes the file
 * @return  TRUE if the close was successfull
 */
bool Document::CloseFile()
{
   if ( Fp )
   {
      Fp->close();
      delete Fp;
      Fp = 0;
   }
   return true;
}

/**
 * \brief Writes in a file all the Entries (Dicom Elements)
 * @param fp file pointer on an already open file (actually: Output File Stream)
 * @param filetype Type of the File to be written
 *          (ACR-NEMA, ExplicitVR, ImplicitVR)
 */
void Document::WriteContent(std::ofstream *fp, FileType filetype, bool, bool)
{
   // Skip if user wants to write an ACR-NEMA file

   if ( filetype == ImplicitVR || filetype == ExplicitVR ||
        filetype == JPEG || filetype == JPEG2000 )
   {
      // writing Dicom File Preamble
      char filePreamble[128];
      memset(filePreamble, 0, 128);
      fp->write(filePreamble, 128);
      fp->write("DICM", 4);
   }
   /*
    * \todo rewrite later, if really usefull
    *       - 'Group Length' element is optional in DICOM
    *       - but un-updated odd groups lengthes can causes pb
    *         (xmedcon breaker)
    *
    * if ( (filetype == ImplicitVR) || (filetype == ExplicitVR) )
    *    UpdateGroupLength(false,filetype);
    * if ( filetype == ACR)
    *    UpdateGroupLength(true,ACR);
    *
    * --> Computing group length for groups with embeded Sequences
    * --> was too much tricky / we were [in a hurry / too lazy]
    * --> We don't write the element 0x0000 (group length)
    */
 // This one is recursive
 // false : outside MetaElements
 // false : outside Sequence
   ElementSet::WriteContent(fp, filetype, false, false);
}

// -----------------------------------------
// Content entries 
/**
 * \brief Loads (from disk) the element content 
 *        when a string is not suitable
 * @param group   group number of the Entry 
 * @param elem  element number of the Entry
 */
void Document::LoadEntryBinArea(uint16_t group, uint16_t elem)
{
   // Search the corresponding DocEntry
   DocEntry *docEntry = GetDocEntry(group, elem);
   if ( !docEntry )
   {
      gdcmDebugMacro(std::hex << group << "|" << elem 
                       <<  " doesn't exist" );
      return;
   }
   DataEntry *dataEntry = dynamic_cast<DataEntry *>(docEntry);
   if ( !dataEntry )
   {
      gdcmWarningMacro(std::hex << group << "|" << elem 
                       <<  " is NOT a DataEntry");
      return;
   }
   LoadEntryBinArea(dataEntry);
}

/**
 * \brief Loads (from disk) the element content 
 *        when a string is not suitable
 * @param entry  Entry whose binArea is going to be loaded
 */
void Document::LoadEntryBinArea(DataEntry *entry) 
{ 
   if( entry->GetBinArea() )
      return;

   bool openFile = !Fp;
   if ( openFile )
      OpenFile();

   //size_t o =(size_t)entry->GetOffset();
   Fp->seekg((size_t)entry->GetOffset(), std::ios::beg);  // FIXME : for each DataEntry !

   size_t l = entry->GetLength();
   uint8_t *data = new uint8_t[l];
   if ( !data )
   {
      gdcmWarningMacro(  "Cannot allocate DataEntry content for : "
                       << std::hex << entry->GetGroup() 
                       << "|" << entry->GetElement() );
      return;
   }

   // Read the data
   Fp->read((char*)data, l);
   if ( Fp->fail() || Fp->eof() )
   {
      delete[] data;
      entry->SetState(DataEntry::STATE_UNREAD);
      return;
   }

   // Swap the data content if necessary
   uint32_t i;
   unsigned short vrLgth = 
                        Global::GetVR()->GetAtomicElementLength(entry->GetVR());

// FIXME : trouble expected if we read an ... OW Entry (LUT, etc ..)
//   if( entry->GetVR() == "OW" )
//      vrLgth = 1;

   switch(vrLgth)
   {
      case 1:
      {
         break;
      }     
      case 2:
      {
         uint16_t *data16 = (uint16_t *)data;
         for(i=0;i<l/vrLgth;i++)
            data16[i] = SwapShort(data16[i]);
         break;
      }
      case 4:
      {
         uint32_t *data32 = (uint32_t *)data;
         for(i=0;i<l/vrLgth;i++)
            data32[i] = SwapLong(data32[i]);
         break;
      }
      case 8:
      {
         double *data64 = (double *)data;
         for(i=0;i<l/vrLgth;i++)
            data64[i] = SwapDouble(data64[i]);
         break;
      }
   }
   
   entry->SetBinArea(data);

   if ( openFile ) // The file is left in the state (open/close) it was at entrance
      CloseFile();
}

/**
 * \brief  Loads the element while preserving the current
 *         underlying file position indicator as opposed to
 *        LoadDocEntry that modifies it
 * \note seems to be unused!.
 * @param entry   DocEntry whose value will be loaded. 
 */
//void Document::LoadDocEntrySafe(DocEntry *entry)
//{
//   if ( Fp )
//   {
//      long PositionOnEntry = Fp->tellg();        // LoadDocEntrySafe is not used
//      LoadDocEntry(entry);
//      Fp->seekg(PositionOnEntry, std::ios::beg); // LoadDocEntrySafe is not used
//   }
//}

/**
 * \brief   Compares two documents, according to DicomDir rules
 * \warning Does NOT work with ACR-NEMA files
 * \todo    Find a trick to solve the pb (use RET fields ?)
 * @param   document to compare with current one
 * @return  true if 'smaller'
 */
bool Document::operator<(Document &document)
{
   // Patient Name
   std::string s1 = GetEntryString(0x0010,0x0010);
   std::string s2 = document.GetEntryString(0x0010,0x0010);
   if (s1 < s2)
   {
      return true;
   }
   else if ( s1 > s2 )
   {
      return false;
   }
   else
   {
      // Patient ID
      s1 = GetEntryString(0x0010,0x0020);
      s2 = document.GetEntryString(0x0010,0x0020);
      if ( s1 < s2 )
      {
         return true;
      }
      else if ( s1 > s2 )
      {
         return false;
      }
      else
      {
         // Study Instance UID
         s1 = GetEntryString(0x0020,0x000d);
         s2 = document.GetEntryString(0x0020,0x000d);
         if ( s1 < s2 )
         {
            return true;
         }
         else if ( s1 > s2 )
         {
            return false;
         }
         else
         {
            // Serie Instance UID
            s1 = GetEntryString(0x0020,0x000e);
            s2 = document.GetEntryString(0x0020,0x000e);    
            if ( s1 < s2 )
            {
               return true;
            }
            else if ( s1 > s2 )
            {
               return false;
            }
         }
      }
   }
   return false;
}

//-----------------------------------------------------------------------------
// Protected

/**
 * \brief Reads a given length of bytes
 *       (in order to avoid to many CPU time-consuming fread-s)
 * @param l length to read 
 */
void Document::ReadBegBuffer(size_t l)
   throw( FormatError )
{   
   Fp->read (BegBuffer, (size_t)l);
   if ( Fp->fail() )
   {
      throw FormatError( "Document::ReadBegBuffer()", " file error." );
   }
   if ( Fp->eof() )
   {
      throw FormatError( "Document::ReadBegBuffer()", "EOF." );
   }
   PtrBegBuffer = BegBuffer;
   CurrentOffsetPosition+=l;
}
/**
 * \brief Reads a supposed to be 16 Bits integer
 *       (swaps it depending on processor endianness) 
 * @return read value
 */
uint16_t Document::ReadInt16()
   throw( FormatError )
{
   uint16_t g;
   Fp->read ((char*)&g, (size_t)2);
   if ( Fp->fail() )
   {
      throw FormatError( "Document::ReadInt16()", " file error." );
   }
   if ( Fp->eof() )
   {
      throw FormatError( "Document::ReadInt16()", "EOF." );
   }
   g = SwapShort(g); 
   return g;
}

/**
 * \brief Gets from BegBuffer a supposed to be 16 Bits integer
 *       (swaps it depending on processor endianness) 
 * @return read value
 */
uint16_t Document::GetInt16()
{
   uint16_t g = *((uint16_t*)PtrBegBuffer);
   g = SwapShort(g);
   PtrBegBuffer+=2; 
   return g;
}
/**
 * \brief  Reads a supposed to be 32 Bits integer
 *        (swaps it depending on processor endianness)  
 * @return read value
 */
uint32_t Document::ReadInt32()
   throw( FormatError )
{
   uint32_t g;
   Fp->read ((char*)&g, (size_t)4);
   if ( Fp->fail() )
   {
      throw FormatError( "Document::ReadInt32()", " file error." );
   }
   if ( Fp->eof() )
   {
      throw FormatError( "Document::ReadInt32()", "EOF." );
   }
   g = SwapLong(g);
   return g;
}

/**
 * \brief Gets from BegBuffer a supposed to be 32 Bits integer
 *       (swaps it depending on processor endianness) 
 * @return read value
 */
uint32_t Document::GetInt32()
{
   uint32_t g = *((uint32_t*)PtrBegBuffer);
   g = SwapLong(g);
   PtrBegBuffer+=4;
   return g;
}

/**
 * \brief   Re-computes the length of the Dicom group 0002.
 */
int Document::ComputeGroup0002Length( )
{
   uint16_t gr;
   VRKey vr;
   
   int groupLength = 0;
   bool found0002 = false;   
  
   // for each zero-level Tag in the DCM Header
   DocEntry *entry = GetFirstEntry();
   while( entry )
   {
      gr = entry->GetGroup();

      if ( gr == 0x0002 )
      {
         found0002 = true;

         if ( entry->GetElement() != 0x0000 )
         {
            vr = entry->GetVR();

            //if ( (vr == "OB")||(vr == "OW")||(vr == "UT")||(vr == "SQ"))
            // (no SQ, OW, OL, UT in group 0x0002;)
               if ( vr == "OB" ) 
               {
                  // explicit VR AND (OB, OW, OL, SQ, UT, UN) : 4 more bytes
                  groupLength +=  4;
               }
            groupLength += 2 + 2 + 4 + entry->GetLength();   
         }
      }
      else if (found0002 )
         break;

      entry = GetNextEntry();
   }
   return groupLength; 
}

/**
 * \brief   CallStartMethod
 */
void Document::CallStartMethod()
{
   Progress = 0.0f;
   Abort    = false;
   CommandManager::ExecuteCommand(this,CMD_STARTPROGRESS);
}

/**
 * \brief   CallProgressMethod
 */
void Document::CallProgressMethod()
{
   CommandManager::ExecuteCommand(this,CMD_PROGRESS);
}

/**
 * \brief   CallEndMethod
 */
void Document::CallEndMethod()
{
   Progress = 1.0f;
   CommandManager::ExecuteCommand(this,CMD_ENDPROGRESS);
}

//-----------------------------------------------------------------------------
// Private
/**
 * \brief Loads all the needed Dictionaries
 * \warning NOT end user intended method !
 */
void Document::Initialize() 
{
   RefPubDict = Global::GetDicts()->GetDefaultPubDict();
   RefShaDict = NULL;
   Filetype   = Unknown;
}

/**
 * \brief   Parses a DocEntrySet (Zero-level DocEntries or SQ Item DocEntries)
 * @param set DocEntrySet we are going to parse ('zero level' or a SQItem)
 * @param offset start of parsing
 * @param l_max  length to parse (meaningless when we are in 'delimitor mode')
 * @param delim_mode : whether we are in 'delimitor mode' (l=0xffffff) or not
 */ 
void Document::ParseDES(DocEntrySet *set, long offset, 
                        long l_max, bool delim_mode)
{
   DocEntry *newDocEntry;
   DataEntry *newDataEntry;
   SeqEntry *newSeqEntry;
   //VRKey vr;
   bool used; // will be set to false when something wrong happens to an Entry.
              // (Entry will then be deleted)
   bool delim_mode_intern = delim_mode;
   bool first = true;
   gdcmDebugMacro( "Enter in ParseDES, delim-mode " <<  delim_mode
                     << " at offset " << std::hex << "0x(" << offset << ")" );
   while (true)
   {
   
   ///\todo FIXME : On 64 bits processors, tellg gives unexpected results after a while ?
   ///              Probabely a bug in gdcm code somewhere (some memory erased ?)

// Uncomment to track the bug

   if( Debug::GetDebugFlag() )
      std::cout << std::dec <<"(long)(Fp->tellg()) " << (long)(Fp->tellg()) // in Debug mode
                << std::hex << " 0x(" <<(long)(Fp->tellg()) <<  ")" << std::endl;


   // if ( !delim_mode && ((long)(Fp->tellg())-offset) >= l_max) // Once per DocEntry   
      if ( !delim_mode ) // 'and then' doesn't exist in C++ :-(
         if ( ((long)(Fp->tellg())-offset) >= l_max) // Once per DocEntry, when no delim mode
         {
            break;
         }

      newDocEntry = ReadNextDocEntry( );

      if ( !newDocEntry )
      {
         break;
      }
      
      // Uncoment this cerr line to be able to 'follow' the DocEntries
      // when something *very* strange happens
      if( Debug::GetDebugFlag() )
         std::cerr<<newDocEntry->GetKey()<<" "<<newDocEntry->GetVR()<<std::endl;

       // an Item Starter found elsewhere but in the first position
       // of a SeqEntry means previous entry was a Sequence
       // but we didn't get it (private Sequence + Implicit VR)
       // we have to backtrack.
      if ( !first && newDocEntry->IsItemStarter() )
      {
         // Debug message within the method !
         newDocEntry = Backtrack(newDocEntry, set);
      }
      else
      {
         PreviousDocEntry = newDocEntry;
      }

      used = true;
      newDataEntry = dynamic_cast<DataEntry*>(newDocEntry);

      if ( newDataEntry )
      {
         //////////////////////////// DataEntry

         //vr = newDocEntry->GetVR(); // useless ?

         if ( !set->AddEntry( newDataEntry ) )
         {
            gdcmDebugMacro( "in ParseDES : cannot add a DataEntry "
                                 << newDataEntry->GetKey()
                                 << " (at offset : 0x(" 
                                 << newDataEntry->GetOffset() << ") )" );
            used=false;
         }
         else
         {
            newDataEntry->Delete();
            // Load only if we can add (not a duplicate key)
            LoadDocEntry( newDataEntry );
         }
         if ( newDataEntry->GetElement() == 0x0000 ) // if on group length
         {
            if ( newDataEntry->GetGroup()%2 != 0 )   // if Shadow Group
            {
               if ( LoadMode & LD_NOSHADOW ) // if user asked to skip shad.gr
               {
                  std::string strLgrGroup = newDataEntry->GetString();

                  int lgrGroup;
                  //if ( newDataEntry->IsUnfound() ) /?!? JPR
                  {
                     lgrGroup = atoi(strLgrGroup.c_str());
                     Fp->seekg(lgrGroup, std::ios::cur); // Once per Shadow group, when NOSHADOW
                     RemoveEntry( newDocEntry );  // Remove and delete
                     continue;
                  }
               }
            }
         }

         bool delimitor = newDataEntry->IsItemDelimitor();
         bool outOfBounds = false;
         if (!delim_mode )
            if ( ((long)(Fp->tellg())-offset) >= l_max ) //Once per DataEntry when no delim mode
               outOfBounds = true;

  //       'and then', 'or else' don't exist in C++ :-(
  //       if ( (delimitor) || 
  //             (!delim_mode && ((long)(Fp->tellg())-offset) >= l_max) ) // Once per DataEntry

         if ( delimitor || outOfBounds )
         {
            if ( !used )
               newDocEntry->Delete();
            break;
         }

         // Just to make sure we are at the beginning of next entry.
         SkipToNextDocEntry(newDocEntry); // FIXME : once per DocEntry, segfault if commented out
      }
      else
      {
         /////////////////////// SeqEntry :  VR = "SQ"

         unsigned long l = newDocEntry->GetReadLength();
         if ( l != 0 ) // don't mess the delim_mode for 'zero-length sequence'
         {
            if ( l == 0xffffffff )
            {
              delim_mode_intern = true;
            }
            else
            {
              delim_mode_intern = false;
            }
         }

         if ( (LoadMode & LD_NOSHADOWSEQ) && ! delim_mode_intern )
         { 
           // User asked to skip SeQuences *only* if they belong to Shadow Group
            if ( newDocEntry->GetGroup()%2 != 0 )
            {
                Fp->seekg( l, std::ios::cur);  // once per SQITEM, when NOSHADOWSEQ
                newDocEntry->Delete();  // Delete, not in the set 
                continue;  
            } 
         } 
         if ( (LoadMode & LD_NOSEQ) && ! delim_mode_intern ) 
         {
           // User asked to skip *any* SeQuence
            Fp->seekg( l, std::ios::cur); // Once per SQ, when NOSEQ
            newDocEntry->Delete(); // Delete, not in the set
            continue;
         }
         // delay the dynamic cast as late as possible
         newSeqEntry = dynamic_cast<SeqEntry*>(newDocEntry);
         
         // no other way to create the Delimitor ...
         newSeqEntry->SetDelimitorMode( delim_mode_intern );

         // At the top of the hierarchy, stands a Document. When "set"
         // is a Document, then we are building the first depth level.
         // Hence the SeqEntry we are building simply has a depth
         // level of one:
        if ( set == this ) // ( dynamic_cast< Document* > ( set ) )
         {
            newSeqEntry->SetDepthLevel( 1 );
         }
         // But when "set" is already a SQItem, we are building a nested
         // sequence, and hence the depth level of the new SeqEntry
         // we are building, is one level deeper:

         // time waste hunting
         else if (SQItem *parentSQItem = dynamic_cast< SQItem* > ( set ) )
         {
            newSeqEntry->SetDepthLevel( parentSQItem->GetDepthLevel() + 1 );
         }

         if ( l != 0 )
         {  // Don't try to parse zero-length sequences

            gdcmDebugMacro( "Entry in ParseSQ, delim " << delim_mode_intern
                               << " at offset 0x(" << std::hex
                               << newDocEntry->GetOffset() << ")");

            bool res = ParseSQ( newSeqEntry, 
                         newDocEntry->GetOffset(),
                         l, delim_mode_intern);

            gdcmDebugMacro( "Exit from ParseSQ, delim " << delim_mode_intern << " -->return : " << res);
         }
         if ( !set->AddEntry( newSeqEntry ) )
         {
            gdcmWarningMacro( "in ParseDES : cannot add a SeqEntry "
                                << newSeqEntry->GetKey()
                                << " (at offset : 0x(" 
                                << newSeqEntry->GetOffset() << ") )" ); 
            used = false;
         }
         else
         {
            newDocEntry->Delete();
         }

      // if ( !delim_mode && ((long)(Fp->tellg())-offset) >= l_max) // Once per SeqEntry
 
         if ( !delim_mode ) // 'and then' doesn't exist in C++ :-(
            if ( ((long)(Fp->tellg())-offset) >= l_max) // Once per SeqEntry when no delim mode

         {
            if ( !used )
               newDocEntry->Delete();
            break;
         }
      }  // end SeqEntry : VR = "SQ"

      if ( !used )
      {
         newDocEntry->Delete();
      }
      first = false;
      
      if (UnexpectedEOF) // some terminator was missing
      {
         break;
      }
   }                               // end While
   gdcmDebugMacro( "Exit from ParseDES, delim-mode " << delim_mode );
}

/**
 * \brief   Parses a Sequence ( SeqEntry after SeqEntry)
 * @return  false if expected fff0,e000 not found
 */ 
bool Document::ParseSQ( SeqEntry *seqEntry,
                        long offset, long l_max, bool delim_mode)
{
   int SQItemNumber = 0;
   bool dlm_mod;
   long offsetStartCurrentSQItem = offset;

   while (true)
   {
      // the first time, we read the fff0,e000 of the first SQItem
      DocEntry *newDocEntry = ReadNextDocEntry();

      if ( !newDocEntry )
      {
         // The most frequent is when a SQ terminator is missing (?!?)
         gdcmWarningMacro("in ParseSQ : should never get here!");
         UnexpectedEOF = true;
         return false;
      }
      if ( delim_mode )
      {
         if ( newDocEntry->IsSequenceDelimitor() )
         {
            seqEntry->SetDelimitationItem( newDocEntry ); 
            newDocEntry->Delete();
            break;
         }
      }
      else // ! delim_mode
      {
         if ( ((long)(Fp->tellg())-offset) >= l_max) // Once per SQItem when no delim mode
         {
            newDocEntry->Delete();
            break;
         }
      }
      // create the current SQItem
      SQItem *itemSQ = SQItem::New( seqEntry->GetDepthLevel() );
      unsigned int l = newDocEntry->GetReadLength();
      
      if ( l == 0xffffffff )
      {
         dlm_mod = true;
      }
      else
      {
         dlm_mod = false;
      }

      // avoid infinite loop when Bad assumption was made on illegal 'unknown length' UN //JPRx
    
      if (offsetStartCurrentSQItem <= OffsetOfPreviousParseDES)
      {
         gdcmWarningMacro("Bad assumption was made on illegal 'unknown length' UN!" << std::endl <<
                          "OffsetOfPreviousParseDES " << std::hex << OffsetOfPreviousParseDES
                           << " offsetStartCurrentSQItem " << offsetStartCurrentSQItem);
         /// \todo when  "Bad assumption (SQ) on illegal 'unknown length' UN", Backtrack again + try OB      
         return false; 
      }
      else 
      {
         OffsetOfPreviousParseDES = offsetStartCurrentSQItem;
      }

      // fill up the current SQItem, starting at the beginning of fff0,e000
      Fp->seekg(offsetStartCurrentSQItem, std::ios::beg);        // Once per SQItem
      ParseDES(itemSQ, offsetStartCurrentSQItem, l+8, dlm_mod);
      offsetStartCurrentSQItem = Fp->tellg();                    // Once per SQItem
 
      seqEntry->AddSQItem( itemSQ, SQItemNumber ); 
      itemSQ->Delete();
      newDocEntry->Delete();
      SQItemNumber++;
      //if ( !delim_mode && ((long)(Fp->tellg())-offset ) >= l_max ) //JPRx
      if ( !delim_mode && (offsetStartCurrentSQItem-offset ) >= l_max )
      {
         break;
      }
   }
   return true;
}

/**
 * \brief   When a private Sequence + Implicit VR is encountered
 *           we cannot guess it's a Sequence till we find the first
 *           Item Starter. We then backtrack to do the job.
 * @param   docEntry Item Starter that warned us
 * @param   set DocEntrySet (ElementSet/SQItem) the DocEntry will belong
 */
DocEntry *Document::Backtrack(DocEntry *docEntry, DocEntrySet *set)
{
   // delete the Item Starter, built erroneously out of any Sequence
   // it's not yet in the HTable/chained list
   docEntry->Delete();

   // Get all info we can from PreviousDocEntry
   uint16_t group = PreviousDocEntry->GetGroup();
   uint16_t elem  = PreviousDocEntry->GetElement();
   uint32_t lgt   = PreviousDocEntry->GetLength();
   long offset    = PreviousDocEntry->GetOffset();

   gdcmDebugMacro( "Backtrack :" << std::hex << group 
                                 << "|" << elem
                                 << " at offset 0x(" <<offset << ")" );
   
   set->RemoveEntry( PreviousDocEntry );

   // forge the Seq Entry
   DocEntry *newEntry = NewSeqEntry(group, elem);
   newEntry->SetLength(lgt);
   newEntry->SetOffset(offset);

   // Move back to the beginning of the Sequence

   Fp->seekg(offset, std::ios::beg); // Only for Shadow Implicit VR SQ
   return newEntry; // It will added where it has to be!
}

/**
 * \brief   Loads (or not) the element content depending if its length exceeds
 *          or not the value specified with Document::SetMaxSizeLoadEntry()
 * @param   entry Header Entry (Dicom Element) to be dealt with
 * @param forceLoad whether you want to force loading of 'long' elements
 */
void Document::LoadDocEntry(DocEntry *entry, bool forceLoad)
{
   uint16_t group   = entry->GetGroup();
   uint16_t elem    = entry->GetElement();
   const VRKey  &vr = entry->GetVR();
   uint32_t length  = entry->GetLength();

 //  Fp->seekg((long)entry->GetOffset(), std::ios::beg); // JPRx

   // A SeQuence "contains" a set of Elements.  
   //          (fffe e000) tells us an Element is beginning
   //          (fffe e00d) tells us an Element just ended
   //          (fffe e0dd) tells us the current SeQuence just ended
   //          (fffe 0000) is an 'impossible' tag value, 
   //                                    found in MR-PHILIPS-16-Multi-Seq.dcm
   
   if ( (group == 0xfffe && elem != 0x0000 ) || vr == "SQ" )
   {
      // NO more value field for SQ !
      return;
   }

   DataEntry *dataEntryPtr = dynamic_cast< DataEntry* >(entry);
   if( !dataEntryPtr )
   {
      return;
   }

   // When the length is zero things are easy:
   if ( length == 0 )
   {
      dataEntryPtr->SetBinArea(NULL,true);
      return;
   }

   // The elements whose length is bigger than the specified upper bound
   // are not loaded.

   if (!forceLoad)
   {
      if (length > MaxSizeLoadEntry)
      {
         dataEntryPtr->SetBinArea(NULL,true);
         dataEntryPtr->SetState(DataEntry::STATE_NOTLOADED);

       // to be sure we are at the end of the value ...
       //  Fp->seekg((long)entry->GetOffset()+(long)entry->GetLength(),
       //           std::ios::beg);  //JPRx
         return;
      }
   }

   /// \todo: a method that *doesn't* load anything (maybe with MaxSizeLoadEntry=0 ?)
   ///       + a ForceLoad call on the +/- 20 'usefull' fields  
   ///       Allow user to tell the fields he wants to ForceLoad 
   ///       during initial stage.
   ///       Later, a GetString or GetBinArea will load the value from disk, if not loaded
   ///       + a method that load *everything* that's not yet loaded
   
   LoadEntryBinArea(dataEntryPtr); // last one, not to erase length !
}

/**
 * \brief  Find the value Length of the passed Doc Entry
 * @param  entry Header Entry whose length of the value shall be loaded. 
 */
void Document::FindDocEntryLength( DocEntry *entry )
   throw ( FormatError )
{
   const VRKey &vr  = entry->GetVR();
   uint16_t length16;       
   if ( Filetype == ExplicitVR && !entry->IsImplicitVR() ) 
   {

   // WARNING :
   //
   // For some images, length of UN elements is coded on 2 bytes (instead of 4)
   // There are *not* readable !
   // You can make a quick and dirty patch, commenting out 
   //| vr == "UN"
   // in the following line.
   // (the 'straight' images will no longer be readable ...)

      if ( vr == "OB" || vr == "OW" || vr == "OL" || vr == "SQ" || vr == "UT" 
                                                         || vr == "UN"  || changeFromUN == true)
      {
         changeFromUN = false;
         // The following reserved two bytes (see PS 3.5-2003, section
         // "7.1.2 Data element structure with explicit vr", p 27) must be
         // skipped before proceeding on reading the length on 4 bytes.

         //Fp->seekg( 2L, std::ios::cur); // Once per OB,OW,OL,UT,UN,SQ DocEntry
         uint32_t length32 = ReadInt32(); // Once per OB,OW,OL,UT,UN,SQ DocEntry
         CurrentOffsetPosition+=4;
         if ( (vr == "OB" || vr == "OW" || vr == "OL") && length32 == 0xffffffff ) 
         {
            uint32_t lengthOB;
            try 
            {
               lengthOB = FindDocEntryLengthOBOrOW();// for encapsulation of encoded pixel 
            }
            catch ( FormatUnexpected )
            {
               // Computing the length failed (this happens with broken
               // files like gdcm-JPEG-LossLess3a.dcm). We still have a
               // chance to get the pixels by deciding the element goes
               // until the end of the file. Hence we artificially fix the
               // the length and proceed.
               gdcmWarningMacro( " Computing the length failed for " << 
                                   entry->GetKey() <<" in " <<GetFileName());

               long currentPosition = Fp->tellg(); // Only for gdcm-JPEG-LossLess3a.dcm-like
               Fp->seekg(0L,std::ios::end);        // Only for gdcm-JPEG-LossLess3a.dcm-like

               long lengthUntilEOF = (long)(Fp->tellg())-currentPosition; // Only for gdcm-JPEG-LossLess3a.dcm-like
               Fp->seekg(currentPosition, std::ios::beg);                 // Only for gdcm-JPEG-LossLess3a.dcm-like

               entry->SetReadLength(lengthUntilEOF);
               entry->SetLength(lengthUntilEOF);
               return;
            }
            entry->SetReadLength(lengthOB);
            entry->SetLength(lengthOB);
            return;
         }
         FixDocEntryFoundLength(entry, length32); 
         return;
      }
      // Length is encoded on 2 bytes.
      //length16 = ReadInt16();
      length16 = GetInt16();
      // 0xffff means that we deal with 'No Length' Sequence 
      //        or 'No Length' SQItem
      if ( length16 == 0xffff) 
      {           
         length16 = 0;
      }
      FixDocEntryFoundLength( entry, (uint32_t)length16 );
      return;
   }
   else
   {
      // Either implicit VR or a non DICOM conformal (see note below) explicit
      // VR that ommited the VR of (at least) this element. Farts happen.
      // [Note: according to the part 5, PS 3.5-2001, section 7.1 p25
      // on Data elements "Implicit and Explicit VR Data Elements shall
      // not coexist in a Data Set and Data Sets nested within it".]
      // Length is on 4 bytes.

     // Well ... group 0002 is always coded in 'Explicit VR Litle Endian'
     // even if Transfer Syntax is 'Implicit VR ...'
     // --> Except for 'Implicit VR Big Endian Transfer Syntax GE Private' 
     //     where Group 0x0002 is *also* encoded in Implicit VR !

      FixDocEntryFoundLength( entry, GetInt32() /*ReadInt32()*/ );
      return;
   }
}

/**
 * \brief  Find the Length till the next sequence delimiter
 * \warning NOT end user intended method !
 * @return 
 */
uint32_t Document::FindDocEntryLengthOBOrOW()
   throw( FormatUnexpected )
{
   // See PS 3.5-2001, section A.4 p. 49 on encapsulation of encoded pixel data.
   
   long positionOnEntry = Fp->tellg(); // Only for OB,OW DataElements

   bool foundSequenceDelimiter = false;
   uint32_t totalLength = 0;

   while ( !foundSequenceDelimiter )
   {
      uint16_t group;
      uint16_t elem;

      try
      {  ///\todo make sure there is never OL encoded pixel data!
      
         //group = ReadInt16(); // Once per fragment (if any) of OB,OW DataElements
         //elem  = ReadInt16(); // Once per fragment (if any) of OB,OW DataElements 
         ReadBegBuffer(4); // Once per fragment (if any) of OB,OW DataElements
      }
      catch ( FormatError )
      {
         throw FormatError("Unexpected end of file encountered during ",
                           "Document::FindDocEntryLengthOBOrOW()");
      }
      group = GetInt16();
      elem  = GetInt16();

      // We have to decount the group and element we just read
      totalLength += 4;     
      if ( group != 0xfffe || ( ( elem != 0xe0dd ) && ( elem != 0xe000 ) ) )
      {
         gdcmWarningMacro( 
              "Neither an Item tag nor a Sequence delimiter tag on :" 
           << std::hex << group << "|" << elem << ") Pos. on entry was 0x(" <<positionOnEntry<< ") "
            );
  
         Fp->seekg(positionOnEntry, std::ios::beg); // Once per fragment (if any) of OB,OW DataElements
         throw FormatUnexpected( 
               "Neither an Item tag nor a Sequence delimiter tag.");
      }
      if ( elem == 0xe0dd )
      {
         foundSequenceDelimiter = true;
      }
      uint32_t itemLength = ReadInt32(); // Once per fragment (if any) of OB,OW DataElements
      // We add 4 bytes since we just read the ItemLength with ReadInt32
      totalLength += itemLength + 4;
      SkipBytes(itemLength);
      
      if ( foundSequenceDelimiter )
      {
         break;
      }
   }
   Fp->seekg( positionOnEntry, std::ios::beg); // Only once for OB,OW DataElements
   return totalLength;
}

/**
 * \brief     Find the Value Representation of the current Dicom Element.
 * @return    Value Representation of the current Entry
 */
VRKey Document::FindDocEntryVR()
{
   if ( Filetype != ExplicitVR )
   {
      return GDCM_VRUNKNOWN;
   }

   // Delimiters (0xfffe), are not explicit VR ... 
   if ( CurrentGroup == 0xfffe )
      return GDCM_VRUNKNOWN;

   //long positionOnEntry;
   //if( Debug::GetWarningFlag() )
   //  positionOnEntry = Fp->tellg(); // Only in Warning Mode

   // Warning: we believe this is explicit VR (Value Representation) because
   // we used a heuristic that found "UL" in the first tag and/or
   // 'Transfer Syntax' told us it is.
   // Alas this doesn't guarantee that all the tags will be in explicit VR. 
   // In some cases one finds implicit VR tags mixed within an explicit VR file
   // Well...
   // 'Normaly' the only case is : group 0002 Explicit, and other groups Implicit
   //
   // Hence we make sure the present tag is in explicit VR and try to fix things
   // if it happens not to be the case.

   VRKey vr;
   //Fp->read(&(vr[0]),(size_t)2);
   vr[0] = *PtrBegBuffer++;
   vr[1] = *PtrBegBuffer++;

   //if ( !CheckDocEntryVR(vr) ) // avoid useless function call
   if ( !Global::GetVR()->IsValidVR(vr) )
   {

      gdcmWarningMacro( "Unknown VR " << vr.GetHexaRepresentation() << std::hex
                        << " at offset : 0x(" << CurrentOffsetPosition-4
                        << ") for group " << std::hex << CurrentGroup );

      //Fp->seekg(positionOnEntry, std::ios::beg); //JPRx
      //Fp->seekg((long)-2, std::ios::cur);// only for unrecognized VR (?!?)
                                         //see :MR_Philips_Intera_PrivateSequenceExplicitVR.dcm
      PtrBegBuffer-=2;
      return GDCM_VRUNKNOWN;
   }
   return vr;
}

/**
 * \brief     Check the correspondance between the VR of the header entry
 *            and the taken VR. If they are different, the header entry is 
 *            updated with the new VR.
 * @param     vr    Dicom Value Representation
 * @return    false if the VR is incorrect or if the VR isn't referenced
 *            otherwise, it returns true
*/
bool Document::CheckDocEntryVR(const VRKey &vr)
{
   return Global::GetVR()->IsValidVR(vr);
}

/**
 * \brief   Skip a given Header Entry 
 * @param   entry entry to skip
 */
void Document::SkipDocEntry(DocEntry *entry) 
{
   SkipBytes(entry->GetLength());
}

/**
 * \brief   Skips to the beginning of the next Header Entry 
 * @param   currentDocEntry entry to skip
 */
void Document::SkipToNextDocEntry(DocEntry *currentDocEntry) 
{
   long l = currentDocEntry->GetReadLength();
   if ( (uint32_t) l == (uint32_t)-1 ) // length = 0xffff shouldn't appear here ...
                  // ... but PMS imagers happen !
      return;
   Fp->seekg((size_t)(currentDocEntry->GetOffset()), std::ios::beg); //FIXME :each DocEntry
   if (currentDocEntry->GetGroup() != 0xfffe)  // for fffe pb
   {
      Fp->seekg( l,std::ios::cur);                                 //FIXME :each DocEntry
   }
}

/**
 * \brief   When the length of an element value is obviously wrong (because
 *          the parser went Jabberwocky) one can hope improving things by
 *          applying some heuristics.
 * @param   entry entry to check
 * @param   foundLength first assumption about length (before bug fix, or set to zero if =0xffffffff)    
 */
void Document::FixDocEntryFoundLength(DocEntry *entry,
                                      uint32_t foundLength)
{
   entry->SetReadLength( foundLength );// will be updated only if a bug is found
   
   if ( foundLength == 0xffffffff)
   {
      //foundLength = 0;
      //entry->SetLength(foundLength);
      entry->SetLength(0);
      return;  // return ASAP; don't waist time on useless tests
   }

   uint16_t gr   = entry->GetGroup();
   uint16_t elem = entry->GetElement(); 
     
   if ( foundLength % 2)
   {
      gdcmWarningMacro( "Warning : Tag (" << std::hex << gr << "|" << elem << ") with uneven length " 
        << std::dec << foundLength << " 0x(" << std::hex << foundLength << ") "
        //<< " at offset x(" << offset << ")"
       );
   }

   //////// Fix for some naughty General Electric images.
   // Allthough not recent many such GE corrupted images are still present
   // on Creatis hard disks. Hence this fix shall remain when such images
   // are no longer in use (we are talking a few years, here)...
   // Note: XMedCon probably uses such a trick since it is able to read
   //       those pesky GE images ...
   if ( foundLength == 13)
   {
      // Only happens for this length !
      if ( gr != 0x0008 || ( elem != 0x0070 && elem != 0x0080 ) )
      {
         foundLength = 10;
         entry->SetReadLength(10); // a bug is to be fixed !?
      }
   }

   //////// Fix for some brain-dead 'Leonardo' Siemens images.
   // Occurence of such images is quite low (unless one leaves close to a
   // 'Leonardo' source. Hence, one might consider commenting out the
   // following fix on efficiency reasons.
   else if ( gr == 0x0009 && ( elem == 0x1113 || elem == 0x1114 ) )
   {
   // Ideally we should check we are in Explicit and double check
   // that VR=UL... this is done properly in gdcm2
      if( foundLength == 6 )
      {
         gdcmWarningMacro( "Replacing Length from 6 into 4" );
         foundLength = 4;
         entry->SetReadLength(4); // a bug is to be fixed !
      }
      else if ( foundLength%4 )
      {
        gdcmErrorMacro( "This looks like to a buggy Siemens DICOM file."
        "The length of this tag seems to be wrong" );
      }
   }

   else if ( entry->GetVR() == "SQ" )
   {
      foundLength = 0;      // ReadLength is unchanged
   }

   //////// We encountered a 'delimiter' element i.e. a tag of the form
   // "fffe|xxxx" which is just a marker. Delimiters length should not be
   // taken into account.
   else if ( gr == 0xfffe )
   {
     // According to the norm, fffe|0000 shouldn't exist. BUT the Philips
     // image gdcmData/gdcm-MR-PHILIPS-16-Multi-Seq.dcm happens to
     // causes extra troubles...
     if ( elem != 0x0000 )
     {
        foundLength = 0;
     }
     else
     {
        foundLength=12; // to skip the mess that follows this bugged Tag !
     }
   }
   entry->SetLength(foundLength);
}

/**
 * \brief   Apply some heuristics to predict whether the considered 
 *          element value contains/represents an integer or not.
 * @param   entry The element value on which to apply the predicate.
 * @return  The result of the heuristical predicate.
 */
bool Document::IsDocEntryAnInteger(DocEntry *entry)
{
   uint16_t elem         = entry->GetElement();
   uint16_t group        = entry->GetGroup();
   const VRKey &vr       = entry->GetVR();
   uint32_t length       = entry->GetLength();

   // When we have some semantics on the element we just read, and if we
   // a priori know we are dealing with an integer, then we shall be
   // able to swap its element value properly.
   if ( elem == 0 )  // This is the group length of the group
   {  
      if ( length == 4 )
      {
         return true;
      }
      else 
      {
         // Although this should never happen, still some images have a
         // corrupted group length [e.g. have a glance at offset x(8336) of
         // gdcmData/gdcm-MR-PHILIPS-16-Multi-Seq.dcm.
         // Since for dicom compliant and well behaved headers, the present
         // test is useless (and might even look a bit paranoid), when we
         // encounter such an ill-formed image, we simply display a warning
         // message and proceed on parsing (while crossing fingers).
         long filePosition = Fp->tellg(); // Only when elem 0x0000 length is not 4 (?!?)
         (void)filePosition;
         gdcmWarningMacro( "Erroneous Group Length element length  on : ("
           << std::hex << group << " , " << elem
           << ") -before- position x(" << filePosition << ")"
           << "lgt : " << length );
      }
   }

   if ( vr == "UL" || vr == "US" || vr == "SL" || vr == "SS" )
   {
      return true;
   }   
   return false;
}

/**
 * \brief   Discover what the swap code is (among little endian, big endian,
 *          bad little endian, bad big endian).
 *          sw is set
 * @return false when we are absolutely sure 
 *               it's neither ACR-NEMA nor DICOM
 *         true  when we hope ours assuptions are OK
 */
bool Document::CheckSwap()
{   
   uint32_t  s32;
   uint16_t  s16;
       
   char deb[256];
    
   // First, compare HostByteOrder and NetworkByteOrder in order to
   // determine if we shall need to swap bytes (i.e. the Endian type).
   bool net2host = Util::IsCurrentProcessorBigEndian();
         
   // The easiest case is the one of a 'true' DICOM header, we just have
   // to look for the string "DICM" inside the file preamble.
   Fp->read(deb, 256);
   
   char *entCur = deb + 128;
   if ( memcmp(entCur, "DICM", (size_t)4) == 0 )
   {
      gdcmDebugMacro( "Looks like DICOM Version3 (preamble + DCM)" );
      
      // Group 0002 should always be VR, and the first element 0000
      // Let's be carefull (so many wrong headers ...)
      // and determine the value representation (VR) : 
      // Let's skip to the first element (0002,0000) and check there if we find
      // "UL"  - or "OB" if the 1st one is (0002,0001) -,
      // in which case we (almost) know it is explicit VR.
      // WARNING: if it happens to be implicit VR then what we will read
      // is the length of the group. If this ascii representation of this
      // length happens to be "UL" then we shall believe it is explicit VR.
      // We need to skip :
      // * the 128 bytes of File Preamble (often padded with zeroes),
      // * the 4 bytes of "DICM" string,
      // * the 4 bytes of the first tag (0002, 0000),or (0002, 0001)
      // i.e. a total of  136 bytes.
      entCur = deb + 136;
     
      // group 0x0002 *is always* Explicit VR Sometimes,
      // even if elem 0002,0010 (Transfer Syntax) tells us the file is
      // *Implicit* VR  (see former 'gdcmData/icone.dcm')
      
      if ( memcmp(entCur, "UL", (size_t)2) == 0 ||
           memcmp(entCur, "OB", (size_t)2) == 0 ||
           memcmp(entCur, "UI", (size_t)2) == 0 ||
           memcmp(entCur, "CS", (size_t)2) == 0 )  // CS, to remove later
                                                   // when Write DCM *adds*
      // FIXME
      // Use Document::dicom_vr to test all the possibilities
      // instead of just checking for UL, OB and UI !? group 0000 
      {
         Filetype = ExplicitVR;
         gdcmDebugMacro( "Group 0002 : Explicit Value Representation");
      } 
      else 
      {
         Filetype = ImplicitVR;
         gdcmWarningMacro( "Group 0002 :Not an explicit Value Representation;"
                        << "Looks like a bugged Header!");
      }
      
      // Here, we assume that the file IS kosher Dicom !
      // (The meta elements - group 0x0002 - ARE little endian !)
      if ( net2host )
      {
         SwapCode = 4321;
         gdcmDebugMacro( "HostByteOrder != NetworkByteOrder, SwapCode = 4321");
      }
      else 
      {
         SwapCode = 1234;
         gdcmDebugMacro( "HostByteOrder = NetworkByteOrder, SwapCode = 1234");
      }
      
      // Position the file position indicator at first tag 
      // (i.e. after the file preamble and the "DICM" string).

      Fp->seekg ( 132L, std::ios::beg); // Once per Document
      CurrentOffsetPosition = 132;
      return true;
   } // ------------------------------- End of DicomV3 ----------------

   // Alas, this is not a DicomV3 file and whatever happens there is no file
   // preamble. We can reset the file position indicator to where the data
   // is (i.e. the beginning of the file).

   gdcmWarningMacro( "Not a Kosher DICOM Version3 file (no preamble)");

   Fp->seekg(0, std::ios::beg); // Once per ACR-NEMA Document
   CurrentOffsetPosition = 0;
   // Let's check 'No Preamble Dicom File' :
   // Should start with group 0x0002
   // and be Explicit Value Representation

   s16 = *((uint16_t *)(deb));
   SwapCode = 0;     
   switch ( s16 )
   {
      case 0x0002 :
         SwapCode = 1234;
         entCur = deb + 4;
         break;
      case 0x0200 :
         SwapCode = 4321;
         entCur = deb + 6;
    } 

   if ( SwapCode != 0 )
   {
      if ( memcmp(entCur, "UL", (size_t)2) == 0 ||
           memcmp(entCur, "OB", (size_t)2) == 0 ||
           memcmp(entCur, "UI", (size_t)2) == 0 ||
           memcmp(entCur, "SH", (size_t)2) == 0 ||
           memcmp(entCur, "AE", (size_t)2) == 0 ||
           memcmp(entCur, "OB", (size_t)2) == 0 )
         {
            Filetype = ExplicitVR;  // FIXME : not enough to say it's Explicit
                                    // Wait untill reading Transfer Syntax
            gdcmDebugMacro( "Group 0002 : Explicit Value Representation");
            return true;
          }
    }
// ------------------------------- End of 'No Preamble' DicomV3 -------------

   // Our next best chance would be to be considering a 'clean' ACR/NEMA file.
   // By clean we mean that the length of the first group is written down.
   // If this is the case and since the length of the first group HAS to be
   // four (bytes), then determining the proper swap code is straightforward.

   entCur = deb + 4;
   // We assume the array of char we are considering contains the binary
   // representation of a 32 bits integer. Hence the following dirty
   // trick :
   s32 = *((uint32_t *)(entCur));
   switch( s32 )
   {
      case 0x00040000 :
         SwapCode = 3412;
         Filetype = ACR;
         return true;
      case 0x04000000 :
         SwapCode = 4321;
         Filetype = ACR;
         return true;
      case 0x00000400 :
         SwapCode = 2143;
         Filetype = ACR;
         return true;
      case 0x00000004 :
         SwapCode = 1234;
         Filetype = ACR;
         return true;
      default :
         // We are out of luck. It is not a DicomV3 nor a 'clean' ACR/NEMA file.
         // It is time for despaired wild guesses. 
         // So, let's check if this file wouldn't happen to be 'dirty' ACR/NEMA,
         //  i.e. the 'group length' element is not present :     
         
         //  check the supposed-to-be 'group number'
         //  in ( 0x0001 .. 0x0008 )
         //  to determine ' SwapCode' value .
         //  Only 0 or 4321 will be possible 
         //  (no oportunity to check for the formerly well known
         //  ACR-NEMA 'Bad Big Endian' or 'Bad Little Endian' 
         //  if unsuccessfull (i.e. neither 0x0002 nor 0x0200 etc-3, 4, ..., 8-)
         //  the file IS NOT ACR-NEMA nor DICOM V3
         //  Find a trick to tell it the caller...
      
         s16 = *((uint16_t *)(deb));

         gdcmDebugMacro("not a DicomV3 nor a 'clean' ACR/NEMA;"
                     << " (->despaired wild guesses !)");
         switch ( s16 )
         {
            case 0x0001 :
            case 0x0002 :
            case 0x0003 :
            case 0x0004 :
            case 0x0005 :
            case 0x0006 :
            case 0x0007 :
            case 0x0008 :
            case 0x0028 :
               SwapCode = 1234;
               // Brute hack to allow reading DICOM RT files
               //Filetype = ACR;  // DICOM RT are *not* ACR-Nema files!
               Filetype = ExplicitVR;
               return true;
            case 0x0100 :
            case 0x0200 :
            case 0x0300 :
            case 0x0400 :
            case 0x0500 :
            case 0x0600 :
            case 0x0700 :
            case 0x0800 :
            case 0x2800 :
               SwapCode = 4321;
               Filetype = ACR;
               return true;
            default :

               s16 = *((uint16_t *)(deb));
               if (s16 != 0x0000)
                   return false;
               s16 = *((uint16_t *)(deb+2));

               Fp->seekg ( 0L, std::ios::beg); // Once per Document
               CurrentOffsetPosition = 0;
               switch(s16)  // try an other trick!
                            // -> to be able to decode 0029|1010 DataElement
                            // -> and be not less cleaver than dcmdump ;-)
               {
                  case 0x0004 :
                     SwapCode = 1234; 
                     break;
                  case 0x0400 :
                     SwapCode = 3412;
                     break;
                  default:
                     gdcmWarningMacro("ACR/NEMA unfound swap info (Hopeless !)");
                     Filetype = Unknown;
                     return false;
               }
               // Check if next 2 bytes are a VR
               // Probabely something more time-consuming exists with std::string
               const char VRvalues[] = "AEASATCSDADTFLFDISLOLTPNSHSLSSSTTMUIULUSUTOBOWOLOFATUNSQRT";
               int nbVal = 29;
               const char *pt = VRvalues;
               for (int i=0;i<nbVal;i++)
               {
                  if(*(deb+4) == *pt++) {
                    if(*(deb+5) == *pt++) {
                       Filetype = ExplicitVR;
                       return true;
                    }
                    else {
                       pt++;
                    }
                 }
              }
              Filetype = ImplicitVR;
              return true;
         }
   }
}

/**
 * \brief Change the Byte Swap code. 
 */
void Document::SwitchByteSwapCode() 
{
   gdcmDebugMacro( "Switching Byte Swap code from "<< SwapCode
                     << " at: 0x" << std::hex << Fp->tellg() );  // Only when DEBUG
   if ( SwapCode == 1234 ) 
   {
      SwapCode = 4321;
   }
   else if ( SwapCode == 4321 ) 
   {
      SwapCode = 1234;
   }
   else if ( SwapCode == 3412 ) 
   {
      SwapCode = 2143;
   }
   else if ( SwapCode == 2143 )
   {
      SwapCode = 3412;
   }
   gdcmDebugMacro( " Into: "<< SwapCode );
}

/**
 * \brief  during parsing, Header Elements too long are not loaded in memory
 * @param newSize new size
 */
void Document::SetMaxSizeLoadEntry(long newSize)
{
   if ( newSize < 0 )
   {
      return;
   }
   if ((uint32_t)newSize >= (uint32_t)0xffffffff )
   {
      MaxSizeLoadEntry = 0xffffffff;
      return;
   }
   MaxSizeLoadEntry = newSize;
}

/**
 * \brief   Read the next tag WITHOUT loading it's value
 *          (read the 'Group Number', the 'Element Number',
 *          gets the Dict Entry
 *          gets the VR, gets the length, gets the offset value)
 * @return  On succes : the newly created DocEntry, NULL on failure.
 */
DocEntry *Document::ReadNextDocEntry()
{
   try
   {
      ReadBegBuffer(8); // Avoid to many time consuming freads
      //CurrentGroup = ReadInt16();
      //CurrentElem  = ReadInt16();
   }
   catch ( FormatError )
   {
      // We reached the EOF (or an error occured) therefore
      // header parsing has to be considered as finished.
      return 0;
   }

   changeFromUN = false;
   CurrentGroup = GetInt16();
   CurrentElem  = GetInt16();

   // In 'true DICOM' files Group 0002 is always little endian
   if ( HasDCMPreamble )
   {
      if ( !Group0002Parsed && CurrentGroup != 0x0002) // avoid calling a function when useless
         HandleOutOfGroup0002(CurrentGroup, CurrentElem);
      else
         // Sometimes file contains groups of tags with reversed endianess.
         HandleBrokenEndian(CurrentGroup, CurrentElem);
    }

   VRKey vr = FindDocEntryVR();
   VRKey realVR = vr;

   if ( vr == GDCM_VRUNKNOWN )
   {
      if ( CurrentElem == 0x0000 ) // Group Length
      {
         realVR = "UL";     // must be UL
      }
      else if (CurrentGroup == 0xfffe) // Don't get DictEntry for Delimitors
      {
         realVR = "UL";
      }

      // Was commented out in order not to generate 'Shadow Groups' where some 
      // Data Elements are Explicit VR and some other ones Implicit VR
      // -> Better we fix the problem at Write time
     
      else if (CurrentGroup%2 == 1 )
      {
         if (CurrentElem >= 0x0010 && CurrentElem <=0x00ff )
         {
            // DICOM PS 3-5 7.8.1 a) states that :
            // Private Creator Data Elements numbered (gggg,0010-00FF) (gggg is odd)
            // attributes have to be LO (Long String) and the VM shall be equal to 1
            realVR = "LO";
    
            // Seems not to be true
            // Still in gdcmtk, David Clunnie disagrees, Marco Eichelberg says it's OK ...
            // We let it for a while? 
            //(We should check length==4, for more security, but we don't have it yet !)
         }
         else if ( CurrentElem == 0x0001)
         {
            realVR = "UL"; // Private Group Length To End
         }
         else  // check the private dictionary for shadow elements when Implicit VR!
         {
            DictEntry *dictEntry = GetDictEntry(CurrentGroup,CurrentElem);
            if ( dictEntry )
            {
               realVR = dictEntry->GetVR(); 
               dictEntry->Unregister(); // GetDictEntry registered it
            }   
         }               
      }
      
      else
      {
         DictEntry *dictEntry = GetDictEntry(CurrentGroup,CurrentElem);//only when ImplicitVR
         if ( dictEntry )
         {
            realVR = dictEntry->GetVR();
            dictEntry->Unregister(); // GetDictEntry registered it 
         }
      }
   }

   // if UN found, let's check the dictionary, and trust it!
   // (maybe a private dictionary exists?)
   else if (vr == "UN")
   {
      DictEntry *dictEntry = GetDictEntry(CurrentGroup,CurrentElem);
      if ( dictEntry )
      {
         realVR = dictEntry->GetVR();
         dictEntry->Unregister(); // GetDictEntry registered it

         // for VR = "UN", length is always stored on 4 bytes.
         // remember this info, in order not to crash later
         changeFromUN=true;
         /// \todo : fixme If inside a vr = "UN" DataElement (but SQ according to a private dictionnary)
         ///         there is some more vr = "UN" DataElements, it will probabely fail.
         ///         --> find a -non time consuming- trick to store changeFromUN info at DataElement level,
         ///         not at the Document level.
         /// --> ?!? JPR
  
      }   
   }

   DocEntry *newEntry;
   //if ( Global::GetVR()->IsVROfSequence(realVR) )
   if (realVR == "SQ")
   {
      newEntry = NewSeqEntry(CurrentGroup, CurrentElem);
   }
   else
   {
      newEntry = NewDataEntry(CurrentGroup, CurrentElem, realVR);
      static_cast<DataEntry *>(newEntry)->SetState(DataEntry::STATE_NOTLOADED);
   }

   if ( vr == GDCM_VRUNKNOWN )
   {
      if ( Filetype == ExplicitVR )
      {
         // We thought this was explicit VR, but we end up with an
         // implicit VR tag. Let's backtrack.

         //if ( newEntry->GetGroup() != 0xfffe )
         if (CurrentGroup != 0xfffe)
         {
            int offset = Fp->tellg();//Only when heuristic for Explicit/Implicit was wrong

            gdcmWarningMacro("Entry (" << newEntry->GetKey() << ") at x("
                     <<  std::hex << offset << ") should be Explicit VR");
          }
      }
      newEntry->SetImplicitVR();
   }

   try
   {
      FindDocEntryLength(newEntry);
   }
   catch ( FormatError )
   {
      // Call it quits
      newEntry->Delete();
      return 0;
   }

   newEntry->SetOffset(Fp->tellg());  // for each DocEntry
   return newEntry;
}

/**
 * \brief   Handle broken private tag from Philips NTSCAN
 *          where the endianess is being switched to BigEndian 
 *          for no apparent reason
 * @return  no return
 */
void Document::HandleBrokenEndian(uint16_t &group, uint16_t &elem)
{
 // for strange PMS Gyroscan Intera images
 // Item 'starter' has a tag : 0x3f3f,0x3f00, for no apparent reason
 
 // --- Feel free to remove this test *on your own coy of gdcm*
 //     if you are sure you'll never face this problem.
 
   if ((group == 0x3f3f) && (elem == 0x3f00))
   {
     // start endian swap mark for group found
     gdcmDebugMacro( " delimiter 0x3f3f  found." );
     // fix the tag
     group = 0xfffe;
     elem  = 0xe000;
     return;
   }
   // --- End of removable code
   
   // Endian reversion. 
   // Some files contain groups of tags with reversed endianess.
   static int reversedEndian = 0;
   // try to fix endian switching in the middle of headers
   if ((group == 0xfeff) && (elem == 0x00e0))
   {
     // start endian swap mark for group found
     gdcmDebugMacro( "Start endian swap mark found." );
     reversedEndian++;
     SwitchByteSwapCode();
     // fix the tag
     group = 0xfffe;
     elem  = 0xe000;
   } 
   else if (group == 0xfffe && elem == 0xe00d && reversedEndian) 
   {
     // end of reversed endian group
     gdcmDebugMacro( "End of reversed endian." );
     reversedEndian--;
     SwitchByteSwapCode();
   }
   else if (group == 0xfeff && elem == 0xdde0) 
   {
     // reversed Sequence Terminator found
     // probabely a bug in the header !
     // Do what you want, it breaks !
     //reversedEndian--;
     //SwitchByteSwapCode();
     gdcmWarningMacro( "Should never get here! reversed Sequence Terminator!" );
     // fix the tag
      group = 0xfffe;
      elem  = 0xe0dd;  
   }
   else if (group == 0xfffe && elem == 0xe0dd) 
   {
      gdcmDebugMacro( "Straight Sequence Terminator." );  
   }
}

/**
 * \brief   Group 0002 is always coded Little Endian
 *          whatever Transfer Syntax is
 * @return  no return
 */
void Document::HandleOutOfGroup0002(uint16_t &group, uint16_t &elem)
{
   // Endian reversion. 
   // Some files contain groups of tags with reversed endianess.
   
      Group0002Parsed = true;
      // we just came out of group 0002
      // if Transfer Syntax is Big Endian we have to change CheckSwap

      std::string ts = GetTransferSyntax();
      TS::SpecialType s = Global::GetTS()->GetSpecialTransferSyntax(ts);

      // Group 0002 is always 'Explicit ...' 
      // even when Transfer Syntax says 'Implicit ..." 

      if ( s == TS::ImplicitVRLittleEndian 
        ||
          s == TS::ImplicitVRBigEndianPrivateGE
         )
      {
         Filetype = ImplicitVR;
      }
       
      // FIXME Strangely, this works with 
      //'Implicit VR BigEndian Transfer Syntax' (GE Private)
      //
      // --> Probabely normal, since we considered we never have 
      // to trust manufacturers.
      // (we often find 'Implicit VR' tag, 
      // even when Transfer Syntax tells us it's Explicit ...

       // NEVER trust the meta elements!
       // (see what ezDICOM does ...)

      /*
      if ( s ==  TS::ExplicitVRBigEndian )
      {
         gdcmDebugMacro("Transfer Syntax Name = [" 
                        << GetTransferSyntaxName() << "]" );
         SwitchByteSwapCode();
         group = SwapShort(group);
         elem  = SwapShort(elem);
      }
      */
    //-- Broken ACR  may start with a Shadow Group --
    // worse : some ACR-NEMA like files start 00028 group ?!? 
    if ( !( (group >= 0x0001 && group <= 0x0008) || group == 0x0028 ) )
    {
       // We trust what we see.
       SwitchByteSwapCode();
       group = SwapShort(group);
       elem  = SwapShort(elem); 
       // not what we where told (by meta elements) !
       gdcmDebugMacro("Transfer Syntax Name = ["       
                       << GetTransferSyntaxName() << "]" );
    }

      /// \todo  find a trick to warn user and stop processing

      if ( s == TS::DeflatedExplicitVRLittleEndian)
      {
           gdcmWarningMacro("Transfer Syntax [" 
                        << GetTransferSyntaxName() << "] :"
                        << " not yet dealt with ");
           return;
      }

      // The following shouldn't occur very often
      // Let's check at the very end.

      if ( ts == GDCM_UNKNOWN )
      {
         gdcmDebugMacro("True DICOM File, with NO Transfer Syntax (?!) " );
         return;
      }

      if ( !Global::GetTS()->IsTransferSyntax(ts) )
      {
         gdcmWarningMacro("True DICOM File, with illegal Transfer Syntax: [" 
                          << ts << "]");
         return;
      }
}

//-----------------------------------------------------------------------------
// Print

//-----------------------------------------------------------------------------
} // end namespace gdcm
