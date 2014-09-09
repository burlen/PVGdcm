/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmElementSet.cxx,v $
  Language:  C++
  Date:      $Date: 2008/05/19 09:25:42 $
  Version:   $Revision: 1.80 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#include "gdcmElementSet.h"
#include "gdcmDebug.h"
#include "gdcmSeqEntry.h"
#include "gdcmDataEntry.h"

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
// Constructor / Destructor
/**
 * \brief   Constructor for a given ElementSet
 */
ElementSet::ElementSet() 
          : DocEntrySet()
{
}

/**
 * \brief   Canonical destructor.
 */
ElementSet::~ElementSet() 
{
   ClearEntry();
}

//-----------------------------------------------------------------------------
// Public
/**
  * \brief   Writes the Header Entries (Dicom Elements)
  *          from the H Table
  * @param fp ofstream to write to  
  * @param filetype    ExplicitVR/ImplicitVR/ACR/ACR_LIBIDO/JPEG/JPEG2000/...
  */ 
void ElementSet::WriteContent(std::ofstream *fp, FileType filetype, bool dummy, bool dummy2)
{
   bool insideMetaElements     = false;
   bool yetOutsideMetaElements = false;
   (void)dummy2;(void)dummy;
   
   for (TagDocEntryHT::const_iterator i = TagHT.begin(); 
                                     i != TagHT.end(); 
                                    ++i)
   {
        int group = (i->second)->GetGroup();
       
       if (yetOutsideMetaElements==false && group == 0x0002)
          insideMetaElements = true;
    
       if (insideMetaElements == true && group != 0x0002)
       {
          yetOutsideMetaElements = true;
          insideMetaElements     = false;
       }
   
       // depending on the gdcm::Document type 
       // (gdcm::File; gdcm::DicomDir, (more to come ?)
       // some groups *cannot* be present.
       // We hereby protect gdcm for writting stupid things
       // if they were found in the original document. 
       if ( !MayIWrite( group ) )
          continue;
  
      // Skip 'Group Length' element, since it may be wrong.
      //       except for Group 0x0002
      // ( keep it as well for Group 0x0008 of ACR Files, 
      //  since some ACR readers *need* it )
      
       if ( (i->second)->GetElement() != 0x0000 
           || 
            (  (i->second)->GetGroup() == 0x0002 
             ||( (filetype == ACR || filetype == ACR_LIBIDO ) && (i->second)->GetGroup() == 0x0008 ) )
        )
       {           
             // There are DocEntries, written recursively
             // false : we are outside any Sequence
             i->second->WriteContent(fp, filetype, insideMetaElements, false );
       }             
   } 
}

/**
 * \brief   add a new Dicom Element pointer to the H Table
 * @param   newEntry entry to add
 */
bool ElementSet::AddEntry(DocEntry *newEntry)
{
   const TagKey &key = newEntry->GetKey();

   if ( TagHT.count(key) == 1 )
   {
      gdcmWarningMacro( "Key already present: " << key );
      return false;
   }
   else
   {
      TagHT.insert(TagDocEntryHT::value_type(newEntry->GetKey(), newEntry));
      newEntry->Register();
      return true;
   }
}

/**
 * \brief   Clear the hash table from given entry AND delete the entry.
 * @param   entryToRemove Entry to remove AND delete.
 */
bool ElementSet::RemoveEntry( DocEntry *entryToRemove)
{
   const TagKey &key = entryToRemove->GetKey();
   if ( TagHT.count(key) == 1 )
   {
      TagHT.erase(key);
      entryToRemove->Unregister();
      return true;
   }

   gdcmWarningMacro( "Key not present : " << key);
   return false ;
}

/**
 * \brief   delete all entries in the ElementSet
 */
void ElementSet::ClearEntry()
{
   for(TagDocEntryHT::iterator cc = TagHT.begin();cc != TagHT.end(); ++cc)
   {
      if ( cc->second )
      {
         cc->second->Unregister();
      }
   }
   TagHT.clear();
}

/**
 * \brief   Get the first entry while visiting *the* 'zero level' DocEntrySet
 *              (DocEntries out of any Sequence)
 * \return  The first DocEntry if found, otherwhise NULL
 */
DocEntry *ElementSet::GetFirstEntry()
{
   ItTagHT = TagHT.begin();
   if (ItTagHT != TagHT.end())
      return  ItTagHT->second;
   return NULL;
}

/**
 * \brief   Get the next entry while visiting *the* 'zero level' DocEntrySet
 *              (DocEntries out of any Sequence) 
 * \note : meaningfull only if GetFirstEntry already called 
 * \return  The next DocEntry if found, otherwhise NULL
 */
DocEntry *ElementSet::GetNextEntry()
{
   gdcmAssertMacro (ItTagHT != TagHT.end());

   ++ItTagHT;
   if (ItTagHT != TagHT.end())
      return  ItTagHT->second;
   return NULL;
}

/**
 * \brief  retrieves a Dicom Element using (group, element)
 * @param   group  Group number of the searched Dicom Element 
 * @param   elem Element number of the searched Dicom Element 
 * @return  
 */
DocEntry *ElementSet::GetDocEntry(uint16_t group, uint16_t elem) 
{
   TagKey key = DictEntry::TranslateToKey(group, elem);
   TagDocEntryHT::iterator it = TagHT.find(key);

   if ( it!=TagHT.end() )
      return it->second;
   return NULL;
}

/**
 * \brief Copies all the attributes from an other DocEntrySet 
 * @param set entry to copy from
 * @remarks The contained DocEntries a not copied, only referenced
 */
void ElementSet::Copy(DocEntrySet *set)
{
   // Remove all previous entries
   ClearEntry();

   DocEntrySet::Copy(set);

   ElementSet *eltSet = dynamic_cast<ElementSet *>(set);
   if( eltSet )
   {
      TagHT = eltSet->TagHT;
      for(ItTagHT = TagHT.begin();ItTagHT != TagHT.end();++ItTagHT)
      {
         (ItTagHT->second)->Register();
      }
   }
}

/**
 * \brief Checks whether *all* the DataEntries of the group have all
 *        the same type for VR (ImplicitVR or ExplicitVR) 
 * @param group group number to be checked
 * @return 1:ImplicitVR 2:ExplicitVR -1:NotCoherent 
 */
int ElementSet::IsVRCoherent( uint16_t group )
{
   uint16_t currentGroup;
   int codeVR = -1;
   int currentCodeVR;
   for(TagDocEntryHT::iterator cc = TagHT.begin();cc != TagHT.end(); ++cc)
   {
      currentGroup = cc->second->GetGroup();

      if ( currentGroup < group )
         continue;   
      if ( currentGroup > group )
         break;
      // currentGroup == group
      if (codeVR == -1)
      {
         if (cc->second->IsImplicitVR() )
            codeVR = 1;
         else 
            codeVR = 2;
         continue;
      }
      else
      {
         if (cc->second->IsImplicitVR() )
            currentCodeVR = 1; //Implicit
         else 
            currentCodeVR = 2; // Explicit  
  
         if ( currentCodeVR == codeVR )
           continue;
         else
            return -1;    // -1 : not coherent 
      }
   }   
   return codeVR;
}


//-----------------------------------------------------------------------------
// Protected

//-----------------------------------------------------------------------------
// Private

//-----------------------------------------------------------------------------
// Print
/**
  * \brief   Prints the Header Entries (Dicom Elements) from the H Table
  * @param os ostream to write to  
  * @param indent Indentation string to be prepended during printing
  */ 
void ElementSet::Print(std::ostream &os, std::string const & )
{
   // Let's change the 'warning value' for Pixel Data,
   // to avoid human reader to be confused by 'gdcm::NotLoaded'.   
   DataEntry *pixelElement = GetDataEntry(0x7fe0,0x0010);
   if ( pixelElement != 0 )
   {
      pixelElement->SetFlag( DataEntry::FLAG_PIXELDATA );
   }

   for( TagDocEntryHT::const_iterator i = TagHT.begin(); i != TagHT.end(); ++i)
   {
      DocEntry *entry = i->second;

      entry->SetPrintLevel(PrintLevel);
      entry->Print(os);   

      if ( dynamic_cast<SeqEntry*>(entry) )
      {
         // Avoid the newline for a sequence:
         continue;
      }
      os << std::endl;
   }
}

//-----------------------------------------------------------------------------
} // end namespace gdcm
