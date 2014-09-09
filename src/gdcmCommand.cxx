/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2007/10/30 13:34:42 $
  Version:   $Revision: 1.5 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
// ---------------------------------------------------------------
#include "gdcmCommand.h"

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
// Constructor / Destructor
/**
 * \brief Constructor used when we want to generate dicom files from scratch
 */
Command::Command()
  :  Text(""), Object(NULL), ConstObject(NULL), Cmd(CMD_UNKNOWN)
{
//   Cmd = CMD_UNKNOWN;
//   Object = NULL;
//   ConstObject = NULL;
//   Text = "";
}


/**
 * \brief   Canonical destructor.
 */
Command::~Command ()
{
}

//-----------------------------------------------------------------------------
// Public
void Command::SetType(unsigned int type)
{
   Cmd = type;
}

unsigned int Command::GetType() const
{
   return Cmd;
}

void Command::SetObject(Base *object)
{
   Object = object;
}

Base *Command::GetObject() const
{
   return Object;
}

void Command::SetConstObject(const Base *object)
{
   ConstObject = object;
}

const Base *Command::GetConstObject() const
{
   if(ConstObject)
      return ConstObject;
   else
      return GetObject();
}

void Command::SetText(const std::string &text)
{
   Text = text;
}

const std::string &Command::GetText(void) const
{
   return Text;
}

void Command::Execute()
{
}

const char *Command::GetCommandAsString(unsigned int command)
{
   switch(command)
   {
      case CMD_UNKNOWN:
         return "Unknown";
      case CMD_DEBUG:
         return "Debug";
      case CMD_WARNING:
         return "Warning";
      case CMD_ERROR:
         return "Error";
      case CMD_ASSERT:
         return "Assert";
      case CMD_STARTPROGRESS:
         return "StartProgress";
      case CMD_ENDPROGRESS:
         return "EndProgress";
      case CMD_PROGRESS:
         return "Progress";
      default:
         return "Undefined !!!";
   }
}

//-----------------------------------------------------------------------------
// Protected

//-----------------------------------------------------------------------------
// Private

//-----------------------------------------------------------------------------
// Print

//-----------------------------------------------------------------------------
} // end namespace gdcm
