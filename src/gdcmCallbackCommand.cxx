/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmCallbackCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2007/10/25 08:02:38 $
  Version:   $Revision: 1.5 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
// ---------------------------------------------------------------
#include "gdcmCallbackCommand.h"

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
// Constructor / Destructor
/**
 * \brief Constructor used when we want to generate dicom files from scratch
 */
CallbackCommand::CallbackCommand() :
   Callback(NULL), CallbackArgDelete(NULL), CallbackArg(NULL)
   
{
//   Callback             = NULL;
//   CallbackArgDelete    = NULL;
//   CallbackArg          = NULL;
}

/**
 * \brief   Canonical destructor.
 */
CallbackCommand::~CallbackCommand ()
{
//   SetCallback(NULL,NULL,NULL);
}

//-----------------------------------------------------------------------------
// Public
/**
 * \brief   Set the callback method
 * @param   callback Method to call
 * @param   arg    Argument to pass to the method
 * \warning In python : the arg parameter isn't considered
 */
 
/*void CallbackCommand::SetCallback( CallbackCommand::CbkMethod *callback,void *arg )
{
   SetCallback(callback,arg,NULL);
}*/

/*
 *\brief   Set the callback method to delete the argument
 *          The argument is destroyed when the callback method is changed 
 *          or when the class is destroyed
 * @param   callback Method to call to delete the argument
 */
/*void CallbackCommand::SetCallbackArgDelete( CallbackCommand::CbkMethod *callback ) 
{
   CallbackArgDelete = callback;
}*/

/*
 *\brief   Set the callback method
 * @param   callback Method to call
 * @param   arg    Argument to pass to the method
 * @param   argDelete    Argument 
 * \warning In python : the arg parameter isn't considered
 */
/*void CallbackCommand::SetCallback( void(*callback)(void *),
                                  void *arg, void(*argDelete)(void *) )
{
   if ( CallbackArg && CallbackArgDelete )
   {
      CallbackArgDelete( CallbackArg );
   }

   Callback          = callback;
   CallbackArg       = arg;
   CallbackArgDelete = argDelete;
}*/

void CallbackCommand::SetCallback(CallbackCommand::CbkMethod *callback)
{
   Callback=callback;
}

void CallbackCommand::Execute()
{
   if(Callback)
      Callback(this);
}

//-----------------------------------------------------------------------------
// Protected

//-----------------------------------------------------------------------------
// Private

//-----------------------------------------------------------------------------
// Print

//-----------------------------------------------------------------------------
} // end namespace gdcm
