#-----------------------------------------------------------------------------
# Rebuild gdcm whenever a file starting with gdcm* is modified
INCLUDE_REGULAR_EXPRESSION("^gdcm.*$")

SET(GDCM_LIBRARIES gdcm CACHE INTERNAL "libraries for GDCM")

SET(OPJ_PREFIX gdcm)
SUBDIRS(
  gdcmjpeg
  #gdcmmpeg2
  #gdcmjpegls
  gdcmopenjpeg
  )

# "jpeglib.h" is defined here:
INCLUDE_DIRECTORIES(
  ${GDCM_SOURCE_DIR}/src
  ${GDCM_SOURCE_DIR}/src/gdcmmpeg2/src/mpeg2dec
  ${GDCM_BINARY_DIR} #for gdcmConfigure.h
  ${GDCM_BINARY_DIR}/src #for gdcmjpeg 8, 12 and 16 
  ${GDCM_SOURCE_DIR}/src/gdcmopenjpeg/libopenjpeg
)

SET(GDCM_SOURCES
   gdcmArgMgr.cxx
   gdcmBase.cxx
   gdcmCallbackCommand.cxx
   gdcmCommand.cxx
   gdcmCommandManager.cxx
   gdcmDataEntry.cxx
   gdcmDebug.cxx
   gdcmDicomDir.cxx
   gdcmDicomDirElement.cxx
   gdcmDicomDirImage.cxx
   gdcmDicomDirPrivate.cxx   
   gdcmDicomDirMeta.cxx
   gdcmDicomDirObject.cxx
   gdcmDicomDirPatient.cxx
   gdcmDicomDirVisit.cxx
   gdcmDicomDirSerie.cxx
   gdcmDicomDirStudy.cxx
   gdcmDict.cxx
   gdcmDictEntry.cxx
   gdcmDictGroupName.cxx
   gdcmDictSet.cxx
   gdcmDirList.cxx
   gdcmDocEntry.cxx
   gdcmDocEntryArchive.cxx
   gdcmDocEntrySet.cxx
   gdcmDocument.cxx
   gdcmElementSet.cxx
   gdcmException.cxx
   gdcmFile.cxx
   gdcmFileHelper.cxx
   gdcmGlobal.cxx
   gdcmJPEGFragment.cxx
   gdcmJPEGFragmentsInfo.cxx
   gdcmJpeg8.cxx
   gdcmJpeg12.cxx
   gdcmJpeg16.cxx
   gdcmJpeg2000.cxx
   #gdcmMpeg.cxx
   gdcmOrientation.cxx
   gdcmPixelReadConvert.cxx
   gdcmPixelWriteConvert.cxx
   gdcmRefCounter.cxx
   gdcmRLEFrame.cxx
   gdcmRLEFramesInfo.cxx
   gdcmSeqEntry.cxx
   gdcmSerieHelper.cxx
   gdcmSQItem.cxx
   gdcmTS.cxx
   gdcmUtil.cxx
   gdcmValidator.cxx
   gdcmVR.cxx
   ${GDCM_BINARY_DIR}/src/gdcmDefaultDicts.cxx
   )

# Since OpenJPEG does not used configured headers we have to 
# do the -D alternative
IF(NOT BUILD_SHARED_LIBS)
 SET_SOURCE_FILES_PROPERTIES(
   ${GDCM_SOURCE_DIR}/src/gdcmJpeg2000.cxx
   COMPILE_FLAGS -DOPJ_STATIC)
ENDIF(NOT BUILD_SHARED_LIBS)

ADD_LIBRARY(gdcm ${GDCM_SOURCES})
TARGET_LINK_LIBRARIES(gdcm 
  gdcmjpeg8 
  gdcmjpeg12
  gdcmjpeg16
  #gdcmmpeg2
  gdcmopenjpeg
)
IF(WIN32)
  IF(NOT BORLAND)
    TARGET_LINK_LIBRARIES(gdcm wsock32 snmpapi)
  ENDIF(NOT BORLAND)
ENDIF(WIN32)

# Need nsl to resolve gethostbyname on SunOS-5.8
# and socket also
IF(CMAKE_SYSTEM MATCHES "SunOS.*")
  TARGET_LINK_LIBRARIES(gdcm socket nsl)
ENDIF(CMAKE_SYSTEM MATCHES "SunOS.*")

IF(NOT GDCM_INSTALL_NO_DEVELOPMENT)
  # Install header file used to construct the lib
  #INSTALL_FILES(
  #  ${GDCM_INSTALL_INCLUDE_DIR} .h ${GDCM_SOURCES})
  INSTALL_FILES(${GDCM_INSTALL_INCLUDE_DIR} "\\.h$")
ENDIF(NOT GDCM_INSTALL_NO_DEVELOPMENT)

IF(NOT GDCM_INSTALL_NO_LIBRARIES)
  #Install the target itself
  INSTALL_TARGETS(
    ${GDCM_INSTALL_LIB_DIR}
    RUNTIME_DIRECTORY ${GDCM_INSTALL_BIN_DIR}
    ${GDCM_LIBRARIES})
ENDIF(NOT GDCM_INSTALL_NO_LIBRARIES)

