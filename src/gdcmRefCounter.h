/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmRefCounter.h,v $
  Language:  C++
  Date:      $Date: 2007/10/04 17:50:42 $
  Version:   $Revision: 1.13 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMREFCOUNTER_H_
#define _GDCMREFCOUNTER_H_

#include "gdcmBase.h"
//#include "gdcmDebug.h"
#include <typeinfo>

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
/**
 * \brief Integration of reference counting with a destruction of the
 *        object only when the reference is to zero
 */
class GDCM_EXPORT RefCounter : public Base
{
   gdcmTypeMacro(RefCounter);

public:
// Allocator / Unallocator
   /// \brief Delete the object
   /// \remarks The object is deleted only if its reference counting is to zero
   void Delete() { Unregister(); }

// Reference count
   /// \brief Register the object
   /// \remarks It increments the reference counting
   void Register() { RefCount++; }

   /// \brief Unregister the object
   /// \remarks It decrements the reference counting
   void Unregister()
   {
//std::cout <<"================Unreg " << typeid(*this).name() << std::endl;
      RefCount--;
      if(RefCount<=0)
        delete this;
   }
   /// \brief Get the reference counting
   /// \return Reference count
   const unsigned long &GetRefCount() const
   {
      return RefCount;
   }

protected:
   /// Constructor
   RefCounter() : RefCount(1) { }
   /// Destructor
   virtual ~RefCounter() {}

private:
   /// \brief Reference count
   unsigned long RefCount;
};
} // end namespace gdcm

//-----------------------------------------------------------------------------
#endif
