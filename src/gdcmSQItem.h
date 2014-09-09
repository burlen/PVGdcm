/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmSQItem.h,v $
  Language:  C++
  Date:      $Date: 2007/09/17 12:16:01 $
  Version:   $Revision: 1.56 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#ifndef _GDCMSQITEM_H_
#define _GDCMSQITEM_H_

#include "gdcmDocEntrySet.h"
#include "gdcmElementSet.h"

#include <list>
#include <fstream>

namespace GDCM_NAME_SPACE 
{
class DocEntry;

//-----------------------------------------------------------------------------
typedef std::list<DocEntry *> ListDocEntry;

//-----------------------------------------------------------------------------
/**
 * \brief a SeqEntry is composed by a set of SQItems.
 *        Each SQItem is composed by a set of DocEntry
 *        A DocEntry may be a SeqEntry
 *        ... and so forth 
 */ 
class GDCM_EXPORT SQItem : public DocEntrySet 
{
   gdcmTypeMacro(SQItem);

public:
   static SQItem *New(int depthLevel) {return new SQItem(depthLevel);}

   virtual void Print(std::ostream &os = std::cout, 
                      std::string const &indent = "" ); 
   void WriteContent(std::ofstream *fp, FileType filetype,
                           bool insideMetaElements, bool insideSequence);
   uint32_t ComputeFullLength();

   bool AddEntry(DocEntry *Entry); // add to the List
   bool RemoveEntry(DocEntry *EntryToRemove);
   void ClearEntry();
  
   DocEntry *GetFirstEntry();
   DocEntry *GetNextEntry();

   DocEntry *GetDocEntry(uint16_t group, uint16_t elem);

   bool IsEmpty() { return DocEntries.empty(); }

   /// \brief   returns the ordinal position of a given SQItem
   int GetSQItemNumber() { return SQItemNumber; }
   /// \brief   Sets the ordinal position of a given SQItem
   void SetSQItemNumber(int itemNumber) { SQItemNumber = itemNumber; }

   ///  \brief Accessor on  SQDepthLevel.
   int GetDepthLevel() { return SQDepthLevel; }                                                                             

   ///  \brief Accessor on  SQDepthLevel.
   void SetDepthLevel(int depth) { SQDepthLevel = depth; }

   virtual void Copy(DocEntrySet *set);

protected:
   SQItem(int depthLevel);
   ~SQItem();

// Variables that need to be accessed in subclasses
   /// \brief Chained list of Doc Entries
   ListDocEntry DocEntries;
   /// Iterator, used to visit the entries
   ListDocEntry::iterator ItDocEntries;
  
private:
   /// \brief Sequences can be nested. This  SQDepthLevel represents
   ///        the level of the nesting of instances of this class.
   ///         SQDepthLevel and its  SeqEntry::SQDepthLevel
   ///        counterpart are only defined on printing purposes
   ///        (see  Print).
   int SQDepthLevel;

   /// \brief SQ Item ordinal number 
   int SQItemNumber;
};
} // end namespace gdcm
//-----------------------------------------------------------------------------
#endif
