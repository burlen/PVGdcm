# Python testing directory, 'demo' is not very explicit ...

#-----------------------------------------------------------------------------
# DOH! python is not found by default
FIND_PROGRAM(PYTHON_EXECUTABLE
    NAMES python python2.5 python2.4 python2.3 python2.2 python2.1 python2.0 python1.6 python1.5
    PATHS
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.5\\InstallPath]    
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.4\\InstallPath]
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.3\\InstallPath]
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.2\\InstallPath]
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.1\\InstallPath]
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.0\\InstallPath]
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\1.6\\InstallPath]
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\1.5\\InstallPath]
)
MARK_AS_ADVANCED(PYTHON_EXECUTABLE)

IF(PYTHON_EXECUTABLE)
  ADD_TEST(Python-PrintDict ${PYTHON_EXECUTABLE}
    ${GDCM_BINARY_DIR}/gdcmPython/demo/PrintDict.py
    )
  ADD_TEST(Python-PrintFile ${PYTHON_EXECUTABLE}
    ${GDCM_BINARY_DIR}/gdcmPython/demo/PrintFile.py
    )
  ADD_TEST(Python-PrintDicomDir ${PYTHON_EXECUTABLE}
    ${GDCM_BINARY_DIR}/gdcmPython/demo/PrintDicomDir.py
    )
  ADD_TEST(Python-MakeDicomDir ${PYTHON_EXECUTABLE}
    ${GDCM_BINARY_DIR}/gdcmPython/demo/PrintDicomDir.py
    )
ELSE(PYTHON_EXECUTABLE)
  MESSAGE(FATAL_ERROR "You requested testing for python, but python was not
    found")
ENDIF(PYTHON_EXECUTABLE)

#-----------------------------------------------------------------------------
# Copy all the demo directory content to the cmake bin 
# Without it, tests can't be launched
CONFIGURE_FILE(
    ${GDCM_SOURCE_DIR}/gdcmPython/demo/gdcmConfigDemo.py.in
    ${GDCM_BINARY_DIR}/gdcmPython/demo/gdcmConfigDemo.py
)

CONFIGURE_FILE(
    ${GDCM_SOURCE_DIR}/gdcmPython/demo/PrintFile.py.in
    ${GDCM_BINARY_DIR}/gdcmPython/demo/PrintFile.py
)

CONFIGURE_FILE(
    ${GDCM_SOURCE_DIR}/gdcmPython/demo/PrintDicomDir.py.in
    ${GDCM_BINARY_DIR}/gdcmPython/demo/PrintDicomDir.py
)

CONFIGURE_FILE(
    ${GDCM_SOURCE_DIR}/gdcmPython/demo/MakeDicomDir.py.in
    ${GDCM_BINARY_DIR}/gdcmPython/demo/MakeDicomDir.py
)

CONFIGURE_FILE(
    ${GDCM_SOURCE_DIR}/gdcmPython/demo/PrintDict.py.in
    ${GDCM_BINARY_DIR}/gdcmPython/demo/PrintDict.py
)

INSTALL_FILES(/lib/gdcm/gdcmPython/demo FILES 
   "${GDCM_BINARY_DIR}/gdcmPython/demo/PrintFile.py"
   "${GDCM_BINARY_DIR}/gdcmPython/demo/PrintDicomDir.py"
   "${GDCM_BINARY_DIR}/gdcmPython/demo/PrintDict.py"
)

#-----------------------------------------------------------------------------
# Same but for VTK
IF(GDCM_VTK)
  IF(PYTHON_EXECUTABLE)
    ADD_TEST(Python-VTK-Reader ${PYTHON_EXECUTABLE}
        ${GDCM_BINARY_DIR}/gdcmPython/demo/vtkGdcmReader.py
    )
    ADD_TEST(Python-VTK-Writer ${PYTHON_EXECUTABLE}
        ${GDCM_BINARY_DIR}/gdcmPython/demo/vtkGdcmWriter.py
    )
  ENDIF(PYTHON_EXECUTABLE)

  CONFIGURE_FILE(
      ${GDCM_SOURCE_DIR}/gdcmPython/demo/vtkGdcmReader.py.in
      ${GDCM_BINARY_DIR}/gdcmPython/demo/vtkGdcmReader.py
  )

  CONFIGURE_FILE(
      ${GDCM_SOURCE_DIR}/gdcmPython/demo/vtkGdcmWriter.py.in
      ${GDCM_BINARY_DIR}/gdcmPython/demo/vtkGdcmWriter.py
  )

  INSTALL_FILES(/lib/gdcm/gdcmPython/demo FILES 
     "${GDCM_BINARY_DIR}/gdcmPython/demo/vtkGdcmReader.py"
     "${GDCM_BINARY_DIR}/gdcmPython/demo/vtkGdcmWriter.py"
  )

ENDIF(GDCM_VTK)

#-----------------------------------------------------------------------------

