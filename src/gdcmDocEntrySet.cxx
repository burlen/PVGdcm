/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDocEntrySet.cxx,v $
  Language:  C++
  Date:      $Date: 2007/09/17 12:16:02 $
  Version:   $Revision: 1.76 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmDocEntrySet.h"

#include "gdcmDebug.h"
#include "gdcmCommon.h"
#include "gdcmDictSet.h"
#include "gdcmGlobal.h"
#include "gdcmDocEntry.h"
#include "gdcmSeqEntry.h"
#include "gdcmUtil.h"
#include "gdcmDataEntry.h"
#include "gdcmVR.h"

#if defined(__BORLANDC__)
   #include <mem.h> // for memset
#endif 

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
// Constructor / Destructor
DocEntrySet::DocEntrySet() 
{ 
   PreviousDocEntry = 0;
}
//-----------------------------------------------------------------------------
// Public
/**
 * \brief   Get the "std::string representable" value of the Dicom entry
 * @param   group  Group number of the searched tag.
 * @param   elem Element number of the searched tag.
 * @return  Corresponding element value when it exists,
 *          and the string GDCM_UNFOUND otherwise.
 */
std::string DocEntrySet::GetEntryString(uint16_t group, uint16_t elem)
{
   DataEntry *entry = dynamic_cast<DataEntry *>(GetDocEntry(group,elem));
   if ( entry )
   {
      if( entry->IsNotLoaded() )
         return GDCM_NOTLOADED;
      if( entry->IsUnfound() )
         return GDCM_UNFOUND;
      if( entry->IsUnread() )
         return GDCM_UNREAD;
      return entry->GetString();
   }
   return GDCM_UNFOUND;
}

/**
 * \brief   Gets (from Header) a 'non string' element value 
 * @param group   group number of the Entry 
 * @param elem  element number of the Entry
 * @return Pointer to the 'non string' area
 */
void *DocEntrySet::GetEntryBinArea(uint16_t group, uint16_t elem) 
{
   DataEntry *entry = GetDataEntry(group, elem);
   if ( entry )
      return entry->GetBinArea();
   return 0;
}

/**
 * \brief   Searches within the DocEntrySet
 *          for the value length of a given tag..
 * @param   group  Group number of the searched tag.
 * @param   elem Element number of the searched tag.
 * @return  Corresponding element length; -1 if not found
 */
int DocEntrySet::GetEntryLength(uint16_t group, uint16_t elem)
{
   DocEntry *entry = GetDocEntry(group, elem);
   if ( entry )
      return entry->GetLength();
   return -1;
}

/**
 * \brief  Same as DocEntrySet::GetDocEntry except it returns a result 
 *         only when the corresponding entry is of type DataEntry.
 * @param   group  Group number of the searched Dicom Element 
 * @param   elem Element number of the searched Dicom Element  
 * @return When present, the corresponding DataEntry. 
 */
DataEntry *DocEntrySet::GetDataEntry(uint16_t group, uint16_t elem)
{
   DocEntry *currentEntry = GetDocEntry(group, elem);
   if ( !currentEntry )
      return NULL;

   return dynamic_cast<DataEntry*>(currentEntry);
}

/**
 * \brief  Same as DocEntrySet::GetDocEntry except it returns a result
 *          only when the corresponding entry is of type SeqEntry.
 * @param   group  Group number of the searched Dicom Element 
 * @param   elem Element number of the searched Dicom Element  
 * @return When present, the corresponding SeqEntry. 
 */
SeqEntry *DocEntrySet::GetSeqEntry(uint16_t group, uint16_t elem)
{
   DocEntry *currentEntry = GetDocEntry(group, elem);
   if ( !currentEntry )
      return NULL;

   return dynamic_cast<SeqEntry*>(currentEntry);
}

/**
 * \brief   Accesses an existing DocEntry (i.e. a Dicom Element)
 *          through its (group, element) and modifies its content with
 *          the given value.
 * @param   content new value (string) to substitute with
 * @param   group  group number of the Dicom Element to modify
 * @param   elem element number of the Dicom Element to modify
 */
bool DocEntrySet::SetEntryString(std::string const &content, 
                                 uint16_t group, uint16_t elem) 
{
   DataEntry *entry = GetDataEntry(group, elem);
   if (!entry )
   {
      gdcmWarningMacro( "No corresponding DataEntry " << std::hex << group <<
                         "," << elem << " element (try promotion first).");
      return false;
   }
   return SetEntryString(content,entry);
}

/**
 * \brief   Accesses an existing DocEntry (i.e. a Dicom Element)
 *          through its (group, element) and modifies its content with
 *          the given value.
 * @param   content new value (void*  -> uint8_t*) to substitute with
 * @param   lgth new value length
 * @param   group  group number of the Dicom Element to modify
 * @param   elem element number of the Dicom Element to modify
 */
bool DocEntrySet::SetEntryBinArea(uint8_t *content, int lgth, 
                                  uint16_t group, uint16_t elem) 
{
   DataEntry *entry = GetDataEntry(group, elem);
   if (!entry )
   {
      gdcmWarningMacro( "No corresponding DataEntry " << std::hex << group <<
                        "," << elem << " element (try promotion first).");
      return false;
   }

   return SetEntryBinArea(content,lgth,entry);
} 

/**
 * \brief   Accesses an existing DocEntry (i.e. a Dicom Element)
 *          and modifies its content with the given value.
 * @param  content new value (string) to substitute with
 * @param  entry Entry to be modified
 */
bool DocEntrySet::SetEntryString(std::string const &content, DataEntry *entry)
{
   if (entry)
   {
      entry->SetString(content);
      return true;
   }
   return false;
}

/**
 * \brief   Accesses an existing DataEntry (i.e. a Dicom Element)
 *          and modifies its content with the given value.
 * @param   content new value (void*  -> uint8_t*) to substitute with
 * @param  entry Entry to be modified 
 * @param  lgth new value length
 */
bool DocEntrySet::SetEntryBinArea(uint8_t *content, int lgth, DataEntry *entry)
{
   if (entry)
   {
      entry->SetLength(lgth);
      entry->SetBinArea(content);  
      return true;
   }
   return false;
}

/**
 * \brief   Modifies the value of a given Doc Entry (Dicom Element)
 *          when it exists. Creates it with the given value when unexistant.
 * @param   value (string) Value to be set
 * @param   group   Group number of the Entry 
 * @param   elem  Element number of the Entry
 * @param   vr  V(alue) R(epresentation) of the Entry -if private Entry-
 * \return  pointer to the modified/created Header Entry (NULL when creation
 *          failed).
 */ 
DataEntry *DocEntrySet::InsertEntryString(std::string const &value, 
                                          uint16_t group, uint16_t elem,
                                          VRKey const &vr )
{
   DataEntry *dataEntry = 0;
   DocEntry *currentEntry = GetDocEntry( group, elem );
   VRKey localVR = vr;
   if (currentEntry)
   {
      dataEntry = dynamic_cast<DataEntry *>(currentEntry);

      // Verify the VR
      if ( dataEntry )
         if ( dataEntry->GetVR()!=vr )
            dataEntry = NULL;

      // if currentEntry doesn't correspond to the requested dataEntry
      if ( !dataEntry)
      {
         if ( !RemoveEntry(currentEntry) )
         {
            gdcmWarningMacro( "Removal of previous DocEntry failed.");
            return NULL;
         }
      }
   }
  
   else // the 'currentEntry' was not found
   {
      if ( vr == GDCM_VRUNKNOWN ) // user didn't specify a VR.
                                  //  Probabely he trusts the Dicom Dict !
      {
          DictEntry *e = 
            Global::GetDicts()->GetDefaultPubDict()->GetEntry(group, elem);
          if ( e )
          {
             localVR = e->GetVR();  
             e->Register(); // ?? JPRx
         }
      }
   }

   // Create a new dataEntry if necessary
   if ( !dataEntry )
   {
      dataEntry = NewDataEntry( group, elem, localVR );

      if ( !AddEntry(dataEntry) )
      {
         gdcmWarningMacro("AddEntry " << dataEntry->GetKey() 
                 << " failed although this is a creation.");
         dataEntry->Delete();
         return NULL;
      }
      dataEntry->Delete(); // ?!? JPRx
   }

   // Set the dataEntry value
   SetEntryString(value, dataEntry); // The std::string value
   return dataEntry;
}

/**
 * \brief   Modifies the value of a given Header Entry (Dicom Element)
 *          when it exists. Create it with the given value when unexistant.
 *          A copy of the binArea is made to be kept in the Document.
 * @param   binArea (binary) value to be set
 * @param   lgth length of the Bin Area we want to set
 * @param   group   Group number of the Entry 
 * @param   elem  Element number of the Entry
 * @param   vr  V(alue) R(epresentation) of the Entry -if private Entry-
 * \return  pointer to the modified/created Header Entry (NULL when creation
 *          failed).
 */
DataEntry *DocEntrySet::InsertEntryBinArea(uint8_t *binArea, int lgth, 
                                           uint16_t group, uint16_t elem,
                                           VRKey const &vr )
{
   DataEntry *dataEntry = 0;
   DocEntry *currentEntry = GetDocEntry( group, elem );

   // Verify the currentEntry
   if (currentEntry)
   {
      dataEntry = dynamic_cast<DataEntry *>(currentEntry);

      // Verify the VR
      if ( dataEntry )
         if ( dataEntry->GetVR()!=vr )
            dataEntry = NULL;

      // if currentEntry doesn't correspond to the requested dataEntry
      if ( !dataEntry)
      {
         if ( !RemoveEntry(currentEntry) )
         {
            gdcmWarningMacro( "Removal of previous DocEntry failed.");
            return NULL;
         }
      }
   }

   // Create a new dataEntry if necessary
   if ( !dataEntry)
   {
      dataEntry = NewDataEntry(group, elem, vr);

      if ( !AddEntry(dataEntry) )
      {
         gdcmWarningMacro( "AddEntry failed although this is a creation.");
         dataEntry->Delete();
         return NULL;
      }
      dataEntry->Delete();
   }

   // Set the dataEntry value
   uint8_t *tmpArea;
   if ( lgth>0 && binArea )
   {
      tmpArea = new uint8_t[lgth];
      memcpy(tmpArea,binArea,lgth);
   }
   else
   {
      tmpArea = 0;
   }
   if ( !SetEntryBinArea(tmpArea,lgth,dataEntry) )
   {
      if ( tmpArea )
      {
         delete[] tmpArea;
      }
   }
   return dataEntry;
}  

/**
 * \brief   Creates a new gdcm::SeqEntry and adds it to the current DocEntrySet.
 *          (remove any existing entry with same group,elem)
 * @param   group   Group number of the Entry 
 * @param   elem  Element number of the Entry
 * \return  pointer to the created SeqEntry (NULL when creation
 *          failed).
 */
SeqEntry *DocEntrySet::InsertSeqEntry(uint16_t group, uint16_t elem)
{
   SeqEntry *seqEntry = 0;
   DocEntry *currentEntry = GetDocEntry( group, elem );

   // Verify the currentEntry
   if ( currentEntry )
   {
      seqEntry = dynamic_cast<SeqEntry *>(currentEntry);

      // Verify the VR
      if ( seqEntry )
         seqEntry = NULL;

      // if currentEntry doesn't correspond to the requested seqEntry
      if ( !seqEntry )
      {
         if (!RemoveEntry(currentEntry))
         {
            gdcmWarningMacro( "Removal of previous DocEntry failed for ("
               <<std::hex << group << "|" << elem <<")" );
            return NULL;
         }
      }
   }
   // Create a new seqEntry if necessary
   if ( !seqEntry )
   {
      seqEntry = NewSeqEntry(group, elem);

      if ( !AddEntry(seqEntry) )
      {
         gdcmWarningMacro( "AddEntry failed although this is a creation for ("
            <<std::hex << group << "|" << elem <<")" );
         seqEntry->Delete();
         return NULL;
      }
      seqEntry->Delete();
   }
   // Remark :
   // SequenceDelimitationItem will be added at the end of the SeqEntry,
   // at write time
   return seqEntry;
} 
 
/**
 * \brief   Checks if a given Dicom Element exists within the DocEntrySet
 * @param   group   Group number of the searched Dicom Element 
 * @param   elem  Element number of the searched Dicom Element 
 * @return true is found
 */
bool DocEntrySet::CheckIfEntryExist(uint16_t group, uint16_t elem )
{
   return GetDocEntry(group,elem)!=NULL;
}


/**
 * \brief   Build a new DataEntry from all the low level arguments. 
 * @param   group Group number   of the new Entry
 * @param   elem  Element number of the new Entry
 * @param   vr    V(alue) R(epresentation) of the new Entry 
 * \remarks The user of this method must destroy the DataEntry when unused
 */
DataEntry *DocEntrySet::NewDataEntry(uint16_t group, uint16_t elem,
                                     VRKey const &vr) 
{

   DataEntry *newEntry = DataEntry::New(group, elem, vr);
   if (!newEntry) 
   {
      gdcmWarningMacro( "Failed to allocate DataEntry for ("
          <<std::hex << group << "|" << elem <<")" );
      return 0;
   }
   return newEntry;
}

/**
 * \brief   Build a new SeqEntry from all the low level arguments. 
 * @param   group Group   number of the new Entry
 * @param   elem  Element number of the new Entry
 * \remarks The user of this method must destroy the SeqEntry when unused
 */
SeqEntry* DocEntrySet::NewSeqEntry(uint16_t group, uint16_t elem) 
{
   //DictEntry *dictEntry = GetDictEntry(group, elem, "SQ");

   //SeqEntry *newEntry = SeqEntry::New( dictEntry );
   SeqEntry *newEntry = SeqEntry::New( group, elem );
   //dictEntry->Unregister(); // GetDictEntry register it
   if (!newEntry)
   {
      gdcmWarningMacro( "Failed to allocate SeqEntry for ("
         <<std::hex << group << "|" << elem <<")" );
      return 0;
   }     
   return newEntry;
}

//-----------------------------------------------------------------------------
// Protected
/**
 * \brief   Searches [both] the public [and the shadow dictionary (when they
 *          exist)] for the presence of the DictEntry with given
 *          group and element. The public dictionary has precedence on the
 *          shadow one(s), if any.
 * @param   group  Group number of the searched DictEntry
 * @param   elem Element number of the searched DictEntry
 * @return  Corresponding DictEntry when it exists, NULL otherwise.
 * \remarks The returned DictEntry is registered when existing
 */
DictEntry *DocEntrySet::GetDictEntry(uint16_t group,uint16_t elem) 
{
   DictEntry *found = 0;
   /// \todo store the DefaultPubDict somwhere, in order not to access the HTable
   ///       every time ! --> Done!
   Dict *pubDict = Global::GetDicts()->GetDefaultPubDict();
   if (!pubDict) 
   {
      gdcmWarningMacro( "We SHOULD have a default dictionary");
   }
   else
   {
      found = pubDict->GetEntry(group, elem);
      if( found )
         found->Register();
   }
   return found;
}

//-----------------------------------------------------------------------------
// Private

//-----------------------------------------------------------------------------
// Print

//-----------------------------------------------------------------------------
} // end namespace gdcm
