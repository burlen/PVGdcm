/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmCommandManager.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:08 $
  Version:   $Revision: 1.5 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
// ---------------------------------------------------------------
#include "gdcmCommandManager.h"
#include "gdcmCommand.h"
#include <typeinfo> // for typeif (needed by __BORLANDC__ v6)

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
CommandManager CommandManager::Instance;

//-----------------------------------------------------------------------------
// Constructor / Destructor
/**
 * \brief Constructor used when we want to generate dicom files from scratch
 */
CommandManager::CommandManager()
{
}


/**
 * \brief   Canonical destructor.
 */
CommandManager::~CommandManager ()
{
   if( this == GetInstance() )
      InClearCommand();
}

//-----------------------------------------------------------------------------
// Public
void CommandManager::SetCommand(const Base *object, unsigned int type, Command *command)
{
   Instance.InSetCommand(object, type, command);
}

Command *CommandManager::GetCommand(const Base *object, unsigned int type)
{
   return(Instance.InGetCommand(object, type));
}

bool CommandManager::ExecuteCommand(Base *object, unsigned int type, std::string text)
{
   return(Instance.InExecuteCommand(object, type, text));
}

bool CommandManager::ExecuteCommandConst(const Base *object, unsigned int type, std::string text)
{
   return(Instance.InExecuteCommandConst(object,type,text));
}

const CommandManager *CommandManager::GetInstance()
{
   return &Instance;
}

//-----------------------------------------------------------------------------
// Protected
void CommandManager::InClearCommand(void)
{
   CommandHT::iterator it;
   for(it=CommandList.begin(); it != CommandList.end(); ++it)
   {
      if( it->second )
         it->second->Delete();
   }
}

void CommandManager::InSetCommand(const Base *object, unsigned int type, Command *command)
{
   CommandKey key = CommandKey(object, type);
   Command *cmd = CommandList[key];
   if( cmd != command )
   {
      if( cmd )
         cmd->Unregister();
      if( command )
      {
         CommandList[key] = command;
         command->Register();
      }
      else
         CommandList.erase(key);
   }
}

Command *CommandManager::InGetCommand(const Base *object,unsigned int type)
{
   CommandKey key = CommandKey(object,type);
   try
   {
      return CommandList[key];
   }
   catch(...)
   {
      return NULL;
   }
}

bool CommandManager::InExecuteCommand(Base *object,unsigned int type,std::string text)
{
   Command *cmd = GetCommand(object,type);
   if( cmd )
   {
      cmd->SetText(text);
      cmd->SetObject(object);
      cmd->SetType(type);
      cmd->Execute();
      return true;
   }
   return false;
}

bool CommandManager::InExecuteCommandConst(const Base *object,unsigned int type,std::string text)
{
   Command *cmd = GetCommand(object,type);
   if( cmd )
   {
      cmd->SetText(text);
      cmd->SetConstObject(object);
      cmd->SetType(type);
      cmd->Execute();
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------
// Private

//-----------------------------------------------------------------------------
// Print
void CommandManager::Print(std::ostream &os, std::string const &indent)
{
   os<<indent<<"Command list : \n";
   CommandHT::iterator it;
   for(it=CommandList.begin();it!=CommandList.end();++it)
   {
      os<<indent<<"   "<<typeid(it->first.first).name()<<" ("<<it->first.first<<") - "
        <<Command::GetCommandAsString(it->first.second)
        <<" : "<<typeid(it->second).name()<<" ("<<it->second<<")"
        <<std::endl;
   }
}

//-----------------------------------------------------------------------------
} // end namespace gdcm
