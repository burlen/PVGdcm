/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: gdcmUtil.cxx,v $
  Language:  C++
  Date:      $Date: 2008/01/02 14:58:00 $
  Version:   $Revision: 1.190 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#include "gdcmUtil.h"
#include "gdcmDebug.h"

#include <iostream>
#include <stdarg.h> // for va_list

// For GetCurrentDate, GetCurrentTime
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__MINGW32__)
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

#include <stdarg.h>  //only included in implementation file
#include <stdio.h>   //only included in implementation file

#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__MINGW32__)
   #include <winsock.h>  // for gethostname and gethostbyname and GetTickCount...
// I haven't find a way to determine wether we need to under GetCurrentTime or not...
// I think the best solution would simply to get rid of this problematic function
// and use a 'less' common name...
#if !defined(__BORLANDC__) || (__BORLANDC__ >= 0x0560)
   #undef GetCurrentTime
#endif
#else
   #include <unistd.h>  // for gethostname
   #include <netdb.h>   // for gethostbyname
#endif

// For GetMACAddress
#ifdef _WIN32
   #include <snmp.h>
   #include <conio.h>
#else
   #include <unistd.h>
   #include <stdlib.h>
   #include <string.h>
   #include <sys/types.h>
#endif

#ifdef CMAKE_HAVE_SYS_IOCTL_H
   #include <sys/ioctl.h>  // For SIOCGIFCONF on Linux
#endif
#ifdef CMAKE_HAVE_SYS_SOCKET_H
   #include <sys/socket.h>
#endif
#ifdef CMAKE_HAVE_SYS_SOCKIO_H
   #include <sys/sockio.h>  // For SIOCGIFCONF on SunOS
#endif
#ifdef CMAKE_HAVE_NET_IF_H
   #include <net/if.h>
#endif
#ifdef CMAKE_HAVE_NETINET_IN_H
   #include <netinet/in.h>   //For IPPROTO_IP
#endif
#ifdef CMAKE_HAVE_NET_IF_DL_H
   #include <net/if_dl.h>
#endif
#if defined(CMAKE_HAVE_NET_IF_ARP_H) && defined(__sun)
   // This is absolutely necessary on SunOS
   #include <net/if_arp.h>
#endif

// For GetCurrentThreadID()
#ifdef __linux__
   #include <sys/types.h>
   #include <linux/unistd.h>
#endif
#ifdef __sun
   #include <thread.h>
#endif

namespace GDCM_NAME_SPACE
{
//-------------------------------------------------------------------------
const std::string Util::GDCM_UID = "1.2.826.0.1.3680043.2.1143";
std::string Util::RootUID        = GDCM_UID;
/*
 * File Meta Information Version (0002,0001) shall contain a two byte OB 
 * value consisting of a 0x00 byte, followed by 0x01 byte, and not the 
 * value 0x0001 encoded as a little endian 16 bit short value, 
 * which would be the other way around...
 */

#if defined(GDCM_WORDS_BIGENDIAN) || defined(GDCM_FORCE_BIGENDIAN_EMULATION)
   const uint16_t Util::FMIV = 0x0001;
#else
   const uint16_t Util::FMIV = 0x0100;
#endif   
   uint8_t *Util::FileMetaInformationVersion = (uint8_t *)&FMIV;

   std::string Util::GDCM_MAC_ADDRESS = GetMACAddress();

//-------------------------------------------------------------------------
// Public
/**
 * \brief Provide a better 'c++' approach for sprintf
 * For example c code is:
 * char result[2048]; // hope 2048 is enough
 * sprintf(result, "%04x|%04x", group , elem);
 *
 * c++ code is 
 * std::ostringstream buf;
 * buf << std::right << std::setw(4) << std::setfill('0') << std::hex
 *     << group << "|" << std::right << std::setw(4) << std::setfill('0') 
 *     << std::hex <<  elem;
 * buf.str();
 *
 * gdcm style code is
 * string result;
 * result = gdcm::Util::Format("%04x|%04x", group , elem);
 */
std::string Util::Format(const char *format, ...)
{
   char buffer[2048]; // hope 2048 is enough
   va_list args;
   va_start(args, format);
   vsprintf(buffer, format, args);  //might be a security flaw
   va_end(args); // Each invocation of va_start should be matched 
                 // by a corresponding invocation of va_end
                 // args is then 'undefined'
   return buffer;
}


/**
 * \brief Because not available in C++ (?)
 * @param str string to check
 * @param tokens std::vector to receive the tokenized substrings
 * @param delimiters string containing the character delimitors
 
 */
void Util::Tokenize (const std::string &str,
                     std::vector<std::string> &tokens,
                     const std::string &delimiters)
{
   std::string::size_type lastPos = str.find_first_not_of(delimiters,0);
   std::string::size_type pos     = str.find_first_of    (delimiters,lastPos);
   while (std::string::npos != pos || std::string::npos != lastPos)
   {
      tokens.push_back(str.substr(lastPos, pos - lastPos));
      lastPos = str.find_first_not_of(delimiters, pos);
      pos     = str.find_first_of    (delimiters, lastPos);
   }
}

/**
 * \brief Because not available in C++ (?)
 *        Counts the number of occurences of a substring within a string
 * @param str string to check
 * @param subStr substring to count
 */
 
int Util::CountSubstring (const std::string &str,
                          const std::string &subStr)
{
   int count = 0;                 // counts how many times it appears
   std::string::size_type x = 0;  // The index position in the string

   do
   {
      x = str.find(subStr,x);     // Find the substring
      if (x != std::string::npos) // If present
      {
         count++;                 // increase the count
         x += subStr.length();    // Skip this word
      }
   }
   while (x != std::string::npos);// Carry on until not present

   return count;
}

/**
 * \brief  Checks whether a 'string' is printable or not (in order
 *         to avoid corrupting the terminal of invocation when printing)
 * @param s string to check
 */
bool Util::IsCleanString(std::string const &s)
{
   for(unsigned int i=0; i<s.size(); i++)
   {
      if (!isprint((unsigned char)s[i]) )
      {
         return false;
      }
   }
   return true;   
}

/**
 * \brief  Checks whether an 'area' is printable or not (in order
 *         to avoid corrupting the terminal of invocation when printing)
 * @param s area to check (uint8_t is just for prototyping. feel free to cast)
 * @param l area length to check
 */
bool Util::IsCleanArea(uint8_t *s, int l)
{
   for( int i=0; i<l; i++)
   {
      if (!isprint((unsigned char)s[i]) )
      {
         return false;
      }
   }
   return true;   
}
/**
 * \brief  Weed out a string from the non-printable characters (in order
 *         to avoid corrupting the terminal of invocation when printing)
 * @param s string to check (uint8_t is just for prototyping. feel free to cast)
 */
std::string Util::CreateCleanString(std::string const &s)
{
   std::string str = s;

   for(unsigned int i=0; i<str.size(); i++)
   {
      if (!isprint((unsigned char)str[i]) )
      {
         str[i] = '.';
      }
   }
   if (str.size() > 0 )
   {
      if (!isprint((unsigned char)s[str.size()-1]) )
      {
         if (s[str.size()-1] == 0 )
         {
            str[str.size()-1] = ' ';
         }
      }
   }
   return str;
}
/**
 * \brief  Replaces all special characters
 * @param s string to modify
 * @param rep replacement char
 */
void Util::ReplaceSpecChar(std::string &s, std::string &rep)
{
   unsigned int s_size = s.size();
   for(unsigned int i=0; i<s_size; i++)
   {
      if (! ( s[i] == '.' || s[i] == '%' || s[i] == '_'
          || (s[i] >= '+' && s[i] <= '-')       
          || (s[i] >= 'a' && s[i] <= 'z')
          || (s[i] >= '0' && s[i] <= '9')
          || (s[i] >= 'A' && s[i] <= 'Z')))
     {
        s.replace(i, 1, rep);
     }
   }
      // deal with Dicom strings trailing '\0' 
   if(s[s_size-1] == rep.c_str()[0])
      s.erase(s_size-1, 1);
}


/**
 * \brief  Weed out a string from the non-printable characters (in order
 *         to avoid corrupting the terminal of invocation when printing)
 * @param s area to process (uint8_t is just for prototyping. feel free to cast)
 * @param l area length to check
 */
std::string Util::CreateCleanString(uint8_t *s, int l)
{
   std::string str;

   for( int i=0; i<l; i++)
   {
      if (!isprint((unsigned char)s[i]) )
      {
         str = str + '.';
      }
   else
      {
         str = str + (char )s[i];
      }
   }

   return str;
}
/**
 * \brief   Add a SEPARATOR to the end of the name if necessary
 * @param   pathname file/directory name to normalize 
 */
std::string Util::NormalizePath(std::string const &pathname)
{
/*
   const char SEPARATOR_X      = '/';
   const char SEPARATOR_WIN    = '\\';
#ifdef _WIN32
   const std::string SEPARATOR = "\\";
#else
   const std::string SEPARATOR = "/";
#endif
*/
   std::string name = pathname;
   int size = name.size();

//   if ( name[size-1] != SEPARATOR_X && name[size-1] != SEPARATOR_WIN )
   if ( name[size-1] != GDCM_FILESEPARATOR )
   {
      name += GDCM_FILESEPARATOR;
   }
   return name;
}

/**
 * \brief   Get the (directory) path from a full path file name
 * @param   fullName file/directory name to extract Path from
 */
std::string Util::GetPath(std::string const &fullName)
{
   std::string res = fullName;
/*
   
   int pos1 = res.rfind("/");
   int pos2 = res.rfind("\\");
   if ( pos1 > pos2 )
   {
      res.resize(pos1);
   }
   else
   {
      res.resize(pos2);
   }
*/
   int pos = res.rfind(GDCM_FILESEPARATOR);
   res.resize(pos);
   return res;
}

/**
 * \brief   Get the (last) name of a full path file name
 * @param   fullName file/directory name to extract end name from
 */
std::string Util::GetName(std::string const &fullName)
{   
  std::string filename = fullName;
/*
  std::string::size_type slash_pos = filename.rfind("/");
  std::string::size_type backslash_pos = filename.rfind("\\");
  // At least with my gcc4.0.1, unfound char results in pos =4294967295 ...
  //slash_pos = slash_pos > backslash_pos ? slash_pos : backslash_pos;  
  slash_pos = slash_pos < backslash_pos ? slash_pos : backslash_pos;
*/
  std::string::size_type slash_pos = filename.rfind(GDCM_FILESEPARATOR);
  if (slash_pos != std::string::npos )
    {
    return filename.substr(slash_pos + 1);
    }
  else
    {
    return filename;
    }
} 

/**
 * \brief   Get the current date of the system in a dicom string
 */
std::string Util::GetCurrentDate()
{
    char tmp[512];
    time_t tloc;
    time (&tloc);    
    strftime(tmp,512,"%Y%m%d", localtime(&tloc) );
    return tmp;
}

/**
 * \brief   Get the current time of the system in a dicom string
 */
std::string Util::GetCurrentTime()
{
    char tmp[512];
    time_t tloc;
    time (&tloc);
    strftime(tmp,512,"%H%M%S", localtime(&tloc) );
    return tmp;  
}

/**
 * \brief  Get both the date and time at the same time to avoid problem 
 * around midnight where the two calls could be before and after midnight
 */
std::string Util::GetCurrentDateTime()
{
   char tmp[40];
   long milliseconds;
   time_t timep;
  
   // We need implementation specific functions to obtain millisecond precision
#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__MINGW32__)
   struct timeb tb;
   ::ftime(&tb);
   timep = tb.time;
   milliseconds = tb.millitm;
#else
   struct timeval tv;
   gettimeofday (&tv, NULL);
   timep = tv.tv_sec;
   // Compute milliseconds from microseconds.
   milliseconds = tv.tv_usec / 1000;
#endif
   // Obtain the time of day, and convert it to a tm struct.
   struct tm *ptm = localtime (&timep);
   // Format the date and time, down to a single second.
   strftime (tmp, sizeof (tmp), "%Y%m%d%H%M%S", ptm);

   // Add milliseconds
   // Don't use Util::Format to accelerate execution of code
   char tmpAll[80];
   sprintf(tmpAll,"%s%03ld",tmp,milliseconds);

   return tmpAll;
}

unsigned int Util::GetCurrentThreadID()
{
// FIXME the implementation is far from complete
#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__MINGW32__)
  return (unsigned int)GetCurrentThreadId();
#else
#ifdef __linux__
   return 0;
   // Doesn't work on fedora, but is in the man page...
   //return (unsigned int)gettid();
#else
#ifdef __sun
   return (unsigned int)thr_self();
#else
   //default implementation
   return 0;
#endif // __sun
#endif // __linux__
#endif // Win32
}

unsigned int Util::GetCurrentProcessID()
{
#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__MINGW32__)
  // NOTE: There is also a _getpid()...
  return (unsigned int)GetCurrentProcessId();
#else
  // get process identification, POSIX
  return (unsigned int)getpid();
#endif
}

/**
 * \brief   tells us whether the processor we are working with is BigEndian or not
 */
bool Util::IsCurrentProcessorBigEndian()
{
#if defined(GDCM_WORDS_BIGENDIAN)
   return true;
#else
   return false;
#endif
}

/**
 * \brief Create a /DICOM/ string:
 * It should a of even length (no odd length ever)
 * It can contain as many (if you are reading this from your
 * editor the following character is backslash followed by zero
 * that needed to be escaped with an extra backslash for doxygen) \\0
 * as you want.
 */
std::string Util::DicomString(const char *s, size_t l)
{
   std::string r(s, s+l);
   gdcmAssertMacro( !(r.size() % 2) ); // == basically 'l' is even
   return r;
}

/**
 * \brief Create a /DICOM/ string:
 * It should a of even length (no odd length ever)
 * It can contain as many (if you are reading this from your
 * editor the following character is backslash followed by zero
 * that needed to be escaped with an extra backslash for doxygen) \\0
 * as you want.
 * This function is similar to DicomString(const char*), 
 * except it doesn't take a length. 
 * It only pad with a null character if length is odd
 */
std::string Util::DicomString(const char *s)
{
   size_t l = strlen(s);
   if ( l%2 )
   {
      l++;
   }
   std::string r(s, s+l);
   gdcmAssertMacro( !(r.size() % 2) );
   return r;
}

/**
 * \brief Safely check the equality of two Dicom String:
 *        - Both strings should be of even length
 *        - We allow padding of even length string by either
 *          a null character of a space
 */
bool Util::DicomStringEqual(const std::string &s1, const char *s2)
{
  // s2 is the string from the DICOM reference e.g. : 'MONOCHROME1'
  std::string s1_even = s1; //Never change input parameter
  std::string s2_even = DicomString( s2 );
  if ( s1_even[s1_even.size()-1] == ' ' )
  {
    s1_even[s1_even.size()-1] = '\0'; //replace space character by null
  }
  return s1_even == s2_even;
}

/**
 * \brief Safely compare two Dicom String:
 *        - Both strings should be of even length
 *        - We allow padding of even length string by either
 *          a null character of a space
 */
bool Util::CompareDicomString(const std::string &s1, const char *s2, int op)
{
  // s2 is the string from the DICOM reference e.g. : 'MONOCHROME1'
  std::string s1_even = s1; //Never change input parameter
  std::string s2_even = DicomString( s2 );
  if ( s1_even[s1_even.size()-1] == ' ' )
  {
    s1_even[s1_even.size()-1] = '\0'; //replace space character by null
  }
  switch (op)
  {
     case GDCM_EQUAL :
        return s1_even == s2_even;
     case GDCM_DIFFERENT :  
        return s1_even != s2_even;
     case GDCM_GREATER :  
        return s1_even >  s2_even;  
     case GDCM_GREATEROREQUAL :  
        return s1_even >= s2_even;
     case GDCM_LESS :
        return s1_even <  s2_even;
     case GDCM_LESSOREQUAL :
        return s1_even <= s2_even;
     default :
        gdcmDebugMacro(" Wrong operator : " << op);
        return false;
  }
}

#ifdef _WIN32
   typedef BOOL(WINAPI * pSnmpExtensionInit) (
           IN DWORD dwTimeZeroReference,
           OUT HANDLE * hPollForTrapEvent,
           OUT AsnObjectIdentifier * supportedView);

   typedef BOOL(WINAPI * pSnmpExtensionTrap) (
           OUT AsnObjectIdentifier * enterprise,
           OUT AsnInteger * genericTrap,
           OUT AsnInteger * specificTrap,
           OUT AsnTimeticks * timeStamp,
           OUT RFC1157VarBindList * variableBindings);

   typedef BOOL(WINAPI * pSnmpExtensionQuery) (
           IN BYTE requestType,
           IN OUT RFC1157VarBindList * variableBindings,
           OUT AsnInteger * errorStatus,
           OUT AsnInteger * errorIndex);

   typedef BOOL(WINAPI * pSnmpExtensionInitEx) (
           OUT AsnObjectIdentifier * supportedView);
#endif //_WIN32

#ifdef __sgi
static int SGIGetMacAddress(unsigned char *addr)
{
  FILE *f = popen("/etc/nvram eaddr","r");
  if(f == 0)
    {
    return -1;
    }
  unsigned int x[6];
  if(fscanf(f,"%02x:%02x:%02x:%02x:%02x:%02x",
            x,x+1,x+2,x+3,x+4,x+5) != 6)
    {
    pclose(f);
    return -1;
    }
  for(unsigned int i = 0; i < 6; i++)
    {
    addr[i] = static_cast<unsigned char>(x[i]);
    }
  return 0;
}
#endif

/// \brief gets current M.A.C adress (for internal use only)
int GetMacAddrSys ( unsigned char *addr );
int GetMacAddrSys ( unsigned char *addr )
{
#ifdef _WIN32
   WSADATA WinsockData;
   if ( (WSAStartup(MAKEWORD(2, 0), &WinsockData)) != 0 ) 
   {
      std::cerr << "in Get MAC Adress (internal) : This program requires Winsock 2.x!" 
             << std::endl;
      return -1;
   }

   HANDLE PollForTrapEvent;
   AsnObjectIdentifier SupportedView;
   UINT OID_ifEntryType[]  = { 1, 3, 6, 1, 2, 1, 2, 2, 1, 3 };
   UINT OID_ifEntryNum[]   = { 1, 3, 6, 1, 2, 1, 2, 1 };
   UINT OID_ipMACEntAddr[] = { 1, 3, 6, 1, 2, 1, 2, 2, 1, 6 };
   AsnObjectIdentifier MIB_ifMACEntAddr = {
       sizeof(OID_ipMACEntAddr) / sizeof(UINT), OID_ipMACEntAddr };
   AsnObjectIdentifier MIB_ifEntryType = {
       sizeof(OID_ifEntryType) / sizeof(UINT), OID_ifEntryType };
   AsnObjectIdentifier MIB_ifEntryNum = {
       sizeof(OID_ifEntryNum) / sizeof(UINT), OID_ifEntryNum };
   RFC1157VarBindList varBindList;
   RFC1157VarBind varBind[2];
   AsnInteger errorStatus;
   AsnInteger errorIndex;
   AsnObjectIdentifier MIB_NULL = { 0, 0 };
   int ret;
   int dtmp;
   int j = 0;

   // Load the SNMP dll and get the addresses of the functions necessary
   HINSTANCE m_hInst = LoadLibrary("inetmib1.dll");
   if (m_hInst < (HINSTANCE) HINSTANCE_ERROR)
   {
      return -1;
   }
   pSnmpExtensionInit m_Init =
       (pSnmpExtensionInit) GetProcAddress(m_hInst, "SnmpExtensionInit");
   pSnmpExtensionQuery m_Query =
       (pSnmpExtensionQuery) GetProcAddress(m_hInst, "SnmpExtensionQuery");
   m_Init(GetTickCount(), &PollForTrapEvent, &SupportedView);

   /* Initialize the variable list to be retrieved by m_Query */
   varBindList.list = varBind;
   varBind[0].name = MIB_NULL;
   varBind[1].name = MIB_NULL;

   // Copy in the OID to find the number of entries in the
   // Inteface table
   varBindList.len = 1;        // Only retrieving one item
   SNMP_oidcpy(&varBind[0].name, &MIB_ifEntryNum);
   m_Query(ASN_RFC1157_GETNEXTREQUEST, &varBindList, &errorStatus,
                 &errorIndex);
//   printf("# of adapters in this system : %i\n",
//          varBind[0].value.asnValue.number);
   varBindList.len = 2;

   // Copy in the OID of ifType, the type of interface
   SNMP_oidcpy(&varBind[0].name, &MIB_ifEntryType);

   // Copy in the OID of ifPhysAddress, the address
   SNMP_oidcpy(&varBind[1].name, &MIB_ifMACEntAddr);

   do
   {
      // Submit the query.  Responses will be loaded into varBindList.
      // We can expect this call to succeed a # of times corresponding
      // to the # of adapters reported to be in the system
      ret = m_Query(ASN_RFC1157_GETNEXTREQUEST, &varBindList, &errorStatus,
                    &errorIndex); 
      if (!ret)
      {
         ret = 1;
      }
      else
      {
         // Confirm that the proper type has been returned
         ret = SNMP_oidncmp(&varBind[0].name, &MIB_ifEntryType,
                            MIB_ifEntryType.idLength);
      }
      if (!ret)
      {
         j++;
         dtmp = varBind[0].value.asnValue.number;

         // Type 6 describes ethernet interfaces
         if (dtmp == 6)
         {
            // Confirm that we have an address here
            ret = SNMP_oidncmp(&varBind[1].name, &MIB_ifMACEntAddr,
                               MIB_ifMACEntAddr.idLength);
            if ( !ret && varBind[1].value.asnValue.address.stream != NULL )
            {
               if ( (varBind[1].value.asnValue.address.stream[0] == 0x44)
                 && (varBind[1].value.asnValue.address.stream[1] == 0x45)
                 && (varBind[1].value.asnValue.address.stream[2] == 0x53)
                 && (varBind[1].value.asnValue.address.stream[3] == 0x54)
                 && (varBind[1].value.asnValue.address.stream[4] == 0x00) )
               {
                   // Ignore all dial-up networking adapters
                   std::cerr << "in Get MAC Adress (internal) : Interface #" 
                             << j << " is a DUN adapter\n";
                   continue;
               }
               if ( (varBind[1].value.asnValue.address.stream[0] == 0x00)
                 && (varBind[1].value.asnValue.address.stream[1] == 0x00)
                 && (varBind[1].value.asnValue.address.stream[2] == 0x00)
                 && (varBind[1].value.asnValue.address.stream[3] == 0x00)
                 && (varBind[1].value.asnValue.address.stream[4] == 0x00)
                 && (varBind[1].value.asnValue.address.stream[5] == 0x00) )
               {
                  // Ignore NULL addresses returned by other network
                  // interfaces
                  std::cerr << "in Get MAC Adress (internal) :  Interface #" 
                            << j << " is a NULL address\n";
                  continue;
               }
               memcpy( addr, varBind[1].value.asnValue.address.stream, 6);
            }
         }
      }
   } while (!ret);

   // Free the bindings
   SNMP_FreeVarBind(&varBind[0]);
   SNMP_FreeVarBind(&varBind[1]);
   return 0;
#endif //Win32 version

#if defined(__sgi)
   return SGIGetMacAddress(addr);
#endif // __sgi


// implementation for POSIX system
#if defined(CMAKE_HAVE_NET_IF_ARP_H) && defined(__sun)
   //The POSIX version is broken anyway on Solaris, plus would require full
   //root power
   struct  arpreq          parpreq;
   struct  sockaddr_in     *psa;
   struct  hostent         *phost;
   char                    hostname[MAXHOSTNAMELEN];
   char                    **paddrs;
   int                     sock, status=0;

   if (gethostname(hostname,  MAXHOSTNAMELEN) != 0 )
   {
      perror("in Get MAC Adress (internal) : gethostname");
      return -1;
   }
   phost = gethostbyname(hostname);
   paddrs = phost->h_addr_list;

   sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
   if (sock == -1 )
   {
      perror("in Get MAC Adress (internal) : sock");
      return -1;
   }
   memset(&parpreq, 0, sizeof(struct arpreq));
   psa = (struct sockaddr_in *) &parpreq.arp_pa;

   memset(psa, 0, sizeof(struct sockaddr_in));
   psa->sin_family = AF_INET;
   memcpy(&psa->sin_addr, *paddrs, sizeof(struct in_addr));

   status = ioctl(sock, SIOCGARP, &parpreq);
   if (status == -1 )
   {
      perror("in Get MAC Adress (internal) : SIOCGARP");
      return -1;
   }
   memcpy(addr, parpreq.arp_ha.sa_data, 6);

   return 0;
#else
#ifdef CMAKE_HAVE_NET_IF_H
   int       sd;
   struct ifreq    ifr, *ifrp;
   struct ifconf    ifc;
   char buf[1024];
   int      n, i;
   unsigned char    *a;
#if defined(AF_LINK) && (!defined(SIOCGIFHWADDR) && !defined(SIOCGENADDR))
   struct sockaddr_dl *sdlp;
#endif

//
// BSD 4.4 defines the size of an ifreq to be
// max(sizeof(ifreq), sizeof(ifreq.ifr_name)+ifreq.ifr_addr.sa_len
// However, under earlier systems, sa_len isn't present, so the size is 
// just sizeof(struct ifreq)
// We should investigate the use of SIZEOF_ADDR_IFREQ
//
#ifdef HAVE_SA_LEN
   #ifndef max
      #define max(a,b) ((a) > (b) ? (a) : (b))
   #endif
   #define ifreq_size(i) max(sizeof(struct ifreq),\
        sizeof((i).ifr_name)+(i).ifr_addr.sa_len)
#else
   #define ifreq_size(i) sizeof(struct ifreq)
#endif // HAVE_SA_LEN

   if ( (sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0 )
   {
      return -1;
   }
   memset(buf, 0, sizeof(buf));
   ifc.ifc_len = sizeof(buf);
   ifc.ifc_buf = buf;
   if (ioctl (sd, SIOCGIFCONF, (char *)&ifc) < 0)
   {
      close(sd);
      return -1;
   }
   n = ifc.ifc_len;
   for (i = 0; i < n; i+= ifreq_size(*ifrp) )
   {
      ifrp = (struct ifreq *)((char *) ifc.ifc_buf+i);
      strncpy(ifr.ifr_name, ifrp->ifr_name, IFNAMSIZ);
#ifdef SIOCGIFHWADDR
      if (ioctl(sd, SIOCGIFHWADDR, &ifr) < 0)
         continue;
      a = (unsigned char *) &ifr.ifr_hwaddr.sa_data;
#else
#ifdef SIOCGENADDR
      // In theory this call should also work on Sun Solaris, but apparently
      // SIOCGENADDR is not implemented properly thus the call 
      // ioctl(sd, SIOCGENADDR, &ifr) always returns errno=2 
      // (No such file or directory)
      // Furthermore the DLAPI seems to require full root access
      if (ioctl(sd, SIOCGENADDR, &ifr) < 0)
         continue;
      a = (unsigned char *) ifr.ifr_enaddr;
#else
#ifdef AF_LINK
      sdlp = (struct sockaddr_dl *) &ifrp->ifr_addr;
      if ((sdlp->sdl_family != AF_LINK) || (sdlp->sdl_alen != 6))
         continue;
      a = (unsigned char *) &sdlp->sdl_data[sdlp->sdl_nlen];
#else
      perror("in Get MAC Adress (internal) : No way to access hardware");
      close(sd);
      return -1;
#endif // AF_LINK
#endif // SIOCGENADDR
#endif // SIOCGIFHWADDR
      if (!a[0] && !a[1] && !a[2] && !a[3] && !a[4] && !a[5]) continue;

      if (addr) 
      {
         memcpy(addr, a, 6);
         close(sd);
         return 0;
      }
   }
   close(sd);
#endif
   // Not implemented platforms (or no Ethernet cable !)
   
   /// \todo FIXME I wish we don't pollute command line applications when no Ethernet cable !
   //perror("Probabely your computer is not connected on a network, therefore its MAC adress cannot be found (or there is a configuration problem on your platform)");

   // But the following -> error: invalid use of 'this' in non-member function
   //gdcmWarningMacro( "Probabely your computer is not connected on a network, therefore its MAC adress cannot be found (or there is a configuration problem on your platform)");

   memset(addr,0,6);
   return -1;
#endif //__sun
}

/**
 * \brief Mini function to return the last digit from a number express in base 256
 *        pre condition data contain an array of 6 unsigned char
 *        post condition carry contain the last digit
 */
inline int getlastdigit(unsigned char *data)
{
  int extended, carry = 0;
  for(int i=0;i<6;i++)
    {
    extended = (carry << 8) + data[i];
    data[i] = extended / 10;
    carry = extended % 10;
    }
  return carry;
}

/**
 * \brief Encode the mac address on a fixed length string of 15 characters.
 * we save space this way.
 */
std::string Util::GetMACAddress()
{
   // This code is the result of a long internet search to find something
   // as compact as possible (not OS independant). We only have to separate
   // 3 OS: Win32, SunOS and 'real' POSIX
   // http://groups-beta.google.com/group/comp.unix.solaris/msg/ad36929d783d63be
   // http://bdn.borland.com/article/0,1410,26040,00.html
   unsigned char addr[6];

   int stat = GetMacAddrSys(addr);
   if (stat == 0)
   {
      // We need to convert a 6 digit number from base 256 to base 10, using integer
      // would requires a 48bits one. To avoid this we have to reimplement the div + modulo 
      // with string only
      bool zero = false;
      int res;
      std::string sres;
      while(!zero)
      {
         res = getlastdigit(addr);
         sres.insert(sres.begin(), '0' + res);
         zero = (addr[0] == 0) && (addr[1] == 0) && (addr[2] == 0) 
             && (addr[3] == 0) && (addr[4] == 0) && (addr[5] == 0);
      }

      return sres;
   }
   else
   {
      gdcmStaticWarningMacro("Problem in finding the MAC Address");
      return "";
   }
}

/**
 * \brief Creates a new UID. As stipulated in the DICOM ref
 *        each time a DICOM image is created it should have 
 *        a unique identifier (URI)
 * @param root is the DICOM prefix assigned by IOS group
 */
std::string Util::CreateUniqueUID(const std::string &root)
{
   std::string prefix;
   std::string append;
   if ( root.empty() )
   {
      // gdcm UID prefix, as supplied by http://www.medicalconnections.co.uk
      assert( !RootUID.empty() );
      prefix = RootUID; 
   }
   else
   {
      prefix = root;
   }

   // A root was specified use it to forge our new UID:
   append += ".";
   //append += Util::GetMACAddress(); // to save CPU time
   if( !Util::GDCM_MAC_ADDRESS.empty() ) // When mac address was empty we would end up with a double . which is illegal
     {
     append += Util::GDCM_MAC_ADDRESS;
     append += ".";
     }
   append += Util::GetCurrentDateTime();
   append += ".";
   //Also add a mini random number just in case:
   char tmp[10];
   int r = (int) (100.0*rand()/RAND_MAX);
   // Don't use Util::Format to accelerate the execution
   sprintf(tmp,"%02d", r);
   append += tmp;

   // If append is too long we need to rehash it
   if ( (prefix + append).size() > 64 )
   {
      gdcmStaticErrorMacro( "Size of UID is too long." );
      // we need a hash function to truncate this number
      // if only md5 was cross plateform
      // MD5(append);
   }

   return prefix + append;
}

void Util::SetRootUID(const std::string &root)
{
   if ( root.empty() )
      RootUID = GDCM_UID;
   else
      RootUID = root;
}

const std::string &Util::GetRootUID()
{
   return RootUID;
}

//-------------------------------------------------------------------------
/**
 * \brief binary_write binary_write
 * @param os ostream to write to 
 * @param val 16 bits value to write
 */ 
std::ostream &binary_write(std::ostream &os, const uint16_t &val)
{
#if defined(GDCM_WORDS_BIGENDIAN) || defined(GDCM_FORCE_BIGENDIAN_EMULATION)
   uint16_t swap;
   swap = ( val << 8 | val >> 8 );

   return os.write(reinterpret_cast<const char*>(&swap), 2);
#else
   return os.write(reinterpret_cast<const char*>(&val), 2);
#endif //GDCM_WORDS_BIGENDIAN
}

/**
 * \brief binary_write binary_write
 * @param os ostream to write to
 * @param val 32 bits value to write
 */ 
std::ostream &binary_write(std::ostream &os, const uint32_t &val)
{
#if defined(GDCM_WORDS_BIGENDIAN) || defined(GDCM_FORCE_BIGENDIAN_EMULATION)
   uint32_t swap;
   swap = (  (val<<24)               | ((val<<8)  & 0x00ff0000) | 
            ((val>>8)  & 0x0000ff00) |  (val>>24)               );
   return os.write(reinterpret_cast<const char*>(&swap), 4);
#else
   return os.write(reinterpret_cast<const char*>(&val), 4);
#endif //GDCM_WORDS_BIGENDIAN
}

/**
 * \brief binary_write binary_write
 * @param os ostream to write to
 * @param val double (64 bits) value to write
 */ 
std::ostream &binary_write(std::ostream &os, const double &val)
{
#if defined(GDCM_WORDS_BIGENDIAN) || defined(GDCM_FORCE_BIGENDIAN_EMULATION)    
   double swap = val;
   
   char *beg = (char *)&swap;
   char *end = beg + 7;
   char t;
   for (unsigned int i = 0; i<7; i++)
   {
      t    = *beg;
      *beg = *end;
      *end = t;
      beg++,
      end--;  
   }  
   return os.write(reinterpret_cast<const char*>(&swap), 8);
#else
   return os.write(reinterpret_cast<const char*>(&val), 8);
#endif //GDCM_WORDS_BIGENDIAN
}

/**
 * \brief  binary_write binary_write
 * @param os ostream to write to
 * @param val 8 bits characters aray to write
 */ 
std::ostream &binary_write(std::ostream &os, const char *val)
{
   return os.write(val, strlen(val));
}

/**
 * \brief  binary_write binary_write
 * @param os ostream to write to
 * @param val std::string value to write
 */ 
std::ostream &binary_write(std::ostream &os, std::string const &val)
{
   return os.write(val.c_str(), val.size());
}

/**
 * \brief  binary_write binary_write
 * @param os ostream to write to
 * @param val 8 bits 'characters' aray to write
 * @param len length of the 'value' to be written
 */ 
std::ostream &binary_write(std::ostream &os, const uint8_t *val, size_t len)
{
   // We are writting sizeof(char) thus no need to swap bytes
   return os.write(reinterpret_cast<const char*>(val), len);
}

/**
 * \brief  binary_write binary_write
 * @param os ostream to write to
 * @param val 16 bits words aray to write
 * @param len length (in bytes) of the 'value' to be written 
 */ 
std::ostream &binary_write(std::ostream &os, const uint16_t *val, size_t len)
{
// This is tricky since we are writting two bytes buffer. 
// Be carefull with little endian vs big endian. 
// Also this other trick is to allocate a small (efficient) buffer that store
// intermediate result before writting it.
#if defined(GDCM_WORDS_BIGENDIAN) || defined(GDCM_FORCE_BIGENDIAN_EMULATION)
   const int BUFFER_SIZE = 4096;
   static char buffer[BUFFER_SIZE];
   uint16_t *binArea16 = (uint16_t*)val; //for the const
 
   // how many BUFFER_SIZE long pieces in binArea ?
   int nbPieces = len/BUFFER_SIZE; //(16 bits = 2 Bytes)
   int remainingSize = len%BUFFER_SIZE;

   for (int j=0;j<nbPieces;j++)
   {
      uint16_t *pbuffer  = (uint16_t*)buffer; //reinitialize pbuffer
      for (int i = 0; i < BUFFER_SIZE/2; i++)
      {
         *pbuffer = *binArea16 >> 8 | *binArea16 << 8;
         pbuffer++;
         binArea16++;
      }
      os.write ( buffer, BUFFER_SIZE );
   }
   if ( remainingSize > 0)
   {
      uint16_t *pbuffer  = (uint16_t*)buffer; //reinitialize pbuffer
      for (int i = 0; i < remainingSize/2; i++)
      {
         *pbuffer = *binArea16 >> 8 | *binArea16 << 8;
         pbuffer++;
         binArea16++;
      }
      os.write ( buffer, remainingSize );
   }
   return os;
#else
   return os.write(reinterpret_cast<const char*>(val), len);
#endif
}

std::string Util::ConvertToMD5 (std::string &inPszToCrypt)
{
   char *szChar      = new char[ inPszToCrypt.size()+1 ];
   char *szHexOutput = new char[ 16 * 2 + 1 ];
   int nLen,nDi;
   strcpy( szChar, inPszToCrypt.c_str() );
   // création du code md5
   nLen = strlen( szChar );
   md5_state_t state;
   uint8_t digest[ 16 ];
   md5_init   ( &state );
   md5_append ( &state, (const uint8_t *)szChar, nLen);
   md5_finish ( &state, digest );
   for ( nDi = 0; nDi < 16; nDi++)
      sprintf( szHexOutput + nDi * 2, "%02x", digest[ nDi ] );
   szHexOutput[16 * 2]=0;
   delete [] szChar;
   std::string md5String=szHexOutput;
   delete [] szHexOutput;
   return md5String;
}

//-------------------------------------------------------------------------
// Protected

//-------------------------------------------------------------------------
// Private
/**
 * \brief   Return the IP adress of the machine writting the DICOM image
 */
std::string Util::GetIPAddress()
{
   // This is a rip from 
   // http://www.codeguru.com/Cpp/I-N/internet/network/article.php/c3445/
#ifndef HOST_NAME_MAX
   // SUSv2 guarantees that `Host names are limited to 255 bytes'.
   // POSIX 1003.1-2001 guarantees that `Host names (not including the
   // terminating NUL) are limited to HOST_NAME_MAX bytes'.
#define HOST_NAME_MAX 255
   // In this case we should maybe check the string was not truncated.
   // But I don't known how to check that...
#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__MINGW32__)
   // with WinSock DLL we need to initialize the WinSock before using gethostname
   WORD wVersionRequested = MAKEWORD(1,0);
   WSADATA WSAData;
   int err = WSAStartup(wVersionRequested,&WSAData);
   if (err != 0)
   {
      // Tell the user that we could not find a usable
      // WinSock DLL.
      WSACleanup();
      return "127.0.0.1";
   }
#endif
  
#endif //HOST_NAME_MAX

   std::string str;
   char szHostName[HOST_NAME_MAX+1];
   int r = gethostname(szHostName, HOST_NAME_MAX);
 
   if ( r == 0 )
   {
      // Get host adresses
      struct hostent *pHost = gethostbyname(szHostName);
 
      for( int i = 0; pHost!= NULL && pHost->h_addr_list[i]!= NULL; i++ )
      {
         for( int j = 0; j<pHost->h_length; j++ )
         {
            if ( j > 0 ) str += ".";
 
            str += Util::Format("%u", 
                (unsigned int)((unsigned char*)pHost->h_addr_list[i])[j]);
         }
         // str now contains one local IP address 
 
#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__MINGW32__)
   WSACleanup();
#endif

      }
   }
   // If an error occur r == -1
   // Most of the time it will return 127.0.0.1...
   return str;
}

void Util::hfpswap(double *a, double *b)
{
   double tmp;
   tmp=*a;
   *a=*b;
   *b=tmp;
}

// for MD5
/*
  Copyright (C) 1999, 2000, 2002 Aladdin Enterprises.  All rights reserved.

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  L. Peter Deutsch

  ghost@aladdin.com
 */

/* $Id: gdcmUtil.cxx,v 1.190 2008/01/02 14:58:00 malaterre Exp $ */

/*
  Independent implementation of MD5 (RFC 1321).
  This code implements the MD5 Algorithm defined in RFC 1321, whose
  text is available at

 http://www.ietf.org/rfc/rfc1321.txt

  The code is derived from the text of the RFC, including the test suite
  (section A.5) but excluding the rest of Appendix A.  It does not include
  any code or documentation that is identified in the RFC as being
  copyrighted.

  The original and principal author of md5.c is L. Peter Deutsch
  <ghost@aladdin.com>.  Other authors are noted in the change history
  that follows (in reverse chronological order):

  2002-04-13 lpd Clarified derivation from RFC 1321; now handles byte order
      either statically or dynamically; added missing #include <string.h>
      in library.
  2002-03-11 lpd Corrected argument list for main(), and added int return
      type, in test program and T value program.
  2002-02-21 lpd Added missing #include <stdio.h> in test program.
  2000-07-03 lpd Patched to eliminate warnings about "constant is
      unsigned in ANSI C, signed in traditional"; made test program
      self-checking.
  1999-11-04 lpd Edited comments slightly for automatic TOC extraction.
  1999-10-18 lpd Fixed typo in header comment (ansi2knr rather than md5).
  1999-05-03 lpd Original version.
 */

//#include "md5.h"
//#include <string.h>

#undef BYTE_ORDER     /* 1 = big-endian, -1 = little-endian, 0 = unknown */
#ifdef ARCH_IS_BIG_ENDIAN
#  define BYTE_ORDER (ARCH_IS_BIG_ENDIAN ? 1 : -1)
#else
#  define BYTE_ORDER 0
#endif


#define T_MASK ((uint16_t)~0)
#define T1 /* 0xd76aa478 */ (T_MASK ^ 0x28955b87)
#define T2 /* 0xe8c7b756 */ (T_MASK ^ 0x173848a9)
#define T3    0x242070db
#define T4 /* 0xc1bdceee */ (T_MASK ^ 0x3e423111)
#define T5 /* 0xf57c0faf */ (T_MASK ^ 0x0a83f050)
#define T6    0x4787c62a
#define T7 /* 0xa8304613 */ (T_MASK ^ 0x57cfb9ec)
#define T8 /* 0xfd469501 */ (T_MASK ^ 0x02b96afe)
#define T9    0x698098d8
#define T10 /* 0x8b44f7af */ (T_MASK ^ 0x74bb0850)
#define T11 /* 0xffff5bb1 */ (T_MASK ^ 0x0000a44e)
#define T12 /* 0x895cd7be */ (T_MASK ^ 0x76a32841)
#define T13    0x6b901122
#define T14 /* 0xfd987193 */ (T_MASK ^ 0x02678e6c)
#define T15 /* 0xa679438e */ (T_MASK ^ 0x5986bc71)
#define T16    0x49b40821
#define T17 /* 0xf61e2562 */ (T_MASK ^ 0x09e1da9d)
#define T18 /* 0xc040b340 */ (T_MASK ^ 0x3fbf4cbf)
#define T19    0x265e5a51
#define T20 /* 0xe9b6c7aa */ (T_MASK ^ 0x16493855)
#define T21 /* 0xd62f105d */ (T_MASK ^ 0x29d0efa2)
#define T22    0x02441453
#define T23 /* 0xd8a1e681 */ (T_MASK ^ 0x275e197e)
#define T24 /* 0xe7d3fbc8 */ (T_MASK ^ 0x182c0437)
#define T25    0x21e1cde6
#define T26 /* 0xc33707d6 */ (T_MASK ^ 0x3cc8f829)
#define T27 /* 0xf4d50d87 */ (T_MASK ^ 0x0b2af278)
#define T28    0x455a14ed
#define T29 /* 0xa9e3e905 */ (T_MASK ^ 0x561c16fa)
#define T30 /* 0xfcefa3f8 */ (T_MASK ^ 0x03105c07)
#define T31    0x676f02d9
#define T32 /* 0x8d2a4c8a */ (T_MASK ^ 0x72d5b375)
#define T33 /* 0xfffa3942 */ (T_MASK ^ 0x0005c6bd)
#define T34 /* 0x8771f681 */ (T_MASK ^ 0x788e097e)
#define T35    0x6d9d6122
#define T36 /* 0xfde5380c */ (T_MASK ^ 0x021ac7f3)
#define T37 /* 0xa4beea44 */ (T_MASK ^ 0x5b4115bb)
#define T38    0x4bdecfa9
#define T39 /* 0xf6bb4b60 */ (T_MASK ^ 0x0944b49f)
#define T40 /* 0xbebfbc70 */ (T_MASK ^ 0x4140438f)
#define T41    0x289b7ec6
#define T42 /* 0xeaa127fa */ (T_MASK ^ 0x155ed805)
#define T43 /* 0xd4ef3085 */ (T_MASK ^ 0x2b10cf7a)
#define T44    0x04881d05
#define T45 /* 0xd9d4d039 */ (T_MASK ^ 0x262b2fc6)
#define T46 /* 0xe6db99e5 */ (T_MASK ^ 0x1924661a)
#define T47    0x1fa27cf8
#define T48 /* 0xc4ac5665 */ (T_MASK ^ 0x3b53a99a)
#define T49 /* 0xf4292244 */ (T_MASK ^ 0x0bd6ddbb)
#define T50    0x432aff97
#define T51 /* 0xab9423a7 */ (T_MASK ^ 0x546bdc58)
#define T52 /* 0xfc93a039 */ (T_MASK ^ 0x036c5fc6)
#define T53    0x655b59c3
#define T54 /* 0x8f0ccc92 */ (T_MASK ^ 0x70f3336d)
#define T55 /* 0xffeff47d */ (T_MASK ^ 0x00100b82)
#define T56 /* 0x85845dd1 */ (T_MASK ^ 0x7a7ba22e)
#define T57    0x6fa87e4f
#define T58 /* 0xfe2ce6e0 */ (T_MASK ^ 0x01d3191f)
#define T59 /* 0xa3014314 */ (T_MASK ^ 0x5cfebceb)
#define T60    0x4e0811a1
#define T61 /* 0xf7537e82 */ (T_MASK ^ 0x08ac817d)
#define T62 /* 0xbd3af235 */ (T_MASK ^ 0x42c50dca)
#define T63    0x2ad7d2bb
#define T64 /* 0xeb86d391 */ (T_MASK ^ 0x14792c6e)


void
Util::md5_process(md5_state_t *pms, const uint8_t *data /*[64]*/)
{
    uint16_t
    a = pms->abcd[0], b = pms->abcd[1],
    c = pms->abcd[2], d = pms->abcd[3];
    uint16_t t;

#if BYTE_ORDER > 0
    /* Define storage only for big-endian CPUs. */
    uint16_t X[16];
#else
    /* Define storage for little-endian or both types of CPUs. */
    uint16_t xbuf[16];
    const uint16_t *X;
#endif
    {
#if BYTE_ORDER == 0
/*
 * Determine dynamically whether this is a big-endian or
 * little-endian machine, since we can use a more efficient
 * algorithm on the latter.
 */
static const int w = 1;

if (*((const uint8_t *)&w)) /* dynamic little-endian */
#endif
#if BYTE_ORDER <= 0  /* little-endian */
{
    /*
     * On little-endian machines, we can process properly aligned
     * data without copying it.
     */
    if (!((data - (const uint8_t *)0) & 3)) {
    /* data are properly aligned */
       X = (const uint16_t *)data;
    } else {
     /* not aligned */
     memcpy(xbuf, data, 64);
     X = xbuf;
    }
}
#endif
#if BYTE_ORDER == 0
else  /* dynamic big-endian */
#endif
#if BYTE_ORDER >= 0 /* big-endian */
{
    /*
     * On big-endian machines, we must arrange the bytes in the
     * right order.
     */
    const uint8_t *xp = data;
    int i;

#  if BYTE_ORDER == 0
     X = xbuf; /* (dynamic only) */
#  else
#    define xbuf /* (static only)  */
#  endif
      for (i = 0; i < 16; ++i, xp += 4)
         xbuf[i] = xp[0] + (xp[1] << 8) + (xp[2] << 16) + (xp[3] << 24);
      }
#endif
    }

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

    /* Round 1. */
    /* Let [abcd k s i] denote the operation
       a = b + ((a + F(b,c,d) + X[k] + T[i]) <<< s). */

#define F(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define SET(a, b, c, d, k, s, Ti) \
     t = a + F(b,c,d) + X[k] + Ti;\
     a = ROTATE_LEFT(t, s) + b
    /* Do the following 16 operations. */
    SET(a, b, c, d,  0,  7,  T1);
    SET(d, a, b, c,  1, 12,  T2);
    SET(c, d, a, b,  2, 17,  T3);
    SET(b, c, d, a,  3, 22,  T4);
    SET(a, b, c, d,  4,  7,  T5);
    SET(d, a, b, c,  5, 12,  T6);
    SET(c, d, a, b,  6, 17,  T7);
    SET(b, c, d, a,  7, 22,  T8);
    SET(a, b, c, d,  8,  7,  T9);
    SET(d, a, b, c,  9, 12, T10);
    SET(c, d, a, b, 10, 17, T11);
    SET(b, c, d, a, 11, 22, T12);
    SET(a, b, c, d, 12,  7, T13);
    SET(d, a, b, c, 13, 12, T14);
    SET(c, d, a, b, 14, 17, T15);
    SET(b, c, d, a, 15, 22, T16);
#undef SET
     /* Round 2. */
     /* Let [abcd k s i] denote the operation
          a = b + ((a + G(b,c,d) + X[k] + T[i]) <<< s). */

#define G(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define SET(a, b, c, d, k, s, Ti)\
  t = a + G(b,c,d) + X[k] + Ti;  \
  a = ROTATE_LEFT(t, s) + b
     /* Do the following 16 operations. */
    SET(a, b, c, d,  1,  5, T17);
    SET(d, a, b, c,  6,  9, T18);
    SET(c, d, a, b, 11, 14, T19);
    SET(b, c, d, a,  0, 20, T20);
    SET(a, b, c, d,  5,  5, T21);
    SET(d, a, b, c, 10,  9, T22);
    SET(c, d, a, b, 15, 14, T23);
    SET(b, c, d, a,  4, 20, T24);
    SET(a, b, c, d,  9,  5, T25);
    SET(d, a, b, c, 14,  9, T26);
    SET(c, d, a, b,  3, 14, T27);
    SET(b, c, d, a,  8, 20, T28);
    SET(a, b, c, d, 13,  5, T29);
    SET(d, a, b, c,  2,  9, T30);
    SET(c, d, a, b,  7, 14, T31);
    SET(b, c, d, a, 12, 20, T32);
#undef SET
     /* Round 3. */
     /* Let [abcd k s t] denote the operation
          a = b + ((a + H(b,c,d) + X[k] + T[i]) <<< s). */
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define SET(a, b, c, d, k, s, Ti)\
  t = a + H(b,c,d) + X[k] + Ti;  \
  a = ROTATE_LEFT(t, s) + b
  
     /* Do the following 16 operations. */
    SET(a, b, c, d,  5,  4, T33);
    SET(d, a, b, c,  8, 11, T34);
    SET(c, d, a, b, 11, 16, T35);
    SET(b, c, d, a, 14, 23, T36);
    SET(a, b, c, d,  1,  4, T37);
    SET(d, a, b, c,  4, 11, T38);
    SET(c, d, a, b,  7, 16, T39);
    SET(b, c, d, a, 10, 23, T40);
    SET(a, b, c, d, 13,  4, T41);
    SET(d, a, b, c,  0, 11, T42);
    SET(c, d, a, b,  3, 16, T43);
    SET(b, c, d, a,  6, 23, T44);
    SET(a, b, c, d,  9,  4, T45);
    SET(d, a, b, c, 12, 11, T46);
    SET(c, d, a, b, 15, 16, T47);
    SET(b, c, d, a,  2, 23, T48);
#undef SET
     /* Round 4. */
     /* Let [abcd k s t] denote the operation
          a = b + ((a + I(b,c,d) + X[k] + T[i]) <<< s). */
#define I(x, y, z) ((y) ^ ((x) | ~(z)))
#define SET(a, b, c, d, k, s, Ti)\
  t = a + I(b,c,d) + X[k] + Ti;  \
  a = ROTATE_LEFT(t, s) + b
  
     /* Do the following 16 operations. */
    SET(a, b, c, d,  0,  6, T49);
    SET(d, a, b, c,  7, 10, T50);
    SET(c, d, a, b, 14, 15, T51);
    SET(b, c, d, a,  5, 21, T52);
    SET(a, b, c, d, 12,  6, T53);
    SET(d, a, b, c,  3, 10, T54);
    SET(c, d, a, b, 10, 15, T55);
    SET(b, c, d, a,  1, 21, T56);
    SET(a, b, c, d,  8,  6, T57);
    SET(d, a, b, c, 15, 10, T58);
    SET(c, d, a, b,  6, 15, T59);
    SET(b, c, d, a, 13, 21, T60);
    SET(a, b, c, d,  4,  6, T61);
    SET(d, a, b, c, 11, 10, T62);
    SET(c, d, a, b,  2, 15, T63);
    SET(b, c, d, a,  9, 21, T64);

#undef SET
     /* Then perform the following additions. (That is increment each
        of the four registers by the value it had before this block
        was started.) */
    pms->abcd[0] += a;
    pms->abcd[1] += b;
    pms->abcd[2] += c;
    pms->abcd[3] += d;
}
void
Util::md5_init(md5_state_t *pms)
{
    pms->count[0] = pms->count[1] = 0;
    pms->abcd[0] = 0x67452301;
    pms->abcd[1] = /*0xefcdab89*/ T_MASK ^ 0x10325476;
    pms->abcd[2] = /*0x98badcfe*/ T_MASK ^ 0x67452301;
    pms->abcd[3] = 0x10325476;
}


void
Util::md5_append(md5_state_t *pms, const uint8_t *data, int nbytes)
{
    const uint8_t *p = data;
    int left = nbytes;
    int offset = (pms->count[0] >> 3) & 63;
    uint16_t nbits = (uint16_t)(nbytes << 3);
    if (nbytes <= 0)
       return;
    /* Update the message length. */
    pms->count[1] += nbytes >> 29;
    pms->count[0] += nbits;
    if (pms->count[0] < nbits)
       pms->count[1]++;
    /* Process an initial partial block. */
    if (offset) {
       int copy = (offset + nbytes > 64 ? 64 - offset : nbytes);
       memcpy(pms->buf + offset, p, copy);
       if (offset + copy < 64)
          return;
       p += copy;
       left -= copy;
       md5_process(pms, pms->buf);
    }
    /* Process full blocks. */
    for (; left >= 64; p += 64, left -= 64)
      md5_process(pms, p);
    /* Process a final partial block. */
    if (left)
      memcpy(pms->buf, p, left);
}
void
Util::md5_finish(md5_state_t *pms, uint8_t digest[16])
{
    static const uint8_t pad[64] = {
       0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    uint8_t data[8];
    int i;
    /* Save the length before padding. */
    for (i = 0; i < 8; ++i)
       data[i] = (uint8_t)(pms->count[i >> 2] >> ((i & 3) << 3));
    /* Pad to 56 bytes mod 64. */
    md5_append(pms, pad, ((55 - (pms->count[0] >> 3)) & 63) + 1);
    /* Append the length. */
    md5_append(pms, data, 8);
    for (i = 0; i < 16; ++i)
      digest[i] = (uint8_t)(pms->abcd[i >> 2] >> ((i & 3) << 3));
}

//-------------------------------------------------------------------------
} // end namespace gdcm

