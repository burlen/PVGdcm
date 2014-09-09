/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmCommand.h,v $
  Language:  C++
  Date:      $Date: 2007/08/22 16:14:03 $
  Version:   $Revision: 1.4 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMCOMMAND_H_
#define _GDCMCOMMAND_H_

#include "gdcmRefCounter.h"

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
class CommandManager;

//-----------------------------------------------------------------------------
/// Command list
enum CommandType {
   CMD_UNKNOWN       =0,
   CMD_DEBUG,
   CMD_WARNING,
   CMD_ERROR,
   CMD_ASSERT,
   CMD_STARTPROGRESS,
   CMD_ENDPROGRESS,
   CMD_PROGRESS
};

//-----------------------------------------------------------------------------
/**
 * \brief Command base class to react on a gdcm event
 *
 * \remarks The execution parameter depends on the
 */
class GDCM_EXPORT Command : public RefCounter
{
   gdcmTypeMacro(Command);
   gdcmNewMacro(Command);

public:

   void SetType(unsigned int type);
   unsigned int GetType() const;

   void SetObject(Base *object);
   Base *GetObject() const;
   void SetConstObject(const Base *object);
   const Base *GetConstObject() const;

   void SetText(const std::string &text);
   const std::string &GetText() const;

   virtual void Execute();

   static const char *GetCommandAsString(unsigned int command);

protected:
   Command();
   virtual ~Command();

private:
   std::string Text;
   Base *Object;
   const Base *ConstObject;
   unsigned int Cmd;
};
} // end namespace gdcm

//-----------------------------------------------------------------------------
#endif
