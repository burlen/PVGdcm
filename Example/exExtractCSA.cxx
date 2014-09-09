/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: exExtractCSA.cxx,v $
  Language:  C++
  Date:      $Date: 2007/10/03 09:14:55 $
  Version:   $Revision: 1.8 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

/*
 * http://www.enac.northwestern.edu/~tew/archives/2003/02/25/incomplete-dicom-headers/
 * http://www.nmr.mgh.harvard.edu/~rudolph/software/vox2ras/download/vox2ras_rsolve.m
 * http://www.mail-archive.com/freesurfer@nmr.mgh.harvard.edu/msg03409.html
 * http://www.mmrrcc.upenn.edu/CAMRIS/cfn/dicomhdr.html
 */

// See this one :
// http://www.mmrrcc.upenn.edu/CAMRIS/cfn/dicomhdr.html

#include <iostream>
#include <fstream>
#include <stdint.h>
#include <assert.h>
#include <vector>
#include <map>

#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmCommon.h"
#include "gdcmDebug.h"
#include "gdcmDocEntry.h"
#include "gdcmDataEntry.h"
#include "gdcmSeqEntry.h"
#include "gdcmSQItem.h"
#include "gdcmArgMgr.h"


// --------------------------------------------------------

typedef struct {
   uint32_t  Item_xx[4];
   int Len;
   std::string Value;
} CSA_item;

typedef std::vector<CSA_item *> ItemVector;

typedef struct {
   std::string Name;
   int VM;
   std::string VR;
   int Syngodt;
   int Nitems;
   ItemVector Items_set;   
} CSA_entry;

typedef std::map<std::string, CSA_entry *> CSA_content;

// --------------------------------------------------------


struct equ
{ 
  uint32_t syngodt;
  char vr[2+1];
};

// Looks like there is mapping in between syngodt and vr...
//  O <=> UN
//  3 <=> DS
//  4 <=> FD
//  5 <=> FL
//  6 <=> IS
//  9 <=> UL
// 10 <=> US
// 16 <=> CS
// 19 <=> LO
// 20 <=> LT
// 22 <=> SH
// 25 <=> UI
static equ mapping[] = {
  {  0 , "UN" },
  {  3 , "DS" },
  {  4 , "FD" },
  {  5 , "FL" },
  {  6 , "IS" },
  {  7 , "SL" },
  {  8 , "SS" },
  {  9 , "UL" },
  { 10 , "US" },
  { 16 , "CS" },
  { 19 , "LO" },
  { 20 , "LT" },
  { 22 , "SH" },
  { 23 , "ST" },
  { 25 , "UI" },
  { 27 , "UT" }
};

bool check_mapping(uint32_t syngodt, const char *vr)
{
  static const unsigned int max = sizeof(mapping) / sizeof(equ);
  unsigned int s = 0;
  const equ *p = mapping;
  assert( syngodt <= mapping[max-1].syngodt );
  while(p->syngodt < syngodt )
  {
    //std::cout << "mapping:" << p->vr << std::endl;
    ++p;
  }
  assert( p->syngodt == syngodt ); // or else need to update mapping
  const char* lvr = p->vr;
  int check = strcmp(vr, lvr) == 0;
  assert( check );
  return true;
}


     ///\to  fix the Destructor!
void  DeleteCSA_content (CSA_content &myMap) {
   for ( CSA_content::const_iterator it = myMap.begin();
                                    it != myMap.end();
                                  ++it)
   { 
     ItemVector item_v = (*it).second->Items_set;
     for ( ItemVector::const_iterator it2  = item_v.begin();
                                      it2 != item_v.end();
                                    ++it2)
     {
       // delete (*it2); 
     }
     //delete (*it).second;   
   } 
}

void PrintCSA_content(CSA_content &myMap) {
   int item_no;
   for ( CSA_content::const_iterator it = myMap.begin();
                                    it != myMap.end();
                                  ++it)
   { 
     std::cout << "[" << (*it).second->Name << "] : VR=[" << (*it).second->VR 
               << "] vm = " << (*it).second->VM << std::endl;
     item_no = 0;
     ItemVector item_v = (*it).second->Items_set;
     for ( ItemVector::const_iterator it2 = item_v.begin();
                                      it2 != item_v.end();
                                    ++it2)
     {
       std::cout << "      --- item no : " << item_no << std::endl;
       std::cout << "      Item_xxx : " << (*it2)->Item_xx[0] << " " 
                 << (*it2)->Item_xx[1] 
                 << " " << (*it2)->Item_xx[2] << " " << (*it2)->Item_xx[3] 
                 << std::endl;
    
       std::cout << "      Len = " << (*it2)->Len ;
       std::cout << "      Value = [" << (*it2)->Value << "]" << std::endl;
   
       item_no++;
     } 
   }
}

int main(int argc, char *argv[])
{
   START_USAGE(usage)
   "\n exExtractCSA :\n                                                       ",
   "Extracts and displays the CSA tag(s) of gdcm-parsable Dicom file          ",
   "                                                                          ",
   "usage: exExtractCSA {filein=inputFileName|dirin=inputDirectoryName}       ",
   "                   tmp=temporaryWorkFileName                              ",
   "                       [extract=listOfElementsToExtract]                  ",
   "                       [ { [noshadowseq] | [noshadow][noseq] } ] [debug]  ",
   "       inputFileName : Name of the (single) file user wants to anonymize  ",
   "       listOfElementsExtract : group-elem,g2-e2,... (in hexa, no space)   ",
   "                                of Elements to extract                    ",
   "                              default : 0029-1010,0029-1020               ",
   "       noshadowseq: user doesn't want to load Private Sequences           ",
   "       noshadow   : user doesn't want to load Private groups (odd number) ",
   "       noseq      : user doesn't want to load Sequences                   ",
   "       verbose    : developper wants to run the program in 'verbose mode' ",
   "       debug      : developper wants to run the program in 'debug mode'   ",
   FINISH_USAGE

   // ----- Initialize Arguments Manager ------
  
   GDCM_NAME_SPACE::ArgMgr *am = new GDCM_NAME_SPACE::ArgMgr(argc, argv);
  
   if (am->ArgMgrDefined("usage") || argc == 1) 
   {
      am->ArgMgrUsage(usage); // Display 'usage'
      delete am;
      return 1;
   }

   if (am->ArgMgrDefined("debug"))
      GDCM_NAME_SPACE::Debug::DebugOn();

   bool verbose = am->ArgMgrDefined("verbose");
   
   const char *fileName = am->ArgMgrGetString("filein");

   int loadMode = GDCM_NAME_SPACE::LD_ALL;
   if ( am->ArgMgrDefined("noshadowseq") )
      loadMode |= GDCM_NAME_SPACE::LD_NOSHADOWSEQ;
   else 
   {
      if ( am->ArgMgrDefined("noshadow") )
         loadMode |= GDCM_NAME_SPACE::LD_NOSHADOW;
      if ( am->ArgMgrDefined("noseq") )
         loadMode |= GDCM_NAME_SPACE::LD_NOSEQ;
   }
   
   const char *tempWorkFile = am->ArgMgrGetString("tmp");

   int extractNb;
   uint16_t *elemsToExtract;
   if (am->ArgMgrDefined("extract")) 
   {
      am->ArgMgrGetXInt16Enum("extract", &extractNb);
      std::cout << "extractNb=" << extractNb << std::endl;
      if (extractNb =! 0)
         for (int k=0;k<extractNb; k++)
            std::cout << std::hex << elemsToExtract[2*k] << "|" << elemsToExtract[2*k+1] <<std::endl;
   }
   else 
   {
     elemsToExtract = new  uint16_t[4];
     elemsToExtract[0] = 0x0029;
     elemsToExtract[1] = 0x1010;
     elemsToExtract[2] = 0x0029;  
     elemsToExtract[3] = 0x1020;
     extractNb=2;
   }     

   /* if unused Param we give up */
   if ( am->ArgMgrPrintUnusedLabels() )
   {
      am->ArgMgrUsage(usage);
      delete am;
      return 1;
   }  
   delete am;  // ------ we don't need Arguments Manager any longer ------

// ============================================================
//   Read the input image.
// ============================================================ 
  
   GDCM_NAME_SPACE::File *f = GDCM_NAME_SPACE::File::New( );

   //f->SetLoadMode(GDCM_NAME_SPACE::LD_NOSEQ | GDCM_NAME_SPACE::LD_NOSHADOW);
   f->SetFileName( fileName );
   f->SetMaxSizeLoadEntry(0xffff);
   bool res = f->Load();  

   if( GDCM_NAME_SPACE::Debug::GetDebugFlag())
   {
      std::cout << "---------------------------------------------" << std::endl;
      f->Print();
      std::cout << "---------------------------------------------" << std::endl;
   }
   if (!res) {
       std::cerr << "Sorry, " << fileName << " not a gdcm-readable "
           << "DICOM / ACR File"
           << std::endl;
      f->Delete();
      return 1;
   }
   std::cout << " ... is readable " << std::endl;
   
   
// --------------------------------------------------------
CSA_content myMap;
CSA_entry  *myEntry; 
CSA_item   *myItem;
// --------------------------------------------------------

// For each tag user wants to extract :

for (int tag_no=0; tag_no<extractNb; tag_no++) {

   uint16_t group = elemsToExtract[2*tag_no];
   uint16_t elem = elemsToExtract[2*tag_no+1];
   
   if (verbose)
      std::cout << "Let's try tag : " << std::hex << group << "|" << elem << std::endl;
      
   std::string dicom_tag_value = f->GetEntryString(group, elem);
   if (dicom_tag_value == GDCM_NAME_SPACE::GDCM_UNFOUND)
   {
     GDCM_NAME_SPACE::DictEntry *dictEntry = f->GetPubDict()->GetEntry( group, elem);
     if (dictEntry != NULL)
        std::cerr << "Image doesn't contain any tag: " << dictEntry->GetName() 
                  << std::endl;
     else
        std::cerr << "Dicom Dictionary doesn't contain any tag: " 
          << std::hex << group << "|" << elem << std::endl; 
     f->Delete();
     return 1;
   }

   GDCM_NAME_SPACE::DocEntry *dicom_tag_doc = f->GetDocEntry(group, elem);
   GDCM_NAME_SPACE::DataEntry *dicom_tag = dynamic_cast<GDCM_NAME_SPACE::DataEntry *>(dicom_tag_doc);
   if( !dicom_tag )
   {
      std::cerr << "Sorry DataEntry only please" << std::endl;
      f->Delete();
      return 1;
   }

   // Write out the data as a file:
   std::ofstream o(tempWorkFile);
   if( !o )
   {
      std::cerr << "Problem opening file: [" << tempWorkFile << "]" 
                << std::endl;
      f->Delete();
      return 1;
   }
   o.write((char*)dicom_tag->GetBinArea(), dicom_tag->GetLength());
   o.close();
   
   
//#define OLDFORMAT

std::ifstream is( tempWorkFile );
#ifndef OLDFORMAT
char dummy[4+1];
dummy[4] = 0;
is.read(dummy, 4);
if (verbose)
 std::cout << dummy << std::endl;
if( strcmp( dummy, "SV10" )  ) 
{
   std::cerr << "Either not a SV10 header or filled with 0..." << std::endl;
   return 1;
}
// wotsit ?
is.read(dummy, 4);
if( strcmp( dummy, "\4\3\2\1" )  ) 
{
  std::cerr << "Either not a SV10 header or filled with 0..." << std::endl;
  return 1;
}
if (verbose)
   std::cout << (int)dummy[0] << (int)dummy[1] << (int)dummy[2] 
             << (int)dummy[3]<< std::endl;
#endif
uint32_t n;
is.read((char*)&n, sizeof(n));
if (verbose)
   std::cout << "number of entries " <<n << std::endl;
uint32_t unused;
is.read((char*)&unused, sizeof(unused));
if (verbose)
   std::cout << "unused " << unused<< std::endl;
assert( unused == 77 ); // 'M' character...

for(uint32_t i = 0; i < n; ++i)
{
if (verbose)
     std::cout << "============================================== " << i <<
     std::endl;
  char name[64+1];
  name[64] = 0; // security
  //std::cout << "Pos 0x" << std::hex << is.tellg() << std::dec << std::endl;
  is.read(name, 64);
if (verbose)
     std::cout << "Name=[" << name << "]" << std::endl;
  uint32_t vm;
  is.read((char*)&vm, sizeof(vm));
if (verbose)
     std::cout << "vm=" << vm <<  std::endl;
  char vr[4];
  is.read(vr, 4);
#ifndef OLDFORMAT
     assert( vr[2] == vr[3] && vr[2] == 0 );
#endif
if (verbose)
     std::cout << "vr=[" << vr << "]" <<std::endl;
  uint32_t syngodt;
  is.read((char*)&syngodt, sizeof(syngodt));
  check_mapping(syngodt, vr);

if (verbose)
     std::cout << "syngodt=" << syngodt << std::endl;
  uint32_t nitems;
  is.read((char*)&nitems, sizeof(nitems));
if (verbose)
     std::cout << "nitems=" << nitems<< std::endl;
  uint32_t xx;
  is.read((char*)&xx, sizeof(xx));
  //std::cout << "xx=" << xx<< std::endl;
  assert( xx == 77 || xx == 205 );

  myEntry = new CSA_entry;
  myEntry->Name    = name;
  myEntry->VM      = vm;
  myEntry->VR      = vr;
  myEntry->Syngodt = syngodt;
  myEntry->Nitems  = nitems;

  for( uint32_t j = 0; j < nitems; ++j)
  {
if (verbose)
       std::cout << "-------------------------------------------- " << j
                 << std::endl;
    uint32_t item_xx[4];
    is.read((char*)&item_xx, 4*sizeof(uint32_t));
if (verbose)
       std::cout << std::dec 
                 << "item_xx=" << item_xx[0] << " " << item_xx[1] << " " 
                 << item_xx[2] << " " << item_xx[3] << std::endl;
    //std::cout << "0x" << std::hex << is.tellg() << std::dec << std::endl;
    assert( item_xx[2] == 77 || item_xx[2] == 205 );
    uint32_t len = item_xx[1]; // 2nd element
if (verbose)
       std::cout << "len=" << len << std::endl;
    assert( item_xx[0] == item_xx[1] && item_xx[1] == item_xx[3] );
    char *val = new char[len+1];
    val[len] = 0; // security
    is.read(val,len);
    // WARNING vr does not means anything AFAIK, 
    // simply print the value as if it was IS/DS or LO (ASCII)
if (verbose)
     std::cout << "val=[" << val << "]" << std::endl;

    char dummy[4];
    uint32_t dummy_len = (4 - len % 4) % 4;
    is.read(dummy, dummy_len );


    for(uint32_t d= 0; d < dummy_len; ++d)
    {
     // I think dummy should always be 0
      if( dummy[d] )
      {
if (verbose)
           std::cout << "dummy=" << (int)dummy[d] << std::endl;
      }
    }

     myItem = new CSA_item;
     for (int i2=0; i2<4; i2++)
        myItem->Item_xx[i2] = item_xx[i2];
     myItem->Len = len;
     myItem->Value = val;
     
     myEntry->Items_set.push_back(myItem);
     delete[] val;     

   }
   myMap[name] = myEntry;
  }

std::cout << "================================================================"
          << std::endl
          << "=== Extract :" << std::hex << group << " " << elem  << "========"
          << std::endl
          << "================================================================"
          << std::endl;

  PrintCSA_content (myMap);
  DeleteCSA_content (myMap);
}

f->Delete();   
return 0;
}
