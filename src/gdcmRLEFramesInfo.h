/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmRLEFramesInfo.h,v $
  Language:  C++
  Date:      $Date: 2007/09/17 12:16:01 $
  Version:   $Revision: 1.25 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/


#ifndef _GDCMRLEFRAMESINFO_H_
#define _GDCMRLEFRAMESINFO_H_

#include "gdcmRLEFrame.h"

#include <list>

namespace GDCM_NAME_SPACE 
{
/**
 * \brief Utility class for gathering the informations of the collection
 *        of RLE frame[s] (see  RLEFrame)  when handling
 *        "Encapsulated RLE Compressed Images" (see PS 3.5-2003 annex G). 
 *        Note: a classical image can be considered as the degenerated case
 *              of a multiframe image. In this case the collection is limited
 *              to a single individual frame.
 *        The informations on each frame are obtained during the pixel parsing
 *        of a gdcm::File (refer to
 *           File::ComputeRLEInfo() ).
 *        They shall be used when (if necessary) decoding the frames.
 *
 *        This class is simply a stl list<> of RLEFrame.
 */
class GDCM_EXPORT RLEFramesInfo
{
friend class PixelReadConvert;
friend class File;

private:
   ~RLEFramesInfo();
   void Print( std::ostream &os = std::cout, std::string indent = "" );
   bool DecompressRLEFile( std::ifstream *fp, uint8_t *subRaw, int xSize, 
                           int ySize, int zSize, int tSize, int bitsAllocated );
   bool ConvertRLE16BitsFromRLE8Bits( uint8_t *subRaw, int xSize, int ySize,
                                      int tSize, int numberOfFrames);

   void AddFrame(RLEFrame *frame);

   RLEFrame *GetFirstFrame();
   RLEFrame *GetNextFrame();

   typedef std::list<RLEFrame *> RLEFrameList;

   RLEFrameList Frames;
   RLEFrameList::iterator ItFrames;
};
} // end namespace gdcm

//-----------------------------------------------------------------------------
#endif
