/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDocEntrySet.h,v $
  Language:  C++
  Date:      $Date: 2007/09/17 12:16:02 $
  Version:   $Revision: 1.73 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMDOCENTRYSET_H_
#define _GDCMDOCENTRYSET_H_

#include "gdcmRefCounter.h"
#include "gdcmVRKey.h"
#include "gdcmTagKey.h"

#include <fstream>

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
class DocEntry;
class DataEntry;
class SeqEntry;
class DictEntry;

//-----------------------------------------------------------------------------
/**
 * \brief
 *  DocEntrySet is an abstract base class for ElementSet, SQItem
 *  which are both containers for DocEntries.
 *  -  ElementSet is based on the STL map<> container
 * (see  ElementSet::TagHT)
 *  -  SQItem is based on an STL list container (see  ListDocEntry).
 *
 * Since the syntax for adding a new element to a map<> or a list<>
 * differ,  DocEntrySet is designed as an adapter to unify the
 * interfaces of  DocEntrySet and  ElementSet.
 *
 * As an illustration of this design, please refer to the implementation
 * of  AddEntry (or any pure virtual method) in both derived classes.
 * This adapter unification of interfaces enables the parsing of a
 * DICOM header containing (optionaly heavily nested) sequences to be
 * written recursively [see  Document::ParseDES 
 * which calls  Document::ParseSQ, which in turn calls 
 *  Document::ParseDES ].
 *
 * \note Developpers should strongly resist to the temptation of adding
 *       members to this class since this class is designed as an adapter 
 *       in the form of an abstract base class.
 */
class GDCM_EXPORT DocEntrySet : public RefCounter
{
   gdcmTypeMacro(DocEntrySet);

public:
   /// \brief write any type of entry to the entry set
   virtual void WriteContent (std::ofstream *fp, FileType filetype,
                               bool insideMetaElements,bool insideSequence ) = 0;

   /// \brief Remove all Entry of the current set
   virtual void ClearEntry() = 0;
   /// \brief adds any type of entry to the current set
   virtual bool AddEntry(DocEntry *entry) = 0;
   /// \brief Removes any type of entry out of the entry set, and destroys it
   virtual bool RemoveEntry(DocEntry *entryToRemove) = 0;
   /// \brief Gets the first entry (of any type) of the current set
   virtual DocEntry *GetFirstEntry()=0;
   /// \brief Gets the next entry (of any type) of the current set
   virtual DocEntry *GetNextEntry()=0;

   virtual std::string GetEntryString(uint16_t group, uint16_t elem);
   virtual void *GetEntryBinArea(uint16_t group, uint16_t elem);
   virtual int GetEntryLength(uint16_t group, uint16_t elem);

   /// \brief Gets any type of DocEntry, identified by its (group,elem)
   virtual DocEntry *GetDocEntry(uint16_t group, uint16_t elem) = 0;
   /// \brief Gets a DataEntry, identified by its (group, elem)
   DataEntry *GetDataEntry(uint16_t group, uint16_t elem);
   /// \brief Gets a SeqEntry, identified by its (group,elem)
   SeqEntry *GetSeqEntry(uint16_t group, uint16_t elem);

   bool SetEntryString(std::string const &content,
                       uint16_t group, uint16_t elem);
   bool SetEntryBinArea(uint8_t *content, int lgth,
                        uint16_t group, uint16_t elem);
   bool SetEntryString(std::string const &content, DataEntry *entry);
   bool SetEntryBinArea(uint8_t *content, int lgth, DataEntry *entry);

   DataEntry *InsertEntryString(std::string const &value,
                                   uint16_t group, uint16_t elem,
                                   VRKey const &vr = GDCM_VRUNKNOWN);
   DataEntry *InsertEntryBinArea(uint8_t *binArea, int lgth,
                                    uint16_t group, uint16_t elem,
                                    VRKey const &vr = GDCM_VRUNKNOWN);
   SeqEntry *InsertSeqEntry(uint16_t group, uint16_t elem);
   /// \brief Tells us if the set contains no entry
   virtual bool IsEmpty() = 0;
   virtual bool CheckIfEntryExist(uint16_t group, uint16_t elem);

// DocEntry  related utilities 
   DataEntry *NewDataEntry(uint16_t group,uint16_t elem,
                         VRKey const &vr = GDCM_VRUNKNOWN);
   SeqEntry *NewSeqEntry(uint16_t group, uint16_t elem);

   virtual void Copy(DocEntrySet *) {};

protected:
   /// Canonical Constructor
   DocEntrySet();
   /// Canonical Destructor
   virtual ~DocEntrySet() {}

// DictEntry  related utilities
   DictEntry *GetDictEntry(uint16_t group, uint16_t elem);
 //  DictEntry *GetDictEntry(uint16_t group, uint16_t elem,
 //                          VRKey const &vr);
   /// To be able to backtrack (Private Sequence, Implicit VR related pb)
   DocEntry *PreviousDocEntry;

private:
};

} // end namespace gdcm
//-----------------------------------------------------------------------------
#endif

