 
 /*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: exDicomRTStructSetFile.cxx,v $
  Language:  C++
  Date:      $Date: 2007/06/21 15:06:13 $
  Version:   $Revision: 1.2 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

  /* =================================================
   * freely inspired from : clitkDicomRTIOCommon.cxx
   * written by : Laurent ZAGNI
   * on : 18 August 2006
   * 
   =================================================*/
   
    
#include "gdcmFile.h"
// ----------
//#include "gdcmValEntry.h"
#include "gdcmDataEntry.h"
// ----------
#include "gdcmSeqEntry.h"
#include "gdcmSQItem.h"
#include "gdcmDocEntrySet.h"
#include "gdcmSerieHelper.h"
#include "gdcmDirList.h"
#include "gdcmUtil.h"

// ---------- 
#define GetEntryValue(g,e) GetEntryString(g,e)
#define GetValEntry(g,e)   GetDataEntry(g,e)
#define GetValue()         GetString()
#define ValEntry           DataEntry
#define InsertValEntry(a,b,c) InsertEntryString(a,b,c) // warning mind the VR!
// ----------

#define itkGenericExceptionMacro(a) std::cout<<"===========================Exception "a<<std::endl;return false;

#include "gdcmArgMgr.h" 
 
bool TestDicomRTStructSetFile(GDCM_NAME_SPACE::File* file); 
bool TestDicomCTSerie(const GDCM_NAME_SPACE::FileList serie);

  //====================================================================
  
  //====================================================================
  //In a dicom seq try to find an item with a gived tag/(int)value pair, else creates it
  GDCM_NAME_SPACE::SQItem* GetAnItemWithTagValue(GDCM_NAME_SPACE::SeqEntry* seqEntry, const uint16_t group, 
      const uint16_t elem, const int value, const bool writeItem) {
    int foundValue;
    std::stringstream valueStream;
    valueStream<<value;
    std::string strValue = valueStream.str();
    GDCM_NAME_SPACE::SQItem* curItem = seqEntry->GetFirstSQItem();
    GDCM_NAME_SPACE::ValEntry* valEntry;
    while (curItem != NULL) {
      valEntry = curItem->GetValEntry(group,elem);
      std::istringstream(valEntry->GetValue())>>foundValue;
      if (foundValue == value) {
         return curItem;
      }
      curItem = seqEntry->GetNextSQItem();
    }
    if (writeItem == true) {
      unsigned int newItemNumber = seqEntry->GetNumberOfSQItems ();
// ----------       
      //GDCM_NAME_SPACE::SQItem* newItem = new GDCM_NAME_SPACE::SQItem(seqEntry->GetDepthLevel()+1);
       GDCM_NAME_SPACE::SQItem* newItem = GDCM_NAME_SPACE::SQItem::New(seqEntry->GetDepthLevel()+1);
// ---------- 
      seqEntry->AddSQItem(newItem,(int)newItemNumber);
      newItem->InsertValEntry(strValue, group, elem);  /// \TODO : si VR absent, le chercher dans le dict!
      return newItem;
    } else {
      return NULL;
    }
  }  
  

  /* =================================================
   * freely inspired from : clitkDicomRTIOCommon.cxx
   * written by : Laurent ZAGNI
   * on : 18 August 2006
   * 
   =================================================*/
  // Test if a file list is  a valid CT serie
       
  bool TestDicomCTSerie(const GDCM_NAME_SPACE::FileList serie) {
  
    if (serie.size() < 2) {
      itkGenericExceptionMacro(<<"Serie must contain at least 2 files !");
    }
    
    float firstSliceImagePosition[3];
    float currentSliceImagePosition[3];    
    
    GDCM_NAME_SPACE::FileList::const_iterator first = serie.begin();
    GDCM_NAME_SPACE::FileList::const_iterator it = first ++;
    
    bool res = (*first)->GetImageOrientationPatient(firstSliceImagePosition);
    if (!res) {
       itkGenericExceptionMacro(<<"No ImagePositionPatient found;");
    }
    
    while (it != serie.end()) {
      if (!GDCM_NAME_SPACE::Util::DicomStringEqual((*it)->GetEntryValue(0x0008,0x0016),"1.2.840.10008.5.1.4.1.1.2")) {
           itkGenericExceptionMacro();
           //CT Dicom slices must have a SOP Class UID [0008|0016] = [1.2.840.10008.5.1.4.1.1.2] ==> [CT Image Storage]
      }
      
      if (!GDCM_NAME_SPACE::Util::DicomStringEqual((*it)->GetEntryValue(0x0008,0x0060),"CT")) {
        itkGenericExceptionMacro();
       //CT Dicom slices must have a Modality [0008|0060] = [CT]
      }
      
      if (!GDCM_NAME_SPACE::Util::DicomStringEqual((*it)->GetEntryValue(0x0020,0x0037),"1.0000\\0.0000\\0.0000\\0.0000\\1.0000\\0.0000")) {
         itkGenericExceptionMacro("CT Dicom slices must an Image Orientation [0020|0037]"
         <<" = [1.0000\0.0000\0.0000\0.0000\1.0000\0.0000]");  
      }
      
      res =(*it)->GetImageOrientationPatient(currentSliceImagePosition);
      if (!res) {
         itkGenericExceptionMacro(<<"No ImagePositionPatient found;");
      }
      if ((firstSliceImagePosition[0] != currentSliceImagePosition[0])||(firstSliceImagePosition[1] != currentSliceImagePosition[1])){
         itkGenericExceptionMacro("All CT Dicom slices must have same image position on x and y [0020|0032]");      
      }
      
      if ((*it)->GetEntryValue(0x0028,0x0030) != (*first)->GetEntryValue(0x0028,0x0030).c_str()) {
        itkGenericExceptionMacro("All CT Dicom slices must have same Pixel Spacing [0028|0030]");
      }
      
      if ((*it)->GetEntryValue(0x0028,0x0010) != (*first)->GetEntryValue(0x0028,0x0010).c_str()) {
         itkGenericExceptionMacro("All CT Dicom slices must have same Rows number [0028|0010]");      
      }
      
      if ((*it)->GetEntryValue(0x0028,0x0011) != (*first)->GetEntryValue(0x0028,0x0011).c_str()) {
         itkGenericExceptionMacro("All CT Dicom slices must have same Columns number [0028|0011]");     
      }
      
      if ((*it)->GetEntryValue(0x0018,0x0050) != (*first)->GetEntryValue(0x0018,0x0050).c_str()) {
         itkGenericExceptionMacro("All CT Dicom slices must have same slice thickness [0018|0050]");    
      }
      it++;
    }
    //it = serie.begin();
    //     while (it != serie.end()) {
    //       fileName = (*it)->GetFileName();
    //       std::cout << fileName << std::endl;
    //       it++;
    //     }
  }




// -----------------------------------------------------------------------------------------------------------------
  /* =================================================
   * freely inspired from : clitkDicomRTIOCommon.cxx
   * written by : Laurent ZAGNI
   * on : 18 August 2006
   * 
   =================================================*/
    
 // Test if a file is a valid Dicom-RT Structure-Set file (readable by us) 
  bool TestDicomRTStructSetFile(GDCM_NAME_SPACE::File* file) {
// ----------  
    //GDCM_NAME_SPACE::ValEntry* valEntry;
    GDCM_NAME_SPACE::DataEntry* valEntry;
// ---------- 

    std::string exception0 = "Not a [RT Structure Set Storage]";
    std::string exception1 = "2 or more different 'Roi Referenced Frame UID'!";
    std::string exception2 = "No 'Series Instance UID'!";
    std::string exception3 = "Modality not= RTSTRUCT"; 
            
    //Verify if the file is a RT-Structure-Set dicom file
    if (!GDCM_NAME_SPACE::Util::DicomStringEqual(file->GetEntryValue(0x0008,0x0016),"1.2.840.10008.5.1.4.1.1.481.3")) {  //SOP clas UID
      itkGenericExceptionMacro(<<exception0);
      // (the file must have a SOP Class UID [0008|0016] = 1.2.840.10008.5.1.4.1.1.481.3 ==> [RT Structure Set Storage] !
    }
        
    if (!GDCM_NAME_SPACE::Util::DicomStringEqual(file->GetEntryValue(0x0008,0x0060),"RTSTRUCT")) {  //SOP clas UID
      itkGenericExceptionMacro(<<exception3);
      // (the file must have a Modality tag = RTSTRUCT !
    }    

    //Verify only one Referenced Frame UID and one or more Series UID

    GDCM_NAME_SPACE::SeqEntry* seqEntry;
    GDCM_NAME_SPACE::SQItem* currentItem;
    std::string currentFrameRefUID;
    
    seqEntry  = file->GetSeqEntry(0x3006,0x0020); //Structure Set ROI sequence
    if (seqEntry->GetNumberOfSQItems() > 1) {
      currentItem = seqEntry->GetFirstSQItem();
      valEntry = currentItem->GetValEntry(0x3006,0x0024); //Roi Frame of Reference UID
      currentFrameRefUID = valEntry->GetValue();
      currentItem = seqEntry->GetNextSQItem();
      while (currentItem != NULL) {
        valEntry = currentItem->GetValEntry(0x3006,0x0024); //Roi Frame of Reference UID
        if (valEntry->GetValue() != currentFrameRefUID) {
        // std::cout<<"..ref UID"<<currentFrameRefUID<<std::endl;
        // std::cout<<"..new UID"<<valEntry->GetValue().c_str()<<std::endl;
        // std::cout<<"..exception"<<std::endl;
        itkGenericExceptionMacro(<<exception1);
        }
        currentItem = seqEntry->GetNextSQItem();
      }
    }
    
    seqEntry = file->GetSeqEntry(0x3006,0x0010); //Referenced Frame of Reference Sequence
    currentItem = seqEntry->GetFirstSQItem();
    while (currentItem != NULL) {
      valEntry = currentItem->GetValEntry(0x0020,0x0052);  //Frame of Reference UID
      if (valEntry->GetValue() == currentFrameRefUID) { //search for the ROI Frame of Reference UID
        break;
      }else {
         currentItem = seqEntry->GetNextSQItem();
      }
    }
    if (currentItem == NULL) {
      itkGenericExceptionMacro(<<exception1);
    }
    
    GDCM_NAME_SPACE::SeqEntry* seqEntry2 = currentItem->GetSeqEntry(0x3006,0x0012); //Referenced Study sequence
    if (seqEntry2->GetNumberOfSQItems() < 1) {
      itkGenericExceptionMacro(<<exception2);    
    }
    
    valEntry = file->GetValEntry(0x0020,0x0010); //study ID
    if ((valEntry == 0) || (valEntry->GetValue() == "")) { //(type 1 : mandatory attribute)
      itkGenericExceptionMacro("RT-Structure-Set file must contain Study ID [0020|0010] !");    
    }
    
    valEntry = file->GetValEntry(0x3006,0x002); //structure set label 
    if ((valEntry == 0) || (valEntry->GetValue() == "")) { //(type 1 : mandatory attribute)
      itkGenericExceptionMacro("RT-Structure-Set file must contain Structure Set Label [0006|0002] !");   
    }
    
    seqEntry = file->GetSeqEntry(0x3006,0x0020); //Structure Set ROI sequence
    if (seqEntry != 0) {
      currentItem = seqEntry->GetFirstSQItem();
      while (currentItem != NULL) {
         valEntry = currentItem->GetValEntry(0x3006,0x0022); //Roi number
         if ((valEntry == 0) || (valEntry->GetValue() == "")) { //(type 1 : mandatory attribute)
           itkGenericExceptionMacro("All ROI in Structure Set Roi Sequence must contain Roi Number [3006|0022] !");   
         }
         currentItem = seqEntry->GetNextSQItem();
      }
    }
    
    seqEntry = file->GetSeqEntry(0x3006,0x0080); //ROI Observation sequence
    if (seqEntry == 0) { //(type 1 : mandatory attribute)
      itkGenericExceptionMacro("RT-Structure-Set file must contain ROI Observations Sequence [3006|0080] !");        
    }
    
    seqEntry = file->GetSeqEntry(0x3006,0x0039); //ROI Contour sequence
    if (seqEntry == 0) { //(type 1 : mandatory attribute)
      itkGenericExceptionMacro("RT-Structure-Set file must contain ROI Contour Sequence [3006|0039] !");    
    }
    std::cout<<"..."<<file->GetFileName()<<" is a valid DICOM RT-Structure-Set file"<<std::endl;
  }

// -------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
   START_USAGE(usage)
   "\n exExtractCSA :\n                                                       ",
   "Extracts and displays the CSA tag(s) of gdcm-parsable Dicom file          ",
   "                                                                          ",
   "usage: exExtractCSA {filein=inputFileName|dirin=inputDirectoryName}       ",
   "                   tmp=temporaryWorkFileName                              ",
   "                       [extract=listOfElementsToExtract]                  ",
   "                       [ { [noshadowseq] | [noshadow][noseq] } ] [debug]  ",
   "       inputFileName : Name of the (single) file user wants to anonymize  ",
   "       listOfElementsExtract : group-elem,g2-e2,... (in hexa, no space)   ",
   "                                of Elements to extract                    ",
   "                              default : 0029-1210,0029-1220               ",
   "       noshadowseq: user doesn't want to load Private Sequences           ",
   "       noshadow   : user doesn't want to load Private groups (odd number) ",
   "       noseq      : user doesn't want to load Sequences                   ",
   "       verbose    : developper wants to run the program in 'verbose mode' ",
   "       debug      : developper wants to run the program in 'debug mode'   ",
   FINISH_USAGE

   // ----- Initialize Arguments Manager ------
  
   GDCM_NAME_SPACE::ArgMgr *am = new GDCM_NAME_SPACE::ArgMgr(argc, argv);
  
   if (am->ArgMgrDefined("usage") || argc == 1) 
   {
      am->ArgMgrUsage(usage); // Display 'usage'
      delete am;
      return 0;
   }
   
      if (am->ArgMgrDefined("debug"))
      GDCM_NAME_SPACE::Debug::DebugOn();

   bool verbose = am->ArgMgrDefined("verbose");
   
   const char *fileName = am->ArgMgrGetString("filein");
   
      /* if unused Param we give up */
   if ( am->ArgMgrPrintUnusedLabels() )
   {
      am->ArgMgrUsage(usage);
      delete am;
      return 0;
   }  
   delete am;  // ------ we don't need Arguments Manager any longer ------

// ============================================================
//   Read the input image.
// ============================================================ 

   GDCM_NAME_SPACE::File *f = GDCM_NAME_SPACE::File::New( );

   //f->SetLoadMode(GDCM_NAME_SPACE::LD_NOSEQ | GDCM_NAME_SPACE::LD_NOSHADOW);
   f->SetFileName( fileName );
   f->SetMaxSizeLoadEntry(0xffff);
   bool res = f->Load();  

   if( GDCM_NAME_SPACE::Debug::GetDebugFlag())
   {
      std::cout << "---------------------------------------------" << std::endl;
      f->Print();
      std::cout << "---------------------------------------------" << std::endl;
   }
   if (!res) {
       std::cerr << "Sorry, " << fileName << " not a gdcm-readable "
           << "DICOM / ACR File"
           << std::endl;
      f->Delete();
      return 1;
   }
   std::cout << " ... is readable " << std::endl;
      
   
   TestDicomRTStructSetFile(f);
   std::cout << " ... is readable " << std::endl;   

   
   
   f->Delete();   
return 0;
}      
      
