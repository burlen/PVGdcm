# JPEG 16 bits project

INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${GDCMJPEG_SOURCE_DIR}")
ADD_LIBRARY(gdcmjpeg16 ${JPEG_SOURCES})

IF(NOT GDCM_INSTALL_NO_LIBRARIES)
  INSTALL_TARGETS(
    ${GDCM_INSTALL_LIB_DIR}
    RUNTIME_DIRECTORY ${GDCM_INSTALL_BIN_DIR}
    gdcmjpeg16)
ENDIF(NOT GDCM_INSTALL_NO_LIBRARIES)

