/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDocEntry.h,v $
  Language:  C++
  Date:      $Date: 2007/09/17 12:16:02 $
  Version:   $Revision: 1.71 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMDOCENTRY_H_
#define _GDCMDOCENTRY_H_

#include "gdcmRefCounter.h"
#include "gdcmDictEntry.h"

#include <iostream>
#include <fstream>

namespace GDCM_NAME_SPACE 
{
class File;
class SeqEntry;

//-----------------------------------------------------------------------------
/**
 * \brief   The dicom header of a Dicom file contains a set of such entries
 *          (when successfuly parsed against a given Dicom dictionary)
 */
class GDCM_EXPORT DocEntry : public RefCounter
{
   gdcmTypeMacro(DocEntry);

public:
   virtual void Print (std::ostream &os = std::cout, std::string const &indent = ""); 
   virtual void WriteContent(std::ofstream *fp, FileType filetype, 
                             bool insideMetaElements, bool insideSequence);

   /// \brief  Gets the DicEntry of the current Dicom entry
   /// @return The DicEntry of the current Dicom entry
   DictEntry * GetDictEntry() { return DicomDict; } 

   /// Returns the Dicom Group number of the current Dicom entry
   uint16_t const &GetGroup() const   { return Key[0];  }
   //const uint16_t &GetGroup() const   { return DicomDict->GetGroup();  }

   /// Returns the Dicom Element number of the current Dicom entry
   uint16_t const &GetElement() const { return Key[1];}   
   //const uint16_t &GetElement() const { return DicomDict->GetElement();}

   /// Returns the 'key' of the current Dicom entry
   TagKey const &GetKey() const { return Key; }   
   //TagKey GetKey() const { return DicomDict->GetKey(); }

   /// \brief Returns the 'Name' '(e.g. "Patient's Name") found in the Dicom
   /// Dictionnary of the current Dicom Header Entry
   std::string const &GetName();

   /// \brief Returns the 'Value Representation' (e.g. "PN" : Person Name,
   /// "SL" : Signed Long), found in the Dicom header or in the Dicom
   /// Dictionnary, of the current Dicom entry
   VRKey const &GetVR() const { return VR; }   
   //VRKey const &GetVR() const { return DicomDict->GetVR(); }

   /// \brief Returns the 'Value Multiplicity' (e.g. "1", "6", "1-n", "3-n"),
   /// found in the Dicom entry or in the Dicom Dictionnary
   /// of the current Dicom entry
   std::string const &GetVM();

   /// Sets the 'Value Multiplicity' of the current Dicom entry
   //void SetVM( TagName const &v) { DicomDict->SetVM(v); } 
   void SetVM( TagName &) { std::cout << "-----------------FIXME : SetVM "; }
    
   /// \brief Returns offset (since the beginning of the file, including
   /// the File Preamble, if any) of the value of the current Dicom entry
   /// \warning offset of the *value*, not of the Dicom entry
   const size_t &GetOffset() const { return Offset; }

   /// \brief Sets only 'Read Length' (*not* 'Usable Length') of the current
   /// Dicom entry
   void SetReadLength(uint32_t l) { ReadLength = l; }
   /// \brief Returns the 'read length' of the current Dicom entry
   /// \warning this value is the one stored in the Dicom header but not
   ///          mandatoryly the one thats's used (in case of SQ, or delimiters,
   ///          the usable length is set to zero)
   const uint32_t &GetReadLength() const { return ReadLength; }

   /// \brief Sets both 'Read Length' and 'Usable Length' of the current
   /// Dicom entry
   virtual void SetLength(uint32_t l) { Length = l; }
   /// \brief Returns the actual value length of the current Dicom entry
   /// \warning this value is not *always* the one stored in the Dicom header
   ///          in case of well known bugs
   const uint32_t &GetLength() const { return Length; }

   uint32_t GetFullLength();
   virtual uint32_t ComputeFullLength() = 0;

// The following members, for internal use only !

   /// \brief   Sets the offset of the Dicom entry
   /// \warning use with caution !
   /// @param   of offset to be set
   void SetOffset(size_t of) { Offset = of; }

   /// Sets to TRUE the ImplicitVr flag of the current Dicom entry
   void SetImplicitVR() { ImplicitVR = true; }
 
    /// Sets the 'Value Representation' of the current Dicom entry
   /// @param   vr VR to be set    
   void SetVR( VRKey const &vr) { VR = vr; } 

    /// Sets the 'Value Representation' of the current Dicom entry
   /// @param   key TagKey to be set
   void SetTag( TagKey const &key) { Key = key; } 
  
// -----------end of members to be used with caution
   
   /// \brief Tells us if the current Dicom entry was checked as ImplicitVr
   /// @return true if the current Dicom entry was checked as ImplicitVr
   bool IsImplicitVR() const { return ImplicitVR; }

   /// \brief Tells us if the VR of the current Dicom entry is Unknown
   /// @return true if the VR is unknown
//   bool IsVRUnknown() const { return DicomDict->IsVRUnknown(); }
     bool IsVRUnknown() const { return VR == GDCM_VRUNKNOWN; }

   /// \brief Tells us if the VM of the current Dicom entry is Unknown
   /// @return true if the VM is unknown
//   bool IsVMUnknown() const { return DicomDict->IsVMUnknown(); }
   bool IsVMUnknown() { return GetVM() == GDCM_UNKNOWN; }

/// \brief   tells us if entry is the last one of a 'no length' SequenceItem 
///        (fffe,e00d) 
   bool IsItemDelimitor() 
                      {return (GetGroup() == 0xfffe && GetElement() == 0xe00d);}
///\brief   tells us if entry is the last one of a 'no length' Sequence 
///         (fffe,e0dd)       
   bool IsItemStarter(){ if (GetGroup() != 0xfffe) return false;
                         return (GetElement() == 0xe000); }
 /// \brief   tells us if entry is the last one of a 'no length' Sequence 
 ///          (fffe,e0dd) 
   bool IsSequenceDelimitor() { return (GetGroup() == 0xfffe && GetElement() == 0xe0dd);}  

   virtual void Copy(DocEntry *doc);

protected:
  // DocEntry(DictEntry*);
   DocEntry(uint16_t group, uint16_t elem, VRKey const &vr);
   virtual ~DocEntry();

   /// \brief pointer to the underlying Dicom dictionary element
   DictEntry *DicomDict;
      
   /// \brief Corresponds to the real length of the data
   /// This length might always be even
   uint32_t Length; 
  
   /// \brief Length to read in the file to obtain data
   uint32_t ReadLength;

   /// \brief Even when reading explicit vr files, some elements happen to
   /// be implicit. Flag them here since we can't use the entry->vr without
   /// breaking the underlying dictionary.
   bool ImplicitVR;

   /// Offset from the beginning of file for direct user access
   size_t Offset; 

   /// \brief Value Representation (to avoid accessing Dicom Dict every time!)
   VRKey VR; // JPRx
      
   /// \brief Dicom TagKey. Contains Dicom Group number and Dicom Element number
   ///        (to avoid accessing Dicom Dict every time !) // JPRx
   TagKey Key; // JPRx
private:

};
} // end namespace gdcm
//-----------------------------------------------------------------------------
#endif
