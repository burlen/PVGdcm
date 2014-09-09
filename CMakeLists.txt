PROJECT(GDCM)
CMAKE_MINIMUM_REQUIRED(VERSION 2.0)
# okay in fact you need at least cmake 2.0.4 to get swig working properly
# and you cannot use cmake 2.0.6 out of the box due to a bug (patched in debian package)


     if(COMMAND cmake_policy)
       cmake_policy(SET CMP0003 NEW)
     endif(COMMAND cmake_policy)



#-----------------------------------------------------------------------------
# GDCM version number, usefull for packaging and doxygen doc:
SET(GDCM_MAJOR_VERSION 1)
SET(GDCM_MINOR_VERSION 3)
SET(GDCM_BUILD_VERSION 2)
SET(GDCM_VERSION
    "${GDCM_MAJOR_VERSION}.${GDCM_MINOR_VERSION}.${GDCM_BUILD_VERSION}")

#-----------------------------------------------------------------------------
# Provide compatibility options.
OPTION(GDCM_LEGACY_REMOVE "Remove all legacy code completely." OFF)
OPTION(GDCM_LEGACY_SILENT "Silence all legacy code messages." OFF)
MARK_AS_ADVANCED(GDCM_LEGACY_REMOVE GDCM_LEGACY_SILENT)

# VS 2005 declared that some C functions were deprecated...
IF(CMAKE_COMPILER_2005)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
  ADD_DEFINITIONS(-D_SCL_SECURE_NO_DEPRECATE)
ENDIF(CMAKE_COMPILER_2005) 

#-----------------------------------------------------------------------------
# Build shared lib by default
OPTION(BUILD_SHARED_LIBS "Build GDCM with shared libraries." ON)

SET(GDCM_CMAKE_DIR "${GDCM_SOURCE_DIR}/Utilities/CMake")
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${GDCM_CMAKE_DIR}")

SET(GDCM_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES})

OPTION(GDCM_DEBUG "Turn verbosity of some statement ON." OFF)
MARK_AS_ADVANCED(GDCM_DEBUG)

# --------------------------------------------------------------------------
SET (GDCM_NAME_SPACE gdcm    CACHE STRING "name space")

# --------------------------------------------------------------------------
# Install directories
STRING(TOLOWER ${PROJECT_NAME} project_name)

IF(NOT GDCM_INSTALL_BIN_DIR)
  SET(GDCM_INSTALL_BIN_DIR "/bin")
ENDIF(NOT GDCM_INSTALL_BIN_DIR)

IF(NOT GDCM_INSTALL_LIB_DIR)
  SET(GDCM_INSTALL_LIB_DIR "/lib/${project_name}")
ENDIF(NOT GDCM_INSTALL_LIB_DIR)

IF(NOT GDCM_INSTALL_DATA_DIR)
  SET(GDCM_INSTALL_DATA_DIR "/share/${project_name}")
ENDIF(NOT GDCM_INSTALL_DATA_DIR)

IF(NOT GDCM_INSTALL_INCLUDE_DIR)
  SET(GDCM_INSTALL_INCLUDE_DIR "/include/${project_name}")
ENDIF(NOT GDCM_INSTALL_INCLUDE_DIR)

IF(NOT GDCM_INSTALL_PACKAGE_DIR)
  SET(GDCM_INSTALL_PACKAGE_DIR ${GDCM_INSTALL_LIB_DIR})
ENDIF(NOT GDCM_INSTALL_PACKAGE_DIR)

IF(NOT GDCM_VTK_INSTALL_PACKAGE_DIR)
  SET(GDCM_VTK_INSTALL_PACKAGE_DIR ${VTK_INSTALL_PACKAGE_DIR})
ENDIF(NOT GDCM_VTK_INSTALL_PACKAGE_DIR)

IF(NOT GDCM_INSTALL_NO_DEVELOPMENT)
  SET(GDCM_INSTALL_NO_DEVELOPMENT 0)
ENDIF(NOT GDCM_INSTALL_NO_DEVELOPMENT)

IF(NOT GDCM_INSTALL_NO_RUNTIME)
  SET(GDCM_INSTALL_NO_RUNTIME 0)
ENDIF(NOT GDCM_INSTALL_NO_RUNTIME)

IF(NOT GDCM_INSTALL_NO_DOCUMENTATION)
  SET(GDCM_INSTALL_NO_DOCUMENTATION 0)
ENDIF(NOT GDCM_INSTALL_NO_DOCUMENTATION)

SET(GDCM_INSTALL_NO_LIBRARIES)
IF(GDCM_BUILD_SHARED_LIBS)
  IF(GDCM_INSTALL_NO_RUNTIME AND GDCM_INSTALL_NO_DEVELOPMENT)
    SET(GDCM_INSTALL_NO_LIBRARIES 1)
  ENDIF(GDCM_INSTALL_NO_RUNTIME AND GDCM_INSTALL_NO_DEVELOPMENT)
ELSE(GDCM_BUILD_SHARED_LIBS)
  IF(GDCM_INSTALL_NO_DEVELOPMENT)
    SET(GDCM_INSTALL_NO_LIBRARIES 1)
  ENDIF(GDCM_INSTALL_NO_DEVELOPMENT)
ENDIF(GDCM_BUILD_SHARED_LIBS)

#-----------------------------------------------------------------------------
SET (EXECUTABLE_OUTPUT_PATH ${GDCM_BINARY_DIR}/bin CACHE PATH "Single output directory for building all executables.")
SET (LIBRARY_OUTPUT_PATH ${GDCM_BINARY_DIR}/bin    CACHE PATH "Single output directory for building all libraries.")
MARK_AS_ADVANCED(LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)


#-----------------------------------------------------------------------------
# Test if compiler defines the pseudo-macro __FUNCTION__
IF("GDCM_COMPILER_HAS_FUNCTION" MATCHES "^GDCM_COMPILER_HAS_FUNCTION$")
  MESSAGE(STATUS "Checking support for __FUNCTION__ in compiler")
  TRY_COMPILE(GDCM_COMPILER_HAS_FUNCTION
              ${GDCM_BINARY_DIR}/CMakeTmp/Function
              ${GDCM_SOURCE_DIR}/CMake/gdcmTestFUNCTION.cxx
              OUTPUT_VARIABLE OUTPUT)
  IF(GDCM_COMPILER_HAS_FUNCTION)
    MESSAGE(STATUS "Checking support for __FUNCTION__ -- yes")
    SET(GDCM_COMPILER_HAS_FUNCTION 1 CACHE INTERNAL "Support for extension C __FUNCTION__")
    WRITE_FILE(${CMAKE_BINARY_DIR}/CMakeOutput.log
      "Determining if the C compiler supports __FUNCTION__ "
      "passed with the following output:\n"
      "${OUTPUT}\n" APPEND)
  ELSE(GDCM_COMPILER_HAS_FUNCTION)
    MESSAGE(STATUS "Checking support for __FUNCTION__ -- no")
    SET(GDCM_COMPILER_HAS_FUNCTION 0 CACHE INTERNAL "Support for extension C __FUNCTION__")
    WRITE_FILE(${CMAKE_BINARY_DIR}/CMakeError.log
      "Determining if the C compiler supports __FUNCTION__ "
      "failed with the following output:\n"
      "${OUTPUT}\n" APPEND)
  ENDIF(GDCM_COMPILER_HAS_FUNCTION)
ENDIF("GDCM_COMPILER_HAS_FUNCTION" MATCHES "^GDCM_COMPILER_HAS_FUNCTION$")

#-----------------------------------------------------------------------------
# Test is compiler support istringstream
IF("GDCM_TEST_ISTRINGSTREAM" MATCHES "^GDCM_TEST_ISTRINGSTREAM$")
  MESSAGE(STATUS "Checking istringstream")
  TRY_RUN(GDCM_TEST_ISTRINGSTREAM GDCM_TEST_ISTRINGSTREAM_COMPILED
          ${GDCM_BINARY_DIR}/CMakeTmp/
          ${GDCM_SOURCE_DIR}/CMake/gdcmTestIstringStream.cxx)
  IF(GDCM_TEST_ISTRINGSTREAM_COMPILED)
    IF(GDCM_TEST_ISTRINGSTREAM)
      MESSAGE(STATUS "Checking istringstream works -- yes")
      SET(GDCM_TEST_ISTRINGSTREAM 1 CACHE INTERNAL "Whether istringstream works.")
    ELSE(GDCM_TEST_ISTRINGSTREAM)
      MESSAGE(STATUS "Checking istringstream works -- no")
      SET(GDCM_TEST_ISTRINGSTREAM 0 CACHE INTERNAL "Whether istringstream works.")
    ENDIF(GDCM_TEST_ISTRINGSTREAM)
  ELSE(GDCM_TEST_ISTRINGSTREAM_COMPILED)
    MESSAGE(STATUS "Checking istringstream -- failed")
  ENDIF(GDCM_TEST_ISTRINGSTREAM_COMPILED)
ENDIF("GDCM_TEST_ISTRINGSTREAM" MATCHES "^GDCM_TEST_ISTRINGSTREAM$")

#-----------------------------------------------------------------------------
# Build directory on which many applications depend
SUBDIRS(
  Dicts
  src
)
#-----------------------------------------------------------------------------
# Example subdir:

OPTION(BUILD_EXAMPLES "Build GDCM examples." ON)
IF(BUILD_EXAMPLES)
  SUBDIRS(Example)
ENDIF(BUILD_EXAMPLES)

#-----------------------------------------------------------------------------
# Adding GDCM_DATA_ROOT
FIND_PATH(GDCM_DATA_ROOT gdcm-ACR-LibIDO.acr
  ${GDCM_SOURCE_DIR}/../gdcmData 
  $ENV{GDCM_DATA}
  $ENV{PUB_DICT_PATH}/../../gdcmData
)
# Adding GDCM_DATA_ROOT
FIND_PATH(GDCM_DATA_EXTRA_ROOT gdcmData.tar.gz
  ${GDCM_SOURCE_DIR}/../gdcmDataExtra
)

# Set the place for the public dictionary, must be set before gdcmConfigure.h
FIND_PATH(GDCM_PUB_DICT_PATH dicomV3.dic
  ${GDCM_SOURCE_DIR}/Dicts
  $ENV{PUB_DICT_PATH}
)

#-----------------------------------------------------------------------------
# SunOS + old gcc fixes:
INCLUDE (${CMAKE_ROOT}/Modules/CMakeBackwardCompatibilityCXX.cmake)
IF(CMAKE_NO_ANSI_STRING_STREAM)
  SET(GDCM_NO_ANSI_STRING_STREAM 1)
ENDIF(CMAKE_NO_ANSI_STRING_STREAM)

# Big endian thing:
INCLUDE (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
TEST_BIG_ENDIAN(GDCM_WORDS_BIGENDIAN)

INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
CHECK_INCLUDE_FILE("stdint.h"       CMAKE_HAVE_STDINT_H)
IF(UNIX) #Avoid polluting Win32 cmakecache
  CHECK_INCLUDE_FILE("inttypes.h"     CMAKE_HAVE_INTTYPES_H)
ENDIF(UNIX)
# Special Time support, two convention: *NIX vs Win32
CHECK_INCLUDE_FILE("sys/times.h"     CMAKE_HAVE_SYS_TIMES_H)
CHECK_INCLUDE_FILE("sys/timeb.h"     CMAKE_HAVE_SYS_TIMEB_H)

# Check if header file exists and add it to the list.
INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFiles.cmake)
MACRO(CHECK_INCLUDE_FILE_CONCAT FILE VARIABLE)
  CHECK_INCLUDE_FILES("${GDCM_INCLUDES};${FILE}" ${VARIABLE})
  IF(${VARIABLE})
    SET(GDCM_INCLUDES ${GDCM_INCLUDES} ${FILE})
  ENDIF(${VARIABLE})
ENDMACRO(CHECK_INCLUDE_FILE_CONCAT)

# Need those header for GetMacAddress in Util
# This is tricky as you need to figure out the proper order to
# test the inclusion of files
CHECK_INCLUDE_FILE_CONCAT("unistd.h"       CMAKE_HAVE_UNISTD_H)
CHECK_INCLUDE_FILE_CONCAT("stdlib.h"       CMAKE_HAVE_STDLIB_H)
IF(UNIX) #Avoid polluting Win32 cmakecache
  CHECK_INCLUDE_FILE_CONCAT("sys/ioctl.h"    CMAKE_HAVE_SYS_IOCTL_H)
  CHECK_INCLUDE_FILE_CONCAT("sys/socket.h"   CMAKE_HAVE_SYS_SOCKET_H)
  CHECK_INCLUDE_FILE_CONCAT("sys/sockio.h"   CMAKE_HAVE_SYS_SOCKIO_H)
  CHECK_INCLUDE_FILE_CONCAT("net/if.h"       CMAKE_HAVE_NET_IF_H)
  CHECK_INCLUDE_FILE_CONCAT("netinet/in.h"   CMAKE_HAVE_NETINET_IN_H)
  CHECK_INCLUDE_FILE_CONCAT("net/if_dl.h"    CMAKE_HAVE_NET_IF_DL_H)
  CHECK_INCLUDE_FILE_CONCAT("net/if_arp.h"   CMAKE_HAVE_NET_IF_ARP_H)
ENDIF(UNIX)

#-----------------------------------------------------------------------------
# Test if os defines a length for sockaddr
# only doing the test when we have sys/socket
IF( ${CMAKE_HAVE_SYS_SOCKET_H} )
  IF("HAVE_SA_LEN" MATCHES "^HAVE_SA_LEN$")
    STRING(ASCII 35 POUND)
    FILE(WRITE ${GDCM_BINARY_DIR}/CMakeTmp/gdcmTestHAVESALEN.c
      "${POUND}include <sys/types.h>\n"
      "${POUND}include <sys/socket.h>\n"
      "int main() { struct sockaddr sa; sa.sa_len = 0; }\n")
    MESSAGE(STATUS "Checking if ifreq has a sa_len")
    TRY_COMPILE(HAVE_SA_LEN
      ${GDCM_BINARY_DIR}
      ${GDCM_BINARY_DIR}/CMakeTmp/gdcmTestHAVESALEN.c
      OUTPUT_VARIABLE OUTPUT)
    IF(HAVE_SA_LEN)
      MESSAGE(STATUS "Checking if ifreq has a sa_len -- yes")
      SET(HAVE_SA_LEN 1 CACHE INTERNAL "Support if ifreq has a sa_len")
      WRITE_FILE(${CMAKE_BINARY_DIR}/CMakeOutput.log
        "Determining if ifreq has a sa_len"
        "passed with the following output:\n"
        "${OUTPUT}\n" APPEND)
    ELSE(HAVE_SA_LEN)
      MESSAGE(STATUS "Checking if ifreq has a sa_len -- no")
      SET(HAVE_SA_LEN 0 CACHE INTERNAL "Support if ifreq has a sa_len")
      WRITE_FILE(${CMAKE_BINARY_DIR}/CMakeError.log
        "Determining if ifreq has a sa_len"
        "failed with the following output:\n"
        "${OUTPUT}\n" APPEND)
    ENDIF(HAVE_SA_LEN)
  ENDIF("HAVE_SA_LEN" MATCHES "^HAVE_SA_LEN$")
ENDIF( ${CMAKE_HAVE_SYS_SOCKET_H} )

#-----------------------------------------------------------------------------
# Force Big Endian emulation on little endian:
OPTION(GDCM_FORCE_BIGENDIAN_EMULATION "Force Big Endian Emulation. Don't use if you don't know what you are doing." OFF)

CONFIGURE_FILE(${GDCM_SOURCE_DIR}/gdcmConfigure.h.in
               ${GDCM_BINARY_DIR}/gdcmConfigure.h @ONLY IMMEDIATE
               )

IF(NOT GDCM_INSTALL_NO_DEVELOPMENT)
  INSTALL_FILES(${GDCM_INSTALL_INCLUDE_DIR} FILES gdcmConfigure.h)
ENDIF(NOT GDCM_INSTALL_NO_DEVELOPMENT)

#-----------------------------------------------------------------------------
# Add the testing directories
OPTION(BUILD_TESTING "Build testing." ON)

IF(BUILD_TESTING)
  CONFIGURE_FILE(${GDCM_SOURCE_DIR}/CMake/CTestCustom.ctest.in
      ${GDCM_BINARY_DIR}/CMake/CTestCustom.ctest @ONLY)
  FILE(WRITE ${GDCM_BINARY_DIR}/CTestCustom.cmake
    "INCLUDE(\"${GDCM_BINARY_DIR}/CMake/CTestCustom.ctest\")\n")
  ENABLE_TESTING()
  INCLUDE(CTest)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)


#-----------------------------------------------------------------------------
OPTION(GDCM_VTK "Build VTK-GDCM Lib." OFF)
IF(GDCM_VTK)

    set(ParaView_DIR 
      /path/to/paraview/build
      CACHE FILEPATH
      "Path to ParaView build.")
    
    if (NOT EXISTS ${ParaView_DIR})
      MESSAGE( FATAL_ERROR 
      "Set ParaView_DIR to the path to a ParaView build." )
    endif (NOT EXISTS ${ParaView_DIR})
    
    find_package(ParaView REQUIRED)
    include(${PARAVIEW_USE_FILE})
    
    SUBDIRS(vtk)


  # Search VTK
  FIND_PACKAGE(VTK)
  # If vtk found
  #IF(VTK_FOUND)
    #INCLUDE(${VTK_USE_FILE})
  #  SUBDIRS(vtk)
  #ELSEIF(VTK_FOUND)
  #  message(FATAL_ERROR "VTK not found")
  #ENDIF(VTK_FOUND)
ENDIF(GDCM_VTK)

#-----------------------------------------------------------------------------
OPTION(GDCM_WRAP_PYTHON "Wrap GDCM classes into the Python language." OFF)
#if wrapping enable
IF(GDCM_WRAP_PYTHON)
  # Search Python libs:
  FIND_PACKAGE(PythonLibs)
  # If PythonLibs found
  IF(PYTHON_LIBRARY)
    SUBDIRS(gdcmPython)
  ENDIF(PYTHON_LIBRARY)
ENDIF(GDCM_WRAP_PYTHON)

#-----------------------------------------------------------------------------
OPTION(GDCM_DOCUMENTATION "Build source documentation using doxygen." OFF)
# if doxygen enable
IF(GDCM_DOCUMENTATION)
  # Search Doxygen:
  FIND_PACKAGE(Doxygen)
  IF(DOXYGEN)
    SUBDIRS(Doc)
  ENDIF(DOXYGEN)
ENDIF(GDCM_DOCUMENTATION)

#-----------------------------------------------------------------------------
IF(WIN32)
  OPTION(GDCM_INSTALLER "Build the install setup using InnoSetup." OFF)
ENDIF(WIN32)
IF(UNIX)
  OPTION(GDCM_INSTALLER "Prepare the rpmBuild use." OFF)
ENDIF(UNIX)
IF(GDCM_INSTALLER)
  SUBDIRS(Builder)
ENDIF(GDCM_INSTALLER)

# --------------------------------------------------------------------------
# Configure the export configuration

SUBDIRS(CMake)

