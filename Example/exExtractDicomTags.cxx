/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: exExtractDicomTags.cxx,v $
  Language:  C++
  Date:      $Date: 2008/04/03 17:00:24 $
  Version:   $Revision: 1.6 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmDocument.h"
#include "gdcmSeqEntry.h"
#include "gdcmSQItem.h"
#include "gdcmDebug.h"
#include "gdcmUtil.h"
#include "gdcmOrientation.h"

#include "gdcmArgMgr.h"
#include <iostream>

//#include <fstream>

#ifndef _WIN32
#include <unistd.h> //for access, unlink
#else
#include <io.h> //for _access
#endif


int main(int argc, char *argv[])
{

   START_USAGE(usage)
   " \n exExtractDicomTags :\n",
   "  ",
   "         ",
   "",
   " usage: exExtractDicomTags filein=inputDicomFileName ", 
   "                      [debug]  ", 
   "        debug    : user wants to run the program in 'debug mode'        ",
   FINISH_USAGE


   // ----- Initialize Arguments Manager ------
   
   GDCM_NAME_SPACE::ArgMgr *am = new GDCM_NAME_SPACE::ArgMgr(argc, argv);

   if (am->ArgMgrDefined("usage")) 
   {
      am->ArgMgrUsage(usage); // Display 'usage'
      delete am;
      return 0;
   }
   char *fileName = am->ArgMgrWantString("filein",usage);
   if ( fileName == NULL )
   {
      delete am;
      return 0;
   }

   if (am->ArgMgrDefined("debug"))
      GDCM_NAME_SPACE::Debug::DebugOn();
 
   // if unused Params we give up
   if ( am->ArgMgrPrintUnusedLabels() )
   { 
      am->ArgMgrUsage(usage);
      delete am;
      return 0;
   }

   delete am;  // we don't need Argument Manager any longer

   // ----------- End Arguments Manager ---------


   int loadMode = 0x0; // load everything
   GDCM_NAME_SPACE::File *f = GDCM_NAME_SPACE::File::New();
   f->SetLoadMode( loadMode );
   f->SetFileName( fileName );
   bool res = f->Load();  

   if ( !res )
   {
      f->Delete();
      return 0;
   }

   // 
   if (!f->IsReadable())
   {
      std::cout << "NOT a Dicom File : " << fileName <<std::endl;
      f->Delete();
      return 1;
   }

   std::string MediaStSOPinstUID;      
   std::string TransferSyntax;
   std::string StudyDate;
   std::string StudyTime;
   std::string Modality;
   std::string PatientName;
   std::string PatientID;
   std::string PatientSex;

   //std::string MediaStSOPinstUID;
   std::string SOPInstanceUID; // == image ID
   std::string StudyInstanceUID;
   std::string SeriesInstanceUID;
   std::string AcquistionDate;
   std::string AcquisitionTime;
   std::string AcquisitionDateTime;
   std::string InstitutionName;
   std::string InstitutionalDepartmentName; // always empty
   std::string ProtocolName;


// ------------> Region (Organ) : *no* DICOM field is expected 
//                        to hold information

// Get informations on the file : 
//  Modality, Transfer Syntax, Study Date, Study Time
// Patient Name, Media Storage SOP Instance UID, etc

   MediaStSOPinstUID   = f->GetEntryString(0x0002,0x0002);
   TransferSyntax      = f->GetEntryString(0x0002,0x0010);
   StudyDate           = f->GetEntryString(0x0008,0x0020);
   StudyTime           = f->GetEntryString(0x0008,0x0030);
   PatientName         = f->GetEntryString(0x0010,0x0010);
   PatientID           = f->GetEntryString(0x0010,0x0020);  //patientid
   PatientSex          = f->GetEntryString(0x0010,0x0040);  //sex
   SOPInstanceUID      = f->GetEntryString(0x0008,0x0018);  //imageid = SOPinsUID
   StudyInstanceUID    = f->GetEntryString(0x0020,0x000d);  //STUInsUID                                              [Study Instance UID] [1.2.840.113680.1.103.56887.1017329008.714317]
   SeriesInstanceUID   = f->GetEntryString(0x0020,0x000e);  //SerInsUID
   AcquistionDate      = f->GetEntryString(0x0008,0x0022);
   AcquisitionTime     = f->GetEntryString(0x0008,0x0032);
   AcquisitionDateTime = f->GetEntryString(0x0008,0x002a);


   Modality            = f->GetEntryString(0x0008,0x0060);  //modality
   InstitutionName     = f->GetEntryString(0x0008,0x0080);  //hospital
   // always empty :-(
   InstitutionalDepartmentName     
                     = f->GetEntryString(0x0008,0x1040);  //departement

   // --> I'll have to ask people working on PACS which one they use. JPRx
   // Radiologist :
   // 0008|0090  [Referring Physician's Name]
   // 0008|1050  [Performing Physician's Name]
   // 0008|1060  [Name of Physician(s) Reading Study] 
   // 0008|1048  [Physician(s) of Record] 
   // 0032|1032  [Requesting Physician]
   
   std::string ReferringPhysiciansName          = f->GetEntryString(0x0008,0x0090);   
   std::string PerformingPhysiciansName         = f->GetEntryString(0x0008,0x1050);
   std::string NameofPhysiciansReadingStudy     = f->GetEntryString(0x0008,0x1060);   
   std::string PhysiciansofRecord               = f->GetEntryString(0x0008,0x1048);
   std::string RequestingPhysician              = f->GetEntryString(0x0032,0x1032);   
        

   ProtocolName = f->GetEntryString(0x0018,0x1030); 
  
   // --> Big trouble with nz (number of planes) and nt (number of 'times')
   // --> that belong to LibIDO, not to DICOM.
   // --> DICOM has 'Number of Frames' (0028|0008), 
   //     that's more or less number of 'times'
   // Volumes are generaly stored in a 'Serie' 
   //  (hope so ... a single Serie may be xti-slice xti-times)


   std::string Rows;
   std::string Columns;
   std::string Planes;

   std::string SamplesPerPixel;
   std::string BitsAllocated;
   std::string BitsStored;
   std::string HighBit;
   std::string PixelRepresentation;
   std::string PixelType;
   
   SamplesPerPixel     = f->GetEntryString(0x0028,0x0002);  // 3 -> RGB
   Rows                = f->GetEntryString(0x0028,0x0010);  //ny
   Columns             = f->GetEntryString(0x0028,0x0011);  //nx
   Planes              = f->GetEntryString(0x0028,0x0012);  //nz

   BitsAllocated       = f->GetEntryString(0x0028,0x0100);
   BitsStored          = f->GetEntryString(0x0028,0x0101);
   HighBit             = f->GetEntryString(0x0028,0x0102);
   PixelRepresentation = f->GetEntryString(0x0028,0x0103);

   PixelType           = f->GetPixelType();                //type 
/*
   int iRows            = (uint32_t) atoi( Rows.c_str() );    //ny
   int iColumns         = (uint32_t) atoi( Columns.c_str() ); //nz
   int iPlanes          = (uint32_t) atoi( Planes.c_str() );  //nz
   int iSamplesPerPixel = (uint32_t) atoi( SamplesPerPixel.c_str() );
   int iBitsAllocated   = (uint32_t) atoi( BitsAllocated.c_str() );
*/



   float sx =  f->GetXSpacing();
   float sy =  f->GetYSpacing();
   float sz =  f->GetZSpacing(); // meaningless for DICOM 
                                 // (cannot be extracted from a single image)

   std::cout << "Rows = ["            << Rows                    << "]" << std::endl;
   std::cout << "Columns = ["         << Columns                 << "]" << std::endl;
   std::cout << "Planes = ["          << Planes                  << "]" << std::endl;
   std::cout << "SamplesPerPixel = [" << SamplesPerPixel         << "]" << std::endl;
   std::cout << "BitsAllocated = ["   << BitsAllocated           << "]" << std::endl;
   std::cout << "BitsStored = ["      << BitsStored              << "]" << std::endl;
   std::cout << "HighBit = ["         << HighBit                 << "]" << std::endl;
   std::cout << "PixelRepresentation = [" << PixelRepresentation << "]" << std::endl;
   std::cout << "PixelType = ["       << PixelType               << "]" << std::endl;

   std::cout << "TransferSyntax = [" << TransferSyntax << "]" << std::endl;
   std::cout << "StudyDate = ["      << StudyDate      << "]" << std::endl;
   std::cout << "StudyTime = ["      << StudyTime      << "]" << std::endl;
   std::cout << "Modality = ["       << Modality       << "]" << std::endl;
   std::cout << "PatientName = ["    << PatientName    << "]" << std::endl;
   std::cout << "PatientID = ["      << PatientID      << "]" << std::endl;
   std::cout << "PatientSex = ["     << PatientSex     << "]" << std::endl;
   std::cout << std::endl;  
   std::cout << "SOPInstanceUID = ["              << SOPInstanceUID << "]" 
          << std::endl; 
   std::cout << "StudyInstanceUID = ["            << StudyInstanceUID 
          << "]" << std::endl;
   std::cout << "SeriesInstanceUID = ["           << SeriesInstanceUID 
          << "]" << std::endl;
   std::cout << "AcquistionDate = ["              << AcquistionDate 
          << "]" << std::endl;
   std::cout << "AcquisitionTime = ["             << AcquisitionTime 
          << "]" << std::endl;
   std::cout << "AcquisitionDateTime = ["         << AcquisitionDateTime << "]" 
             << std::endl;
   
   std::cout << std::endl;
       
   std::cout << "ReferringPhysiciansName = ["             << ReferringPhysiciansName 
          << "]" << std::endl;
   std::cout << "PerformingPhysiciansName = ["            << PerformingPhysiciansName 
          << "]" << std::endl;  
   std::cout << "NameofPhysiciansReadingStudy = ["        << NameofPhysiciansReadingStudy 
          << "]" << std::endl;  
   std::cout << "PhysiciansofRecord = ["                  << PhysiciansofRecord 
          << "]" << std::endl;  
   std::cout << "RequestingPhysician = ["                 << RequestingPhysician 
          << "]" << std::endl; 
   
   std::cout << std::endl;
   
   std::cout << "InstitutionalDepartmentName = [" 
                                        << InstitutionalDepartmentName << "]"
                               << std::endl;
       
   std::cout << "InstitutionName = ["             << InstitutionName 
          << "]" << std::endl;       
       
   std::cout << "ProtocolName = ["                << ProtocolName << "]" << std::endl;

   std::cout << "GetXSpacing = ["            << sx << "]" << std::endl;
   std::cout << "GetYSpacing = ["            << sy << "]" << std::endl;
   std::cout << "GetZSpacing = ["            << sz << "]" << std::endl;


      // Let's get and print some usefull fields about 'Orientation'
      // ------------------------------------------------------------

      std::string strPatientPosition = 
                                      f->GetEntryString(0x0018,0x5100);
      if ( strPatientPosition != GDCM_NAME_SPACE::GDCM_UNFOUND 
        && strPatientPosition != "" )  
            std::cout << "PatientPosition (0x0010,0x5100)= [" 
                      << strPatientPosition << "]" << std::endl;
 
      std::string strViewPosition = 
                                      f->GetEntryString(0x0018,0x5101);
      if ( strViewPosition != GDCM_NAME_SPACE::GDCM_UNFOUND 
        && strViewPosition != "" )  
            std::cout << "View Position (0x0018,0x5101)= [" 
                      << strViewPosition << "]" << std::endl;
      
     std::string strPatientOrientation = 
                                      f->GetEntryString(0x0020,0x0020);
      if ( strPatientOrientation != GDCM_NAME_SPACE::GDCM_UNFOUND
        && strPatientOrientation != "")  
         std::cout << "PatientOrientation (0x0020,0x0020)= [" 
                   << strPatientOrientation << "]" << std::endl;

      std::string strImageOrientationPatient = 
                                      f->GetEntryString(0x0020,0x0037);  
      if ( strImageOrientationPatient != GDCM_NAME_SPACE::GDCM_UNFOUND
        && strImageOrientationPatient != "" )  
         std::cout << "ImageOrientationPatient (0x0020,0x0037)= [" 
                   << strImageOrientationPatient << "]" << std::endl;

      std::string strImageOrientationRET = 
                                      f->GetEntryString(0x0020,0x0035);
      if ( strImageOrientationRET != GDCM_NAME_SPACE::GDCM_UNFOUND
        && strImageOrientationRET != "" )  
         std::cout << "ImageOrientationRET (0x0020,0x0035)= [" 
                   << strImageOrientationRET << "]" << std::endl;

      std::string strImagePositionPatient = 
                                      f->GetEntryString(0x0020,0x0032);  
      if ( strImagePositionPatient != GDCM_NAME_SPACE::GDCM_UNFOUND
        && strImagePositionPatient != "" )  
         std::cout << "ImagePositionPatient (0x0020,0x0032)= [" 
                   << strImagePositionPatient << "]" << std::endl;

      std::string strImagePositionPatientRET = 
                                      f->GetEntryString(0x0020,0x0030);
      if ( strImagePositionPatientRET != GDCM_NAME_SPACE::GDCM_UNFOUND
        && strImagePositionPatientRET != "" )  
         std::cout << "ImagePositionPatientRET (0x0020,0x0030)= [" 
                   << strImagePositionPatientRET << "]" << std::endl;
  
     float iop[6];
     /*bool riop = */f->GetImageOrientationPatient(iop);  
     float ipp[3];
     /*bool ripp = */f->GetImagePositionPatient(ipp);

     std::cout << "Image Position (0x0020,0x0032|0x0030) : "
               << ipp[0] << " , " << ipp[1] << " , "<< ipp[2]
               << std::endl;
     std::cout << "Image Orientation (0x0020,0x0037|0x0035) : "
               << iop[0] << " , " << iop[1] << " , "<< iop[2] << " , "
               << iop[3] << " , " << iop[4] << " , "<< iop[5]
               << std::endl; 


      // Let's compute 'user friendly' results about 'Orientation'
      // ---------------------------------------------------------
 
      GDCM_NAME_SPACE::Orientation *o = GDCM_NAME_SPACE::Orientation::New();

      if ( strImageOrientationPatient != GDCM_NAME_SPACE::GDCM_UNFOUND ||
           strImageOrientationRET     != GDCM_NAME_SPACE::GDCM_UNFOUND )
      {
  
         GDCM_NAME_SPACE::OrientationType orient = o->GetOrientationType( f );
 
         std::cout << "TypeOrientation = " << orient << " (-> " 
                   << o->GetOrientationTypeString(orient) << " )" << std::endl;
      }

      std::string ori = o->GetOrientation ( f );
      if (ori != "\\" )
         std::cout << "Orientation [" << ori << "]" << std::endl;
      o->Delete();
   f->Delete();
   return 0;
}
