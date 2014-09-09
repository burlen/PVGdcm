/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmCallbackCommand.h,v $
  Language:  C++
  Date:      $Date: 2007/09/18 15:59:48 $
  Version:   $Revision: 1.4 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMCALLBACKCOMMAND_H_
#define _GDCMCALLBACKCOMMAND_H_

#include "gdcmDebug.h"
#include "gdcmCommand.h"

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
/**
 * \brief CallbackCommand base class to react on a gdcm event
 *
 * \remarks The execution parameter depends on the
 */
class GDCM_EXPORT CallbackCommand : public Command
{
   gdcmTypeMacro(CallbackCommand);
   gdcmNewMacro(CallbackCommand);

public:

   typedef void CbkMethod(CallbackCommand *);

/*   // Note: the CallbackCommand:: namespace prefix is needed by Swig in the 
   //       following method declarations. Refer to gdcmPython/gdcm.i
   //       for the reasons of this unnecessary notation at C++ level.
   
   void SetCallback(CallbackCommand::CbkMethod *callback,void *arg = NULL );
   
   void SetCallbackArgDelete(CallbackCommand::CbkMethod *callback);
   
   // Note: replace CallbackCommand::Method *method to void(*method)(void *) to
   //       avoid wrapping problems with the typemap conversions
   
   void SetCallback(void(*callback)(void *), // CallbackCommand::Method *method
                    void *arg,
                    void(*argDelete)(void *));
*/
   void SetCallback(CallbackCommand::CbkMethod *callback);

   virtual void Execute();

protected:
   CallbackCommand();
   virtual ~CallbackCommand();

private:
   /// pointer to the initialisation method for any progress bar   
   CbkMethod *Callback;
   /// pointer to the ??? method for any progress bar   
   CbkMethod *CallbackArgDelete;
   /// pointer to the ??? data for any progress bar   
   void *CallbackArg;
};
} // end namespace gdcm

//-----------------------------------------------------------------------------
#endif
