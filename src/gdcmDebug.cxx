/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDebug.cxx,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:08 $
  Version:   $Revision: 1.31 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#include "gdcmDebug.h"
#include "gdcmCommandManager.h"

#include <iostream>

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
// Warning message level to be displayed
const int Debug::LINE_LENGTH = 79;

bool Debug::DebugFlag     = false;
bool Debug::LogFlag       = false;
bool Debug::WarningFlag   = false;
bool Debug::OutputToFile  = false;

std::ofstream Debug::OutputFileStream;
std::ostream &Debug::StandardStream = std::cerr;

//-----------------------------------------------------------------------------
// Constructor / Destructor
Debug::Debug()
{
}

Debug::~Debug()
{
  if ( OutputFileStream.is_open() )
      OutputFileStream.close();     
}

//-----------------------------------------------------------------------------
// Public
/**
 * \brief   Sets both the debug flag and warning flag
 *          (both used for debugging purpose)
 * @param   flag Set the debug flag and warning flag
 */ 
void Debug::SetDebugFlag (bool flag) 
{
   // To help tracking a bug, both flags are necessary
   DebugFlag   = flag;
   WarningFlag = flag;
}

/**
 * \brief   Sets the warning flag
 * @param   flag Set the warning flag
 */ 
void Debug::SetWarningFlag (bool flag) 
{
   // Cannot unset Warning flag if Debug flag is on or if LogFlag is on.
   if (flag == false)
   { 
      if (DebugFlag == true)
         return;
      if (LogFlag == true)
         return;
   }
   WarningFlag = flag;
}

/**
 * \brief   Sets the log flag
 * @param   flag Set the log flag
 */ 
void Debug::SetLogFlag (bool flag) 
{
   // To log oddities, both flags are necessary      
   WarningFlag = flag;   
   LogFlag = flag;
}

/**
 * \brief   Accessor
 * @param   flag whether we want to redirect to file
 */ 
void Debug::SetOutputToFile (bool flag) 
{
   OutputToFile = flag;
}

/**
 * \brief   Accessor to know whether debug info are redirected to file
 */ 
bool Debug::GetOutputToFile ()
{
   return OutputToFile;
}

/**
 * \brief Set the filename the debug stream should be redirect to
 *        Settting a filename also sets DebugToFile to true
 * @param   filename  File to redirect debug info
 *          Absolutely nothing is check. You have to pass in
 *          a correct filename
 */ 
void Debug::SetOutputFileName (std::string const &filename)
{
   OutputToFile = true;  // Just in case ... 
   DebugFlag   = true;  // Just in case ...
   if ( OutputFileStream.is_open() )
      OutputFileStream.close();
   OutputFileStream.open( filename.c_str() );
}

/**
 * \brief Internal use only. Allow us to retrieve the static from anywhere
 *        in gdcm code
 * @return Debug file
 */
std::ostream &Debug::GetOutput ()
{
   if(OutputToFile)
      return OutputFileStream;
   else
      return StandardStream;
}

void Debug::SendToOutput(unsigned int type,std::string const &msg,const Base *object)
{
   bool executed=false;
   if( type != CMD_DEBUG && type != CMD_ASSERT )
      executed=CommandManager::ExecuteCommandConst(object,type,msg);

   if(!executed)
      GetOutput() << Command::GetCommandAsString(type) << ": " << msg;
}

//-----------------------------------------------------------------------------
// Protected

//-----------------------------------------------------------------------------
// Private
   
//-----------------------------------------------------------------------------
// Print

//-----------------------------------------------------------------------------
} // end namespace gdcm
