/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmCommandPy.h,v $
  Language:  C++
  Date:      $Date: 2007/05/23 14:18:07 $
  Version:   $Revision: 1.3 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef GDCMCOMMANDPY_H
#define GDCMCOMMANDPY_H

#include "gdcmDebug.h"
#include "gdcmCommand.h"
#include "Python.h"

namespace GDCM_NAME_SPACE 
{
//-----------------------------------------------------------------------------
/**
 * \brief CommandPy base class to react on a gdcm event
 *
 * \remarks The execution parameter depends on the
 */
class CommandPy : public Command
{
   gdcmTypeMacro(CommandPy);

public:
   static CommandPy *New() {return new CommandPy(); }

   void SetCallback(PyObject *callback)
   {
      Callback = callback;
      Py_INCREF(Callback);
   }

   virtual void Execute()
   {
      PyObject *arglist = Py_BuildValue("()");
      PyObject *result = PyEval_CallObject(Callback, arglist);
      Py_DECREF(arglist);

      if (result)
      {
         Py_XDECREF(result);
      }
      else
      {
         if (PyErr_ExceptionMatches(PyExc_KeyboardInterrupt))
         {
            std::cerr << "Caught a Ctrl-C within python, exiting program.\n";
            Py_Exit(1);
         }
         PyErr_Print();
      }
   }

protected:
   CommandPy()
   {
      Callback = NULL;
   }
   virtual ~CommandPy() 
   {
      if (Callback)
         Py_DECREF(Callback);
   }

private:
   /// pointer to the initialisation method for any progress bar   
   PyObject *Callback;
};
} // end namespace gdcm

//-----------------------------------------------------------------------------
#endif
