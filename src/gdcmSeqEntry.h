/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmSeqEntry.h,v $
  Language:  C++
  Date:      $Date: 2007/09/17 12:16:01 $
  Version:   $Revision: 1.47 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMSQDOCENTRY_H_
#define _GDCMSQDOCENTRY_H_

#include "gdcmDocEntry.h"

#include <list>

namespace GDCM_NAME_SPACE 
{
class SQItem;
//-----------------------------------------------------------------------------
typedef std::list<SQItem *> ListSQItem;

//-----------------------------------------------------------------------------
/**
 * \brief a SeqEntry (as opposed to a DataEntry) is a non elementary DocEntry.
 *        It is composed by a set of SQItems.
 *        Each SQItem is composed by a set of DocEntry
 *        A DocEntry may be a SeqEntry
 *        ... and so forth 
 */ 
class GDCM_EXPORT SeqEntry : public DocEntry 
{
   gdcmTypeMacro(SeqEntry);

public:
/// \brief Contructs a SeqEntry with a RefCounter from DictEntry
   //static SeqEntry *New(DictEntry *e) {return new SeqEntry(e);}
/// \brief Contructs a SeqEntry with a RefCounter from DocEntry
   static SeqEntry *New(DocEntry *d, int depth) {return new SeqEntry(d,depth);}
/// \brief Constructs a SeqEntry with a RefCounter from elementary values
   static SeqEntry *New(uint16_t group,uint16_t elem/*, VRKey const &vr = GDCM_VRUNKNOWN*/) 
                           {return new SeqEntry(group,elem);}
   
   void Print(std::ostream &os = std::cout, std::string const &indent = "" ); 
   void WriteContent(std::ofstream *fp, FileType filetype,
                     bool insideMetaElements, bool insideSequence);
   uint32_t ComputeFullLength();

   void AddSQItem(SQItem *it, int itemNumber);
   void ClearSQItem();
   SQItem *GetFirstSQItem();
   SQItem *GetNextSQItem();
   SQItem *GetSQItem(int itemNumber);
   unsigned int GetNumberOfSQItems();
      
   /// Sets the delimitor mode
   void SetDelimitorMode(bool dm) { DelimitorMode = dm; }
   /// Sets the Sequence Delimitation Item
   void SetDelimitationItem(DocEntry *e);

   /// Gets the Sequence Delimitation Item
   DocEntry *GetDelimitationItem() { return SeqTerm;}

   /// Gets the depth level
   int GetDepthLevel() const { return SQDepthLevel; }
   /// Sets the depth level of a Sequence Entry embedded in a SeQuence
   void SetDepthLevel(int depth) { SQDepthLevel = depth; }

   virtual void Copy(DocEntry *doc);

protected:

private:
   //SeqEntry( DictEntry *e);
   SeqEntry( DocEntry *d, int depth );
   SeqEntry( uint16_t group, uint16_t elem );
   ~SeqEntry();

// Variables
   /// If this Sequence is in delimitor mode (length =0xffffffff) or not
   bool DelimitorMode;
   
   /// Chained list of SQ Items
   ListSQItem Items;
   /// iterator on the SQItems of the current SeqEntry
   ListSQItem::iterator ItSQItem;

   /// sequence terminator item 
   DocEntry *SeqTerm;

   /// \brief Defines the depth level of this  SeqEntry inside
   ///        the (optionaly) nested sequences.  SQDepthLevel
   ///        and its  SQItem::SQDepthLevel counterpart
   ///        are only defined on printing purposes (see  Print).
   int SQDepthLevel;
};
} // end namespace gdcm
//-----------------------------------------------------------------------------
#endif

