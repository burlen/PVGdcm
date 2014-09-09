/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmElementSet.h,v $
  Language:  C++
  Date:      $Date: 2007/09/17 12:16:02 $
  Version:   $Revision: 1.59 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMELEMENTSET_H_
#define _GDCMELEMENTSET_H_

#include "gdcmDocEntrySet.h"

#include <map>
#include <iostream>
#include <fstream>

namespace GDCM_NAME_SPACE 
{
typedef std::map<TagKey, DocEntry *> TagDocEntryHT;

//-----------------------------------------------------------------------------
/**
 * \brief
 *  ElementSet is based on the STL map<> container
 * (see  ElementSet::TagHT), as opposed to  SQItem
 * which is based on an STL list container (see  ListDocEntry).
 * It contains the 'zero-level- DocEntry (out of any Dicom Sequence)
 */
class GDCM_EXPORT ElementSet : public DocEntrySet
{
   gdcmTypeMacro(ElementSet);

public:
   virtual void Print(std::ostream &os = std::cout, 
                      std::string const &indent = "" ); 

   void WriteContent(std::ofstream *fp, FileType filetype,
                                    bool insideMetaElements, bool insideSequence); 

   bool AddEntry(DocEntry *Entry);
   bool RemoveEntry(DocEntry *EntryToRemove);
   void ClearEntry();
   
   DocEntry *GetFirstEntry();
   DocEntry *GetNextEntry();
   DocEntry *GetDocEntry(uint16_t group, uint16_t elem);
   /// Tells us if the ElementSet contains no entry
   bool IsEmpty() { return TagHT.empty(); }
   
   int IsVRCoherent(uint16_t group);

   virtual void Copy(DocEntrySet *set);

protected:
   ElementSet();
   ~ElementSet();
   /// \brief Some group are illegal withing some Dicom Documents
   ///        Only the Document knows it.
   bool MayIWrite(uint16_t )
                 { return true; }
private:
// Variables
   /// Hash Table (map), to provide fast access
   TagDocEntryHT TagHT;
   /// iterator, used to visit the TagHT variable
   TagDocEntryHT::iterator ItTagHT;
};
} // end namespace gdcm
//-----------------------------------------------------------------------------
#endif

