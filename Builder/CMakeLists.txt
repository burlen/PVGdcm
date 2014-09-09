

IF(WIN32)

IF(NOT GDCM_WRAP_PYTHON)
  MESSAGE(ERROR " INNOSETUP_EXECUTABLE require GDCM_WRAP_PYTHON set to YES")
ENDIF(NOT GDCM_WRAP_PYTHON)
IF(NOT GDCM_VTK)
  MESSAGE(ERROR " INNOSETUP_EXECUTABLE require GDCM_VTK set to YES")
ENDIF(NOT GDCM_VTK)
IF(NOT BUILD_EXAMPLES)
  MESSAGE(ERROR " INNOSETUP_EXECUTABLE require BUILD_EXAMPLES set to YES")
ENDIF(NOT BUILD_EXAMPLES)

  # Search InnoSetup:
  FIND_FILE(INNOSETUP_EXECUTABLE ISCC.exe
    "C:/Program Files/Inno Setup 5"
    DOC "InnoSetup executable"
  )

  IF(EXISTS INNOSETUP_EXECUTABLE)
    MESSAGE(ERROR "INNOSETUP_EXECUTABLE isn't set properly")
  ENDIF(EXISTS INNOSETUP_EXECUTABLE)

  #-----------------------------------------------------------------------------
  # Set new variables to have Windows Paths
  STRING(REGEX REPLACE "/" "\\\\" INNO_SOURCE_DIR ${GDCM_SOURCE_DIR})
  STRING(REGEX REPLACE "/" "\\\\" INNO_BINARY_DIR ${GDCM_BINARY_DIR})
  STRING(REGEX REPLACE "/" "\\\\" INNO_DATA_DIR   ${GDCM_DATA_ROOT})

  #-----------------------------------------------------------------------------
  # Build the installer
  CONFIGURE_FILE(
      ${GDCM_SOURCE_DIR}/Builder/gdcm.iss.in
      ${GDCM_BINARY_DIR}/Builder/gdcm.iss
  )

  ADD_CUSTOM_TARGET(InstallerWin32
    ALL
    ${INNOSETUP_EXECUTABLE} ${GDCM_BINARY_DIR}/Builder/gdcm.iss
  )
  #-----------------------------------------------------------------------------
ENDIF(WIN32)

IF(UNIX)
  #-----------------------------------------------------------------------------
  # Build the installer
  CONFIGURE_FILE(
      ${GDCM_SOURCE_DIR}/Builder/gdcm.spec.in
      ${GDCM_BINARY_DIR}/Builder/gdcm.spec
  )

  STRING(REGEX REPLACE ".*/" "" SPEC_SOURCE_DIR ${GDCM_SOURCE_DIR})
  EXEC_PROGRAM("date +%Y%m%d" OUTPUT_VARIABLE GDCM_DATE)

  EXEC_PROGRAM("tar -zcvf ${GDCM_BINARY_DIR}/Builder/gdcm.tar.gz ${SPEC_SOURCE_DIR}" ${GDCM_SOURCE_DIR}/..)
  #-----------------------------------------------------------------------------
ENDIF(UNIX)
