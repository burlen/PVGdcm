/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmVR.cxx,v $
  Language:  C++
  Date:      $Date: 2008/02/13 11:42:13 $
  Version:   $Revision: 1.63 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#include "gdcmVR.h"
#include "gdcmUtil.h"
#include "gdcmDictSet.h"
#include "gdcmDebug.h"

#include <fstream>
#include <iostream>
#include <string.h>

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
/// \brief auto generated function, to fill up the 'Value Representation'
///        Dictionnary, if relevant file is not found on user's disk
void FillDefaultVRDict(VRHT &vr);

//-----------------------------------------------------------------------------
// Constructor / Destructor
/**
 * \brief Constructor
 */
VR::VR() 
{
   std::string filename = DictSet::BuildDictPath() + DICT_VR;
   std::ifstream from(filename.c_str());
   if ( !from )
   {
      gdcmWarningMacro("Can't open dictionary " << filename.c_str());
      FillDefaultVRDict(vr);
   }
   else
   {
      char buff[1024];
      VRKey key;
      VRAtr name;
   
      while (!from.eof()) 
      {
         from >> std::ws;
         from.getline(buff, 1024, ' ');
         if( strcmp(buff,"") == 0)
            continue;

         key = buff;
         from >> std::ws;
         from.getline(buff, 1024, ';');
         name = buff;
   
         from >> std::ws;
         from.getline(buff, 1024, '\n');
   
         vr[key] = name;
      }
      from.close();
   }
}

/**
 * \brief Destructor
 */
VR::~VR()
{
   vr.clear();
}

//-----------------------------------------------------------------------------
// Public

/**
 * \brief   Simple predicate that checks whether the given argument
 *          corresponds to the Value Representation of a DataEntry .
 * @param   tested value representation to check for.
 */
bool VR::IsVROfBinaryRepresentable(VRKey const &tested)
{
   if ( IsVROfStringRepresentable(tested) )
      return false;

   if ( IsVROfSequence(tested) )
      return false;

   return true;
}

/**
 * \brief   Simple predicate that checks whether the given argument
 *          corresponds to the Value Representation of a
 *          'std::string representable' value.
 * @param   tested value representation to be checked.
 */
bool VR::IsVROfStringRepresentable(VRKey const &tested)
{
   return tested == "AE" ||
          tested == "AS" ||
          tested == "CS" ||
          tested == "DA" ||
          tested == "DS" ||
          tested == "DT" || 
          tested == "FL" ||
          tested == "FD" ||
          tested == "IS" || 
          tested == "LO" ||
          tested == "LT" ||
          tested == "PN" ||
          tested == "SH" ||
          tested == "SL" ||
          tested == "SS" ||
          tested == "ST" ||
          tested == "TM" ||
          tested == "UI" ||
          tested == "UL" ||
          tested == "US" ||
          tested == "UT";

   // Should be quicker
   // --> will *never* work : any rotten value would be considered as OK !
/*
   return tested != "OB" &&
          tested != "OW" &&
          tested != "OF" &&
          tested != "AT" && // Attribute Tag ?!? contain no printable character
          tested != "UN" && // UN is an actual VR !
          tested != "SQ" ;
*/
}
/// \brief returns the length of a elementary elem whose VR is passed
unsigned short VR::GetAtomicElementLength(VRKey const &tested)
{
   // Unsigned & signed short
   if( tested == "US" || tested == "SS" )
      return 2;
   // Unsigned & signed long
   if( tested == "UL" || tested == "SL" )
      return 4;
   // Float
   if( tested == "FL" )
      return 4;
   // Double
   if( tested == "FD" )
      return 8;
   // Word string
   if( tested == "OW" )
      return 2;
   // Float string
   if( tested == "OF" || tested == "OL")
      return 4;   
   return 1;
}

// VS6 need a single implementation in the dll
#if defined(_MSC_VER) && (_MSC_VER == 1200)
/// \brief checks is a supposed-to-be VR is a 'legal' one.
bool VR::IsValidVR(VRKey const &key)
{
  return vr.find(key) != vr.end();
#ifdef USECOMPLETELYUNTESTEDCODE
  // to avoid to search in a std::map of std::string,
  // since each std::string is 2 characters longs, we check the array
  // (29 char comparisons in the worst case!)
  
  // "RT" is an undocumented VR, found in some Siemens images.
  // We added it here to be able to read these images.
  // Dict/gdcmVR.dic, Document has to be updated, too.
  static const char VRvalues[] =
    "AEASATCSDADSDTFLFDISLOLTOBOWOLOFPNSHSLSQSSSTTMUIULUNUSUTRT";

  //int nbVal = strlen(VRvalues)/2; // save CPU time.
  int nbVal = 29;
 
  const char *pt = VRvalues;
  for (int i=0;i<nbVal;i++)
  {
     if(key[0] == *pt++) {
       if(key[1] == *pt++) {
          return true;
       }
     }  
     else {
        pt++;
     }        
  }
  return false;
#endif

}
#endif

//-----------------------------------------------------------------------------
// Protected

//-----------------------------------------------------------------------------
// Private

//-----------------------------------------------------------------------------
// Print
/**
 * \brief   Print all 
 * @param   os The output stream to be written to.
 */
void VR::Print(std::ostream &os,std::string const &) 
{
   for (VRHT::iterator it = vr.begin(); it != vr.end(); ++it)
   {
      os << "VR : " << it->first << " = " << it->second << std::endl;
   }
}

//-----------------------------------------------------------------------------
} // end namespace gdcm
