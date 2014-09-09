# gdcm Python wrapping stuff:

INCLUDE_DIRECTORIES(
  ${GDCM_SOURCE_DIR}/src
  ${GDCM_SOURCE_DIR}/gdcmPython
  ${PYTHON_INCLUDE_PATH}
  ${GDCM_BINARY_DIR}/
)

#-----------------------------------------------------------------------------
# SWIG Wrapping
#

# CMake 2.0 will fully support Swig wrapping so split based on cmake version:

# We need to pass that to swig to be sure all modules are generated in the proper dir
SET(CMAKE_SWIG_FLAGS "-outdir \"${GDCM_BINARY_DIR}/gdcmPython\"")
#SET(CMAKE_SWIG_OUTDIR "${GDCM_BINARY_DIR}/gdcmPython")
# Looks like a bug to me in cmake 2.0.x:
SEPARATE_ARGUMENTS(CMAKE_SWIG_FLAGS)

FIND_PACKAGE(SWIG REQUIRED)
IF(SWIG_FOUND)
  INCLUDE(${SWIG_USE_FILE})
ENDIF(SWIG_FOUND)


SET_SOURCE_FILES_PROPERTIES(gdcm.i PROPERTIES CPLUSPLUS ON)
# BUG: This is only needed with cmake <= 2.0.5
SET_SOURCE_FILES_PROPERTIES(gdcm.i PROPERTIES SWIG_FLAGS "")
# END OF BUG
#FIXME :
#where must we put the keyword SHARED 
#- just like in ADD_LIBRARY(vtkgdcmPython SHARED ${gdcmWrapSOURCES}) -
#to allow use of a static/shared libray ?
# JPRx
SWIG_ADD_MODULE(gdcm python gdcm.i)  #gdcm_wrap.cxx, or gdcmPYTHON_wrap.cxx
SWIG_LINK_LIBRARIES(gdcm gdcm ${PYTHON_LIBRARIES})

# Get rid of -Wall / -W / -pedantic if exist
SET(CMAKE_CXX_FLAGS "")
# I guess I need to put them back on this file:
#SET_SOURCE_FILES_PROPERTIES(
#  ${swig_generated_file_fullname}
#  PROPERTIES COMPILE_FLAGS "")

INSTALL_TARGETS(/lib/gdcm/ _gdcm)

#-----------------------------------------------------------------------------
# VTK Wrapping
# 
IF(GDCM_VTK)
  # Include the VTK library
  INCLUDE(${VTK_USE_FILE})

  INCLUDE_DIRECTORIES(
    ${GDCM_SOURCE_DIR}/vtk
  )

  SET(vtkgdcmPython_la_SOURCES
      ${GDCM_SOURCE_DIR}/vtk/vtkGdcmReader.cxx
      ${GDCM_SOURCE_DIR}/vtk/vtkGdcmWriter.cxx
  )
  SET_SOURCE_FILES_PROPERTIES(vtkGdcmReaderPython.cxx GENERATED)
  
  # Configure Python module, which is the plugin itself
  IF (GDCM_WRAP_PYTHON)
    IF(NOT VTK_WRAP_PYTHON)
      MESSAGE(FATAL_ERROR "Can't build gdcm python wrapping if VTK_WRAP_PYTHON is OFF" )
    ENDIF(NOT VTK_WRAP_PYTHON)
    IF("${VTK_MAJOR_VERSION}" GREATER 4)
      INCLUDE(${VTK_CMAKE_DIR}/vtkWrapPython.cmake)
    ENDIF("${VTK_MAJOR_VERSION}" GREATER 4)
    VTK_WRAP_PYTHON2(vtkgdcmPython gdcmWrapSOURCES
                   ${vtkgdcmPython_la_SOURCES})
    # The C++ files must be made into a C++ library
    ADD_LIBRARY(vtkgdcmPython MODULE ${gdcmWrapSOURCES})  #MODULE on apple ?

    # Python extension modules on Windows must have the extension ".pyd"
    # instead of ".dll" as of Python 2.5.  Older python versions do support
    # this suffix.
    IF(WIN32 AND NOT CYGWIN)
      SET_TARGET_PROPERTIES(vtkgdcmPython PROPERTIES SUFFIX ".pyd")
    ENDIF(WIN32 AND NOT CYGWIN)

    # set the libraries to link against
    #SET_TARGET_PROPERTIES(${foobar} PROPERTIES PREFIX "")

    #GET_TARGET_PROPERTY(swig_gdcm_fullpath "${SWIG_MODULE_gdcm_REAL_NAME}" LOCATION)
    IF("${VTK_MAJOR_VERSION}" LESS 5)
      TARGET_LINK_LIBRARIES (vtkgdcmPython
                             vtkgdcm
                             vtkCommonPython
                             vtkIOPython)
    ELSE("${VTK_MAJOR_VERSION}" LESS 5)
      TARGET_LINK_LIBRARIES (vtkgdcmPython
                             vtkgdcm
                             vtkCommonPythonD
                             vtkIOPythonD)
    ENDIF("${VTK_MAJOR_VERSION}" LESS 5)
    INSTALL_TARGETS(/lib/gdcm/ vtkgdcmPython)

  ENDIF (GDCM_WRAP_PYTHON)
ENDIF(GDCM_VTK)

# generate a setup.py according to VTK installation
# put it in subdir in order to not override old one
#
#GET_FILENAME_COMPONENT(VTKPATH ${VTK_DIR}/../../ ABSOLUTE)
#CONFIGURE_FILE(
#    ${GDCM_SOURCE_DIR}/gdcmPython/setup.py.in
#    ${GDCM_BINARY_DIR}/setup.py
#)

#-----------------------------------------------------------------------------
# Python package configuration
FILE(WRITE ${GDCM_BINARY_DIR}/gdcmPython/gdcmVersion.py
    "gdcmVERSION=\"${GDCM_VERSION}\"\n"
    )

FILE(WRITE ${GDCM_BINARY_DIR}/gdcmPython/__init__.py
    "__all__ = [\"core\",\n"
    "           \"vtk\",\n"
    "          ]\n"
    )

FILE(WRITE ${GDCM_BINARY_DIR}/gdcmPython/core.py
    "from gdcmVersion import *\n"
    "import gdcm\n"
    "GDCM_DATA_ROOT = \"${GDCM_DATA_ROOT}\"\n"
    )

IF(GDCM_VTK)
  FILE(WRITE ${GDCM_BINARY_DIR}/gdcmPython/vtk.py
    "import os\n"
    "from gdcmVersion import *\n"
    "if os.name == 'posix':\n"
    "  from libvtkgdcmPython import *\n"
    "else:\n"
    "  from vtkgdcmPython import *\n"
    "GDCM_DATA_ROOT = \"${GDCM_DATA_ROOT}\"\n"
    )
ENDIF(GDCM_VTK)

#-----------------------------------------------------------------------------
# Python installation
IF(WIN32)
  FILE(WRITE ${GDCM_BINARY_DIR}/gdcmPython/gdcmPython.pth
       "${GDCM_BINARY_DIR}\n"
       "${GDCM_BINARY_DIR}/bin/release\n"
  )
  FILE(WRITE ${GDCM_BINARY_DIR}/gdcmPython/InstallOnly/gdcmPython.pth
       "${GDCM_SOURCE_DIR}\n"
       "${GDCM_SOURCE_DIR}/bin\n"
  )
ELSE(WIN32)
  FILE(WRITE ${GDCM_BINARY_DIR}/gdcmPython/gdcmPython.pth
       "${CMAKE_INSTALL_PREFIX}/lib/gdcm\n"
  )
  FILE(WRITE ${GDCM_BINARY_DIR}/gdcmPython/InstallOnly/gdcmPython.pth
       "/usr/lib/gdcm\n"
  )
ENDIF(WIN32)

# Install a python path file in site-packages that tells
# python where to find the gdcmPython module
IF(UNIX)
  FIND_PATH(PYTHON_DIR "site-packages"
     /usr/lib/python2.5  
     /usr/lib/python2.4
     /usr/lib/python2.3
     /usr/lib/python2.2
     /usr/lib/python2.1
  ) 
  STRING(REGEX REPLACE "/usr" "" PYTHON_DIR ${PYTHON_DIR})

  INSTALL_FILES(${PYTHON_DIR}/site-packages FILES 
    "${GDCM_BINARY_DIR}/gdcmPython/InstallOnly/gdcmPython.pth"
  )
  INSTALL_FILES(/lib/gdcm/gdcmPython FILES 
    "gdcm.py" 
    "__init__.py" 
    "gdcmVersion.py" 
    "core.py" 
    "vtk.py"
  )
ENDIF(UNIX)

#-----------------------------------------------------------------------------
# Add test from demo subdir:

SUBDIRS(demo)
