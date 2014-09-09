/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDataEntry.cxx,v $
  Language:  C++
  Date:      $Date: 2010/07/09 09:20:20 $
  Version:   $Revision: 1.56 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#include "gdcmDataEntry.h"
#include "gdcmVR.h"
#include "gdcmTS.h"
#include "gdcmGlobal.h"
#include "gdcmUtil.h"
#include "gdcmDebug.h"

#include <fstream>

#if defined(__BORLANDC__)
 #include <mem.h>    // for memcpy
 #include <stdlib.h> // for atof
 #include <ctype.h>  // for isdigit
#endif
#include <string.h> // memcpy
#include <stdlib.h> // atof

// Could be defined like MAX_SIZE_LOAD_ELEMENT_VALUE
#define GDCM_MAX_LENGTH_TO_CONVERT_TO_HEXA 8

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
#define MAX_SIZE_PRINT_ELEMENT_VALUE 0x7fffffff
uint32_t DataEntry::MaxSizePrintEntry = MAX_SIZE_PRINT_ELEMENT_VALUE;

//-----------------------------------------------------------------------------
// Constructor / Destructor
/**
 * \brief   Constructor for a given DataEntry
 * @param   group group number of the Data Entry to be created
 * @param   elem element number of the Data Entry to be created
 * @param   vr Value Representation of the Data Entry to be created 
 */
DataEntry::DataEntry(uint16_t group,uint16_t elem,
                                     VRKey const &vr) 
            : DocEntry(group,elem,vr)
{
   State = STATE_LOADED;
   Flag = FLAG_NONE;

   StrArea     = 0;
   StrHexaArea = 0;   
   BinArea     = 0;
   SelfArea    = true;
}

/**
 * \brief   Constructor for a given DocEntry
 * @param   e Pointer to existing Doc entry
 */
DataEntry::DataEntry(DocEntry *e)
            //: DocEntry(e->GetDictEntry())
            : DocEntry(e->GetGroup(),e->GetElement(), e->GetVR()  )
{
   Flag = FLAG_NONE;
   BinArea = 0;

   SelfArea = true;

   Copy(e);
}

/**
 * \brief   Canonical destructor.
 */
DataEntry::~DataEntry ()
{
   DeleteBinArea();
}

//-----------------------------------------------------------------------------
// Public
/**
 * \brief Sets the value (non string) of the current DataEntry
 * @param area area
 * @param self self=true : The area : *belongs" to the DataEntry 
 *                                  : will be delete with the DataEntry
 *             self=false  The area *is not* deleted with the DataEntry
 *              
 */
void DataEntry::SetBinArea( uint8_t *area, bool self )  
{ 
   DeleteBinArea();

   BinArea = area;
   SelfArea = self;

   State = STATE_LOADED;
}
/**
 * \brief Inserts the value (non string) into the current DataEntry
 * @param area area
 * @param length length 
 */
void DataEntry::CopyBinArea( uint8_t *area, uint32_t length )
{
   DeleteBinArea();

   uint32_t lgh = length + length%2;
   SetLength(lgh);

   if( area && length > 0 )
   {
      NewBinArea();
      memcpy(BinArea,area,length);
      if( length!=lgh )
         BinArea[length]=0; // padd with zero

      State = STATE_LOADED;
   }
}

/**
 * \brief Checks wether the current DataEntry contains number(s)
 */

bool DataEntry::IsNumerical()
{
   const VRKey &vr = GetVR();

   return 
          vr == "DS" ||
          vr == "FL" ||
          vr == "FD" ||
          vr == "IS" || 
          vr == "SH" ||
          vr == "SL" ||
          vr == "SS" ||
          vr == "UI" ||
          vr == "UL" ||
          vr == "US" ;      
}

/**
 * \brief Gets a std::vector of 'double' holding the value(s) of any 'numerical' DataEntry
 * @param valueVector std::vector double of value(s)
 * \return false if VR not "a 'numerical" one
 */
 bool DataEntry::GetNumerical(std::vector <double> &valueVector)
 {
    valueVector.clear();
    
    if (!IsNumerical()) // never trust a user !
       return false;    

    const VRKey &vr = GetVR();
    int loop;
    if (vr == "IS" || vr == "DS")
    {
       /// \todo rewrite the whole method, in order *not to use* std::string !
       std::vector<std::string> tokens;
    
       Util::Tokenize ( GetString().c_str(), tokens, "\\" );

       int nbValues= tokens.size();
       if (nbValues == 0)
          return false;

       if (vr == "DS")        
          for (loop=0; loop<nbValues; loop++) 
             valueVector.push_back(atof(tokens[loop].c_str()));
       else
          for (loop=0; loop<nbValues; loop++) 
             valueVector.push_back(atoi(tokens[loop].c_str()));
      
       return true;     
    }    

    uint32_t nbValues = GetValueCount();
    if (nbValues == 0)
       return false;
  
    if( vr == "US") {
       for (loop=0; loop<nbValues; loop++)
             valueVector.push_back(((uint16_t *)BinArea)[loop]);  
       return true;
    }
    if( vr == "SS" ) {
       for (loop=0; loop<nbValues; loop++)
             valueVector.push_back(((int16_t *)BinArea)[loop]);  
       return true;
    }
    if( vr == "UL") {
       for (loop=0; loop<nbValues; loop++) 
             valueVector.push_back(((uint32_t *)BinArea)[loop]);      
       return true;
    }
    if(vr == "SL" ) {
       for (loop=0; loop<nbValues; loop++) 
             valueVector.push_back(((int32_t *)BinArea)[loop]);      
       return true;
    }       
    if( vr == "FL" ) {
       for (loop=0; loop<nbValues; loop++) 
             valueVector.push_back(((float *)BinArea)[loop]);     
       return true;
    }
    if( vr == "FD" ) {
       for (loop=0; loop<nbValues; loop++) 
             valueVector.push_back(((double *)BinArea)[loop]);
       return true;
    }     
    return false;   // ?!? 
 }

/**
 * \brief Inserts the elementary (non string) value into the current DataEntry
 * @param id index of the elementary value to be set
 * @param val value, passed as a double 
 */
void DataEntry::SetValue(const uint32_t &id, const double &val)
{
   if( !BinArea )
      NewBinArea();
   State = STATE_LOADED;

   if( id > GetValueCount() )
   {
      gdcmErrorMacro("Index (" << id << ") is greater than the data size");
      return;
   }

   const VRKey &vr = GetVR();
   if( vr == "US" || vr == "SS" )
   {
      uint16_t *data = (uint16_t *)BinArea;
      data[id] = (uint16_t)val;
   }
   else if( vr == "UL" || vr == "SL" )
   {
      uint32_t *data = (uint32_t *)BinArea;
      data[id] = (uint32_t)val;
   }
   else if( vr == "FL" )
   {
      float *data = (float *)BinArea;
      data[id] = (float)val;
   }
   else if( vr == "FD" )
   {
      double *data = (double *)BinArea;
      data[id] = (double)val;
   }
   else if( Global::GetVR()->IsVROfStringRepresentable(vr) )
   {
      gdcmErrorMacro("SetValue on String representable not implemented yet");
   }
   else
   {
      BinArea[id] = (uint8_t)val;
   }
}
/**
 * \brief returns, as a double one of the values 
 *      (when entry is multivaluated), identified by its index.
 *      Returns 0.0 if index is wrong
 * @param id id
 */
double DataEntry::GetValue(const uint32_t &id) const
{
   if( !BinArea )
   {
      if (GetLength() != 0) // avoid stupid messages
   /// \todo warn the user there was a problem !
         gdcmErrorMacro("BinArea not set " << std::hex 
                     << GetGroup() << " " << GetElement() 
                     << " Can't get the value");
      return 0.0;
   }

   uint32_t count = GetValueCount();
   if( id > count )
   {
      gdcmErrorMacro("Index (" << id << ") is greater than the data size");
      return 0.0;
   }

   // if user *knows* that entry contains a US, 
   // he just has to cast the double he receives
   
   const VRKey &vr = GetVR();

   if( vr == "US" || vr == "SS" )
      return ((uint16_t *)BinArea)[id];
   else if( vr == "UL" || vr == "SL" )
      return ((uint32_t *)BinArea)[id];
   else if( vr == "FL" )
      return ((float *)BinArea)[id];
   else if( vr == "FD" )
      return ((double *)BinArea)[id];
   else if( Global::GetVR()->IsVROfStringRepresentable(vr) )
   {
      // this is for VR = "DS", ...
      if( GetLength() )
      {
         // Don't use std::string to accelerate processing
         double val;
         char *tmp = new char[GetLength()+1];
         memcpy(tmp,BinArea,GetLength());
         tmp[GetLength()]=0;

         if( count == 0 )
         {
            val = atof(tmp);
         }
         else
         {
            count = id;
            char *beg = tmp;
            for(uint32_t i=0;i<GetLength();i++)
            {
               if( tmp[i] == '\\' )
               {
                  if( count == 0 )
                  {
                     tmp[i] = 0;
                     break;
                  }
                  else
                  {
                     count--;
                     beg = &(tmp[i+1]);
                  }
               }
            }
            val = atof(beg);
         }

         delete[] tmp;
         return val;
      }
      else 
         return 0.0;
   }
   else
      return BinArea[id];
}

/**
 * \brief Checks if the multiplicity of the value follows Dictionary VM
 */
bool DataEntry::IsValueCountValid() /*const*/
{
  uint32_t vm;
  const std::string &strVM = GetVM();
  uint32_t vc = GetValueCount();
  bool valid = vc == 0;
  if( valid )
    return true;
  
  // FIXME : what shall we do with VM = "2-n", "3-n", etc
  
  if( strVM == "1-n" )
  {
    // make sure there is at least one ??? FIXME
    valid = vc >= 1;
  }
  else
  {
    std::istringstream os;
    os.str( strVM );
    os >> vm;
    // Two cases:
    // vm respects the one from the dict
    // vm is 0 (we need to check if this element is allowed to be empty) FIXME

    // note  (JPR)
    // ----    
    // Entries whose type is 1 are mandatory, with a mandatory value.
    // Entries whose type is 1c are mandatory-inside-a-Sequence,
    //                          with a mandatory value.
    // Entries whose type is 2 are mandatory, with an optional value.
    // Entries whose type is 2c are mandatory-inside-a-Sequence,
    //                          with an optional value.
    // Entries whose type is 3 are optional.

    // case vc == 0 is only applicable for 'type 2' entries.
    // Problem : entry type may depend on the modality and/or the Sequence
    //           it's embedded in !
    //          (Get the information in the 'Conformance Statements' ...)  
    valid = vc == vm;
  }
  return valid;
}

/**
 * \brief returns the number of elementary values
 */ 
uint32_t DataEntry::GetValueCount( ) const
{
   const VRKey &vr = GetVR();
   if( vr == "US" || vr == "SS" )
      return GetLength()/sizeof(uint16_t);
   else if( vr == "UL" || vr == "SL" )
      return GetLength()/sizeof(uint32_t);
   else if( vr == "FL" || vr == "OF" )
      return GetLength()/4 ; // FL has a *4* length! sizeof(float);
   else if( vr == "FD" )
      return GetLength()/8;  // FD has a *8* length! sizeof(double);
   else if( Global::GetVR()->IsVROfStringRepresentable(vr) )
   {
      // Some element in DICOM are allowed to be empty
      if( !GetLength() ) 
         return 0;
      // Don't use std::string to accelerate processing
      uint32_t count = 1;
      for(uint32_t i=0;i<GetLength();i++)
      {
         if( BinArea[i] == '\\')
            count++;
      }
      return count;
   }
   return GetLength();
}

/**
 * \brief Gets a std::vector of 'double' holding the value(s) of a DS DataEntry
 * @param valueVector std::vector double of value(s)
 * \return false if VR not "DS" or DataEntry empty
 */
 bool DataEntry::GetDSValue(std::vector <double> &valueVector)
 {
    /// \todo rewrite the whole method, in order *not to use* std::string !
    std::vector<std::string> tokens;
    
    if (GetVR() != "DS") // never trust a user !
       return false;    
       
    Util::Tokenize ( GetString().c_str(), tokens, "\\" );
        
    int nbValues= tokens.size();
    if (nbValues == 0)
       return false;
               
    for (int loop=0; loop<nbValues; loop++) 
       valueVector.push_back(atof(tokens[loop].c_str()));
    
    return true;  
 }
 
/**
 * \brief Sets the 'value' of a DataEntry, passed as a std::string
 * @param value string representation of the value to be set
 */ 
void DataEntry::SetString(std::string const &value)
{
   DeleteBinArea();
   const VRKey &vr = GetVR();
   if ( vr == "US" || vr == "SS" )
   {
      std::vector<std::string> tokens;
      Util::Tokenize (value, tokens, "\\");
      SetLength(tokens.size()*sizeof(uint16_t));
      NewBinArea();

      uint16_t *data = (uint16_t *)BinArea;
      for (unsigned int i=0; i<tokens.size();i++)
         data[i] = atoi(tokens[i].c_str());
      tokens.clear();
   }
   else if ( vr == "UL" || vr == "SL" )
   {
      std::vector<std::string> tokens;
      Util::Tokenize (value, tokens, "\\");
      SetLength(tokens.size()*sizeof(uint32_t));
      NewBinArea();

      uint32_t *data = (uint32_t *)BinArea;
      for (unsigned int i=0; i<tokens.size();i++)
         data[i] = atoi(tokens[i].c_str());
      tokens.clear();
   }
   else if ( vr == "FL" )
   {
      std::vector<std::string> tokens;
      Util::Tokenize (value, tokens, "\\");
      SetLength(tokens.size()*sizeof(float));
      NewBinArea();

      float *data = (float *)BinArea;
      for (unsigned int i=0; i<tokens.size();i++)
         data[i] = (float)atof(tokens[i].c_str());
      tokens.clear();
   }
   else if ( vr == "FD" )
   {
      std::vector<std::string> tokens;
      Util::Tokenize (value, tokens, "\\");
      SetLength(tokens.size()*sizeof(double));
      NewBinArea();

      double *data = (double *)BinArea;
      for (unsigned int i=0; i<tokens.size();i++)
         data[i] = atof(tokens[i].c_str());
      tokens.clear();
   }
   else
   {      
      size_t l =  value.size();    
      SetLength(l + l%2);
      NewBinArea();
      memcpy(BinArea, value.c_str(), l);
      if (l%2) // padded with blank except for UI
        {
        if ( vr == "UI" )
          BinArea[l] = '\0';
        else
          BinArea[l] = ' ';
        }
   }
   State = STATE_LOADED;
}
/**
 * \brief   returns as a string (when possible) the value of the DataEntry
 */
std::string const &DataEntry::GetString() const
{
  static std::ostringstream s;
  const VRKey &vr = GetVR();
  s.str("");
  
  if (!StrArea)
     StrArea = new std::string();
  else 
     *StrArea="";

  if( !BinArea )
     return *StrArea;
  // When short integer(s) are stored, convert the following (n * 2) characters
  // as a displayable string, the values being separated by a back-slash
  if( vr == "US" )
  {
     uint16_t *data=(uint16_t *)BinArea;
     for (unsigned int i=0; i < GetValueCount(); i++)
     {
        if( i!=0 )
           s << '\\';
        s << data[i];
     }
     *StrArea=s.str();
  }
  else if (vr == "SS" )
  {
     int16_t *data=(int16_t *)BinArea;
     for (unsigned int i=0; i < GetValueCount(); i++)
     {
        if( i!=0 )
           s << '\\';
        s << data[i];
     }
     *StrArea=s.str();
  }      // See above comment on multiple short integers (mutatis mutandis).
  else if( vr == "UL" )
  {
     uint32_t *data=(uint32_t *)BinArea;
     for (unsigned int i=0; i < GetValueCount(); i++)
     {
        if( i!=0 )
           s << '\\';
        s << data[i];
     }
     *StrArea=s.str();
  }
  else if( vr == "SL" )
  {
     int32_t *data=(int32_t *)BinArea;
     for (unsigned int i=0; i < GetValueCount(); i++)
     {
        if( i!=0 )
           s << '\\';
        s << data[i];
     }
     *StrArea=s.str();
  }     // See above comment on multiple short integers (mutatis mutandis).
  else if( vr == "FL" )
  {
     float *data=(float *)BinArea;
     for (unsigned int i=0; i < GetValueCount(); i++)
     {
        if( i!=0 )
           s << '\\';
        s << data[i];
     }
     *StrArea=s.str();
  }     // See above comment on multiple short integers (mutatis mutandis).
  else if( vr == "FD" )
  {
     double *data=(double *)BinArea;
     for (unsigned int i=0; i < GetValueCount(); i++)
     {
        if( i!=0 )
           s << '\\';
        s << data[i];
     }
     *StrArea=s.str();
  }
  else
  {
     StrArea->append((const char *)BinArea,GetLength());
     // to avoid gdcm to propagate oddities in lengthes
     if ( GetLength()%2)
        StrArea->append(" ",1);
   }
  return *StrArea;
}

/**
 * \brief   returns an hexadecimal representation of the DataEntry value
 */
std::string const &DataEntry::GetHexaRepresentation() const
{ 
  static std::ostringstream s2;
  const VRKey &vr = GetVR();
   
  s2.str("");
  if (!StrHexaArea)
     StrHexaArea = new std::string();
  else 
     *StrHexaArea="";
  if( !BinArea )
     return *StrHexaArea;
  // When short integer(s) are stored, convert the following (n * 2) characters
  // as a displayable string, the values being separated by a back-slash
  
  s2 << std::hex;
  
  if( vr == "US" )
  {
     uint16_t *data=(uint16_t *)BinArea;
     for (unsigned int i=0; i < GetValueCount(); i++)
     {
        s2  << std::setw( 2 ) << std::setfill( '0' );     
        if( i!=0 )
           s2 << '\\';
        s2 << data[i];
     }
     *StrHexaArea=s2.str();
  }
  else if (vr == "SS" )
  {
     int16_t *data=(int16_t *)BinArea;
     for (unsigned int i=0; i < GetValueCount(); i++)
     {
        s2  << std::setw( 4 ) << std::setfill( '0' );
        if( i!=0 )
           s2 << '\\';
        s2  << data[i];
     }
     *StrHexaArea=s2.str();
  }      // See above comment on multiple short integers (mutatis mutandis).
  else if( vr == "UL" )
  {
     uint32_t *data=(uint32_t *)BinArea;
     for (unsigned int i=0; i < GetValueCount(); i++)
     {
        s2  << std::setw( 4 ) << std::setfill( '0' );
        if( i!=0 ) 
           s2  << '\\';
        s2 << data[i];
     }
     *StrHexaArea=s2.str();
  }
  else if( vr == "SL" )
  {
     int32_t *data=(int32_t *)BinArea;
     for (unsigned int i=0; i < GetValueCount(); i++)
     {
        s2  << std::setw( 4 ) << std::setfill( '0' );
         if( i!=0 )
           s2  << '\\';
        s2 << data[i];
     }
     *StrHexaArea=s2.str();
  }
  else if( vr == "FL" )
  {
     unsigned char *toto=(unsigned char *)BinArea;
     for (unsigned int i=0; i < GetValueCount(); i++)
     {
         s2.str("");
         if( i!=0 )
           s2 << '\\';
         unsigned int a4;
         for(int iif=0; iif<4; iif++)
         {
            a4=toto[iif];
            s2 << a4; 
         }
     }
     *StrHexaArea=s2.str();
  }
  else if( vr == "FD" )
  {
     //double *data=(double *)BinArea;
     unsigned char *toto=(unsigned char *)BinArea;
     for (unsigned int i=0; i < GetValueCount(); i++)
     {
        s2.str("");
        if( i!=0 )
           s2 << '\\';
        //s2 << data[i];

         unsigned int a4;
         for(int iid=0; iid<8; iid++)
         {
            a4=toto[iid];
            s2 << a4; 
         }

     }
     *StrHexaArea=s2.str();
  }
  else
  {
     unsigned int l = (Length > GDCM_MAX_LENGTH_TO_CONVERT_TO_HEXA) ? GDCM_MAX_LENGTH_TO_CONVERT_TO_HEXA : Length;
     uint8_t *data=(uint8_t *)BinArea;
     for (unsigned int i=0; i < l; i++)
     {
        if( i!=0 )
           s2 << '\\';
        s2 << std::setw( 2 ) << (int)(data[i]);
     }
     if (Length > 16)
        s2 << "\\...";
     *StrHexaArea=s2.str();
   }
  return *StrHexaArea;
}

/**
 * \brief Copies all the attributes from an other DocEntry 
 * @param doc entry to copy from
 * @remarks The content BinArea is copied too (StrArea is not)
 */
void DataEntry::Copy(DocEntry *doc)
{
   DocEntry::Copy(doc);

   DataEntry *entry = dynamic_cast<DataEntry *>(doc);
   if ( entry )
   {
      State = entry->State;
      Flag = entry->Flag;
      CopyBinArea(entry->BinArea,entry->GetLength());      
   }
}

/**
 * \brief   Writes the 'common part' + the 'value' area of a DataEntry
 * @param fp already open ofstream pointer
 * @param filetype type of the file (ACR, ImplicitVR, ExplicitVR, ...)
 */
void DataEntry::WriteContent(std::ofstream *fp, FileType filetype, 
                                                      bool insideMetaElements, bool insideSequence)
{ 
   // writes the 'common part'
   DocEntry::WriteContent(fp, filetype, insideMetaElements, insideSequence);

   if ( GetGroup() == 0xfffe )
   {
      return; //delimitors have NO value
   }
   
   // --> We only deal with Little Endian writting.
   // --> forget Big Endian Transfer Syntax writting!
   //     Next DICOM version will give it up ...
 
   // WARNING - For Implicit VR private element,
   //           we have *no choice* but considering them as
   //           something like 'OB' values.
   //           we rewrite them as we found them on disc.
   //           Some trouble will occur if element was 
   //           *actually* OW, if image was produced 
   //           on Big endian based processor, read and writen 
   //           on Little endian based processor
   //           and, later on, somebody needs
   //           this 'OW' Implicit VR private element (?!?)
   //           (Same stuff, mutatis mutandis, for Little/Big)
 
   // 8/16 bits Pixels problem should be solved automatiquely,
   // since we ensure the VR (OB vs OW) is conform to Pixel size.
        
   uint8_t *data = BinArea; //safe notation
   size_t l = GetLength(); 
//   gdcmDebugMacro("in DataEntry::WriteContent " << GetKey() << " AtomicLength: "
//              << Global::GetVR()->GetAtomicElementLength(this->GetVR() ) // << " BinArea in :" << &BinArea
//             );
   if (BinArea) // the binArea was *actually* loaded
   {
#if defined(GDCM_WORDS_BIGENDIAN) || defined(GDCM_FORCE_BIGENDIAN_EMULATION)
      unsigned short vrLgth = 
                        Global::GetVR()->GetAtomicElementLength(this->GetVR());
      unsigned int i;
      switch(vrLgth)
      {
         case 1:
         {
            binary_write (*fp, data, l );           
            break;
         }     
         case 2:
         {
            uint16_t *data16 = (uint16_t *)data;
            for(i=0;i<l/vrLgth;i++)
               binary_write( *fp, data16[i]);
            break;
         }
         case 4:
         {
            uint32_t *data32 = (uint32_t *)data;
            for(i=0;i<l/vrLgth;i++)
               binary_write( *fp, data32[i]);
            break;
         }
         case 8:
         {
            double *data64 = (double *)data;
            for(i=0;i<l/vrLgth;i++)
               binary_write( *fp, data64[i]);
            break;
         }
      }
#else
   binary_write (*fp, data, l );
#endif //GDCM_WORDS_BIGENDIAN

   }
   else
   {
      // nothing was loaded, but we need to skip space on disc     
      if (l != 0)
      {
      //  --> WARNING : nothing is written; 
      //  --> the initial data (on the the source image) is lost
      //  --> user is *not* informed !      
         gdcmDebugMacro ("Nothing was loaded, but we need to skip space on disc. "
                      << "Length =" << l << " for " << GetKey() );   
         fp->seekp(l, std::ios::cur); // At Write time, for unloaded elems
      }
   }
   // to avoid gdcm to propagate oddities
   // (length was already modified)  
   if (l%2)
      fp->seekp(1, std::ios::cur);  // At Write time, for non even length elems
}

/**
 * \brief   Compute the full length of the elementary DataEntry (not only value
 *          length) depending on the VR.
 */
uint32_t DataEntry::ComputeFullLength()
{
   return GetFullLength();
}

//-----------------------------------------------------------------------------
// Protected

/// \brief Creates a DataEntry owned BinArea 
///       (remove previous one if any and relevant StrArea if any)
void DataEntry::NewBinArea( )
{
   DeleteBinArea();
   if( GetLength() > 0 )
      BinArea = new uint8_t[GetLength()];
   SelfArea = true;
}
/// \brief Removes the BinArea, if owned by the DataEntry, 
///        and the relevant StrArea if any
void DataEntry::DeleteBinArea(void)
{
   if (BinArea && SelfArea)
   {
      delete[] BinArea;
      BinArea = NULL;
   }
   if (StrArea)
   {
      delete StrArea;
      StrArea = 0;
   }
   if (StrHexaArea)
   {
      delete StrHexaArea;
      StrHexaArea = 0;
   }   
}

//-----------------------------------------------------------------------------
// Private

//-----------------------------------------------------------------------------
// Print
/**
 * \brief   Prints a DataEntry (Dicom entry)
 * @param   os ostream we want to print in
 * @param indent Indentation string to be prepended during printing
 */
void DataEntry::Print(std::ostream &os, std::string const & )
{
   //os << "D ";
   
   // First, Print the common part (vr [length offset] name).
   DocEntry::Print(os);

   uint16_t g = GetGroup();
   if (g == 0xfffe) // delimiters have NO value
   {          
      return; // just to avoid identing all the remaining code 
   }

   std::ostringstream s;
   TSAtr v;

   if( BinArea )
   {
      v = GetString();
      const VRKey &vr = GetVR();

      if( vr == "US" || vr == "SS" || vr == "UL" || vr == "SL" 
       || vr == "FL" || vr == "FD")
         s << " [" << GetString() << "] =0x(" << GetHexaRepresentation() << ")";
      else
      { 
         if(Global::GetVR()->IsVROfStringRepresentable(vr))
         {
            // replace non printable characters by '.'
            std::string cleanString = Util::CreateCleanString(v);
            if ( cleanString.length() <= GetMaxSizePrintEntry()
              || PrintLevel >= 3
              || IsNotLoaded() )
           // FIXME : when IsNotLoaded(), you create a Clean String ?!?
           // FIXME : PrintLevel<2 *does* print the values 
           //        (3 is only for extra offsets printing)
           // What do you wanted to do ? JPR
            {
               s << " [" << cleanString << "]";
            }
            else
            {
               s << " [GDCM_NAME_SPACE::too long for print (" << cleanString.length() << ") ]";
            }
         }
         else
         {
            // A lot of Private elements (with no VR) contain actually 
            // only printable characters;
            // Let's deal with them as is they were VR std::string representable
    
            if ( Util::IsCleanArea( GetBinArea(), GetLength()  ) )
            {
               // FIXME : since the 'Area' *is* clean, just use
               //         a 'CreateString' method, to save CPU time.
               std::string cleanString = 
                     Util::CreateCleanString( BinArea,GetLength()  );
               s << " [" << cleanString << "]";
            }
            else
            {
               s << " [" << GDCM_BINLOADED << ";"
               << "length = " << GetLength() << "] =0x(" << GetHexaRepresentation() << ")";      
            }
         }
      }
   }
   else
   {
      if( IsNotLoaded() )
         s << " [" << GDCM_NOTLOADED << "]";
      else if( IsUnfound() )
         s << " [" << GDCM_UNFOUND << "]";
      else if( IsUnread() )
         s << " [" << GDCM_UNREAD << "]";
      else if ( GetLength() == 0 )
         s << " []";
   }

   if( IsPixelData() )
      s << " (" << GDCM_PIXELDATA << ")";

   // Display the UID value (instead of displaying only the rough code)
   // First 'clean' trailing character (space or zero) 
   if(BinArea)
   {
      const uint16_t &gr = GetGroup();
      const uint16_t &elt = GetElement();
      TS *ts = Global::GetTS();

      if (gr == 0x0002)
      {
         // Any more to be displayed ?
         if ( elt == 0x0010 || elt == 0x0002 )
         {
            if ( v.length() != 0 )  // for brain damaged headers
            {
               if ( ! isdigit((unsigned char)v[v.length()-1]) )
               {
                  v.erase(v.length()-1, 1);
               }
            }
            s << "  ==>\t[" << ts->GetValue(v) << "]";
         }
      }
      else if (gr == 0x0008)
      {
         if ( elt == 0x0016 || elt == 0x1150 )
         {
            if ( v.length() != 0 )  // for brain damaged headers
            {
               if ( ! isdigit((unsigned char)v[v.length()-1]) )
               {
                  v.erase(v.length()-1, 1);
               }
            }
            s << "  ==>\t[" << ts->GetValue(v) << "]";
         }
      }
      else if (gr == 0x0004)
      {
         if ( elt == 0x1510 || elt == 0x1512  )
         {
            if ( v.length() != 0 )  // for brain damaged headers  
            {
               if ( ! isdigit((unsigned char)v[v.length()-1]) )
               {
                  v.erase(v.length()-1, 1);  
               }
            }
            s << "  ==>\t[" << ts->GetValue(v) << "]";
         }
      }
   }

   os << s.str();
}

//-----------------------------------------------------------------------------
} // end namespace gdcm

