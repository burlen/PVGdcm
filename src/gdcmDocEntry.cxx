/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDocEntry.cxx,v $
  Language:  C++
  Date:      $Date: 2007/10/30 09:14:42 $
  Version:   $Revision: 1.96 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#include "gdcmDocEntry.h"
#include "gdcmDataEntry.h"
#include "gdcmTS.h"
#include "gdcmVR.h"
#include "gdcmGlobal.h"
#include "gdcmUtil.h"
#include "gdcmDebug.h"
#include "gdcmDictSet.h"
#include <iomanip> // for std::ios::left, ...
#include <fstream>

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------

// Constructor / Destructor
/**
 * \brief   Constructor from a given DocEntry
 * @param   group Group number
 * @param   elem Element number
 * @param   vr VR 
 */
DocEntry::DocEntry(uint16_t group, uint16_t elem, VRKey const &vr)
{
   ImplicitVR = false;
   DicomDict  = 0;   
   Offset     = 0 ; // To avoid further misprinting

   // init some variables
   ReadLength = 0;
   Length = 0;

   VR = vr;
   Key.SetGroupElem(group,elem);
}

/**
 * \brief   Destructor from a given DocEntry
 */
DocEntry::~DocEntry()
{
   if (DicomDict)
   {
      gdcmAssertMacro(DicomDict);
      DicomDict->Unregister();
   }
}

//-----------------------------------------------------------------------------
// Public
/**
 * \brief   Writes the common part of any DataEntry, SeqEntry
 * @param fp already open ofstream pointer
 * @param filetype type of the file (ACR, ImplicitVR, ExplicitVR, JPEG, JPEG2000...)
 */
void DocEntry::WriteContent(std::ofstream *fp, FileType filetype, bool insideMetaElements, bool insideSequence)
{
   uint32_t ffff  = 0xffffffff;
   uint16_t group = GetGroup();

   ///\todo allow skipping Shadow groups 
 
   VRKey vr       = GetVR();
   uint16_t elem  = GetElement();
   uint32_t lgth  = GetLength();
  
   if ( group == 0xfffe && elem == 0x0000 )
   {
     // Fix in order to make some MR PHILIPS images e-film readable
     // see gdcmData/gdcm-MR-PHILIPS-16-Multi-Seq.dcm:
     // we just *always* ignore spurious fffe|0000 tag !   
      return;
   }

   //
   // ----------- Writes the common part
   //
    // To avoid gdcm to propagate oddities.
    // --> Don't forget to *write* an even length value   
   if (lgth%2)
      lgth ++;
   
 // ----------- Writes the common part : the Tag   
   binary_write( *fp, group); //group number
   binary_write( *fp, elem);  //element number

   // Dicom V3 group 0x0002 is *always* Explicit VR !
   if ( filetype == ExplicitVR || filetype == JPEG || filetype == JPEG2000 || (group == 0x0002 && insideMetaElements) )
   {
// ----------- Writes the common part : the VR + the length 
  
      // Special case of delimiters:
      if (group == 0xfffe)
      {   
         // Delimiters have NO Value Representation
         // Hence we skip writing the VR.
         //
         // In order to avoid further troubles, we choose to write them
         // as 'no-length' Item Delimitors (we pad by writing 0xffffffff)
         // We shall force the end of a given SeqItem by writting 
         //  a Item Delimitation Item (fffe, e00d)

         uint32_t ff = 0xffffffff;
         binary_write(*fp, ff);
         return;
      }
      uint16_t zero = 0;
      uint16_t shortLgr = (uint16_t)lgth;

      if( IsVRUnknown() )  
      {
      // if VR was not set by user, we set it to "UN"
         SetVR("UN");
         vr=GetVR();
      }      
     
      //{
         binary_write(*fp, vr.str());
         // See PS 3.5-2004 page 33, 36                  
         if ( (vr == "SQ") || (vr == "OB") || (vr == "OW") || (vr == "OL") || (vr == "OF") 
          ||  (vr == "UN") || (vr == "UT") )
         {
            binary_write(*fp, zero);

           if ( (filetype == JPEG || filetype == JPEG2000) && group == 0x7fe0 && elem == 0x0010 && !insideSequence)
            { 
              // Only the 'true' Pixel Element may be compressed (hope so!)
               binary_write(*fp, ffff);
            }  
            else if (vr == "SQ")
            {
               // we set SQ length to ffffffff
               // and  we shall write a Sequence Delimitor Item 
               // at the end of the Sequence! 
               binary_write(*fp, ffff);
            }
            else
            {
               binary_write(*fp, lgth);
            }
         }
         else
         {
            binary_write(*fp, shortLgr);
         }
      //}
   } 
   else // IMPLICIT VR 
   { 
// ----------- Writes the common part : the VR  
      if (vr == "SQ")
      {
         binary_write(*fp, ffff);
      }
      else
      {
         binary_write(*fp, lgth);
      }
   }
}

/// \brief Returns the 'Name' '(e.g. "Patient's Name") found in the Dicom
/// Dictionnary of the current Dicom Header Entry
std::string const &DocEntry::GetName() 
{ 
   if (DicomDict == 0)
      DicomDict =
                 Global::GetDicts()->GetDefaultPubDict()->GetEntry(Key[0],Key[1]);
   if (DicomDict == 0)
      return GDCM_UNKNOWN;
   else
   {
      DicomDict->Register();
      return DicomDict->GetName();
   }
}

   /// \brief Returns the 'Value Multiplicity' (e.g. "1", "6", "1-n", "3-n"),
   /// found in the Dicom entry or in the Dicom Dictionnary
   /// of the current Dicom entry
std::string const &DocEntry::GetVM()
{
   if (DicomDict == 0)
      DicomDict =
                 Global::GetDicts()->GetDefaultPubDict()->GetEntry(Key[0],Key[1]);
   if (DicomDict == 0)
      return GDCM_UNKNOWN;
   else
   {
      DicomDict->Register();
      return DicomDict->GetVM();
   }
}

/**
 * \brief   Gets the full length of the elementary DocEntry (not only value
 *          length) depending on the VR.
 */
uint32_t DocEntry::GetFullLength()
{
   uint32_t l = GetReadLength();
   if ( IsImplicitVR() )
   {
      l = l + 8;  // 2 (gr) + 2 (el) + 4 (lgth) 
   }
   else
   {
      if ( GetVR()=="OB" || GetVR()=="OW" || GetVR()=="OL" || GetVR()=="SQ" )
      {
         l = l + 12; // 2 (gr) + 2 (el) + 2 (vr) + 2 (unused) + 4 (lgth)
      }
      else
      {
         l = l + 8;  // 2 (gr) + 2 (el) + 2 (vr) + 2 (lgth)
      }
   }
   return l;
}

/**
 * \brief Copies all the attributes from an other DocEntry 
 * @param doc entry to copy from
 */
void DocEntry::Copy(DocEntry *doc)
{
   Length     = doc->Length;
   ReadLength = doc->ReadLength;
   ImplicitVR = doc->ImplicitVR;
   Offset     = doc->Offset;
}

//-----------------------------------------------------------------------------
// Protected

//-----------------------------------------------------------------------------
// Private

//-----------------------------------------------------------------------------
// Print
/**
 * \brief   Prints the common part (vr [length offset] name) of DataEntry, SeqEntry
 * @param   os ostream we want to print in
 * @param indent Indentation string to be prepended during printing
 */
void DocEntry::Print(std::ostream &os, std::string const & )
{
   size_t o;
   std::string st;
   TSKey v;
   std::string d2;
   VRKey vr;
   std::ostringstream s;
   uint32_t lgth;

   o  = GetOffset();
   vr = GetVR();
   if ( vr == GDCM_VRUNKNOWN )
      vr = "  ";

   s << DictEntry::TranslateToKey(GetGroup(),GetElement()); 

   if (PrintLevel >= 2)
   {
      s << " lg : ";
      lgth = GetReadLength(); // ReadLength, as opposed to (usable) Length
      if (lgth == 0xffffffff)
      {
         st = " ffff ";
         s.setf(std::ios::left);
         s << std::setw(4);  
         s << "    x(ffff) ";
         s.setf(std::ios::left);
         s << std::setw(8) << "-1"; 
      }
      else
      {
         st = Util::Format("x(%x)",lgth); // we may keep it
         s.setf(std::ios::left);
         s << std::setw(11-st.size()) << " ";
         s << st << " ";
         s.setf(std::ios::left);
         s << std::setw(8) << lgth; 
      }
      s << " Off.: ";
      st = Util::Format("x(%x)",o);  // we may keep it
      s << std::setw(11-st.size()) << " ";
      s << st << " ";
      s << std::setw(8) << o; 
   }
   //if (PrintLevel >= 1)
      s << " ";

   s << "[" << vr  << "] ";

   std::string name;
   uint16_t e = GetElement();
   if ( e == 0x0000 )
      name = "Group Length";
   else if ( GetGroup()%2 == 1 )
   {
      if ( e >= 0x0010 && e <= 0x00ff )
         name = "Private Creator";
      else if (e == 0x0001)
         name = "Private Group Length To End";
   }
   else
   {
      name = GetName();
      // prevent Print from any CR at end of name (hope it's enought!)
      if (name[name.length()-1] == 0x0d || name[name.length()-1] == 0x0a)
      {  
         name.replace(name.length()-1, 1, 1, ' ');
      }
   }
   if (PrintLevel >= 1)
   {
      s.setf(std::ios::left);
      s << std::setw(66-name.length()) << " ";
   }
    
   s << "[" << name << "]";
   os << s.str();      
}

//-----------------------------------------------------------------------------
} // end namespace gdcm
