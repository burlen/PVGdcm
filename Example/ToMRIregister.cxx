/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: ToMRIregister.cxx,v $
  Language:  C++
  Date:      $Date: 2007/10/24 08:03:44 $
  Version:   $Revision: 1.5 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#include "gdcmSerieHelper.h"
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmDebug.h"
#include "gdcmDirList.h"
#include "gdcmUtil.h"
#include "gdcmDataEntry.h"
#include "gdcmArgMgr.h"
#include <iostream>
#include <sstream>

bool AquisitionTime_0008_0032_Compare(GDCM_NAME_SPACE::File *file1, GDCM_NAME_SPACE::File *file2);

bool AquisitionTime_0008_0032_Compare(GDCM_NAME_SPACE::File *file1, GDCM_NAME_SPACE::File *file2)
{
   return file1->GetEntryString(0x0008,0x0032) < file2->GetEntryString(0x0008,0x0032);
}

int main(int argc, char *argv[])
{  

   START_USAGE(usage)
   "\n ToMriregister :\n                                                      ",
   " - Converts the Siemens Sonata MRI '*tfl2d1'                              ",
   " to be processable by MriRegister software                                ",
   " - May be used as a template for GDCM_NAME_SPACE::SerieHelper use.                   ",
   "                                                                          ",
   "usage: ToMriRegister dirin=inputDirectoryName                             ",
   "                     dirout=outputDirectoryName                           ",
   "                     [ { [noshadowseq] | [noshadow][noseq] } ] [debug]    ",
   "                                                                          ",
   "       inputDirectoryName : user wants to analyze *all* the files         ",
   "                            within the directory                          ",
   "       noshadowseq: user doesn't want to load Private Sequences           ",
   "       noshadow   : user doesn't want to load Private groups (odd number) ",
   "       noseq      : user doesn't want to load Sequences                   ",
   "       verbose    : user wants to run the program in 'verbose mode'       ",
   "       debug      : developer wants to run the program in 'debug mode'    ",
   FINISH_USAGE
   
   // ----- Initialize Arguments Manager ------
  
   GDCM_NAME_SPACE::ArgMgr *am = new GDCM_NAME_SPACE::ArgMgr(argc, argv);
  
   if (am->ArgMgrDefined("usage") || argc == 1) 
   {
      am->ArgMgrUsage(usage); // Display 'usage'
      delete am;
      return 0;
   }

   if (am->ArgMgrDefined("debug"))
      GDCM_NAME_SPACE::Debug::DebugOn();

   int verbose  = am->ArgMgrDefined("verbose");
         
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

   const char *dirIn  = am->ArgMgrGetString("dirin");
   if (dirIn == 0)
   {
       std::cout <<std::endl
                 << "'dirin=' must be present;" 
                 <<  std::endl;
       am->ArgMgrUsage(usage); // Display 'usage'  
       delete am;
       return 0;
   }

   const char *dirOut  = am->ArgMgrGetString("dirout");
   if (dirOut == 0)
   {
       std::cout <<std::endl
                 << "'dirout=' must be present;" 
                 <<  std::endl;
       am->ArgMgrUsage(usage); // Display 'usage'  
       delete am;
       return 0;
   }     
       
   /* if unused Param we give up */
   if ( am->ArgMgrPrintUnusedLabels() )
   {
      am->ArgMgrUsage(usage);
      delete am;
      return 0;
   } 
 
   delete am;  // ------ we don't need Arguments Manager any longer ------
   
   // ======================== more checking on the params ==============

   if ( ! GDCM_NAME_SPACE::DirList::IsDirectory(dirIn) )
   {
      std::cout << "KO : [" << dirIn << "] is not a Directory." << std::endl;
      return 0;

   }
   else
   {
      if (verbose)     
         std::cout << "OK : [" << dirIn << "] is a Directory." << std::endl;
   }
   
   std::string systemCommand;
   std::string strDirNameout(dirOut);          // to please gcc 4 
   if (verbose)
      std::cout << "Check for output directory :[" << dirOut << "]."
             <<std::endl;
   if ( ! GDCM_NAME_SPACE::DirList::IsDirectory(dirOut) )    // dirout not found
   {
      systemCommand = "mkdir " + strDirNameout;   // create it!
      if (verbose)
         std::cout << systemCommand << std::endl;
      system (systemCommand.c_str());
      if ( ! GDCM_NAME_SPACE::DirList::IsDirectory(dirOut) ) // be sure it worked
      {
          std::cout << "KO : not a dir : [" << dirOut 
                    << "] (creation failure ?)" << std::endl;
      return 0;

      }
      else
      {
        if (verbose)
           std::cout << "Directory [" << dirOut << "] created." << std::endl;
      }
   }
   else
   {
      if (verbose)
         std::cout << "Output Directory [" << dirOut 
                   << "] already exists; Used as is." << std::endl;
   } 
   
   // ========================== Now, we can do the job! ================     
                 
   // Just to see *all* the file names:
   
   GDCM_NAME_SPACE::DirList dirList(dirIn,true); // gets (recursively) the file list
   GDCM_NAME_SPACE::DirListType fileList = dirList.GetFilenames();
   for( GDCM_NAME_SPACE::DirListType::iterator it  = fileList.begin();
                                 it != fileList.end();
                                 ++it )
   {
      std::cout << "file [" << it->c_str() << "]" << std::endl;  
   }
   
   GDCM_NAME_SPACE::SerieHelper *s;
  
   s = GDCM_NAME_SPACE::SerieHelper::New();
   s->SetLoadMode(GDCM_NAME_SPACE::LD_NOSEQ);     // Don't load Sequences
 
   // we could choose to ignore some Files  
   //GDCM_NAME_SPACE::TagKey t(0x0010,0x0024);  // [Sequence Name]
   // Keep only files where restriction is true
   //s->AddRestriction(t, "*tfl2d1 ", GDCM_NAME_SPACE::GDCM_EQUAL); 
                                                    
   s->SetDirectory(dirIn, true); // true : recursive exploration

/*
   std::cout << " ---------------------------------------- "
             << "'Single UID' Filesets found in :["
             << dirName << "]" << std::endl;

   s->Print();
   std::cout << " ------------------------------------- Result after splitting"
             << std::endl;
*/
   int nbFiles;
   std::string fullFilename, lastFilename;
   char fullWriteFilename[1024]; // should be enough.

   std::ostringstream str;
  
   GDCM_NAME_SPACE::XCoherentFileSetmap xcm;
   GDCM_NAME_SPACE::FileHelper *fh;
   
   // will be used for ordering.
   s->SetUserLessThanFunction(AquisitionTime_0008_0032_Compare);
   int serieNumber = 0;
   int sliceNumber = 0;
   int imageNumber = 0;
         
   // For all the Single SerieUID Files Sets of the GDCM_NAME_SPACE::Serie
   GDCM_NAME_SPACE::FileList *l = s->GetFirstSingleSerieUIDFileSet();
   
   char numero[5];
   while (l)
   {   
      nbFiles = l->size() ;
      if ( l->size() < 8 ) // Hope we skip Scout views !
      {
         std::cout << "Ignore the 'Single SerieUID' FileSet :[" 
                   << s->GetCurrentSerieUIDFileSetUID()
                   << "]  " << nbFiles << " long" << std::endl;
         std::cout << "-----------------------------------" << std::endl;      
      }
      else
      {
         std::cout << "Split the 'Single SerieUID' FileSet :[" 
                   << s->GetCurrentSerieUIDFileSetUID()
                   << "]  " << nbFiles << " long" << std::endl;
         std::cout << "-----------------------------------" << std::endl;

         xcm = s->SplitOnPosition(l);
    
         //int sliceNumber = 0; 
         
         double position =0.0;
         char charPosition[10];
         
         for (GDCM_NAME_SPACE::XCoherentFileSetmap::iterator i = xcm.begin();
                                                  i != xcm.end();
                                                ++i)
         {
             std::cout << "Position : ";    
             std::cout << "[" << (*i).first << "]" << std::endl;
   
            s->OrderFileList((*i).second);  // sort the current XCoherent Fileset
    
            position = position + 1.0;
            sprintf(charPosition, "%f", position);
            
            //int imageNumber = 0;    
            for ( GDCM_NAME_SPACE::FileList::iterator it =  ((*i).second)->begin();
                                           it != ((*i).second)->end();
                                         ++it)
            {
    
            // Set the DataElements MriRegister needs to be happy
            // Probabely one of the following (check it !):
            /*
               0020 0011 IS 1 Series Number
               0020 0012 IS 1 Acquisition Number
               0020 0013 IS 1 Instance Number
           */

           /*
            Sure it needs ACR-NEMA elements : Study ID                20, 10 ?
                                              Image Number            20, 12 ?
                                              Location ('atof-able') -> 20, 50 ?    
           */     
            (*it)->InsertEntryString(charPosition,0x0020,0x0050, "DS");
   
           
            (*it)->InsertEntryString("0",0x0008,0x0000, "UL"); // Needs to be present (actual length doesn't matter !)    

            str.str("");
            str << serieNumber;
            (*it)->InsertEntryString(str.str(),0x0020,0x0011, "IS"); // Series Number

           /*
            str.str("");
            str << imageNumber;
            (*it)->InsertEntryString(str.str(),0x0020,0x0012, "IS"); // Acquisition Number
            (*it)->InsertEntryString(str.str(),0x0020,0x0013, "IS"); // Instance Number
           */
   
   sprintf(numero, "%04d", imageNumber);
   (*it)->InsertEntryString(numero,0x0020,0x0012, "IS"); // Acquisition Number   
   (*it)->InsertEntryString(numero,0x0020,0x0013, "IS"); // Instance Number
 
            // Load the pixels in RAM.    
      
               fh = GDCM_NAME_SPACE::FileHelper::New(*it);     
               uint8_t *imageData = fh->GetImageDataRaw(); // Don't convert (Gray Pixels + LUT) into (RGB pixels) ?!?
               if (!imageData)
                  std::cout << "fail to read [" << (*it)->GetFileName() << std::endl;
               fh->SetWriteTypeToAcr();  
               fh->SetContentType(GDCM_NAME_SPACE::UNMODIFIED_PIXELS_IMAGE);
    
               // forge the file name
      
               fullFilename = (*it)->GetFileName();
               lastFilename =  GDCM_NAME_SPACE::Util::GetName( fullFilename );
               //fullWriteFilename = strDirNameout + GDCM_NAME_SPACE::GDCM_FILESEPARATOR 
               //                                  + lastFilename;
               sprintf(fullWriteFilename, "%s%c%04d-%04d-%04d.dcm", 
                                      dirOut, GDCM_NAME_SPACE::GDCM_FILESEPARATOR,
                                      serieNumber, sliceNumber, imageNumber);

               if (verbose)
               {
                 // show [sliceLocation 0x0020,1041 (if any)] old name, -> newname
                 std::string strSliceLocation;
                 /*  
                 GDCM_NAME_SPACE::DataEntry e = (*it)->GetDataEntry(0x0020,0x1041);
                 if (e)
                    strSliceLocation = e->GetString();
                 else
                    strSliceLocation = "";
                 */ 
                  strSliceLocation = (*it)->GetDataEntry(0x0020,0x1041)->GetString();   
                  std::cout <<strSliceLocation << ": " << fullFilename << " -> " << fullWriteFilename << std::endl;
               }
               // Write it, now
       
               fh->SetWriteTypeToAcrLibido();
       
               if (!fh->Write(fullWriteFilename))
               {
                  std::cout << "Fail to write :[" << fullWriteFilename << "]"
                            << std::endl;
               }
     
               ///\todo FIXME segfaults if uncommented ?!?       
               // delete(imageData);    
               (*it)->Delete();
               fh->Delete();
      
               imageNumber++;  
       
            } 
            std::cout << std::endl; 
            sliceNumber++;  
         }
      } 
      serieNumber++;       
      l = s->GetNextSingleSerieUIDFileSet();
   }   
   s->Delete();
   return 0;
}
