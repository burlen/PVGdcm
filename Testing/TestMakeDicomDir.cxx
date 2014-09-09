/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestMakeDicomDir.cxx,v $
  Language:  C++
  Date:      $Date: 2008/09/15 15:49:21 $
  Version:   $Revision: 1.14 $
                                                                                
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
#include "gdcmDirList.h"
#include "gdcmCommandManager.h"
#include "gdcmDebug.h"

// ---
class CommandStart : public GDCM_NAME_SPACE::Command
{
   gdcmTypeMacro(CommandStart);
   gdcmNewMacro(CommandStart);

public :
   virtual void Execute()
   {
      std::cerr << "Start parsing" << std::endl;
   }

protected :
   CommandStart() {}
};

class CommandEnd : public GDCM_NAME_SPACE::Command
{
   gdcmTypeMacro(CommandEnd);
   gdcmNewMacro(CommandEnd);

public :
   virtual void Execute()
   {
      std::cerr << "End parsing" << std::endl;
   }

protected :
   CommandEnd() {}
};

class CommandProgress : public GDCM_NAME_SPACE::Command
{
   gdcmTypeMacro(CommandProgress);
   gdcmNewMacro(CommandProgress);

public :
   virtual void Execute()
   {
      GDCM_NAME_SPACE::DicomDir *dd=dynamic_cast<GDCM_NAME_SPACE::DicomDir *>(GetObject());

      if(dd)
         std::cerr << "Progress parsing (" << dd->GetProgress() << ")" << std::endl;
      else
         std::cerr << "Progress parsing (NULL)" << std::endl;
   }

protected :
   CommandProgress() {}
};

void EndMethod(void *endMethod) 
{
  (void)endMethod;
   std::cout<<"End parsing"<<std::endl;
}
// ---

/**
  * \brief   - Explores recursively the given directory 
  *            (or GDCM_DATA_ROOT by default)
  *          - Orders the gdcm-readable found Files
  *             according their Patient/Study/Serie/Image characteristics
  *          - Makes the GDCM_NAME_SPACE::DicomDir. 
  *          - Writes a file named "NewDICOMDIR".
  *          - Reads "NewDICOMDIR" file.
  */  

int TestMakeDicomDir(int argc, char *argv[])
{
   //GDCM_NAME_SPACE::Debug::DebugOn();
   std::string dirName;   

   if (argc > 1)
   {
      dirName = argv[1];
   }
   else
   {
      dirName = GDCM_DATA_ROOT;
   }
 
   GDCM_NAME_SPACE::DicomDir *dcmdir;

   // new style (user is allowed no to load Sequences an/or Shadow Groups)
   dcmdir = GDCM_NAME_SPACE::DicomDir::New( );
 
   GDCM_NAME_SPACE::Command *cmd;
   cmd = CommandStart::New();
   GDCM_NAME_SPACE::CommandManager::SetCommand(dcmdir,GDCM_NAME_SPACE::CMD_STARTPROGRESS,cmd);
   cmd->Delete();
   cmd = CommandProgress::New();
   GDCM_NAME_SPACE::CommandManager::SetCommand(dcmdir,GDCM_NAME_SPACE::CMD_PROGRESS,cmd);
   cmd->Delete();
   cmd = CommandEnd::New();
   GDCM_NAME_SPACE::CommandManager::SetCommand(dcmdir,GDCM_NAME_SPACE::CMD_ENDPROGRESS,cmd);
   cmd->Delete();

   // dcmdir->SetLoadMode(GDCM_NAME_SPACE::LD_NOSEQ | GDCM_NAME_SPACE::LD_NOSHADOW);
   // some images have a wrong length for element 0x0000 of private groups
   dcmdir->SetLoadMode(GDCM_NAME_SPACE::LD_NOSEQ);
   dcmdir->SetDirectoryName(dirName);
   dcmdir->Load();

   if ( !dcmdir->GetFirstPatient() ) 
   {
      std::cout << "makeDicomDir: no patient found. Exiting."
                << std::endl;

      dcmdir->Delete();
      return 1;
   }
    
   // Create the corresponding DicomDir
   dcmdir->Write("NewDICOMDIR");
   dcmdir->Delete();

   // Read from disc the just written DicomDir
   GDCM_NAME_SPACE::DicomDir *newDicomDir = GDCM_NAME_SPACE::DicomDir::New();
   newDicomDir->SetFileName("NewDICOMDIR");
   newDicomDir->Load();

   if( !newDicomDir->IsReadable() )
   {
      std::cout<<"          Written DicomDir 'NewDICOMDIR'"
               <<" is not readable"<<std::endl
               <<"          ...Failed"<<std::endl;

      newDicomDir->Delete();
      return 1;
   }

   if( !newDicomDir->GetFirstPatient() )
   {
      std::cout<<"          Written DicomDir 'NewDICOMDIR'"
               <<" has no patient"<<std::endl
               <<"          ...Failed"<<std::endl;

      newDicomDir->Delete();
      return(1);
   }

   std::cout<<std::flush;
   newDicomDir->Delete();
   return 0;
}
