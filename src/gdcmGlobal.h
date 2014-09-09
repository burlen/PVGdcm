/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmGlobal.h,v $
  Language:  C++
  Date:      $Date: 2007/08/22 16:14:04 $
  Version:   $Revision: 1.13 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMGLOBAL_H_
#define _GDCMGLOBAL_H_

#include "gdcmCommon.h"

namespace GDCM_NAME_SPACE 
{
class DictSet;
class VR;
class TS;
class DictGroupName;
class DicomDirElement;
//-----------------------------------------------------------------------------
/**
 * \brief   This class contains all globals elements that might be
 *          instanciated only once (singletons).
 */
class Dict;
class GDCM_EXPORT Global
{
friend class DictSet; // to allow setting DefaultPubDict without 
                      // providing anybody an accesor !
public:
   Global();
   ~Global();

   /// \brief   returns a pointer to Dictionaries Table 
   static DictSet *GetDicts() { return Dicts;}
   /// \brief   returns a pointer to the 'Value Representation Table' 
   static VR *GetVR(){ return ValRes; }
   /// \brief   returns a pointer to the 'Transfer Syntax Table'
   static TS *GetTS(){ return TranSyn; }
   /// \brief   returns a pointer to the Group name correspondance table
   static DictGroupName *GetDictGroupName() { return GroupName; }
   /// \brief   returns a pointer to the DicomDir related elements Table 
   static DicomDirElement *GetDicomDirElements(){ return ddElem; }

private:
   /// Pointer to a container, holding _all_ the Dicom Dictionaries.
   static DictSet *Dicts;
   /// Pointer to a hash table containing the 'Value Representations'.
   static VR *ValRes;
   /// \brief Pointer to a hash table containing the Transfer Syntax codes 
   ///        and their english description 
   static TS *TranSyn; 
   /// \brief Pointer to a hash table containing the Group codes 
   ///        and their english name (from NIH) 
   static DictGroupName *GroupName; 
   /// \brief Pointer to the hash table containing the Dicom Elements necessary 
   ///        to describe each part of a DICOMDIR 
   static DicomDirElement *ddElem;
   /// pointer to the Default Public Dictionnary, redundantly store here, 
   /// in order not to acces the HTable every time!
   static Dict *DefaultPubDict; 
};
} // end namespace gdcm

//-----------------------------------------------------------------------------
#endif
