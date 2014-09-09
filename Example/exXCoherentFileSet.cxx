/*=========================================================================

  Program:   gdcm
  Module:    $RCSfile: exXCoherentFileSet.cxx,v $
  Language:  C++
  Date:      $Date: 2009/05/28 15:44:34 $
  Version:   $Revision: 1.16 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmSerieHelper.h"
#include "gdcmFile.h"
#include "gdcmDebug.h"
#include <iostream>
#include "gdcmDirList.h"
#include "gdcmUtil.h"
#include "gdcmArgMgr.h"
int main(int argc, char *argv[])
{
   START_USAGE(usage)
   "\n exXCoherentFileSet :\n                                                 ",
   "Shows the various 'XCoherent' Filesets within a directory                 ",
   "Optionaly copies the images in a Directories tree                         ",
   "usage: exXCoherentFileSet {dirin=inputDirectoryName}                      ",
   "                           dirout=outputDirectoryName                     ",
   "                       { tag=group-elem | pos | ori } [sort] [write]      ",
   "                       [ { [noshadowseq] | [noshadow][noseq] } ] [debug]  ",
   "                                                                          ",
   "       dirin : user wants to analyze *all* the files                      ",
   "                            within the directory                          ",
   "       write : user wants to create directories                           ",
   "       dirout : will be created if doesn't exist                          ",
   "       pos  : user wants to split each Single SerieUID Fileset on the     ",
   "                         'Image Position '                                ",
   "       ori  : user wants to split each Single SerieUID Fileset on the     ",
   "                         'Image Orientation '                             ",
   "       tag : group-elem    (in hexa, no space)                            ",
   "                       the user wants to split on                         ",
   "       sort :  user wants FileHelper to sort the images                   ",
   "               Warning : will probabely crah if sort has no meaning       ",
   "                (not only look at image names)                            ",
   "       noshadowseq: user doesn't want to load Private Sequences           ",
   "       noshadow   : user doesn't want to load Private groups (odd number) ",
   "       noseq      : user doesn't want to load Sequences                   ",
   "       verbose    : user wants to run the program in 'verbose mode'       ",
   "       debug      : developper wants to run the program in 'debug mode'   ",
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

   const char *dirName  = am->ArgMgrGetString("dirin");
   if (dirName == 0)
   {
       std::cout <<std::endl
                 << "'dirin=' must be present;" 
                 <<  std::endl;
       am->ArgMgrUsage(usage); // Display 'usage'
       delete am;
       return 0;
   }

   const char *dirNameout;
   dirNameout  = am->ArgMgrGetString("dirout",".");

   bool pos  =    ( 0 != am->ArgMgrDefined("pos") );
   bool ori  =    ( 0 != am->ArgMgrDefined("ori") );
   bool sort =    ( 0 != am->ArgMgrDefined("sort") );
   bool write =   ( 0 != am->ArgMgrDefined("write") );
   bool verbose = ( 0 != am->ArgMgrDefined("verbose") );
   bool tag     = ( 0 != am->ArgMgrDefined("tag") );

   if( (tag && (pos || ori)) || (pos && (tag || ori)) || (ori && (tag || pos)) )
   {
      std::cout << " POS, ORI and TAG are mutually exclusive" << std::endl;
      delete am;
      return 0;
   }
   
   if( (!tag && !pos && !ori))
   {
      std::cout << " One of POS, ORI and TAG is mandatory!" << std::endl;
      delete am;
      return 0;
   }
   int nb;
   uint16_t *groupelem;
   if (tag)
   {
      groupelem = am->ArgMgrGetXInt16Enum("tag", &nb);
      if (nb != 1)
      {
         std::cout << "TAG : one and only one group,elem!" << std::endl;
         delete am;
         return 0;
      }
   }

   /* if unused Param we give up */
   if ( am->ArgMgrPrintUnusedLabels() )
   {
      am->ArgMgrUsage(usage);
      delete am;
      return 0;
   }

   delete am;  // ------ we don't need Arguments Manager any longer ------
   
   GDCM_NAME_SPACE::SerieHelper *s;
  
   s = GDCM_NAME_SPACE::SerieHelper::New();
   s->SetLoadMode(GDCM_NAME_SPACE::LD_ALL);     // Load everything for each File
   
   //GDCM_NAME_SPACE::TagKey t(0x0020,0x0013);
   //s->AddRestriction(t, "340", GDCM_NAME_SPACE::GDCM_LESS); // Keep only files where
                                                              // restriction is true

   s->SetDirectory(dirName, true); // true : recursive exploration

   // The Dicom file set is splitted into several 'Single SerieUID Files Sets'
   // (a 'Single SerieUID Files Set' per SerieUID)
   // In some cases, it's not enough, since, in some cases
   // we can find scout view with the same SerieUID

/*
   std::cout << " ---------------------------------------- "
             << "'Single UID' Filesets found in :["
             << dirName << "]" << std::endl;

   s->Print();
   std::cout << " ------------------------------------- Result after splitting"
             << std::endl;
*/

   std::string systemCommand;
   std::string filenameout;
   if (write) {
      if (verbose)
         std::cout << "Check for output directory :[" << dirNameout << "]."
                   <<std::endl;
      if ( ! GDCM_NAME_SPACE::DirList::IsDirectory(dirNameout) )    // dirout not found
      {
         std::string strDirNameout(dirNameout);          // to please gcc 4
         systemCommand = "mkdir " +strDirNameout;        // create it!
         if (verbose)
            std::cout << systemCommand << std::endl;
         system (systemCommand.c_str());
         if ( ! GDCM_NAME_SPACE::DirList::IsDirectory(dirNameout) ) // be sure it worked
         {
             std::cout << "KO : not a dir : [" << dirNameout << "] (creation failure ?)" 
                       << std::endl;
         return 0;
         }
         else
         {
            if (verbose)
               std::cout << "Directory [" << dirNameout << "] created." << std::endl;
         }
      }
      else
      {
         if (verbose)
            std::cout << "Output Directory [" << dirNameout
                      << "] already exists; Used as is."
                      << std::endl;
      }
   }
      // --> End of checking supposed-to-be-directory names

   int nbFiles;
   std::string fileName;

   // For all the Single SerieUID Files Sets of the GDCM_NAME_SPACE::Serie
   GDCM_NAME_SPACE::FileList *l = s->GetFirstSingleSerieUIDFileSet();
   GDCM_NAME_SPACE::XCoherentFileSetmap xcm;

   std::string serieUID;
   std::string currentSerieWriteDir = "";
   std::string xCoherentWriteDir = "";
   std::string xCoherentName = "";
   std::string serieDirectory;
   std::string lastFilename;
   std::string rep("_");
   int controlCount = 0;

   while (l) // for each 'Single SerieUID FileSet'
   { 
      nbFiles = l->size() ;
      if ( l->size() > 0 ) // ignore Series with less than 2 images.
                           // Why not ? Just an example, for testing!
      {
          serieUID = s->GetCurrentSerieUIDFileSetUID();
          GDCM_NAME_SPACE::Util::ReplaceSpecChar(serieUID, rep);

          // --- for write
          if (write)
          {
             currentSerieWriteDir = currentSerieWriteDir + dirNameout;
             unsigned int lg = strlen(dirNameout)-1;
             if ( dirNameout[lg] != '/'  &&  dirNameout[lg] != '\\')
                currentSerieWriteDir = currentSerieWriteDir + GDCM_NAME_SPACE::GDCM_FILESEPARATOR;

             currentSerieWriteDir = currentSerieWriteDir + serieUID;
             if (verbose)
                std::cout << "[" << currentSerieWriteDir<< "]" << std::endl;
            // if ( ! GDCM_NAME_SPACE::DirList::IsDirectory(currentSerieWriteDir) )
             {
                systemCommand   = "mkdir " + currentSerieWriteDir;
                system( systemCommand.c_str());
                if (verbose)
                   std::cout <<  "1 " <<systemCommand << std::endl;
            }
         } 
          // --- end for write

         std::cout << "Split the 'Single SerieUID' FileSet :[" 
                   << serieUID
                   << "]  " << nbFiles << " long" << std::endl;
         std::cout << "-----------------------------------" << std::endl;

         if (ori) 
            xcm = s->SplitOnOrientation(l);
         else if (pos)
            xcm = s->SplitOnPosition(l);
         else if (groupelem != 0) {
            xcm = s->SplitOnTagValue(l, groupelem[0],groupelem[1] );
         }

         for (GDCM_NAME_SPACE::XCoherentFileSetmap::iterator i = xcm.begin(); 
                                                  i != xcm.end();
                                                ++i)
         {
            xCoherentName = (*i).first;
            if (verbose)
               std::cout << "xCoherentName = " << xCoherentName << std::endl;
            GDCM_NAME_SPACE::Util::ReplaceSpecChar(serieUID, rep);
             // --- for write
             if (write)
             {
                xCoherentWriteDir = currentSerieWriteDir + GDCM_NAME_SPACE::GDCM_FILESEPARATOR+ xCoherentName;
               // if ( ! GDCM_NAME_SPACE::DirList::IsDirectory(xCoherentWriteDir) )
                {
                   systemCommand   = "mkdir " + xCoherentWriteDir;
                   system( systemCommand.c_str());
                   if (verbose)
                      std::cout << systemCommand << std::endl;
                }
            } 
            // --- end for write

            if (ori) 
               std::cout << "Orientation : ";
            else if (pos) 
               std::cout << "Position : ";
            else if (groupelem != 0)    
               std::cout << "Tag (" << std::hex << groupelem[0]
                         << "|" << groupelem[1] << ") value : ";
            std::cout << "[" << (*i).first << "]" << std::endl;

            if (verbose)
               std::cout << "xCoherentName [" << xCoherentName << "]" << std::endl;

           // Within a 'just to see' program, 
           // OrderFileList() causes trouble, since some files
           // (eg:MIP views) don't have 'Position', now considered as mandatory
           // Commented out for the moment.

           if (sort) {
              s->OrderFileList((*i).second);  // sort the XCoherent Fileset
              std::cout << "ZSpacing for the file set " << s->GetZSpacing()
                        << std::endl;
            }

            for (GDCM_NAME_SPACE::FileList::iterator it =  ((*i).second)->begin();
                                          it != ((*i).second)->end();
                                        ++it)
            {
               controlCount ++;
               fileName = (*it)->GetFileName();
               std::cout << "    " << fileName << std::endl;

               // --- for write
               if (write)
               {  
                  lastFilename =  GDCM_NAME_SPACE::Util::GetName( fileName );
                  filenameout = xCoherentWriteDir  + GDCM_NAME_SPACE::GDCM_FILESEPARATOR+ lastFilename;
                  systemCommand   = "cp " + fileName + " " + filenameout;
                  system( systemCommand.c_str());
                  if (verbose)
                     std::cout << "3 " << systemCommand << std::endl;
                }
               // --- end for write
            }
            std::cout << std::endl;
         }
      }
      l = s->GetNextSingleSerieUIDFileSet();
   }
    
   if ( controlCount == 0 )
      std::cout << "No suitable file was found!" << std::endl;

   s->Delete();

   return 0;
}
