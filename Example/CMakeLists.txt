# include stuff
INCLUDE_DIRECTORIES(  
  ${GDCM_SOURCE_DIR}/src  
  ${GDCM_BINARY_DIR}  
  ${GDCM_BINARY_DIR}/src)
SET(EXAMPLE_SOURCES
  #names starting with 'ex' are examples
  #-------------------------------------      
  #Txt2Mat  
  #exDicomRTStructSetFile  
  #exExtractCSA
 exPresentationState
 
  exReadPapyrus  
  exReadWriteFile  
  exColorToRGB  
  exGrey2RGB  
  exGC
  exImageLighten  
  #exInLine  
  exExtractOverlaysACR # Extracts ACR-NEMA style overlays 
  exExtractOverlaysDCM # Extracts DICOM style overlays
  exExtractDicomTags
  exExtractTag         
  exCurveData  
  exSerieHelper
  exXCoherentFileSet
  exMoveImagesToSingleSerieUID
  #exExtractSegmentedPalette
  exMakeIcon
  
  # the following are utilities
  #----------------------------
  #BrukerToMhd
  RawToInTagDicom
  RawToDicomStack  
  PrintDicomDir  
  PrintFile  
  MakeDicomDir  
  AnonymizeDicomDir     # without loading it as a gdcm::DicomDir  
  Anonymize             # for full gdcm readable files  
  AnonymizeNoLoad       # without loading the Pixel Data  
  AnonymizeMultiPatient # without loading the Pixel Data  
  AnonymizeReWriteMultiPatient # without loading the Pixel Data  
  PatchHeader  
  ToInTag  
  ReWrite  
  #ReWriteExtended  
  RawToDicom  
  TestValidate
  
 SplitIntoDirectories
 SplitIntoXCoherentDirectories
 SplitIntoXCoherentDirectoriesIgnoreSerieUID 
  
  # the following produce usable files for some softwares.
  #------------------------------------------------------   
  ToMRIregister
  DenseMultiFramesToDicom
  Dense2007ToDicom
  PcpdenseToDicom
    
  #BatchUncompress
    
  #the following will be transformed into 'examples', or 'utilities'
  #              or will be removed
  # Better you don't use them (not fully checked ...)   
  FindTags  
  FlatHashTablePrint  
  Volume2Dicom  
  WriteDicomSimple  

  WriteRead
  #WriteDicomAsJPEG2000  #Use ReWrite ... mode=2
  #WriteDicomAsJPEG      #Use ReWrite ... mode=j

  exCTPET  
  #Slice
)

FOREACH(name ${EXAMPLE_SOURCES})  
  ADD_EXECUTABLE(${name} ${name}.cxx )  
  TARGET_LINK_LIBRARIES(${name} gdcm)  
  INSTALL_TARGETS(/bin/ ${name})
ENDFOREACH(name)
