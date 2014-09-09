/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmSerieHelper.cxx,v $
  Language:  C++
  Date:      $Date: 2010/04/09 15:38:18 $
  Version:   $Revision: 1.70 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                              
=========================================================================*/

#include "gdcmSerieHelper.h"
#include "gdcmDirList.h"
#include "gdcmFile.h"
//#include "gdcmDictEntry.h" // for TranslateToKey : no more !
#include "gdcmDebug.h"
#include "gdcmUtil.h"

#include <math.h>
#include <vector>
#include <algorithm>
#include <map>
#include <stdio.h>  //for sscanf

namespace GDCM_NAME_SPACE
{

//-----------------------------------------------------------------------------
// Constructor / Destructor
/**
 * \brief   Constructor from a given SerieHelper
 */
SerieHelper::SerieHelper()
{
   m_UseSeriesDetails = false;
   ClearAll();
   UserLessThanFunction = 0;
   DirectOrder = true;
   DropDuplicatePositions = false;   
}

/**
 * \brief   Canonical destructor.
 */
SerieHelper::~SerieHelper()
{
   ClearAll();
}

/**
 * \brief  Preventively, clear everything at constructor time.
 *         ( use it at destructor time.)
 */
void SerieHelper::ClearAll()
{
   // For all the 'Single SerieUID' Filesets that may already exist 
   FileList *l = GetFirstSingleSerieUIDFileSet();
   while (l)
   { 
      // For all the GDCM_NAME_SPACE::File of a File set
      for (GDCM_NAME_SPACE::FileList::iterator it  = l->begin();
                                    it != l->end(); 
                                  ++it)
      {
         (*it)->Delete(); // remove each entry
      }
      l->clear();
      delete l;     // remove the container
      l = GetNextSingleSerieUIDFileSet();
   }
   // Need to clear that too:
   SingleSerieUIDFileSetHT.clear();
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

// Public
/**
 * \brief add a GDCM_NAME_SPACE::File to the Fileset corresponding to its Serie UID
 * @param   filename Name of the file to deal with
 */
void SerieHelper::AddFileName(std::string const &filename)
{
   // Create a DICOM file
   File *header = File::New();
   header->SetLoadMode(LoadMode);
   header->SetFileName( filename ); 
   header->Load();

   if ( header->IsReadable() )
   {
      if ( !AddFile( header ) )
      {
         // at least one rule was unmatched we need to deallocate the file:
         header->Delete();
      }
   }
   else
   {
      gdcmWarningMacro("Could not read file: " << filename );
      header->Delete();
   }
}

/**
 * \brief add a GDCM_NAME_SPACE::File to the first (and supposed to be unique) file set
 *        of the GDCM_NAME_SPACE::SerieHelper.
 * \warning : this method should be used by aware users only!
 *           Passing a GDCM_NAME_SPACE::File* has the same effect than passing a file name!
 * \todo : decide which one is wrong (the method, or the commentary)!
 *           the following comment doesn't match the method :-(
 *            User is supposed to know the files he want to deal with
 *           and consider them they belong to the same Serie
 *           (even if their Serie UID is different)
 *           user will probabely OrderFileList() this list (actually, ordering
 *           user choosen gdm::File is the sole interest of this method)
 *           Moreover, using vtkGdcmReader::SetCoherentFileList() will avoid
 *           vtkGdcmReader parsing twice the same files. 
 *           *no* coherence check is performed, but those specified
 *           by SerieHelper::AddRestriction()
 * @param   header GDCM_NAME_SPACE::File* of the file to deal with
 * @return  true if file was added, false if file was rejected
 */
bool SerieHelper::AddFile(File *header)
{
   int allrules = 1;
   // First step the user has defined a set of rules for the DICOM 
   // he is looking for.
   // make sure the file correspond to his set of rules:

   std::string s;
   for(SerieExRestrictions::iterator it2 = ExRestrictions.begin();
     it2 != ExRestrictions.end();
     ++it2)
   {
      const ExRule &r = *it2;
      s = header->GetEntryString( r.group, r.elem );
      if ( !Util::CompareDicomString(s, r.value.c_str(), r.op) )
      {
         // Argh ! This rule is unmatched; let's just quit
         allrules = 0;
         break;
      }
   }

   if ( allrules ) // all rules are respected:
   {
      // Allright! we have a found a DICOM that matches the user expectation. 
      // Let's add it to the specific 'id' which by default is uid (Serie UID)
      // but can be `refined` by user with more paramater (see AddRestriction(g,e))
 
      std::string id = CreateUniqueSeriesIdentifier( header );
      // if id == GDCM_UNFOUND then consistently we should find GDCM_UNFOUND
      // no need here to do anything special

      if ( SingleSerieUIDFileSetHT.count(id) == 0 )
      {
         gdcmDebugMacro(" New/gdcmSerieHelper.cxx Serie UID :[" << id << "]");
         // create a std::list in 'id' position
         SingleSerieUIDFileSetHT[id] = new FileList;
      }
      // Current Serie UID and DICOM header seems to match add the file:
      SingleSerieUIDFileSetHT[id]->push_back( header );
   }
   else
   {
      // one rule not matched, tell user:
      return false;
   }
   return true;
}

/**
 * \brief add a rule for restricting a DICOM file to be in the serie we are
 * trying to find. For example you can select only the DICOM files from a
 * directory which would have a particular EchoTime==4.0.
 * This method is a user level, value is not required to be formatted as a DICOM
 * string
 * \todo find a trick to allow user to say if he wants the Rectrictions 
 *       to be *ored* (and not only *anded*)
 * @param   key  Target tag we want restrict on a given value
 * @param value value to be checked to exclude File
 * @param op  operator we want to use to check
 */
void SerieHelper::AddRestriction(TagKey const &key, 
                                 std::string const &value, int op)
{
   ExRule r;
   r.group = key[0];
   r.elem  = key[1];
   r.value = value;
   r.op    = op;
   ExRestrictions.push_back( r ); 
}

void SerieHelper::AddRestriction(TagKey const &key)
{
  ExRule r;
  r.group = key[0];
  r.elem  = key[1];
  ExRefine.push_back( r );
}

//#ifndef GDCM_LEGACY_REMOVE
/**
 * \brief add a rule for restricting a DICOM file to be in the serie we are
 * trying to find. For example you can select only the DICOM files from a
 * directory which would have a particular EchoTime==4.0.
 * This method is a user level, value is not required to be formatted as a DICOM
 * string
 * @param   group tag group number we want restrict on a given value
 * @param   elem  tag element number we want restrict on a given value 
 * @param value value to be checked to exclude File
 * @param op  operator we want to use to check
 * @deprecated use : AddRestriction(TagKey const &key, 
 *                                 std::string const &value, int op);
 */

void SerieHelper::AddRestriction(uint16_t group, uint16_t elem, 
                                 std::string const &value, int op)
{
  TagKey t(group, elem);
  AddRestriction(t, value, op);
}

//#endif

/**
 * \brief add an extra  'SerieDetail' for building a 'Serie Identifier'
 *        that ensures (hope so) File consistency (Series Instance UID doesn't)
 * @param   group tag group number we want restrict on a given value
 * @param   elem  tag element number we want restrict on a given value
 * @param  convert wether we want 'convertion', to allow further ordering
 *         e.g : 100 would be *before* 20; 000020.00 vs 00100.00 : OK 
 */
void SerieHelper::AddSeriesDetail(uint16_t group, uint16_t elem, bool convert)
{   
   ExDetail d;
   d.group   = group;
   d.elem    = elem;
   d.convert = convert;
   ExDetails.push_back( d ); 
}
/**
 * \brief Sets the root Directory
 * @param   dir Name of the directory to deal with
 * @param recursive whether we want explore recursively the root Directory
 */
void SerieHelper::SetDirectory(std::string const &dir, bool recursive)
{
   DirList dirList(dir, recursive); // OS specific

   DirListType filenames_list = dirList.GetFilenames();
   for( DirListType::const_iterator it = filenames_list.begin(); 
        it != filenames_list.end(); ++it)
   {
     // std::cout << "-----------------------------filename [" << *it << "]"
     //           << std::endl;
      gdcmDebugMacro("filename [" << *it << "]" );
      AddFileName( *it );
   }
}

/**
 * \brief Sets the DicomDirSerie
 * @param   se DicomDirSerie to deal with
 */
void SerieHelper::SetDicomDirSerie(DicomDirSerie *se)
{
   DirList dirList(se);
  
   DirListType filenames_list = dirList.GetFilenames();
   for( DirListType::const_iterator it = filenames_list.begin(); 
        it != filenames_list.end(); ++it)
   {
      AddFileName( *it );
   }
}

/**
 * \brief Sorts the given Fileset
 * \warning This could be implemented in a 'Strategy Pattern' approach
 *          But as I don't know how to do it, I leave it this way
 *          BTW, this is also a Strategy, I don't know this is 
 *          the best approach :)
 */
void SerieHelper::OrderFileList(FileList *fileSet)
{
   // Only computed during ImagePositionPatientOrdering
   // (need to sort the FileList using IPP and IOP !)
   ZSpacing = -1.0;
   
   if ( SerieHelper::UserLessThanFunction )
   {
      gdcmDebugMacro("Use UserLessThanFunction");     
      UserOrdering( fileSet );
      return; 
   }
   else if ( ImagePositionPatientOrdering( fileSet ) )
   { 
      gdcmDebugMacro("ImagePositionPatientOrdering succeeded");
      return ;
   }
   else if ( ImageNumberOrdering(fileSet ) )
   {
      gdcmDebugMacro("ImageNumberOrdering succeeded");   
      return ;
   }
   else  
   {
      gdcmDebugMacro("Use FileNameOrdering");    
      FileNameOrdering(fileSet );
   }
}

/**
 * \brief Elementary coherence checking of the files with the same Serie UID
 * Only sizes and pixel type are checked right now ...
 */ 
bool SerieHelper::IsCoherent(FileList *fileSet)
{
   if(fileSet->size() == 1)
   return true;

   FileList::const_iterator it = fileSet->begin();

   int nX =               (*it)->GetXSize();
   int nY =               (*it)->GetYSize();
   int pixelSize =        (*it)->GetPixelSize();
   bool signedPixelData = (*it)->IsSignedPixelData();
   it ++;
   for ( ;
         it != fileSet->end();
       ++it)
   {
      if ( (*it)->GetXSize() != nX )
         return false;
      if ( (*it)->GetYSize() != nY )
         return false;
      if ( (*it)->GetPixelSize() != pixelSize )
         return false;
      if ( (*it)->IsSignedPixelData() != signedPixelData )
         return false;
      // probabely more is to be checked (?)
   }
   return true;
}

//#ifndef GDCM_LEGACY_REMOVE
/**
 * \brief   accessor (DEPRECATED :  use GetFirstSingleSerieUIDFileSet )
 *          Warning : 'coherent' means here they have the same Serie UID
 * @return  The first FileList if found, otherwhise NULL
 */
 /*
FileList *SerieHelper::GetFirstCoherentFileList()
{
   ItFileSetHt = SingleSerieUIDFileSetHT.begin();
   if ( ItFileSetHt != SingleSerieUIDFileSetHT.end() )
      return ItFileSetHt->second;
   return NULL;
}
*/
/**
 * \brief   accessor (DEPRECATED :  use GetNextSingleSerieUIDFileSet )
 *          Warning : 'coherent' means here they have the same Serie UID
 * \note : meaningfull only if GetFirstCoherentFileList() already called 
 * @return  The next FileList if found, otherwhise NULL
 */
 /*
FileList *SerieHelper::GetNextCoherentFileList()
{
   gdcmAssertMacro (ItFileSetHt != SingleSerieUIDFileSetHT.end());
  
   ++ItFileSetHt;
   if ( ItFileSetHt != SingleSerieUIDFileSetHT.end() )
      return ItFileSetHt->second;
   return NULL;
}
*/

/**
 * \brief   accessor (DEPRECATED :  use GetSingleSerieUIDFileSet )
  *          Warning : 'coherent' means here they have the same Serie UID
 * @param SerieUID SerieUID
 * \return  pointer to the FileList if found, otherwhise NULL
 */
 /*
FileList *SerieHelper::GetCoherentFileList(std::string SerieUID)
{
   if ( SingleSerieUIDFileSetHT.count(SerieUID) == 0 )
      return 0;     
   return SingleSerieUIDFileSetHT[SerieUID];
}
*/
//#endif


/**
 * \brief   Get the first Fileset while visiting the SingleSerieUIDFileSetmap
 * @return  The first FileList (SingleSerieUIDFileSet) if found, otherwhise 0
 */
FileList *SerieHelper::GetFirstSingleSerieUIDFileSet()
{
   ItFileSetHt = SingleSerieUIDFileSetHT.begin();
   if ( ItFileSetHt != SingleSerieUIDFileSetHT.end() )
      return ItFileSetHt->second;
   return NULL;
}

/**
 * \brief   Get the next Fileset while visiting the SingleSerieUIDFileSetmap
 * \note : meaningfull only if GetNextSingleSerieUIDFileSet() already called 
 * @return  The next FileList (SingleSerieUIDFileSet) if found, otherwhise 0
 */
FileList *SerieHelper::GetNextSingleSerieUIDFileSet()
{
   gdcmAssertMacro (ItFileSetHt != SingleSerieUIDFileSetHT.end());
  
   ++ItFileSetHt;
   if ( ItFileSetHt != SingleSerieUIDFileSetHT.end() )
      return ItFileSetHt->second;
   return NULL;
}

/**
 * \brief   Get the SingleSerieUIDFileSet according to its Serie UID
 * @param SerieUID SerieUID to retrieve
 * \return pointer to the FileList (SingleSerieUIDFileSet) if found, otherwhise 0
 */
FileList *SerieHelper::GetSingleSerieUIDFileSet(std::string SerieUID)
{
   if ( SingleSerieUIDFileSetHT.count(SerieUID) == 0 )
      return 0;     
   return SingleSerieUIDFileSetHT[SerieUID];
}

/**
 * \brief   Splits a Single SerieUID Fileset according to the Orientations
 * @param fileSet File Set to be splitted
 * \return  std::map of 'Xcoherent' File sets
 */

XCoherentFileSetmap SerieHelper::SplitOnOrientation(FileList *fileSet)
{
   XCoherentFileSetmap CoherentFileSet;

   int nb = fileSet->size();
   if (nb == 0 ) {
      gdcmWarningMacro("Empty FileList passed to SplitOnOrientation");
      return CoherentFileSet;
   }

   float iop[6];
   std::string strOrient;
   std::ostringstream ossOrient;

   FileList::const_iterator it = fileSet->begin();
   for ( ;
         it != fileSet->end();
       ++it)
   {     
      // Information is in :      
      // 0020 0037 : Image Orientation (Patient) or
      // 0020 0035 : Image Orientation (RET)

      // Let's build again the 'cosines' string, to be sure of its format      
      (*it)->GetImageOrientationPatient(iop);

      ossOrient << iop[0];      
      for (int i = 1; i < 6; i++)
      {
        ossOrient << "\\";
        ossOrient << iop[i]; 
      }      
      strOrient = ossOrient.str();
      ossOrient.str("");
      if ( CoherentFileSet.count(strOrient) == 0 )
      {
         gdcmDebugMacro(" New Orientation :[" << strOrient << "]");
         // create a File set in 'orientation' position
         CoherentFileSet[strOrient] = new FileList;
         gdcmDebugMacro(" CoherentFileSet[strOrient]" << strOrient << "created");
      }
      // Current Orientation and DICOM header match; add the file:
      CoherentFileSet[strOrient]->push_back( (*it) );
      gdcmDebugMacro(" CoherentFileSet[strOrient]" << "pushed back")    
   }
   return CoherentFileSet;
}

/**
 * \brief   Splits a 'Single SerieUID' Fileset according to the Positions
 * @param fileSet File Set to be splitted
 * \return  std::map of 'Xcoherent' File sets
 */

XCoherentFileSetmap SerieHelper::SplitOnPosition(FileList *fileSet)
{
   XCoherentFileSetmap CoherentFileSet;

   int nb = fileSet->size();
   if (nb == 0 ) {
      gdcmWarningMacro("Empty FileList passed to SplitOnPosition");
      return CoherentFileSet;
   }
   float pos[3];
   std::string strImPos;  // read on disc
   std::ostringstream ossPosition;
   std::string strPosition; // re computed
   FileList::const_iterator it = fileSet->begin();
   for ( ;
         it != fileSet->end();
       ++it)
   {     
      // Information is in :      
      // 0020,0032 : Image Position Patient
      // 0020,0030 : Image Position (RET)

      strImPos = (*it)->GetEntryString(0x0020,0x0032);
      if ( strImPos == GDCM_UNFOUND)
      {
         gdcmWarningMacro( "Unfound Image Position Patient (0020,0032)");
         strImPos = (*it)->GetEntryString(0x0020,0x0030); // For ACR-NEMA images
         if ( strImPos == GDCM_UNFOUND )
         {
            gdcmWarningMacro( "Unfound Image Position (RET) (0020,0030)");
            // User wants to split on the 'Position'
            // No 'Position' info found.
            // We return an empty Htable !
            return CoherentFileSet;
         }  
      }

      if ( sscanf( strImPos.c_str(), "%f \\%f \\%f ", 
                                              &pos[0], &pos[1], &pos[2]) != 3 )
      {
            gdcmWarningMacro( "Wrong number for Position : ["
                       << strImPos << "]" );
            return CoherentFileSet;
      }

      // Let's build again the 'position' string, to be sure of it's format      

      ossPosition << pos[0];      
      for (int i = 1; i < 3; i++)
      {
        ossPosition << "\\";
        ossPosition << pos[i]; 
      }      
      strPosition = ossPosition.str();
      ossPosition.str("");

      if ( CoherentFileSet.count(strPosition) == 0 )
      {
         gdcmDebugMacro(" New Position :[" << strPosition << "]");
         // create a File set in 'position' position
         CoherentFileSet[strPosition] = new FileList;
      }
      // Current Position and DICOM header match; add the file:
      CoherentFileSet[strPosition]->push_back( (*it) );
   }   
   return CoherentFileSet;
}

/**
 * \brief   Splits a 'Single SerieUID' File set Coherent according to the
 *          value of a given Tag
 * @param fileSet File Set to be splitted
 * @param   group  group number of the target Element
 * @param   element element number of the target Element
 * \return  std::map of 'Xcoherent' File sets
 */

XCoherentFileSetmap SerieHelper::SplitOnTagValue(FileList *fileSet, 
                                               uint16_t group, uint16_t element)
{
   XCoherentFileSetmap CoherentFileSet;

   int nb = fileSet->size();
   if (nb == 0 ) {
      gdcmWarningMacro("Empty FileList passed to SplitOnPosition");
      return CoherentFileSet;
   }

   std::string strTagValue;  // read on disc

   FileList::const_iterator it = fileSet->begin();
   //it ++;
   for ( ;
         it != fileSet->end();
       ++it)
   {     
      // Information is in :      
      // 0020,0032 : Image Position Patient
      // 0020,0030 : Image Position (RET)

      strTagValue = (*it)->GetEntryString(group,element);
      
      if ( CoherentFileSet.count(strTagValue) == 0 )
      {
         gdcmDebugMacro("  :[" << strTagValue << "]");
         // create a File set in 'position' position
         CoherentFileSet[strTagValue] = new FileList;
      }
      // Current Tag value and DICOM header match; add the file:
      CoherentFileSet[strTagValue]->push_back( (*it) );
   }
   return CoherentFileSet;
}

//-----------------------------------------------------------------------------
// Protected

//-----------------------------------------------------------------------------
// Private
/**
 * \brief sorts the images, according to their Patient Position.
 *  As a side effect, it computes the ZSpacing, according to Jolinda Smith's
 *  algorithm. (get it with double GetZSpacing() !)
 *  We may order, considering :
 *   -# Image Position Patient
 *   -# Image Number
 *   -# file name
 *   -# More to come :-)
 * \note : FileList = std::vector<File* >
 * @param fileList Coherent File list (same Serie UID) to sort
 * @return false only if the header is bugged !
 */
bool SerieHelper::ImagePositionPatientOrdering( FileList *fileList )
//based on Jolinda Smith's algorithm
{
//Tags always use the same coordinate system, where "x" is left
//to right, "y" is posterior to anterior, and "z" is foot to head (RAH).

   //iop is calculated based on the file file
   float cosines[6];
   double normal[3];
   double ipp[3];
   double dist;
   double min = 0, max = 0;
   bool first = true;
   ZSpacing = -1.0;  // will be updated if process doesn't fail

   gdcmDebugMacro("============================================DropDuplicatePositions : " << DropDuplicatePositions );
    
   std::multimap<double,File *> distmultimap;
   // Use a multimap to sort the distances from 0,0,0
   for ( FileList::const_iterator 
         it = fileList->begin();
         it != fileList->end(); ++it )
   {
      gdcmDebugMacro("deal with " << (*it)->GetFileName() );
      if ( first ) 
      {
         (*it)->GetImageOrientationPatient( cosines );

   // The "Image Orientation Patient" tag gives the direction cosines 
   // for the rows and columns for the three axes defined above. 
   // Typical axial slices will have a value 1/0/0/0/1/0: 
   // rows increase from left to right, 
   // columns increase from posterior to anterior. This is your everyday
   // "looking up from the bottom of the head with the eyeballs up" image. 
   
   // The "Image Position Patient" tag gives the coordinates of the first
   // voxel in the image in the "RAH" coordinate system, relative to some
   // origin.   

   // First, calculate the slice normal from IOP : 
          
         // You only have to do this once for all slices in the volume. Next, 
         // for each slice, calculate the distance along the slice normal 
         // using the IPP ("Image Position Patient") tag.
         // ("dist" is initialized to zero before reading the first slice) :
         normal[0] = cosines[1]*cosines[5] - cosines[2]*cosines[4];
         normal[1] = cosines[2]*cosines[3] - cosines[0]*cosines[5];
         normal[2] = cosines[0]*cosines[4] - cosines[1]*cosines[3];

   // For each slice (here : the first), calculate the distance along 
   // the slice normal using the IPP tag 
    
         ipp[0] = (*it)->GetXOrigin();
         ipp[1] = (*it)->GetYOrigin();
         ipp[2] = (*it)->GetZOrigin();

         dist = 0;
         for ( int i = 0; i < 3; ++i )
         {
            dist += normal[i]*ipp[i];
         }
    
         gdcmDebugMacro("dist : " << dist);
         distmultimap.insert(std::pair<const double,File *>(dist, *it));

         max = min = dist;
         first = false;
      }
      else 
      {
   // Next, for each slice, calculate the distance along the slice normal
   // using the IPP tag 
         ipp[0] = (*it)->GetXOrigin();
         ipp[1] = (*it)->GetYOrigin();
         ipp[2] = (*it)->GetZOrigin();

         dist = 0;
         for ( int i = 0; i < 3; ++i )
         {
            dist += normal[i]*ipp[i];
         }

         distmultimap.insert(std::pair<const double,File *>(dist, *it));
         gdcmDebugMacro("dist : " << dist);
         min = (min < dist) ? min : dist;
         max = (max > dist) ? max : dist;
      }
   }

   gdcmDebugMacro("After parsing vector, nb of elements : " << fileList->size() );

   // Find out if min/max are coherent
   if ( min == max )
   {
     gdcmWarningMacro("Looks like all images have the exact same image position. "
                      << "No PositionPatientOrdering sort performed. "
                      << "No 'ZSpacing' calculated! ");
     return false;
   }

   // Check to see if image shares a common position
   bool ok = true;
   for (std::multimap<double, File *>::iterator it2 = distmultimap.begin();
        it2 != distmultimap.end();
        ++it2)
   {
   
      gdcmDebugMacro("Check if image shares a common position : " << (*it2).second->GetFileName() );   
   
      if (distmultimap.count((*it2).first) != 1)
      {
         gdcmWarningMacro("File: ["
              << ((*it2).second->GetFileName())
              << "] : more than ONE file at distance: '"
              << (*it2).first
              << " (position is not unique!) "
              << "No PositionPatientOrdering sort performed. "
              << "No 'ZSpacing' calculated! ");      

         ok = false;
      }
   }
   if (!ok)
   {
      if (! DropDuplicatePositions)
         return false;
   }
      
// Now, we can calculate Z Spacing as the difference
// between the "dist" values for the first two slices.

// The following (un)-commented out code is let here
// to be re-used by whomsoever is interested...

    std::multimap<double, File *>::iterator it5 = distmultimap.begin();
    double d1 = (*it5).first;
    it5++;
    double d2 = (*it5).first;
    ZSpacing = d1-d2;
    if (ZSpacing < 0.0)
       ZSpacing = - ZSpacing;

   fileList->clear();  // doesn't delete list elements, only nodes

// Acording to user requierement, we sort direct order or reverse order.
   if (DirectOrder)
   {  
      for (std::multimap<double, File *>::iterator it3 = distmultimap.begin();
           it3 != distmultimap.end();
           ++it3)
      {
         fileList->push_back( (*it3).second );
         if (DropDuplicatePositions)
         {
            // ImagePositionPatientOrdering  wrong duplicates are found ???
            // --> fixed. See comment

            it3 =  distmultimap.upper_bound((*it3).first); // skip all duplicates
           // the upper_bound function increments the iterator to the next non-duplicate entry
           // The for loop iteration also increments the iterator, which causes the code to skip every other image
           // --> decrement the iterator after the upper_bound function call
            it3--;
            if (it3 == distmultimap.end() )  // if last image, stop iterate
               break;
         }
      }
   }
   else // user asked for reverse order
   {
      std::multimap<double, File *>::const_iterator it4;
      it4 = distmultimap.end();
      do
      {
         it4--;
         fileList->push_back( (*it4).second );
         if (DropDuplicatePositions)  // skip all duplicates
         {
            // lower_bound finds the next element that is 
            // less than or *equal to* the current value!
            //it4 =  distmultimap.lower_bound((*it4).first);
   
           // David Feng's fix
           std::multimap<double, File *>::const_iterator itPrev = it4;
           while (itPrev->first == it4->first)
              --itPrev;
           it4 = itPrev;
    
           if (it4 == distmultimap.begin() ) // if first image, stop iterate
               break;
         } 
      } while (it4 != distmultimap.begin() );
   }

   distmultimap.clear();

   return true;
}

bool SerieHelper::ImageNumberLessThan(File *file1, File *file2)
{
  return file1->GetImageNumber() < file2->GetImageNumber();
}

bool SerieHelper::ImageNumberGreaterThan(File *file1, File *file2)
{
  return file1->GetImageNumber() > file2->GetImageNumber();
}

/**
 * \brief sorts the images, according to their Image Number
 * \note Works only on bona fide files  (i.e image number is a character string
 *                                      corresponding to an integer)
 *             within a bona fide serie (i.e image numbers are consecutive)
 * @param fileList File set (same Serie UID) to sort 
 * @return false if non bona fide stuff encountered
 */
bool SerieHelper::ImageNumberOrdering(FileList *fileList) 
{
   int min, max, pos;
   int n = fileList->size();

   FileList::const_iterator it = fileList->begin();
   min = max = (*it)->GetImageNumber();

   for (; it != fileList->end(); ++it, ++n)
   {
      pos = (*it)->GetImageNumber();
      min = (min < pos) ? min : pos;
      max = (max > pos) ? max : pos;
   }

   // Find out if image numbers are coherent (consecutive)
   if ( min == max || max == 0 || max >= (n+min) )
   {
      gdcmWarningMacro( " 'Image numbers' not coherent. "
                        << " No ImageNumberOrdering sort performed.");
      return false;
   }
   if (DirectOrder)
      Sort(fileList,SerieHelper::ImageNumberLessThan);
   else
      Sort(fileList,SerieHelper::ImageNumberGreaterThan);

   return true;
}

bool SerieHelper::FileNameLessThan(File *file1, File *file2)
{
   return file1->GetFileName() < file2->GetFileName();
}

bool SerieHelper::FileNameGreaterThan(File *file1, File *file2)
{
   return file1->GetFileName() > file2->GetFileName();
}
/**
 * \brief sorts the images, according to their File Name
 * @param fileList Coherent File list (same Serie UID) to sort
 * @return false only if the header is bugged !
 */
bool SerieHelper::FileNameOrdering(FileList *fileList)
{
   if (DirectOrder) 
      Sort(fileList,SerieHelper::FileNameLessThan);
   else
      Sort(fileList,SerieHelper::FileNameGreaterThan);   

   return true;
}

/**
 * \brief sorts the images, according to user supplied function
 * @param fileList Coherent File list (same Serie UID) to sort
 * @return false only if the header is bugged !
 */
bool SerieHelper::UserOrdering(FileList *fileList)
{
   Sort(fileList,SerieHelper::UserLessThanFunction);   
   if (!DirectOrder) 
   {
      std::reverse(fileList->begin(), fileList->end());
   }
   return true;
}

//-----------------------------------------------------------------------------
// Print
/**
 * \brief   Canonical printer.
 */
void SerieHelper::Print(std::ostream &os, std::string const &indent)
{
   // For all the Coherent File lists of the GDCM_NAME_SPACE::Serie
   SingleSerieUIDFileSetmap::iterator itl = SingleSerieUIDFileSetHT.begin();
   if ( itl == SingleSerieUIDFileSetHT.end() )
   {
      gdcmWarningMacro( "No SingleSerieUID File set found" );
      return;
   }
   while (itl != SingleSerieUIDFileSetHT.end())
   { 
      os << "Serie UID :[" << itl->first << "]" << std::endl;

      // For all the files of a SingleSerieUID File set
      for (FileList::iterator it =  (itl->second)->begin();
                              it != (itl->second)->end(); 
                            ++it)
      {
         os << indent << " --- " << (*it)->GetFileName() << std::endl;
      }
      ++itl;
   }
}

void SerieHelper::CreateDefaultUniqueSeriesIdentifier()
{
   // If the user requests, additional information can be appended
   // to the SeriesUID to further differentiate volumes in the DICOM
   // objects being processed.
 
   // 0020 0011 Series Number
   // A scout scan prior to a CT volume scan can share the same
   //   SeriesUID, but they will sometimes have a different Series Number
   AddRestriction( TagKey(0x0020, 0x0011) );
   
   // 0018 0024 Sequence Name
   // For T1-map and phase-contrast MRA, the different flip angles and
   //   directions are only distinguished by the Sequence Name
   AddRestriction( TagKey(0x0018, 0x0024) );
   
   // 0018 0050 Slice Thickness
   // On some CT systems, scout scans and subsequence volume scans will
   //   have the same SeriesUID and Series Number - YET the slice 
   //   thickness will differ from the scout slice and the volume slices.
   AddRestriction( TagKey(0x0018, 0x0050));
   
   // 0028 0010 Rows
   // If the 2D images in a sequence don't have the same number of rows,
   //   then it is difficult to reconstruct them into a 3D volume.
   AddRestriction( TagKey(0x0028, 0x0010));
   
   // 0028 0011 Columns
   // If the 2D images in a sequence don't have the same number of columns,
   //   then it is difficult to reconstruct them into a 3D volume.
   AddRestriction( TagKey(0x0028, 0x0011));
}

/**
 * \brief Heuristics to *try* to build a Serie Identifier that would ensure
 *        all the images are coherent.
 *
 * By default, uses the SeriesUID.  If UseSeriesDetails(true) has been called,
 *         then additional identifying information is used.
 *  We allow user to add his own critierions, using AddSeriesDetail
 *        (he knows more than we do about his images!)
 *        ex : in tagging series, the only pertinent tag is
 *        0018|1312 [In-plane Phase Encoding Direction] value : ROW/COLUMN
 * @param inFile GDCM_NAME_SPACE::File we want to build a Serie Identifier for.
 * @return the SeriesIdentifier
 */
std::string SerieHelper::CreateUniqueSeriesIdentifier( File *inFile )
{
   if( inFile->IsReadable() )
   {
    // 0020 000e UI REL Series Instance UID
    std::string uid = inFile->GetEntryString (0x0020, 0x000e);
    std::string id = uid.c_str();
    if(m_UseSeriesDetails)
      {
      for(SerieExRestrictions::iterator it2 = ExRefine.begin();
        it2 != ExRefine.end();
        ++it2)
        {
        const ExRule &r = *it2;
        std::string s = inFile->GetEntryString( r.group, r.elem );
        if( s == GDCM_UNFOUND )
          {
          s = "";
          }
        if( id == uid && !s.empty() )
          {
          id += "."; // add separator
          }
        id += s;
        }
      }
    // Eliminate non-alnum characters, including whitespace...
    //   that may have been introduced by concats.
    unsigned int s_size = id.size();
    for(unsigned int i=0; i<s_size; i++)
    {
     while(i<s_size
       && !( id[i] == '.' || id[i] == '%' || id[i] == '_'
         || (id[i] >= '+' && id[i] <= '-')       
         || (id[i] >= 'a' && id[i] <= 'z')
         || (id[i] >= '0' && id[i] <= '9')
         || (id[i] >= 'A' && id[i] <= 'Z')))
      {
         id.replace(i, 1, "_");  // ImagePositionPatient related stuff will be more human readable
      }
   }
   // deal with Dicom strings trailing '\0' 
    if(s_size && id[s_size-1] == '_')
      id.erase(s_size-1, 1);
    return id;
    }
  else // Could not open inFile
    {
    gdcmWarningMacro("Could not parse series info.");
    std::string id = GDCM_UNFOUND;
    return id;
    }
}

/**
 * \brief Allow user to build is own File Identifier (to be able to sort
 *        temporal series just as he wants)
 *        Criterions will be set with AddSeriesDetail.
 *        (Maybe the method should be moved elsewhere 
 *       -File class? FileHelper class?-
 * @return FileIdentifier (Tokenizable on '%%%'. Hope it's enough !)
 */
std::string SerieHelper::CreateUserDefinedFileIdentifier( File *inFile )
{
  //     Deal with all user supplied tags.
  //      (user knows more than we do about his images!)
  
   double converted;
   std::string id;
   std::string s; 
   char charConverted[17]; 
   
   for(SeriesExDetails::iterator it2 = ExDetails.begin();
      it2 != ExDetails.end();
      ++it2)
   {
      const ExDetail &r = *it2;
      s = inFile->GetEntryString( r.group, r.elem );
      if (s == "") // avoid troubles when empty string is found
         s = "-";

      // User is allowed to ask for 'convertion', to allow further ordering
      // e.g : 100 would be *before* 20; 000020.00 vs 00100.00 : OK
      if (it2->convert)
      {
         if ( s != GDCM_UNFOUND) // Don't convert unfound fields !
         {
            converted = atof(s.c_str());
            // probabely something much more complicated is possible, 
            // using C++ features
            /// \todo check the behaviour when there are >0 and <0 numbers
            sprintf(charConverted, "%016.6f",converted);
            s = charConverted;
         }
      }
      // Eliminate non-alphanum characters, including whitespace.

      unsigned int s_size = s.size();
      if(s_size == 0)
      { // to avoid further troubles when wild anonymization was performed
         s = "a";
      }
      else
      {
         for(unsigned int i=0; i<s_size; i++)
         {
            while(i<s_size
               && !( s[i] == '.' || s[i] == '%' || s[i] == '_'
                 || (s[i] >= '+' && s[i] <= '-')       
                 || (s[i] >= 'a' && s[i] <= 'z')
                 || (s[i] >= '0' && s[i] <= '9')
                 || (s[i] >= 'A' && s[i] <= 'Z')))
            {
               s.replace(i, 1, "_");  // ImagePositionPatient related stuff will be more human readable
            }
         }
         // deal with Dicom strings trailing '\0' 
         if(s[s_size-1] == '_')
            s.erase(s_size-1, 1);
      }
      id += s.c_str();
      id += "%%%"; // make the FileIdentifier Tokenizable
   }
   id += inFile->GetFileName();
   id += "%%%"; 
   return id;             
}

//-----------------------------------------------------------------------------
// Sort
/**
 * \brief   Sort FileList.
 */
void SerieHelper::Sort(FileList *fileList, bool (*pt2Func)( File *file1, File *file2) )
{
 std::sort(fileList->begin(), fileList->end(), pt2Func );
}

/*
#ifndef GDCM_LEGACY_REMOVE
bool SerieHelper::AddGdcmFile(File* header)
{
  return AddFile(header);
}
#endif
*/

//-----------------------------------------------------------------------------
} // end namespace gdcm
