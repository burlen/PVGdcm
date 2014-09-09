/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDictSet.h,v $
  Language:  C++
  Date:      $Date: 2007/08/22 16:14:04 $
  Version:   $Revision: 1.55 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMDICTSET_H
#define _GDCMDICTSET_H

#include "gdcmRefCounter.h"
#include "gdcmDict.h"
#include "gdcmGlobal.h"

#include <map>
#include <list>

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
typedef std::map<DictKey, Dict*> DictSetHT;

//-----------------------------------------------------------------------------
/**
 * \brief  Container for managing a set of loaded dictionaries (Dict).
 * \note   Hopefully, sharing dictionaries should avoid
 *         - reloading an already loaded dictionary (saving time)
 *         - having many in memory representations of the same dictionary
 *        (saving memory).
 */
class GDCM_EXPORT DictSet : public RefCounter
{
   gdcmTypeMacro(DictSet);

public:
/// \brief Contructs a DictSet with a RefCounter
   static DictSet *New() {return new DictSet();}

   void Print(std::ostream &os = std::cout, std::string const &indent = "" );

   // Probabely useless !
   //EntryNamesList *GetPubDictEntryNames();
   //EntryNamesByCatMap *GetPubDictEntryNamesByCategory();

   Dict *LoadDictFromFile( std::string const &fileName,
                           DictKey const &name );

   Dict *GetDict( DictKey const &DictName );

   /// \brief   Returns the default reference DICOM V3 public dictionary.
   //Dict* GetDefaultPubDict() { return GetDict(PUB_DICT_NAME); }
   Dict* GetDefaultPubDict() { return Global::DefaultPubDict; } 
   
   // \ brief   Returns the virtual references DICOM dictionary.
   // \ warning : not end user intended
   // Dict *GetVirtualDict() { return &VirtualEntries; }

   Dict *GetFirstDict();
   Dict *GetNextDict();

   static std::string BuildDictPath();

protected:
   DictSet();
   ~DictSet();

private:
   /// Hash table of all dictionaries contained in this DictSet
   DictSetHT Dicts;
   /// Iterator to visit the Dictionaries of a given DictSet
   DictSetHT::iterator ItDictHt;

   /// Directory path to dictionaries
   std::string DictPath;
};
} // end namespace gdcm

//-----------------------------------------------------------------------------
#endif
