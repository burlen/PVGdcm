/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:06 $
  Version:   $Revision: 1.5 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmCommand.h"
#include "gdcmCallbackCommand.h"
#include "gdcmCommandManager.h"

#include <iostream>
#include <typeinfo>

class CommandTest : public GDCM_NAME_SPACE::Command
{
   gdcmTypeMacro(CommandTest);
   gdcmNewMacro(CommandTest);

public:
   virtual void Execute()
   {
      std::cout << "Test class command... for "
                << typeid(GetObject()).name()
                << " (" << GetObject() << ")" << std::endl
                << GetText() << std::endl;
      Executed = true;
   }

   bool IsExecuted() {return Executed;}

private:
   CommandTest() {Executed = false;}

   bool Executed;
};

static bool fctExecuted = false;
void CallbackTest(GDCM_NAME_SPACE::CallbackCommand *cmd)
{
   std::cout << "Test class command... for "
             << typeid(cmd->GetObject()).name()
             << " (" << cmd->GetObject() << ")" << std::endl
             << cmd->GetText() << std::endl;

   fctExecuted = true;
}

int TestCommand(int , char *[]) 
{
   int error=0;

   GDCM_NAME_SPACE::CallbackCommand *cbk = GDCM_NAME_SPACE::CallbackCommand::New();
   cbk->SetCallback(CallbackTest);
   GDCM_NAME_SPACE::CommandManager::SetCommand(NULL,1,cbk);
   cbk->Delete();

   CommandTest *cmd = CommandTest::New();
   GDCM_NAME_SPACE::CommandManager::SetCommand(NULL,2,cmd);
   cmd->Delete();

   std::cout << "Test on callback function execution\n";
   GDCM_NAME_SPACE::CommandManager::ExecuteCommand(NULL,1,"Test on callback function");
   if(!fctExecuted)
      std::cout<<"... Failed\n";
   error+=!fctExecuted;
   std::cout << std::endl;

   std::cout << "Test on command class execution\n";
   GDCM_NAME_SPACE::CommandManager::ExecuteCommand(NULL,2,"Test on command class");
   if(!cmd->IsExecuted())
      std::cout<<"... Failed\n";
   error+=!cmd->IsExecuted();
   std::cout << std::endl;

   std::cout << "Test on unset command execution\n";
   GDCM_NAME_SPACE::CommandManager::ExecuteCommand(NULL,3,"Test on callback function");
   std::cout << std::endl;

   return error;
}
