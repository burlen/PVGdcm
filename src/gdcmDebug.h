/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmDebug.h,v $
  Language:  C++
  Date:      $Date: 2008/04/10 12:15:36 $
  Version:   $Revision: 1.59 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#ifndef _GDCMDEBUG_H_
#define _GDCMDEBUG_H_

#include "gdcmCommon.h"
#include "gdcmCommand.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <assert.h>
#include <string.h> // strerror
#include <errno.h>

namespace GDCM_NAME_SPACE
{
//-----------------------------------------------------------------------------
class CommandManager;

//-----------------------------------------------------------------------------
/**
 * \brief Debug is an object for warning/logging/tracing programs.
 * It has the following modes :
 *  - error : for bad library use, seriously wrong DICOM
 *  - debug : for information/debug messages
 *  - warning : Warns the user when some oddity occured.
 *  - log     : we display messages when anything is not Dicom Kosher
 *  - debug : we help developper to trace, at a glance, the execution
 *  - assert : design by contract implementation. A function should have 
 *             proper input and proper output. 
 *             (should not happen, not user controlled)
 * 
 * A debugging message is only shown if the flag is on (DebugFlag)
 * This is static var and can be set at beginning of code:
 *         GDCM_NAME_SPACE::Debug::SetDebugOn();
 *
 * Warning  : Warns the user when some oddity occured, and we used an heuristics
 *            to bypass the trouble.
 *            e.g.  : SamplesPerPixel missing, we assume it's a grey level image
 *            e.g   : Pixel Representation missing : we assume 'unsigned'
 *            (we hope our assumption is OK, but user may disagree.)
 * Log      : we display messages when anything is not Dicom Kosher 
 *            e.g. : non even length field
 *            e.g  : file is declared as Explicit VR, but a DataElement
 *                 is Implicit
 *            e.g  : a file holds an illegal group (0x0005, ...)    
 * Debug : We help developper to trace, at a glance, the execution.
 *         (before refining with a debugging tool)
 *
 * Setting ON Debug leads to set ON Warning (but not Log)
 * Setting ON Log   leads to set ON Warning (but not Debug)
 */

class GDCM_EXPORT Debug
{
public:
   Debug();
   ~Debug();

   /// \brief This is a global flag that controls whether 
   ///        both debug and warning messages are displayed. 
   ///        (used to warn user when file contains some oddity)
   static void SetDebugFlag (bool flag);
   /// \brief   Gets the debug flag value
   static bool GetDebugFlag () {return DebugFlag; }
   /// \brief Sets the Debug Flag to true
   static void DebugOn  () { SetDebugFlag(true);  }
   /// \brief Sets the Debug Flag to false
   static void DebugOff () { SetDebugFlag(false); }
   
   /// \brief This is a global flag that controls whether 
   ///        log messages are displayed.
   static void SetLogFlag (bool flag);
   /// \brief   Gets the Log flag value
   static bool GetLogFlag () {return LogFlag; }
   /// \brief Sets the Log Flag to true
   static void LogOn  () { SetLogFlag(true);  }
   /// \brief Sets the Log Flag to false
   static void LogOff () { SetLogFlag(false); } 
   
   /// \brief This is a global flag that controls whether 
   ///        warning messages are displayed.
   static void SetWarningFlag (bool flag);
   /// \brief   Gets the warning flag value
   static bool GetWarningFlag () {return WarningFlag; }
   /// \brief Sets the Warning Flag to true
   static void WarningOn  () { SetWarningFlag(true);  }
   /// \brief Sets the Warning Flag to false
   static void WarningOff () { SetWarningFlag(false); }      

   /// \brief This is a global flag that controls if debug are redirected
   ///        to a file or not
   static void SetOutputToFile (bool flag);
   static bool GetOutputToFile ();
   /// \brief Next debug messages will be sent in the debug file
   static void OutputToFileOn  () { SetOutputToFile(true);  }
   /// \brief Next debug messages will be sent in the standard output
   static void OutputToFileOff () { SetOutputToFile(false); }

   static void SetOutputFileName (std::string const &filename);

   static std::ostream &GetOutput ();

   static void SendToOutput(unsigned int type,std::string const &msg,
                            const Base *object = NULL);

private:
   static bool WarningFlag;
   static bool LogFlag;
   static bool DebugFlag;

   static bool OutputToFile;

   static std::ofstream OutputFileStream;
   static std::ostream &StandardStream;

   static const int LINE_LENGTH;
};

} // end namespace gdcm

// Here we define function this is the only way to be able to pass
// stuff with indirection like:
// gdcmDebug( "my message:" << i << '\t' ); 
// You cannot use function unless you use vnsprintf ...

// __FUNCTION is not always defined by preprocessor
// In c++ we should use __PRETTY_FUNCTION__ instead...
#ifdef GDCM_COMPILER_HAS_FUNCTION
// Handle particular case for GNU C++ which also defines __PRETTY_FUNCTION__
// which is a lot nice in C++
#ifdef __BORLANDC__
#  define __FUNCTION__ __FUNC__
#endif
#ifdef __GNUC__
#  define GDCM_FUNCTION __PRETTY_FUNCTION__
#else
#  define GDCM_FUNCTION __FUNCTION__ 
#endif //__GNUC__
#else
#  define GDCM_FUNCTION "<unknow>"
#endif //GDCM_COMPILER_HAS_FUNCTION

/**
 * \brief   Debug : To be used to help bug tracking developer
 * @param type type 
 * @param obj obj
 * @param msg message part 
 * @param adds adds
 */
#define gdcmMessageBodyMacro(type, obj, msg, adds)             \
{                                                              \
   std::ostringstream osmacro;                                 \
   osmacro << "In " __FILE__ ", line " << __LINE__             \
           << ", function " << GDCM_FUNCTION << "\n"           \
           << adds << msg << "\n\n";                           \
   GDCM_NAME_SPACE::Debug::SendToOutput(type,osmacro.str(),obj);\
}

// ------------------------------------------------------------------------

/**
 * \brief Debug : To be used to help bug tracking developer
 * @param obj current instance or NULL
 * @param msg message part
 */
#ifdef NDEBUG
#define gdcmDebugBodyMacro(obj, msg) {}
#define gdcmDebugMacro(msg) {}
#define gdcmStaticDebugMacro(msg) {}
#else
#define gdcmDebugBodyMacro(obj, msg)                           \
{                                                              \
   if( Debug::GetDebugFlag() )                                 \
   {                                                           \
      std::string adds="";                                     \
      if( errno )                                              \
      {                                                        \
         adds = "Last system error was: ";                     \
         adds += strerror(errno);                              \
         adds += "\n";                                         \
      }                                                        \
      gdcmMessageBodyMacro(GDCM_NAME_SPACE::CMD_DEBUG,obj,msg,adds);\
   }                                                           \
}
#define gdcmDebugMacro(msg)                                    \
   gdcmDebugBodyMacro(NULL,msg)
#define gdcmStaticDebugMacro(msg)                              \
   gdcmDebugBodyMacro(NULL,msg)
#endif //NDEBUG

// ------------------------------------------------------------------------

/**
 * \brief Log : we display messages when anything is not Dicom Kosher
 * @param obj current instance or NULL
 * @param msg message part
 */
// No NDEBUG test to always have a return of warnings !!!
// -> Rien compris! JPRx
#define gdcmLogBodyMacro(obj, msg)                         \
{                                                          \
   if( Debug::GetLogFlag() )                               \
      gdcmMessageBodyMacro(GDCM_NAME_SPACE::CMD_LOG,obj,msg,"");\
}
#define gdcmLogMacro(msg)                                  \
   gdcmLogBodyMacro(this,msg)
#define gdcmStaticLogMacro(msg)                            \
   gdcmLogBodyMacro(NULL,msg)
   
// ------------------------------------------------------------------------

/**
 * \brief Warning : To be used to warn the user when some oddity occurs
 * @param obj current instance or NULL
 * @param msg message part
 */
// No NDEBUG test to always have a return of warnings !!!
// -> Rien compris! JPRx
#define gdcmWarningBodyMacro(obj, msg)                         \
{                                                              \
   if( Debug::GetWarningFlag() )                               \
      gdcmMessageBodyMacro(GDCM_NAME_SPACE::CMD_WARNING,obj,msg,"");\
}
#define gdcmWarningMacro(msg)                                  \
   gdcmWarningBodyMacro(this,msg)
#define gdcmStaticWarningMacro(msg)                            \
   gdcmWarningBodyMacro(NULL,msg)

// ------------------------------------------------------------------------

/**
 * \brief   Error : To be used when unecoverabale error occurs
 *          at a 'deep' level. (don't use it if file is not ACR/DICOM!)
 * @param obj current instance or NULL 
 * @param msg second message part 
 */
// No NDEBUG test to always have a return of errors !!!
// -> Rien compris! JPRx
#define gdcmErrorBodyMacro(obj, msg)                           \
{                                                              \
   gdcmMessageBodyMacro(GDCM_NAME_SPACE::CMD_ERROR,obj,msg,"");\
}
#define gdcmErrorMacro(msg)                                    \
   gdcmErrorBodyMacro(this,msg)
#define gdcmStaticErrorMacro(msg)                              \
   gdcmErrorBodyMacro(NULL,msg)

// ------------------------------------------------------------------------

/**
 * \brief Assert : To be used when an *absolutely* impossible error occurs
 *        No function should be allowed to stop the process instead of
 *        warning the caller!
 * @param obj current instance or NULL
 * @param arg argument to test
 *        An easy solution to pass also a message is to do:
 *        gdcmAssertMacro( "my message" && 2 < 3 )
 */
// No NDEBUG test to always have a return of asserts !!!
// -> Rien compris! JPRx
#define gdcmAssertBodyMacro(obj, arg)                          \
{                                                              \
   if( !(arg) )                                                \
   {                                                           \
      gdcmMessageBodyMacro(GDCM_NAME_SPACE::CMD_ASSERT,obj,"","");\
      assert ( arg );                                          \
   }                                                           \
}
#define gdcmAssertMacro(msg)                                   \
   gdcmAssertBodyMacro(NULL,msg)
#define gdcmStaticAssertMacro(msg)                             \
   gdcmAssertBodyMacro(NULL,msg)

//-----------------------------------------------------------------------------
#endif
