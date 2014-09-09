/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmTagKey.h,v $
  Language:  C++
  Date:      $Date: 2007/08/22 16:14:05 $
  Version:   $Revision: 1.13 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMTAGKEY_H_
#define _GDCMTAGKEY_H_

#include "gdcmCommon.h"

#include <assert.h>
#include <iostream>
#include <iomanip> // for std::ios::left, ...
#include <stdio.h> // for ugly sprintf

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
class TagKey
{
public :
   TagKey(uint16_t group, uint16_t elem) { tag[0] = group;tag[1] = elem;}
   TagKey() { tag[0] = tag[1] = 0x0000;}

   friend std::ostream& operator<<(std::ostream& _os, const TagKey &_val);

   std::string str() const
   {
      char res[10];
      sprintf(res,"%04x|%04x",tag[0],tag[1]);
      return std::string(res);
   }
   ///\brief sets the Group Number for the TagKey
   void SetGroup(uint16_t group) { tag[0] = group; }
   uint16_t GetGroup() const { return tag[0]; }

   ///\brief sets the Element Number for the TagKey
   void SetElement(uint16_t elem) { tag[1] = elem; }   
   uint16_t GetElement() const { return tag[1]; }

   ///\brief sets the Group Number and Element Number for the TagKey
   void SetGroupElem(uint16_t group, uint16_t elem) 
                               { tag[0] = group;tag[1] = elem; }
   
   TagKey &operator=(const TagKey &_val)
   {
      tag[0] = _val.tag[0];
      tag[1] = _val.tag[1];
      return *this;
   }

   TagKey(const TagKey &_val)
   {
     tag[0] = _val[0];
     tag[1] = _val[1];
   }

   const uint16_t &operator[](const unsigned int &_id) const
   {
      assert(_id<2);
      return tag[_id];
   }
   const uint16_t &operator[](const unsigned int &_id)
   {
      assert(_id<2);
      return tag[_id];
   }

   bool operator==(const TagKey &_val) const
   {
      return tag[0] == _val.tag[0] && tag[1] == _val.tag[1];
   }

   bool operator!=(const TagKey &_val) const
   {
      return tag[0] != _val.tag[0] || tag[1] != _val.tag[1];
   }

   bool operator<(const TagKey &_val) const
   {
      return tag[0] < _val.tag[0] 
        || (tag[0] == _val.tag[0] && tag[1] < _val.tag[1]);
   }

private :
   uint16_t tag[2];
};

//-----------------------------------------------------------------------------
inline std::ostream& operator<<(std::ostream& _os, const TagKey &_val)
{
   _os.setf( std::ios::right);
   _os << std::hex << std::setw( 4 ) << std::setfill( '0' )
       << _val.tag[0] << '|' << std::setw( 4 ) << std::setfill( '0' )
       << _val.tag[1] << std::setfill( ' ' ) << std::dec;
   return _os;
}

//-----------------------------------------------------------------------------

} // end namespace gdcm

//-----------------------------------------------------------------------------
#endif
