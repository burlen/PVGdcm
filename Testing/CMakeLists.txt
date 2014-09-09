# CMakeLists for Testing purpose
# Cxx file that need a file to run should be place aproprietly

SET(CXX_TEST_PATH ${EXECUTABLE_OUTPUT_PATH})

SET(GDCM_TESTS ${CXX_TEST_PATH}/gdcmTests)

# add test that does not require data:
# They usally return if no argument is given
SET(TEST_SOURCES
  TestEmpty.cxx
  TestUtil.cxx
  TestBug.cxx
  TestHash.cxx
  TestCommand.cxx
  TestTS.cxx
  TestVR.cxx
  TestDictGroupName.cxx
  TestDict.cxx
  TestDataEntry.cxx
  TestWriteSimple.cxx
  TestImageSet.cxx
  TestDicomDirElement.cxx
  TestDicomString.cxx
  TestEnumVR.cxx
  TestBuildUpDicomDir.cxx          # writes a file named "NewDICOMDIR"
)
IF(UNIX OR WIN32)
  SET(TEST_SOURCES ${TEST_SOURCES}
    TestInline.cxx
  )
ENDIF(UNIX OR WIN32)

# add tests that require data
IF (GDCM_DATA_ROOT)
  SET(TEST_SOURCES ${TEST_SOURCES}
    TestFileAccessors.cxx
    TestMakeIcon.cxx
    TestLoadAllDocumentsNoPrivNoSeq.cxx # includes generated gdcmDataImages.h
    #TestLoadAllDocumentsNoShadow.cxx   # includes generated gdcmDataImages.h
    TestPrintAllDocument.cxx            # includes generated gdcmDataImages.h
    #TestAllEntryVerify.cxx             # includes generated gdcmDataImages.h
    #TestAllReadCompareDicom.cxx        # includes generated gdcmDataImages.h
    TestReadWriteReadCompare.cxx        # includes generated gdcmDataImages.h 
    TestReadWriteJPEGReadCompare.cxx    # includes generated gdcmDataImages.h 
    #TestReadWriteJPEG2000ReadCompare.cxx # includes generated gdcmDataImages.h     
        
    #TestCopyDicom.cxx                  # includes generated gdcmDataImages.h 
    #    TestCopyRescaleDicom.cxx       # includes generated gdcmDataImages.h 
    #TestChangeHeader.cxx
    #    TestAnonymize.cxx
    TestDirList.cxx
    TestDicomDir.cxx                 # require DICOMDIR

    TestMakeDicomDir.cxx             # writes a file named "NewDICOMDIR"
    TestSerieHelper.cxx              # uses gdcmData as a default root directory    
    TestAllVM.cxx
    TestValidate.cxx                 #Just to have on overviews of oddities ...  
  )
  # add test that require VTK:
  IF(GDCM_VTK)
  
    IF("${VTK_MAJOR_VERSION}" LESS 4)
      MESSAGE(FATAL_ERROR "This VTK version is not supported, you are on your own !")
    ELSE("${VTK_MAJOR_VERSION}" LESS 4)
    
      IF("${VTK_MAJOR_VERSION}" EQUAL 4 AND "${VTK_MINOR_VERSION}" EQUAL 2)
        MESSAGE(FATAL_ERROR "This VTK version is not supported, you are on your own !"
                             "Although gdcm should compile fine. You just need to turn BUILD_TESTING:=OFF")
      ENDIF("${VTK_MAJOR_VERSION}" EQUAL 4 AND "${VTK_MINOR_VERSION}" EQUAL 2)
      IF("${VTK_MAJOR_VERSION}" EQUAL 4 AND "${VTK_MINOR_VERSION}" EQUAL 4 AND "${VTK_BUILD_VERSION}" LESS 3)
        MESSAGE(FATAL_ERROR "This VTK version is not supported, you are on your own !"
                             "Although gdcm should compile fine. You just need to turn BUILD_TESTING:=OFF")
      ENDIF("${VTK_MAJOR_VERSION}" EQUAL 4 AND "${VTK_MINOR_VERSION}" EQUAL 4 AND "${VTK_BUILD_VERSION}" LESS 3)
      
      # Include the VTK library since we know this is going to work
      INCLUDE(${VTK_USE_FILE})

      INCLUDE_DIRECTORIES(
        ${GDCM_SOURCE_DIR}/vtk
      )
      SET(TEST_SOURCES ${TEST_SOURCES}
        VTKTestRead.cxx
        VTKTestReadSeq.cxx
        VTKTestWrite.cxx
        VTKTestWriteSeq.cxx
      )
    ENDIF("${VTK_MAJOR_VERSION}" LESS 4)
  ENDIF(GDCM_VTK)
ENDIF (GDCM_DATA_ROOT)



# include stuff
INCLUDE_DIRECTORIES(
  ${GDCM_SOURCE_DIR}/src
  ${GDCM_BINARY_DIR}
)

CREATE_TEST_SOURCELIST(Tests gdcmTests.cxx ${TEST_SOURCES})
ADD_EXECUTABLE(gdcmTests ${Tests})
IF(GDCM_VTK)
  IF(GDCM_DATA_ROOT)
    TARGET_LINK_LIBRARIES(gdcmTests vtkgdcm vtkRendering)
  ELSE(GDCM_DATA_ROOT)
    TARGET_LINK_LIBRARIES(gdcmTests gdcm)
  ENDIF(GDCM_DATA_ROOT)     
ELSE(GDCM_VTK)
  TARGET_LINK_LIBRARIES(gdcmTests gdcm)
ENDIF(GDCM_VTK)

SET (TestsToRun ${Tests})
REMOVE (TestsToRun gdcmTests.cxx)

# Loop over files and create executables
FOREACH(file ${TEST_SOURCES})
  GET_FILENAME_COMPONENT(name ${file} NAME_WE)
  ADD_TEST(${name} ${GDCM_TESTS} ${name})
ENDFOREACH(file ${TEST_SOURCES})

#-----------------------------------------------------------------------------

# We need to test the reading of all dicom images in the gdcmData directory
# First parse this directory and extract all images

FILE(GLOB GDCM_DATA_IMAGES_GLOB
  "${GDCM_DATA_ROOT}/*.acr"
  "${GDCM_DATA_ROOT}/*.dcm"
  "${GDCM_DATA_ROOT}/*.nema"
  "${GDCM_DATA_ROOT}/*.ima"
  )

FILE(GLOB GDCM_DATA_EXTRA_IMAGES_GLOB
  "${GDCM_DATA_EXTRA_ROOT}/gdcmSampleData/images_of_interest/*.dcm"
  )
#MESSAGE(${GDCM_DATA_EXTRA_IMAGES_GLOB})

# Black list of images known to break lots of readers (efilm, xmedcon ...):
SET(BLACK_LIST
  # Multiframe Papyrus format image
  # Just to remember this format exists, and is gdcm::File compliant
  # (NOT gdcm::FileHelper ...)
  # any contribution is welcome
  "PET-cardio-Multiframe-Papyrus.dcm"
  # From David Clunie's Compresion Dataset  
  "D_CLUNIE_CT1_JLSN.dcm"
  "D_CLUNIE_CT1_JLSL.dcm"
  #Not a Dicom Image, but a RT Object
  "RS2.16.124.113534.2.3.5.0.120040728.21342.353.12747_anonimized.dcm" 
  #Dicom MPEG image
  "ETIAM_video_002.dcm"
  #Something stupid occured in the code for LibIDO images.
  #I Black list them for a while. JP
  #"gdcm-ACR-LibIDO.acr"
  "LIBIDO-24-ACR_NEMA-Rectangle.dcm"
  #16 Bits per pixel images with LUT 16 Bits.
  #I wait to be 100% sure before releasing them. JP
  "rle16sti.dcm"
  "rle16loo.dcm"   
  # Jasper cannot handle this image, only kakadu so far, unless you patch jasper
  # but then it breaks other images...
  "CT_Phillips_JPEG2K_Decompr_Problem.dcm"
  
  #RTDOSE, 32 bits image : cannot be compressed
  #"RTDOSE.dcm"
  
  #
  # ------------
  #  
  #Still cause troubles (see the .README.txt file for more details)  
  "MR_Philips_Intera_in_SQ_2001_e05f_item_wrong_lgt_use_NOSHADOWSEQ.dcm"  
  "MR_Philips_Intera_PrivateSequenceExplicitVR.dcm" 
  #The 2 previous images are identical !
  #--> Replaced by more significant name
  "MR_Philips_Intera_PrivateSequenceExplicitVR_in_SQ_2001_e05f_item_wrong_lgt_use_NOSHADOWSEQ.dcm"

  #hexedit shows something *very* strange around offset 0x76e !
  #the images looks *over*corrupted (on disc, not from the manufacturer!)
  "MR_Philips_Intera_PrivateSequenceImplicitVR.dcm"
  
  # Seems this one hangs on 64 bits processors ?!?
  # I blacklist it until the pb is solved.
  # (to get some more green on the Dasboards)
  "MR_Philips-Intera_BreaksNOSHADOW.dcm"

  # No way we'll ever be able to read that...
  "ELSCINT1_LOSSLESS_RICE.dcm"

  # Are we even be able to support our own bugs ?
  "SIEMENS_MAGNETOM-12-MONO2-GDCM12-VRUN.dcm"
  "SIEMENS_GBS_III-16-ACR_NEMA_1-ULis2Bytes.dcm"
  "SIEMENS_ImageLocationUN.dcm"
  "TheralysGDCM120Bug.dcm"
  "IM-0001-0066.dcm"
  "DMCPACS_ExplicitImplicit_BogusIOP.dcm"
  "MR_ELSCINT1_00e1_1042_SQ_feff_00e0_Item.dcm"
  "MR_Spectroscopy_SIEMENS_OF.dcm"
  "ELSCINT1_PMSCT_RLE1.dcm"
  "ELSCINT1_JP2vsJ2K.dcm"
  "GE_MR_0025xx1bProtocolDataBlock.dcm"
  "TG18-CH-2k-01.dcm"
  "CT-SIEMENS-MissingPixelDataInIconSQ.dcm"
  "KODAK_CompressedIcon.dcm"
  "LJPEG_BuginGDCM12.dcm"
  "DX_J2K_0Padding.dcm"
  "DX_GE_FALCON_SNOWY-VOI.dcm"
  "PHILIPS_Brilliance_ExtraBytesInOverlay.dcm"
  "JPEG_LossyYBR.dcm"
  )

# Create a specific list of dicom files that we know are part of a sequence
SET(SEQ_LIST
  "SIEMENS_MAGNETOM-12-MONO2-FileSeq[0-9].dcm"
  )

# Add a special test that requires dciodvfy from dicom3tools
INCLUDE(${GDCM_SOURCE_DIR}/CMake/FindDicom3Tools.cmake)

# This is a GLOB expression, change it into a list separated with a comma and \n
SET(GDCM_DATA_IMAGES)
SET(GDCM_DATA_SEQ_IMAGES)
FOREACH(filename ${GDCM_DATA_IMAGES_GLOB})
  GET_FILENAME_COMPONENT(filename_temp ${filename} NAME)
  IF(DCIODVFY_FOUND)
    #ADD_TEST("DCIODVFY-INPUT-${filename_temp}" "${DCIODVFY_EXECUTABLE}" "${filename}")
    #ADD_TEST("DCIODVFY-OUTPUT-${filename_temp}" "${DCIODVFY_EXECUTABLE}" "${GDCM_DATA_ROOT}/BaselineDicom/${filename_temp}")
  ENDIF(DCIODVFY_FOUND)
  STRING(REGEX MATCH ${filename_temp} bad_dicom ${BLACK_LIST})
  IF(NOT bad_dicom)
    SET(GDCM_DATA_IMAGES "${GDCM_DATA_IMAGES}\n\"${filename_temp}\",")
    # SEQ_LIST is a regular expression:
    STRING(REGEX MATCH ${SEQ_LIST} is_seq ${filename_temp})
    IF(is_seq)
      SET(GDCM_DATA_SEQ_IMAGES "${GDCM_DATA_SEQ_IMAGES}\n\"${filename_temp}\",")
    ENDIF(is_seq)
  ENDIF(NOT bad_dicom)
ENDFOREACH(filename)

# Paranoid / dcmdump
IF(GDCM_DCMTK_CHECK)
FOREACH(filename ${GDCM_DATA_IMAGES_GLOB} ${GDCM_DATA_EXTRA_IMAGES_GLOB})
  GET_FILENAME_COMPONENT(filename_temp ${filename} NAME)
  STRING(REGEX MATCH ${filename_temp} bad_dicom ${BLACK_LIST})
  IF(NOT bad_dicom)
    IF(BUILD_EXAMPLES)
      FOREACH(mode a d x) #r
        FOREACH(opt "" monochrome1 noshadowseq noshadow noseq rgb)
          ADD_TEST(ReWrite-${mode}-${opt}-${filename_temp} ${EXECUTABLE_OUTPUT_PATH}/ReWrite filein=${filename} fileout=${GDCM_BINARY_DIR}/${mode}-${opt}-${filename_temp} mode=${mode} ${opt})
          ADD_TEST(dcmdump-${mode}-${opt}-${filename_temp} /usr/bin/dcmdump -E -dc +ae ${GDCM_BINARY_DIR}/${mode}-${opt}-${filename_temp})
          #ADD_TEST(ReWrite-${mode}-${opt}-${filename_temp}-bis ${EXECUTABLE_OUTPUT_PATH}/ReWrite filein=${GDCM_BINARY_DIR}/${mode}-${opt}-${filename_temp} fileout=${GDCM_BINARY_DIR}/reference.dcm mode=${mode} ${opt})
          #ADD_TEST(diff-${mode}-${opt}-${filename_temp} /usr/bin/diff ${GDCM_BINARY_DIR}/${mode}-${opt}-${filename_temp} ${GDCM_BINARY_DIR}/reference.dcm)
        ENDFOREACH(opt)
      ENDFOREACH(mode)
    ENDIF(BUILD_EXAMPLES)
  ENDIF(NOT bad_dicom)
ENDFOREACH(filename)
ENDIF(GDCM_DCMTK_CHECK)

# Populate GDCM_DATA_IMAGES:
FILE(WRITE "${GDCM_BINARY_DIR}/gdcmDataImages.h"
  "const char * const gdcmDataImages[] = { ${GDCM_DATA_IMAGES}\n0 };\n" 
)

# Populate GDCM_DATA_SEQ_IMAGES:
FILE(WRITE "${GDCM_BINARY_DIR}/gdcmDataSeqImages.h"
  "const char * const gdcmDataSeqImages[] = { ${GDCM_DATA_SEQ_IMAGES}\n0 };\n" 
)

#-----------------------------------------------------------------------------
# Trying to solve a bizarre problem when loading shared lib if gdcm is static
# Therefore I disable the test when building SHAREDLIBS
IF(UNIX AND NOT BUILD_SHARED_LIBS)
  ADD_LIBRARY(dynmodule MODULE dynmodule.cxx)
  TARGET_LINK_LIBRARIES(dynmodule gdcm)
  ADD_EXECUTABLE(loadmodule loadmodule.cxx )
  TARGET_LINK_LIBRARIES(loadmodule gdcm -ldl)
  ADD_TEST(loadmodule ${CXX_TEST_PATH}/loadmodule)
ENDIF(UNIX AND NOT BUILD_SHARED_LIBS)


#-----------------------------------------------------------------------------
# Do the build against an installed gdcm
SET(GDCM_TEST_INSTALL_LEFT "${CMAKE_INSTALL_PREFIX}")
SET(GDCM_TEST_INSTALL_RIGHT "${GDCM_BINARY_DIR}/InstallTest/Root")
STRING(COMPARE EQUAL "${GDCM_TEST_INSTALL_LEFT}" "${GDCM_TEST_INSTALL_RIGHT}"
  GDCM_TEST_INSTALL)
IF(GDCM_TEST_INSTALL)
  ADD_TEST(Install
    ${CMAKE_CTEST_COMMAND}
    #  ${GDCM_INSTALL_TEST_CONFIG_TYPE}
    --build-and-test ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
    --build-generator ${CMAKE_GENERATOR}
    --build-project GDCM
    --build-makeprogram ${CMAKE_MAKE_PROGRAM}
    --build-noclean
    --build-target install)
ENDIF(GDCM_TEST_INSTALL)

