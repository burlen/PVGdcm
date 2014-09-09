/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDict.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:09 $
  Version:   $Revision: 1.87 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#include "gdcmDict.h"
#include "gdcmUtil.h"
#include "gdcmDebug.h"

#include <fstream>
#include <iostream>
#include <iomanip>

namespace GDCM_NAME_SPACE
{
//-----------------------------------------------------------------------------
/// \brief auto generated function, to fill up the Dicom Dictionnary,
///       if relevant file is not found on user's disk
void FillDefaultDataDict(Dict *d);

//-----------------------------------------------------------------------------
// Constructor / Destructor
/**
 * \brief   Constructor
 */
Dict::Dict( )
{
   Filename="";
}

/**
 * \brief   Constructor
 * @param   filename from which to build the dictionary.
 */
Dict::Dict(std::string const &filename)
{
   gdcmDebugMacro( "in Dict::Dict, filename =[" << filename << "]" );
   std::ifstream from( filename.c_str() );
   if ( !from )
   {
      gdcmWarningMacro( "Can't open dictionary" << filename.c_str());
      // Using default embeded one:
      FillDefaultDataDict( this );
   }
   else
   { 
      gdcmDebugMacro( "in Dict::Dict, DoTheLoadingJob filename =[" 
                    << filename << "]" );
      DoTheLoadingJob(from);
      Filename = filename;
   }
}

/**
 * \brief  Destructor 
 */
Dict::~Dict()
{
   ClearEntry();
}

//-----------------------------------------------------------------------------
// Public

/**
 * \brief   Add all the entries held in a source dictionary
 * \note it concerns only Private Dictionnary
 * @param   filename from which to build the dictionary.
 */
bool Dict::AddDict(std::string const &filename)
{
   std::ifstream from( filename.c_str() );
   if ( !from )
   {
      gdcmWarningMacro( "Can't open dictionary" << filename.c_str());
      return false;
   }
   else
   {
      DoTheLoadingJob(from);
      return true;
   }
}

/**
 * \brief   Removes from the current Dicom Dict all the entries held in a source dictionary
 * \note it concerns only Private Dictionnary
 * @param   filename from which we read the entries to remove.
 */
bool Dict::RemoveDict(std::string const &filename)
{
   std::ifstream from( filename.c_str() );
   if ( !from )
   {
      gdcmWarningMacro( "Can't open dictionary" << filename.c_str());
      return false;
   }
   else
   {
      uint16_t group;
      uint16_t elem;
      TagName vr;
      TagName vm;
      TagName name;

      while ( true )
      {
         from >> std::hex;
         from >> group;

         if (from.eof())
            break;

         from >> elem;
         from >> vr;
         from >> vm;
        // from >> std::ws;  //remove white space
         std::getline(from, name);
 
         RemoveEntry(group,elem);
      }
      from.close();
      return true;
   }
}

/**
 * \brief  adds a new Dicom Dictionary Entry 
 * @param   newEntry entry to add 
 * @return  false if Dicom Element already exists
 */
bool Dict::AddEntry(DictEntry *newEntry) 
{
   const TagKey &key = newEntry->GetKey();

   if ( KeyHt.count(key) == 1 )
   {
      gdcmErrorMacro( "Already present:" << key );
      return false;
   } 
   else 
   {
      newEntry->Register();
      KeyHt.insert( TagKeyHT::value_type(key, newEntry));
      return true;
   }
}

/*
 * \ brief  replaces an already existing Dicom Element by a new one
 * @ param   newEntry new entry (overwrites any previous one with same tag)
 * @ return  false if Dicom Element doesn't exist
 */
 
 /* seems to be useless
 
bool Dict::ReplaceEntry(DictEntry *newEntry) // seems to be useless
{
   const TagKey &key = newEntry->GetKey();
   if ( RemoveEntry(key) )
   {
      newEntry->Register();
      KeyHt.insert( TagKeyHT::value_type(key, newEntry));
      return true;
   } 
   return false;
}
*/

/**
 * \brief  removes an already existing Dicom Dictionary Entry,
 *         identified by its Tag
 * @param   key (group|element)
 * @return  false if Dicom Dictionary Entry doesn't exist
 */
bool Dict::RemoveEntry(TagKey const &key) 
{
   TagKeyHT::const_iterator it = KeyHt.find(key);
   if ( it != KeyHt.end() ) 
   {
      it->second->Unregister(); // delete the entry
      KeyHt.erase(key);         // remove pointer from HTable

      return true;
   } 
   else 
   {
      gdcmWarningMacro( "Unfound entry" << key );
      return false;
  }
}

/**
 * \brief  removes an already existing Dicom Dictionary Entry, 
 *          identified by its group,element number
 * @param   group   Dicom group number of the Dicom Element
 * @param   elem Dicom element number of the Dicom Element
 * @return  false if Dicom Dictionary Entry doesn't exist
 */
bool Dict::RemoveEntry(uint16_t group, uint16_t elem)
{
   return RemoveEntry(DictEntry::TranslateToKey(group, elem));
}

/**
 * \brief   Remove all Dicom Dictionary Entries
 */
void Dict::ClearEntry()
{
   // we assume all the pointed DictEntries are already cleaned-up
   // when we clean KeyHt.
   TagKeyHT::const_iterator it;

   for(it = KeyHt.begin();it!=KeyHt.end();++it)
      it->second->Unregister(); // delete the entry
   KeyHt.clear();               // remove all the entries from HTable

}

/**
 * \brief   Get the dictionary entry identified by a given tag ("group|element")
 * @param   key   tag of the searched entry 
 * @return  the corresponding dictionary entry when existing, NULL otherwise
 */
DictEntry *Dict::GetEntry(TagKey const &key)
{
   TagKeyHT::iterator it = KeyHt.find(key);
   if ( it == KeyHt.end() )
   {
      return 0;
   }
   return it->second;
}
/**
 * \brief   Get the dictionary entry identified by its "group" and "element")
 * @param   group  Group number of the searched entry.
 * @param   elem Element number of the searched entry.
 * @return  the corresponding dictionary entry when existing, NULL otherwise
 */
DictEntry *Dict::GetEntry(uint16_t group, uint16_t elem)
{
   TagKey key = DictEntry::TranslateToKey(group, elem);
   TagKeyHT::iterator it = KeyHt.find(key);
   if ( it == KeyHt.end() )
   {
      return 0;
   }
   return it->second;
}

/**
 * \brief   Get the first entry while visiting the Dict entries
 * \return  The first DicEntry if found, otherwhise NULL
 */
DictEntry *Dict::GetFirstEntry()
{
   ItKeyHt = KeyHt.begin();
   if ( ItKeyHt != KeyHt.end() )
      return ItKeyHt->second;
   return NULL;
}

/**
 * \brief   Get the next entry while visiting the Hash table (KeyHt)
 * \note : meaningfull only if GetFirstEntry already called
 * \return  The next DictEntry if found, otherwhise NULL
 */
DictEntry *Dict::GetNextEntry()
{
   gdcmAssertMacro (ItKeyHt != KeyHt.end());

   ++ItKeyHt;
   if (ItKeyHt != KeyHt.end())
      return ItKeyHt->second;
   return NULL;
}

//-----------------------------------------------------------------------------
// Protected

//-----------------------------------------------------------------------------
// Private
/**
 * \brief Add all the dictionary entries from an already open source file 
 * @param from input stream to read from.
 */
void Dict::DoTheLoadingJob(std::ifstream &from)
{
   if (!from)
      return;
      
   uint16_t group;
   uint16_t elem;
   VRKey vr;
   TagName vm;
   TagName name;

   DictEntry *newEntry;
   while ( true )
   {
      from >> std::hex;
      from >> group;      
      from >> elem;
      from >> vr;
      from >> vm;
      from >> std::ws;  //remove white space
      std::getline(from, name);
      
      if(from.eof()) {
         break;
      }
      
      newEntry = DictEntry::New(group, elem, vr, vm, name);
      AddEntry(newEntry);
      newEntry->Delete();
   }
   from.close();
}
//-----------------------------------------------------------------------------
// Print
/**
 * \brief   Print all the dictionary entries contained in this dictionary.
 *          Entries will be sorted by tag i.e. the couple (group, element).
 * @param   os The output stream to be written to.
 * @param indent Indentation string to be prepended during printing
 */
void Dict::Print(std::ostream &os, std::string const & )
{
   os << "Dict file name : [" << Filename << "]" << std::endl;
   std::ostringstream s;

   for (TagKeyHT::iterator tag = KeyHt.begin(); tag != KeyHt.end(); ++tag)
   {  
std::cout << tag->second->GetKey() << " " << tag->second->GetName() 
          << std::endl;
      s << "Entry : ";
      s << "(" << tag->second->GetKey() << ") = "
        << std::dec;
      s << tag->second->GetVR() << ", ";
      s << tag->second->GetVM() << ", ";
      s << tag->second->GetName() << "."  << std::endl;
     
   }
   os << s.str();

}

//-----------------------------------------------------------------------------
} // end namespace gdcm
