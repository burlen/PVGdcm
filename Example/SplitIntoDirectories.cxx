/*=========================================================================

  Program:   gdcm
  Module:    $RCSfile: SplitIntoDirectories.cxx,v $
  Language:  C++
  Date:      $Date: 2009/05/28 15:44:34 $
  Version:   $Revision: 1.5 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                 
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmDocEntry.h"
#include "gdcmDicomDir.h"
#include "gdcmDicomDirPatient.h"
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmDirList.h"
#include "gdcmDebug.h"
#include "gdcmArgMgr.h"
#include "gdcmUtil.h"
#include "gdcmSerieHelper.h"

#include <iostream>

/**
  * \brief
  *          - explores recursively the given directory
  *          - keeps the requested series
  *          - orders the gdcm-readable found Files
  *            according to their Patient/Study/Serie/Image characteristics
  */

typedef std::map<std::string, GDCM_NAME_SPACE::File*> SortedFiles;

int main(int argc, char *argv[])
{
   START_USAGE(usage)
   " \n SplitIntoDirectories :\n                                              ",
   " - explores recursively the given directory,                              ",
   " - keeps the requested series / drops the unrequested series              ",
   " - orders the gdcm-readable found Files according to their                ",
   "           (0x0010, 0x0010) Patient's Name                                ",
   "           (0x0020, 0x000d) Study Instance UID                            ",
   "           (0x0020, 0x000e) Series Instance UID                           ",
   " - fills a tree-like structure of directories as :                        ",
   "        - Patient                                                         ",
   "        -- Study                                                          ",
   "        --- Serie                                                         ",
   "                                                                          ",
   " usage:                                                                   ",
   " -----                                                                    ",
   " SplitIntoDirectories                                                     ",
   "                  dirin=rootDirectoryName                                 ",
   "                  dirout=outputDirectoryName                              ",
   "                  {  [keep= list of seriesNumber to process]              ",
   "                   | [drop= list of seriesNumber to ignore] }             ",
   "                  [listonly]  [skel] [seriedescr]                         ",
   "                  [noshadowseq][noshadow][noseq] [verbose] [debug]        ",
   "                                                                          ",
   " dirout : will be created if doesn't exist                                ",
   " keep : if user wants to process a limited number of series               ",
   "            he gives the list of 'SeriesNumber' (tag 0020|0011)           ",
   " drop : if user wants to ignore a limited number of series                ",
   "            he gives the list of 'SeriesNumber' (tag 0020|0011)           ",
   "        SeriesNumber are short enough to be human readable                ",
   "        e.g : 1030,1035,1043                                              ",
   " seriedescr : SerieDescription+SerieNumber use for directory name         ",
   "              (instead of SeriesInstanceUID)                              ",
   " skel     : name skeleton eg : patName_1.nema -> skel=patName_            ",
   " noshadowseq: user doesn't want to load Private Sequences                 ",
   " noshadow : user doesn't want to load Private groups (odd number)         ",
   " noseq    : user doesn't want to load Sequences                           ",
   " verbose  : user wants to run the program in 'verbose mode'               ",
   " debug    : *developer*  wants to run the program in 'debug mode'         ",
   FINISH_USAGE


   // VERY IMPORTANT :
   // Respect this order while creating 'UserFileIdentifier'
   // (mind the order of the 'AddSeriesDetail' !)
   
   enum Index
   {
      IND_PatientName,
      IND_StudyInstanceUID,
      IND_SerieInstanceUID,
      IND_SerieDescription,
      IND_SerieNumber,
      IND_FileName
   };
      
   std::cout << "... inside " << argv[0] << std::endl;
   
   // ----- Initialize Arguments Manager ------

   GDCM_NAME_SPACE::ArgMgr *am = new GDCM_NAME_SPACE::ArgMgr(argc, argv);
  
   if (argc == 1 || am->ArgMgrDefined("usage")) 
   {
      am->ArgMgrUsage(usage); // Display 'usage'
      delete am;
      return 0;
   }

   const char *dirNamein;
   dirNamein  = am->ArgMgrGetString("dirin",".");

   const char *dirNameout;   
   dirNameout  = am->ArgMgrGetString("dirout",".");
   
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

   if (am->ArgMgrDefined("debug"))
      GDCM_NAME_SPACE::Debug::DebugOn();

   bool verbose    = ( 0 != am->ArgMgrDefined("verbose") );
   bool listonly   = ( 0 != am->ArgMgrDefined("listonly") );
   bool seriedescr = ( 0 != am->ArgMgrDefined("seriedescr") );

   int nbSeriesToKeep;
   int *seriesToKeep = am->ArgMgrGetListOfInt("keep", &nbSeriesToKeep);
   int nbSeriesToDrop;
   int *seriesToDrop = am->ArgMgrGetListOfInt("drop", &nbSeriesToDrop);

   if ( nbSeriesToKeep!=0 && nbSeriesToDrop!=0)
   {
      std::cout << "KEEP and DROP are mutually exclusive !" << std::endl;
      delete am;
      return 0;
   }

   bool hasSkel = ( 0 != am->ArgMgrDefined("hasSkel") );
   const char *skel;
   if (hasSkel)
      skel = am->ArgMgrGetString("skel");


   const char *input   = am->ArgMgrGetString("input","DCM");
   
   // if unused Param we give up
   if ( am->ArgMgrPrintUnusedLabels() )
   {
      am->ArgMgrUsage(usage);
      delete am;
      return 0;
   }
   delete am;  // we don't need Argument Manager any longer

   // ----- Begin Processing -----
   
     
   // --> Check supposed-to-be-directory names
   
   if ( ! GDCM_NAME_SPACE::DirList::IsDirectory(dirNamein) )
   {
      std::cout << "KO : [" << dirNamein << "] is not a Directory."
                << std::endl;
      return 0;

   }
   else
   {
      std::cout << "OK : [" << dirNamein << "] is a Directory." << std::endl;
   }

   std::string systemCommand;

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
        std::cout << "Directory [" << dirNameout << "] created." << std::endl;
      }
   }
   else
   {
       std::cout << "Output Directory [" << dirNameout 
                 << "] already exists; Used as is."
                 << std::endl;
   }
   // --> End of checking supposed-to-be-directory names
       
   std::string strDirNamein(dirNamein);
   // true ; get recursively the list of files
   GDCM_NAME_SPACE::DirList dirList(strDirNamein, true); 
   
   if (listonly)
   {
      std::cout << "------------List of found files ------------" << std::endl;
      dirList.Print();
      std::cout << std::endl;
   }


// ======================================= The job starts here =========================
   
   GDCM_NAME_SPACE::DirListType fileNames;
   fileNames = dirList.GetFilenames();

   GDCM_NAME_SPACE::SerieHelper *s;     // Needed to use SerieHelper::AddSeriesDetail()
   s = GDCM_NAME_SPACE::SerieHelper::New();

   std::string token = "%%%"; // Hope it's enough!
  
   GDCM_NAME_SPACE::File *f;
   std::vector<std::string> tokens;
   std::vector<std::string> tokensForFileName;
   
   if (verbose)
      std::cout << "------------------Print Break levels-----------------" << std::endl;

   std::string userFileIdentifier;
   SortedFiles sf;


   // VERY IMPORTANT :
   // While coding the various AddSeriesDetail,
   // respect the order you choosed in 'enum Index' !
 
/*
   enum Index
   {
      IND_PatientName,
      IND_StudyInstanceUID,
      IND_SerieInstanceUID,
      IND_SerieDescription,
      IND_SerieNumber,
      IND_FileName
   }; 
*/     
   s->AddSeriesDetail(0x0010, 0x0010, false); // Patient's Name (false : no convert)
   
   // You may prefer 0020 0010  Study ID
   // use :
   // s->AddSeriesDetail(0x0020, 0x0010, true); 
   // Avoid using 0008 0020 Study Date, 
   // since you may have more than one study, for a given Patient, at a given Date!
   // or the field may be empty!   
   s->AddSeriesDetail(0x0020, 0x000d, false); // Study Instance UID (false : no convert)


   // You may prefer 0020 0011 Series Number
   // use :
   // s->AddSeriesDetail(0x0020, 0x0011, true);    
   s->AddSeriesDetail(0x0020, 0x000e, false); // Series Instance UID (false : no convert)

   s->AddSeriesDetail(0x0008, 0x103e, false); // Serie Description
   s->AddSeriesDetail(0x0020, 0x0011, false); // Serie Number (more than 1 serie may have the same Ser.Nbr don't 'convert!)

   
   // Feel free to add more fields, if they can help a suitable (for you)
   // image sorting

   // Loop on all the gdcm-readable files
   for (GDCM_NAME_SPACE::DirListType::iterator it = fileNames.begin();
                                    it != fileNames.end();
                                  ++it)
   {
      f = GDCM_NAME_SPACE::File::New();
      f->SetLoadMode(loadMode);
      f->SetFileName( *it );
      if (verbose)
         std::cout << "Try[" << *it << "]\n";
      f->Load();
      if (!f->Document::IsReadable())
      {
         if (verbose)
            std::cout << "File : [" << *it << "] not gdcm-readable -> skipped !" << std::endl;
         continue;
      }
      if (verbose)
         std::cout << "Loaded!\n";
      std::string strSeriesNumber;
      int seriesNumber;
      int j;

      // keep only requested Series
      bool keep = false;
      if (nbSeriesToKeep != 0)
      {
         strSeriesNumber = f->GetEntryString(0x0020, 0x0011 );
         seriesNumber = atoi( strSeriesNumber.c_str() );
         for (j=0; j<nbSeriesToKeep; j++)
         {
            if(seriesNumber == seriesToKeep[j])
            {
               keep = true;
               break;
            }
         }
         if ( !keep)
         {
            f->Delete();
            continue;
         }
      }
      // drop all unrequested Series
      bool drop = false;
      if (nbSeriesToDrop != 0)
      {
         strSeriesNumber = f->GetEntryString(0x0020, 0x0011 );
         seriesNumber = atoi( strSeriesNumber.c_str() );
         for (j=0;j<nbSeriesToDrop; j++)
         {
            if(seriesNumber == seriesToDrop[j])
            {
               drop = true;
               break;
            }
        }
        if (drop)
        {
           f->Delete();
           continue;
        }
      }

      userFileIdentifier=s->CreateUserDefinedFileIdentifier(f);
      if (verbose)
         std::cout << "userFileIdentifier [" << userFileIdentifier << "]" << std::endl; 
      tokens.clear();
      GDCM_NAME_SPACE::Util::Tokenize (userFileIdentifier, tokens, token);

      char newName[1024];

      ///this is a trick to build up a lexicographical compliant name :
      ///     eg : fich001.ima vs fich100.ima as opposed to fich1.ima vs fich100.ima
      std::string name = GDCM_NAME_SPACE::Util::GetName( *it );

      std::cout << "name :[" << name << "]\n";

      if (hasSkel)
      {
         int imageNum; // Within FileName
         GDCM_NAME_SPACE::Util::Tokenize (name, tokensForFileName, skel);
         imageNum = atoi ( tokensForFileName[0].c_str() );
         // probabely we could write something much more complicated using C++ !
         sprintf (newName, "%s%06d.dcm", skel, imageNum);
         tokens[IND_FileName] = newName;
         tokensForFileName.clear();
       }
       else
       {
         tokens[IND_FileName] = name;
       }

         // Patient's Name
         // Study Instance UID
         // Series Instance UID
         // SerieDescription
         // Serie Number
         // file Name

      userFileIdentifier = tokens[IND_PatientName]      + token +
                           tokens[IND_StudyInstanceUID] + token + 
                           tokens[IND_SerieInstanceUID] + token +

                           tokens[IND_SerieDescription] + token +
                           tokens[IND_SerieNumber]      + token +
                           tokens[IND_FileName];

      if (verbose) 
         std::cout << "[" << userFileIdentifier  << "] : " << *it << std::endl;

      // storing in a map ensures automatic sorting !
      sf[userFileIdentifier] = f;
   }
   
   if (verbose)
      std::cout << " ==== " << std::endl;
      
   std::string fullFilename, lastFilename;
   std::string previousPatientName, currentPatientName;
   std::string previousStudyInstanceUID, currentStudyInstanceUID;   
   std::string previousSerieInstanceUID, currentSerieInstanceUID;
   
   std::string currentSerieDescription, currentSerieNumber;   
      
   std::string writeDir, currentWriteDir;
   std::string currentPatientWriteDir;
   std::string currentStudyWriteDir;
   std::string currentSerieWriteDir; 

   std::string fullWriteFilename;
           
   writeDir = GDCM_NAME_SPACE::Util::NormalizePath(dirNameout);     
   SortedFiles::iterator it2;
 
   previousPatientName            = "";
   previousStudyInstanceUID       = "";    
   previousSerieInstanceUID       = "";   
       
   GDCM_NAME_SPACE::File *currentFile;
     
   for (it2 = sf.begin() ; it2 != sf.end(); ++it2)
   {  
      currentFile = it2->second;
       
      fullFilename =  currentFile->GetFileName();
      lastFilename =  GDCM_NAME_SPACE::Util::GetName( fullFilename );
      if (verbose) 
      std::cout <<" ------------------------------------------------------------------------------" 
                << std::endl << " Deal with [" << it2->first << "] : [" <<fullFilename << "]" 
                << std::endl;
     
      tokens.clear();
      GDCM_NAME_SPACE::Util::Tokenize (it2->first, tokens, token);
      
      currentPatientName            = tokens[IND_PatientName];
      currentStudyInstanceUID       = tokens[IND_StudyInstanceUID];      
      currentSerieInstanceUID       = tokens[IND_SerieInstanceUID];
      currentSerieDescription       = tokens[IND_SerieDescription];
      currentSerieNumber            = tokens[IND_SerieNumber];
             
      if (previousPatientName != currentPatientName)
      {  
         previousPatientName = currentPatientName;
         if (verbose)   
            std::cout << "==== new Patient  [" << currentPatientName  << "]" << std::endl;
    
         previousPatientName            = currentPatientName;
         previousStudyInstanceUID       = ""; 
         previousSerieInstanceUID       = "";

         currentPatientWriteDir = writeDir + currentPatientName;

         systemCommand   = "mkdir " + currentPatientWriteDir;
         if (verbose || listonly)
            std::cout << "[" << systemCommand << "]" << std::endl;
         if (!listonly)
            system ( systemCommand.c_str() );
      }

      if (previousStudyInstanceUID != currentStudyInstanceUID)
      {
         previousStudyInstanceUID       = currentStudyInstanceUID;
         if (verbose)
            std::cout << "==== === new Study [" << currentStudyInstanceUID << "]"
                      << std::endl;

         currentStudyWriteDir  = currentPatientWriteDir + GDCM_NAME_SPACE::GDCM_FILESEPARATOR
                             + currentStudyInstanceUID;
         systemCommand   = "mkdir " + currentStudyWriteDir;
         
         if (listonly)
           std::cout << "[" << systemCommand << "]" << std::endl;         
         else            
            system (systemCommand.c_str());

      }  
      
      if (previousSerieInstanceUID != currentSerieInstanceUID)
      {        
         previousSerieInstanceUID       = currentSerieInstanceUID;
         if (verbose)   
            std::cout << "=== ==== === new Serie [" << currentSerieInstanceUID << "]"
                      << std::endl;
                            
         if (seriedescr) // more human readable!
            currentSerieWriteDir  = currentStudyWriteDir + GDCM_NAME_SPACE::GDCM_FILESEPARATOR
                                  + currentSerieDescription + "_" + currentSerieNumber
                                  + "_" + currentSerieInstanceUID;
         else
            currentSerieWriteDir  = currentStudyWriteDir + GDCM_NAME_SPACE::GDCM_FILESEPARATOR
                                  + currentSerieInstanceUID;         
                      
         systemCommand   = "mkdir " + currentSerieWriteDir;
         
         if (listonly)
            std::cout << "[" << systemCommand << "]" << std::endl;         
         else             
            system (systemCommand.c_str());
      }            
   
      if ( GDCM_NAME_SPACE::Debug::GetDebugFlag())
         std::cout << "--- --- --- --- --- " << it2->first << "  " 
                   << (it2->second)->GetFileName() << " " 
                   << GDCM_NAME_SPACE::Util::GetName( fullFilename ) << std::endl;
 
      // If you want to create file names of your own, here is the place!
      // Just replace 'lastFilename' by anything that's better for you.               
      fullWriteFilename = currentSerieWriteDir + GDCM_NAME_SPACE::GDCM_FILESEPARATOR 
                                         + lastFilename; 

      systemCommand   = "cp " + fullFilename + " " + fullWriteFilename;
      
      if (listonly)
         std::cout << "[" << systemCommand << "]" << std::endl;         
      else             
         system (systemCommand.c_str());

   }
   return 0;
 }

