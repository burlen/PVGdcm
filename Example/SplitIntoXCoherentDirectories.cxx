/*=========================================================================
  
  Program:   gdcm
  Module:    $RCSfile: SplitIntoXCoherentDirectories.cxx,v $
  Language:  C++
  Date:      $Date: 2009/05/28 15:44:34 $
  Version:   $Revision: 1.4 $
 
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
       
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
     
=========================================================================*/
#include "gdcmSerieHelper.h"
#include "gdcmFileHelper.h"
#include "gdcmFile.h"
#include "gdcmDebug.h"
#include <iostream>
#include "gdcmDirList.h"
#include "gdcmUtil.h"
#include "gdcmArgMgr.h"
#include "gdcmDictSet.h"  // for GetName 
int main(int argc, char *argv[])
{

   START_USAGE(usage)
   "\n exXCoherentFileSet :\n                                                 ",
   "Shows the various 'XCoherent' Filesets within a directory                 ",
   "Optionaly copies the images in a Directories tree                         ",
   "usage: exXCoherentFileSet {dirin=inputDirectoryName}                      ",
   "                           dirout=outputDirectoryName                     ",
   "                       { tag=group-elem | pos | ori } [sort]              ",
   "                       [{ write | copy }] [studyUID = ]                   ",
   "                       [ { [noshadowseq] | [noshadow][noseq] } ] [debug]  ",
   "                                                                          ",
   "       dirin : user wants to analyze *all* the files                      ",
   "                            within the directory                          ",
   "       copy  : user wants to copy the files into a directories tree       ",
   "       write : user wants to rewrite the files into a directories tree    ",
   "               each directory with the same 'Series Instance UID'         ",
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
   "       studyUID   : *aware* user wants to add the serie                   ",
   "                                             to an already existing study ",
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
 
   int loadMode;
   int maxSize;

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
   bool copy =    ( 0 != am->ArgMgrDefined("copy") );
   bool write =   ( 0 != am->ArgMgrDefined("write") );
   bool verbose = ( 0 != am->ArgMgrDefined("verbose") );
   bool tag     = ( 0 != am->ArgMgrDefined("tag") );

   if( copy && write )
   {
      std::cout << "COPY and WRITE are mutually exclusive" << std::endl;
      delete am;
      return 0;
   }

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

   bool userDefinedStudy = ( 0 != am->ArgMgrDefined("userDefinedStudy") );
   const char *studyUID  = am->ArgMgrGetString("studyUID");
            
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

   s->SetLoadMode(GDCM_NAME_SPACE::LD_ALL); // Load everything for each File
   s->SetDirectory(dirName, true);          // true : recursive exploration

   GDCM_NAME_SPACE::File *f;
   
   GDCM_NAME_SPACE::DirList dirlist(dirName, true); // recursive exploration
   GDCM_NAME_SPACE::DirListType fileNames = dirlist.GetFilenames();

   GDCM_NAME_SPACE::FileList *l = new GDCM_NAME_SPACE::FileList;
// Loop on all the gdcm-readable files
   for (GDCM_NAME_SPACE::DirListType::iterator it = fileNames.begin();
                                    it != fileNames.end();
                                  ++it)
   {
      if (verbose)
         std::cout << *it << std::endl;

      if (write) {
         loadMode = GDCM_NAME_SPACE::LD_ALL; // load any DataElement
         maxSize  = 0x7fff;                  // load any length
      } else {
         loadMode = GDCM_NAME_SPACE::LD_NOSEQ | GDCM_NAME_SPACE::LD_NOSHADOW ; 
         maxSize  = 0x0100;
      }

      f = GDCM_NAME_SPACE::File::New();
      f->SetLoadMode(loadMode);
      f->SetMaxSizeLoadEntry(maxSize);
      f->SetFileName( *it );
      f->Load();
      l->push_back(f);
   }

   std::string systemCommand;
   std::string filenameout;
   if (write || copy) {
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

  // GDCM_NAME_SPACE::FileList *l = s->GetFirstSingleSerieUIDFileSet();

   l = s->GetFirstSingleSerieUIDFileSet();
   if (l == NULL) {
      std::cout << "No Serie found ?!?" << std::endl;
      exit (0);
   }

   GDCM_NAME_SPACE::XCoherentFileSetmap xcm;

   std::string serieUID;
   std::string currentSerieWriteDir = "";
   std::string xCoherentWriteDir = "";
   std::string xCoherentName = "";
   std::string serieDirectory;
   std::string lastFilename;
   std::string rep("_");
   int controlCount = 0;

   // 'Study Instance UID'
   // The user is allowed to create his own Study, 
   //          keeping the same 'Study Instance UID' for various images
   // The user may add images to a 'Manufacturer Study',
   //          adding new Series to an already existing Study
   std::string strStudyUID;
   if (write) {
      if ( !userDefinedStudy )
         strStudyUID =  GDCM_NAME_SPACE::Util::CreateUniqueUID();
      else
         strStudyUID = studyUID;
   }

   while (l) // for each 'Single SerieUID FileSet' //===> Ignore 'Serie UID"
   { 
 currentSerieWriteDir = "";
      nbFiles = l->size() ;
      if ( l->size() > 2 ) // ignore a Directory with less than 2 images.
                           // Why not ? Just an example, for testing!
      {
          // Just not to make too many modif in the code
          //serieUID = "SingleSerie"; // s->GetCurrentSerieUIDFileSetUID();
          serieUID = s->GetCurrentSerieUIDFileSetUID();

          GDCM_NAME_SPACE::Util::ReplaceSpecChar(serieUID, rep);

          // --- for write
          if (write || copy)
          {
             currentSerieWriteDir = currentSerieWriteDir + dirNameout;
             unsigned int l = strlen(dirNameout)-1;
             if ( dirNameout[l] != '/'  &&  dirNameout[l] != '\\')
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
         // Crashes if DataElement not found
         //std:: cout << GDCM_NAME_SPACE::Global::GetDicts()->GetDefaultPubDict()->GetEntry(groupelem[0],groupelem[1])->GetName() << std::endl;
 
            xcm = s->SplitOnTagValue(l, groupelem[0],groupelem[1] );
         }

         GDCM_NAME_SPACE::FileHelper *fh;

         for (GDCM_NAME_SPACE::XCoherentFileSetmap::iterator i = xcm.begin(); 
                                                  i != xcm.end();
                                                ++i)
         {
            xCoherentName = (*i).first;
            if (verbose)
               std::cout << "==========================================xCoherentName = " << xCoherentName << std::endl;
             GDCM_NAME_SPACE::Util::ReplaceSpecChar(xCoherentName, rep);
             // --- for write
             if (write || copy)
             { 
                xCoherentWriteDir = currentSerieWriteDir + GDCM_NAME_SPACE::GDCM_FILESEPARATOR+ xCoherentName;
               // if ( ! GDCM_NAME_SPACE::DirList::IsDirectory(xCoherentWriteDir) )
                {      
                   systemCommand   = "mkdir " + xCoherentWriteDir;
                   system( systemCommand.c_str());
                   if (verbose)
                      std::cout << "2 " << systemCommand << std::endl;       
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
           // --> Activated on user demand.

           if (sort) {
              s->OrderFileList((*i).second);  // sort the XCoherent Fileset
              std::cout << "ZSpacing for the file set " << s->GetZSpacing()
                        << std::endl;
           } 

            std::string strSerieUID =  GDCM_NAME_SPACE::Util::CreateUniqueUID();

            for (GDCM_NAME_SPACE::FileList::iterator it2 =  ((*i).second)->begin();
                                          it2 != ((*i).second)->end();
                                        ++it2)
            {
               controlCount ++;
               fileName = (*it2)->GetFileName();
               // --- for write
               lastFilename =  GDCM_NAME_SPACE::Util::GetName( fileName );
               filenameout = xCoherentWriteDir  + GDCM_NAME_SPACE::GDCM_FILESEPARATOR+ lastFilename; 
               if (write)
               {  
                  fh = GDCM_NAME_SPACE::FileHelper::New( (*it2) );
                  fh->SetKeepOverlays( true );       
                  fh->InsertEntryString(strSerieUID,0x0020,0x000e,"UI");
                  unsigned int dataSize  = fh->GetImageDataRawSize();
                  uint8_t *imageData = fh->GetImageDataRaw();// somewhat important : Loads the Pixels in memory !
                  if (!imageData)
                     std::cout << "fail to read [" << (*it2)->GetFileName() << std::endl;
                  fh->SetWriteTypeToDcmExplVR();
                  fh->SetContentType(GDCM_NAME_SPACE::UNMODIFIED_PIXELS_IMAGE);
                  if (!fh->Write(filenameout))
                  {
                     std::cout << "Fail to write :[" << filenameout << "]"
                               << std::endl;
                  }
                  fh->Delete();
               }
               else if (copy)
               {
                   systemCommand   = "cp " + fileName + " " + filenameout;
                   system( systemCommand.c_str());
               }
               if (verbose)
                     std::cout << "3 " << systemCommand << std::endl;
            }
         }
         std::cout << std::endl;
      }
      l = s->GetNextSingleSerieUIDFileSet();
   }
    
   if ( controlCount == 0 )
      std::cout << "No suitable file was found!" << std::endl;

   s->Delete();
   return 0;
}
