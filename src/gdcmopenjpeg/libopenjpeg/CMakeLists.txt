INCLUDE_REGULAR_EXPRESSION("^.*$")
# Defines the source code for the library
SET(OpenJPEG_SRCS
  bio.c
  cio.c
  dwt.c
  event.c
  image.c
  j2k.c
  j2k_lib.c
  jp2.c
  jpt.c
  mct.c
  mqc.c
  openjpeg.c
  pi.c
  raw.c
  t1.c
  t2.c
  tcd.c
  tgt.c
)

# Pass proper definition to preprocessor to generate shared lib
IF (WIN32)
  IF (BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DOPJ_EXPORTS)
  ELSE (BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DOPJ_STATIC)
  ENDIF (BUILD_SHARED_LIBS)
ENDIF (WIN32)

# Create the library
ADD_LIBRARY(${OPJ_PREFIX}openjpeg ${OpenJPEG_SRCS})

# Install library
IF(NOT GDCM_INSTALL_NO_LIBRARIES)
  INSTALL_TARGETS(
    ${GDCM_INSTALL_LIB_DIR}
    RUNTIME_DIRECTORY ${GDCM_INSTALL_BIN_DIR}
    ${OPJ_PREFIX}openjpeg)
ENDIF(NOT GDCM_INSTALL_NO_LIBRARIES)

