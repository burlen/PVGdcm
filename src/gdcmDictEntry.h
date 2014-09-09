/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDictEntry.h,v $
  Language:  C++
  Date:      $Date: 2007/09/17 12:16:02 $
  Version:   $Revision: 1.47 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMDICTENTRY_H_
#define _GDCMDICTENTRY_H_

#include "gdcmRefCounter.h"
#include "gdcmTagKey.h"
#include "gdcmVRKey.h"

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
class VRKey;
class TagKey;
/**
 * \brief
 * the DictEntry in an element contained by the Dict.
 * It contains :
 *  - the key referenced by the DICOM norm or the constructor (for private keys)
 *    i.e. the Group number
 *         the Element number
 *  - the VR (Value Representation)
 *  - the VM (Value Multiplicity)
 *  - the corresponding name in english
 */
class GDCM_EXPORT DictEntry : public RefCounter
{
   gdcmTypeMacro(DictEntry);

public:
   static DictEntry *New(uint16_t group, uint16_t elem,
             VRKey const &vr       = GDCM_VRUNKNOWN,
             TagName const &vm     = GDCM_UNKNOWN,
             TagName const &name   = GDCM_UNKNOWN);

// Print
   void Print(std::ostream &os = std::cout, std::string const &indent = "");

   /// \brief  Returns the Dicom Group Number
   /// @return the Dicom Group Number
   const uint16_t &GetGroup() const { return Tag[0]; }

   /// \brief  Returns the Dicom Element Number
   /// @return the Dicom Element Number
   const uint16_t &GetElement() const { return Tag[1]; }   

   /// \brief  Set the Dicom Value Representation 
   /// \param vr the Dicom Value Representation
   virtual void SetVR(VRKey const &vr) { VR = vr; }
   /// \brief  Returns the Dicom Value Representation 
   /// @return the Dicom Value Representation
   const VRKey &GetVR() const { return VR; }
   /// \brief tells if the V(alue) R(epresentation) is known (?!)
   /// @return 
   bool IsVRUnknown() const { return VR == GDCM_VRUNKNOWN; }

   const TagKey &GetKey() const { return Tag; }

// Key creation
   static TagKey TranslateToKey(uint16_t group, uint16_t elem);

   /// \brief   returns the VM field of the current DictEntry
   /// @return  The 'Value Multiplicity' field
   const TagName &GetVM() const { return VM; } 
   /// \brief  Set the VM field of the current DictEntry
   /// \param vm the'Value Multiplicity'
   virtual void SetVM(TagName const &vm) { VM = vm; }
   /// \brief tells if the V(alue) M(ultiplicity) is known (?!)
   /// @return 
   bool IsVMUnknown() const { return VM == GDCM_UNKNOWN; }

   /// \brief  Returns the Dicom Name of the current DictEntry
   ///         e.g. "Patient Name" for Dicom Tag (0x0010, 0x0010) 
   /// @return the Dicom Name
   const TagName &GetName() const { return Name; } 

protected:
   DictEntry(uint16_t group, uint16_t elem,
             VRKey const &vr       = GDCM_VRUNKNOWN,
             TagName const &vm     = GDCM_UNKNOWN,
             TagName const &name   = GDCM_UNKNOWN);

   ~DictEntry();
   
private:
   /// Dicom  TagKey. Contains Dicom Group number and Dicom Element number
   TagKey Tag;

   /// \brief Value Representation i.e. some clue about the nature
   ///        of the data represented e.g. 
   ///        - "FD" short for "Floating Point Double"(see VR)
   ///        - "PN" short for "Person Name"       
   VRKey VR;
   
   /// \brief Value Multiplicity (e.g. "1", "1-n", "2-n", "6")
   TagName VM; 

   /// \brief English name of the entry (e.g. "Patient's Name")                   
   TagName Name;      
};
} // end namespace gdcm
//-----------------------------------------------------------------------------
#endif
