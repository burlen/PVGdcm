/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: PatchHeader.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:04 $
  Version:   $Revision: 1.8 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmCommon.h"
#include "gdcmDebug.h"
#include "gdcmDirList.h"
#include "gdcmDocEntry.h"
#include "gdcmArgMgr.h"

#include <iostream>

// ------------------------------------------------------------------------
// gdcm mechanisms don't allow user to read an image 
// whose header contains wrong physical info (say : wrong Row Number ...)
// and re-write with a right value.
// This program does the job by brutally overwritting the wrong values.
// It may be usefull to save a set of images ...
// (It doesn't allow to *add* a missing field) 
// ------------------------------------------------------------------------

// global variables will be seen inside any function.

GDCM_NAME_SPACE::File *f;
std::fstream *fp;

uint16_t samplesperpixel;
uint16_t planarconfiguration;
uint16_t size;
uint16_t rows;
uint16_t columns;
uint16_t planes;
uint16_t bitsallocated;
uint16_t bitsstored;
uint16_t highbit;
uint16_t pixelrepresentation;

int bsamplesperpixel;
int bplanarconfiguration;
int bsize;
int brows;
int bcolumns;
int bplanes;
int bbitsallocated;
int bbitsstored;
int bhighbit;
int bpixelrepresentation;

void update()
{
   uint32_t offset;
   GDCM_NAME_SPACE::DocEntry *d;
      // 
      //  Fields to replace.
      // 

      // Samples Per Pixel
      if (bsamplesperpixel) 
      {
         d = f->GetDocEntry( 0x0028, 0x0002);
         if (d)
         {
            offset = d->GetOffset();
            fp->seekp( offset, std::ios::beg );
            fp->write( (char *)&samplesperpixel, 2 );
         }
      }
      // Rows
      if (brows) 
      {
         d = f->GetDocEntry( 0x0028, 0x0010);
         offset = d->GetOffset();
         fp->seekp( offset, std::ios::beg );
         fp->write( (char *)&rows, 2 );
      }
      // Columns
      if (bcolumns)
      { 
         d = f->GetDocEntry( 0x0028, 0x0011);
         offset = d->GetOffset();
         fp->seekp( offset, std::ios::beg );
         fp->write( (char *)&columns, 2 );
      }
      // Planes
      if (bplanes) 
      {
         d = f->GetDocEntry( 0x0028, 0x0012);
         if (d)
         {
            offset = d->GetOffset();
            fp->seekp( offset, std::ios::beg );
            fp->write( (char *)&planes, 2 );
         }
      }
      // Bits Allocated
      if (bbitsallocated) 
      {
         d = f->GetDocEntry( 0x0028, 0x0100);
         offset = d->GetOffset();
         fp->seekp( offset, std::ios::beg );
         fp->write( (char *)&bitsallocated, 2 );
      }

      // Bits Stored
      if (bbitsstored) 
      {
         d = f->GetDocEntry( 0x0028, 0x0101);
         offset = d->GetOffset();
         fp->seekp( offset, std::ios::beg );
         fp->write( (char *)&bitsstored, 2 );
      }

      // High Bit
      if (bhighbit) 
      {
         d = f->GetDocEntry( 0x0028, 0x0102);
         offset = d->GetOffset();
         fp->seekp( offset, std::ios::beg );
         fp->write( (char *)&highbit, 2 );
      }

      // Pixel Representation
      if (bpixelrepresentation) 
      {
         d = f->GetDocEntry( 0x0028, 0x0103);
         offset = d->GetOffset();
         fp->seekp( offset, std::ios::beg );
         fp->write( (char *)&pixelrepresentation, 2 );
      }
}

int main(int argc, char *argv[])
{
   START_USAGE(usage)
   "\n PatchHeader :\n                                                        ",
   " Allows aware user to patch an image header, without loading image.       ",
   "         Warning : the image(s) is/are overwritten                        ",
   "                   to preserve image(s) integrity, use a copy.            ",
   "         WARNING : *NO CHECK* is performed on the new values.             ",
   "                   Use only if you are sure the original values are wrong ",
   "                   *and* your values are right...                         ",
   "usage: PatchHeader {filein=inputFileName|dirin=inputDirectoryName}        ",
   "               [ { [size=] | [rows=][columns=] } ] [planes=]              ",
   "               [bitsallocated=] [bitsstored=]                             ",
   "               [highbit=] [samplesperpixel=]                              ",
   "               [pixelrepresentation=] [samplesperpixel=]                  ",
   "               [ { [noshadowseq] | [noshadow][noseq] } ] [debug]          ",
   "                                                                          ",
   "       inputFileName : Name of the (single) file user wants to modify     ",
   "       inputDirectoryName : user wants to modify *all* the files          ",
   "                            within the directory                          ",
   "       size            : new (square) size, to owerwrite old (wrong) one  ",
   "         or                                                               ",
   "       rows            : new Rows number,    to owerwrite old (wrong) one ",
   "       columns         : new Columns number, to owerwrite old (wrong) one ",
   "       planes          : new Planes number,  ...                          ",
   "       bitsallocated   : new Bits Allocated number,  ...                  ",
   "       bitsstored      : new Bits Stored number,  ...                     ",
   "       highbit         : new High Bit number,  ...                        ",
   "       samplesperpixel : new Samples Per Pixel, ...                       ",
   "       pixelrepresentation : new Pixel Representation, ...                ",
   "                                                                          ",
   "       noshadowseq: user doesn't want to load Private Sequences           ",
   "       noshadow   : user doesn't want to load Private groups (odd number) ",
   "       noseq      : user doesn't want to load Sequences                   ",
   "       debug      : user wants to run the program in 'debug mode'         ",
   FINISH_USAGE

   // ----- Initialize Arguments Manager ------
  
   GDCM_NAME_SPACE::ArgMgr *am = new GDCM_NAME_SPACE::ArgMgr(argc, argv);
  
   if (am->ArgMgrDefined("usage") || argc == 1) 
   {
      am->ArgMgrUsage(usage); // Display 'usage'
      delete am;
      return 0;
   }

   const char *fileName = am->ArgMgrGetString("filein");
   const char *dirName  = am->ArgMgrGetString("dirin");

   if ( (fileName == 0 && dirName == 0)
        ||
        (fileName != 0 && dirName != 0) )
   {
       std::cout <<std::endl
                 << "Either 'filein=' or 'dirin=' must be present;" 
                 << std::endl << "Not both" << std::endl;
       am->ArgMgrUsage(usage); // Display 'usage'  
       delete am;
       return 0;
    }

   bsamplesperpixel = am->ArgMgrDefined("samplesperpixel");
   if ( bsamplesperpixel )
      samplesperpixel = am->ArgMgrWantInt("samplesperpixel",usage);

   planarconfiguration = am->ArgMgrDefined("planarconfiguration");
   if ( bplanarconfiguration )
      planarconfiguration = am->ArgMgrWantInt("planarconfiguration",usage);

   bsize = am->ArgMgrDefined("size");
   brows      = am->ArgMgrDefined("rows");
   bcolumns   = am->ArgMgrDefined("columns");

   if (bsize && (brows || bcolumns) )
   {
      printf ("Use 'SIZE=...' or 'ROWS=... COLUMNS=...'\n");
      printf ("Not both\n"); 
      delete am;
      return 0;
   }

   if ( bsize )
    size = am->ArgMgrWantInt("size",usage);

   if ( brows )
      rows = am->ArgMgrWantInt("rows",usage);

   if ( bcolumns )
      columns = am->ArgMgrWantInt("columns",usage);

   if (bsize)
   {
      brows = bcolumns = true;
      rows = columns = size;
   }
   bplanes = am->ArgMgrDefined("planes");
   if ( bplanes )
      planes = am->ArgMgrWantInt("planes",usage);

   bbitsallocated = am->ArgMgrDefined("bitsallocated");
   if ( bbitsallocated )
      bitsallocated = am->ArgMgrWantInt("bitsallocated",usage);

   bbitsstored = am->ArgMgrDefined("bitsstored");
   if ( bbitsstored )
      bitsstored = am->ArgMgrWantInt("bitsstored",usage);

   bhighbit = am->ArgMgrDefined("highbit");
   if ( bhighbit )
      highbit = am->ArgMgrWantInt("highbit",usage);

   bpixelrepresentation = am->ArgMgrDefined("pixelrepresentation");
   if ( bpixelrepresentation )
      pixelrepresentation = am->ArgMgrWantInt("pixelrepresentation",usage);

   if (am->ArgMgrDefined("debug"))
      GDCM_NAME_SPACE::Debug::DebugOn();
 
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

   /* if unused Param we give up */
   if ( am->ArgMgrPrintUnusedLabels() )
   {
      am->ArgMgrUsage(usage);
      delete am;
      return 0;
   } 
 
   delete am;  // ------ we don't need Arguments Manager any longer ------
   

   if ( fileName != 0 ) // ====== Deal with a single file ======
   {
      // 
      //   Parse the input file.
      // 
      f = GDCM_NAME_SPACE::File::New( );
      f->SetLoadMode(loadMode);
      f->SetFileName( fileName );
      bool res = f->Load();

      // GDCM_NAME_SPACE::File::IsReadable() is no usable here, because we deal with
      // any kind of GDCM_NAME_SPACE::Readable *document*
      // not only GDCM_NAME_SPACE::File (as opposed to GDCM_NAME_SPACE::DicomDir)
      if ( !res ) 
      {
         std::cout <<std::endl
            << "Sorry, " << fileName <<"  not a gdcm-readable "
            << "DICOM / ACR Document"
            << std::endl;
         f->Delete();
         return 1;
      }
      std::cout << fileName << " is readable " << std::endl;

      // 
      //      No need to load the pixels in memory.      
      //      File will be overwritten
      // 

      // open the file
      fp = new std::fstream(fileName, 
                              std::ios::in | std::ios::out | std::ios::binary); 

      update ();

      fp->close();
      delete fp; 
      f->Delete();
      return 0;

   }
   else  // ====== Deal with a (single Patient) Directory ======
   {
      std::cout << "dirName [" << dirName << "]" << std::endl;
      GDCM_NAME_SPACE::DirList dirList(dirName,1); // gets recursively the file list
      GDCM_NAME_SPACE::DirListType fileList = dirList.GetFilenames();
      for( GDCM_NAME_SPACE::DirListType::iterator it  = fileList.begin();
                                 it != fileList.end();
                                 ++it )
      {
         f = GDCM_NAME_SPACE::File::New( );
         f->SetLoadMode(loadMode);
         f->SetFileName( it->c_str() );
         bool res = f->Load();

         if ( !res )
         {
            f->Delete(); 
            continue;
         }

         // open the file
         fp = new std::fstream(it->c_str(), 
                              std::ios::in | std::ios::out | std::ios::binary); 
         update();
     

         fp->close();
         delete fp; 
         f->Delete();
      }
   }
   return 0;
}

