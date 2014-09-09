/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: TestBuildUpDicomDir.cxx,v $
  Language:  C++
  Date:      $Date: 2008/09/15 15:49:21 $
  Version:   $Revision: 1.13 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmDocEntry.h"
#include "gdcmDicomDir.h"
#include "gdcmDicomDirPatient.h"
#include "gdcmDicomDirStudy.h"
#include "gdcmDicomDirSerie.h"
#include "gdcmDicomDirImage.h"
#include "gdcmDirList.h"
#include "gdcmDebug.h"
#include "gdcmUtil.h"

// ===============================================================

/**
  * \brief   Builds up ex-nihilo a DICOMDIR file 
  *          adding Patient, Study, Serie, Image descriptions
  *          to an empty gdcmDicomDir occurence
  *          and writes a file named NewDICOMDIR. 
  */  
int TestBuildUpDicomDir(int argc, char *argv[])
{
   if (argc) 
   {
      std::cerr << "Usage: " << argv[0] << " dummy " << std::endl;
   }
   
 // GDCM_NAME_SPACE::Debug::DebugOn();
 
   bool errorFound = false; 
   GDCM_NAME_SPACE::DicomDir *dcmdir;
   std::string dirName;  

   dcmdir = GDCM_NAME_SPACE::DicomDir::New();

   GDCM_NAME_SPACE::DicomDirPatient *p1;
   
   // --- Forget these 4 lines :
   // just to improve test coverage.
   p1=dcmdir->GetFirstPatient();
   if (!p1)
      std::cout << "BEFORE any Patient creation, a DicomDir has no Patient. Pffff"
                << std::endl;
   // --- end forget

   // Create patient ONE
   // ------------------
   p1 = dcmdir->NewPatient();
   p1->SetEntryString("patientONE",0x0010, 0x0010);
   // fill here other patient characteristics

   GDCM_NAME_SPACE::DicomDirStudy *s11;
   // --- Forget these 4 lines :
   // just to improve test coverage.
   s11=p1->GetFirstStudy();
   if (!s11)
      std::cout << "BEFORE any Study creation, a Patient has no Study. Pffff"
                << std::endl;
   // --- end forget

   // Let's create and add a Study for this Patient
   s11 = p1->NewStudy();  
   s11->SetEntryString("StudyDescrOne.One_",0x0008, 0x1030);
   // we know entry (0008,1060) is not yet created
   s11->InsertEntryString("Dr^Mabuse",     0x0008, 0x1060, "PN");
   // fill here other Study characteristics

   GDCM_NAME_SPACE::DicomDirStudy *s12 = p1->NewStudy();    
   s12->SetEntryString("StudyDescrOne.Two",0x0008, 0x1030);
   s12->InsertEntryString("Dr^Zorglub",    0x0008, 0x1060, "PN");
   // fill here other Study characteristics

   GDCM_NAME_SPACE::DicomDirStudy *s13 = p1->NewStudy();  
   s13->SetEntryString("StudyDescrOne.Tree",0x0008, 0x1030);
   s13->InsertEntryString("Dr^Follamour",   0x0008, 0x1060, "PN");
   // fill here other Study characteristics
 
   GDCM_NAME_SPACE::DicomDirSerie *s111;
   // --- Forget these 4 lines :
   // just to improve test coverage.
   s111=s11->GetFirstSerie();
   if (!s111)
      std::cout << "BEFORE any Serie creation, a Study has no Serie. Pffff"
                << std::endl;
   // --- end forget

   // Let's create and add a Serie for this Study
   s111 = s11->NewSerie();
   s111->SetEntryString("01-01-111", 0x0008, 0x0021);
   // fill here other Serie characteristics

   GDCM_NAME_SPACE::DicomDirImage *s1111;

   // --- Forget these 4 lines :
   // just to improve test coverage.
   s1111=s111->GetFirstImage();
   if (!s1111)
      std::cout << "BEFORE any Image creation, a Serie has no Image. Pffff"
                << std::endl;
   // --- end forget

   // Let's create and add a Image for this Serie
   s1111 = s111->NewImage();
   s1111->SetEntryString("imageFileName1111",0x0004,0x1500);
   GDCM_NAME_SPACE::DicomDirImage *s1112 = s111->NewImage();
   s1112->SetEntryString("imageFileName1112",0x0004,0x1500);

   // Create patient TWO
   // ------------------
   GDCM_NAME_SPACE::DicomDirPatient *p2 = dcmdir->NewPatient();
   p2->SetEntryString("patientTWO",0x0010, 0x0010); 
   // fill here other patient characteristics
    
   GDCM_NAME_SPACE::DicomDirStudy *s21 = p2->NewStudy();  
   s21->SetEntryString("StudyDescrTwo.One",0x0008, 0x1030);        
   // fill here other Study characteristics

   GDCM_NAME_SPACE::DicomDirSerie *s211 = s21->NewSerie();
   s111->SetEntryString("01-01-211", 0x0008, 0x0021);
   // fill here other Serie characteristics

   GDCM_NAME_SPACE::DicomDirImage *s2111 = s211->NewImage();
   s2111->SetEntryString("imageFileName2111",0x0004,0x1500);
   // fill here other Image characteristics

   GDCM_NAME_SPACE::DicomDirImage *s2112 = s211->NewImage();
   s2112->SetEntryString("imageFileName1122",0x0004,0x1500);
   // fill here other Image characteristics

   // Create patient TREE
   // -------------------
   GDCM_NAME_SPACE::DicomDirPatient *p3 = dcmdir->NewPatient();
   p3->SetEntryString("patientTHREE",0x0010, 0x0010);
   // fill here other Patient characteristics

   // Add a new Serie/Image for a Patient's Study created a long time ago
   // -------------------------------------------------------------------
   GDCM_NAME_SPACE::DicomDirSerie *s131 = s13->NewSerie();
   s111->SetEntryString("01-01-131", 0x0008, 0x0021);
   // fill here other Serie characteristics

   GDCM_NAME_SPACE::DicomDirImage *s1311 = s131->NewImage();
   s1311->SetEntryString("imageFileName1311",0x0004,0x1500);
   // fill here other Image characteristics
     
   // Print
   std::cout << "Test/BuildUpDicomDir: Test Print of patients\n";
   p1->Print();
   std::cout << "Test/BuildUpDicomDir: end of P1-------------------\n";
   p2->Print();
   std::cout << "Test/BuildUpDicomDir: end of P2-------------------\n";
   p3->Print();
   std::cout << "Test/BuildUpDicomDir: end of P3-------------------\n";   
   std::cout << "Test/BuildUpDicomDir: Test Print of patients ended\n";
   
 
 // Let's loop on Patients.  
   GDCM_NAME_SPACE::DicomDirPatient *p;
   p=dcmdir->GetFirstPatient();
   while (p) {
      std::cout << "one more patient\n";
      p->Print();
      p=dcmdir->GetNextPatient();
   }

  
   if( !dcmdir->IsReadable() )
   {
      std::cout<<"          Created DicomDir "
               <<" is not readable"<<std::endl
               <<"          ...Failed"<<std::endl;

      dcmdir->Delete();
      return 1;
   }


   // Print the 'in memory' built up DicomDir
   std::cout << "Test/BuildUpDicomDir: Print all of the DicomDir" << std::endl;
   dcmdir->SetPrintLevel(-1);
   dcmdir->Print();

   // Write it on disc
   dcmdir->Write("NewDICOMDIR");

   dcmdir->Delete();

   // Read the newly written DicomDir
   GDCM_NAME_SPACE::DicomDir *newDicomDir = GDCM_NAME_SPACE::DicomDir::New();
   newDicomDir->SetFileName("NewDICOMDIR");
   newDicomDir->Load( );
   if( !newDicomDir->IsReadable() )
   {
      std::cout<<"          Written DicomDir 'NewDICOMDIR'"
               <<" is not readable"<<std::endl
               <<"          ...Failed"<<std::endl;

      newDicomDir->Delete();
      return 1;
   }
   // Check some value we are sure
   
   int numberOfPatients = newDicomDir->GetNumberOfPatients();
   if (numberOfPatients != 3)
   {
      std::cout<<"  wrong GetNumberOfPatients() : " << newDicomDir->GetNumberOfPatients()
               <<" (should be 3)" <<std::endl;

      newDicomDir->Delete();
      return 1;   
   }
   
   p1 = newDicomDir->GetFirstPatient();
   p2 = newDicomDir->GetNextPatient();
   p3 = newDicomDir->GetNextPatient();

   if (!p1 || !p2 || !p3)
   {
      std::cout << "A patient is missing in written DicomDir"
          << std::endl;
      newDicomDir->Delete();
      return 1;
   }

   std::cout <<std::endl
             << "----------Final Check ---------------------" 
             <<std::endl;
 
   std::string valueStuff;  
   for (;;) // exit on 'break'
   {
      if ( p1->GetEntryString(0x0010, 0x0010) != "patientONE" )
      {
         std::cout << "0x0010,0x0010 [" 
                   << p1->GetEntryString(0x0010, 0x0010)
                   << "]" << std::endl;
         errorFound = true;
         break;
      }
      std::cout << "Patient : [" 
                << p1->GetEntryString(0x0010, 0x0010)
                << "]" << std::endl;

      if ( !(s11 = p1->GetFirstStudy()) )
      {
         std::cout << "missing first Study Patient One" << std::endl;  
         errorFound = true;
         break;
      }
      valueStuff = s11->GetEntryString(0x0008, 0x1030);
      if ( valueStuff.find("StudyDescrOne.One_") >= valueStuff.length() )
      {
         std::cout << "1 : 0x0008,0x1030 [" 
                   << valueStuff
                   << "]" << std::endl;
         errorFound = true;
         break;
      }
      std::cout << "Study : [" 
                << valueStuff
                << "]" << std::endl;
 
      valueStuff = s11->GetEntryString(0x0008, 0x1060);
std::cout << "----------------length-----------------" << valueStuff.length() <<
std::endl;     
      if (!GDCM_NAME_SPACE::Util::DicomStringEqual(valueStuff, "Dr^Mabuse") )
      {
         std::cout << "2 : 0x0008,0x1060 [" 
                   << s11->GetEntryString(0x0008,0x1060)
                   << "]" << std::endl;
         errorFound = true;
         break;
      }
      std::cout << "Physician : [" 
                << valueStuff
                << "]" << std::endl;
      if ( (s12 = p1->GetNextStudy()) == 0 )
      {
         errorFound = true;
         break;
      }
      if ( GDCM_NAME_SPACE::Util::DicomStringEqual(s12->GetEntryString(0x0008,
                                           0x1030),"StudyDescrOne.Two " ))
      {
         std::cout << "3 0x0008,0x1030 [" 
                   << s12->GetEntryString(0x0008,0x1030)
                   << "]" << std::endl;
         errorFound = true;
         break;
      }
      std::cout << "Study Descr : [" 
                << s12->GetEntryString(0x0008,0x1030)
                << "]" << std::endl;

      if ( GDCM_NAME_SPACE::Util::DicomStringEqual(s12->GetEntryString(0x0008,
                                           0x1060),"Dr^Zorglub " ))
      {
         std::cout << "4 0x0008,0x1060 [" 
                   << s12->GetEntryString(0x0008,0x1060)
                   << "]" << std::endl;
         errorFound = true;
         break;
      }
   std::cout << "___________________________________" << std::endl;
      std::cout << "Pysician Reading Study: [" 
                << s12->GetEntryString(0x0008,0x1060)
                << "]" << std::endl;

      if ( (s13 = p1->GetNextStudy()) == 0 )
      {
         std::cout << "Study StudyDescrOne.Tree missing" << std::endl;
         break;
      }         
      if ( s13->GetEntryString(0x0008, 0x1030) != "StudyDescrOne.Tree" )
      {
         errorFound = true;
         break;
      }
      std::cout << "Study : [" 
                << valueStuff
                << "]" << std::endl;

      valueStuff = s13->GetEntryString(0x0008, 0x1060);
      if (!GDCM_NAME_SPACE::Util::DicomStringEqual(valueStuff, "Dr^Follamour") )
      {
         std::cout << "5 0x0008,0x1060 [" 
                   << valueStuff
                   << "]" << std::endl;
         errorFound = true;
         break;
      }
      std::cout << "Pysician : [" 
                << valueStuff
                << "]" << std::endl;

      if ((s111 = s11->GetFirstSerie()) == 0 )
      {
         std::cout << "Serie 01-01-111 missing" << std::endl;
         errorFound = true;
         break;
      }

      valueStuff = s111->GetEntryString(0x0008, 0x0021);
      if (!GDCM_NAME_SPACE::Util::DicomStringEqual(valueStuff, "01-01-131") )
      {
         std::cout << "6 0x0008,0x0021 [" 
                   << valueStuff
                   << "]" << std::endl;
         errorFound = true;
         break;
      }
      std::cout << "Serie : [" 
                << valueStuff
                << "]" << std::endl;

      if ( (s1111 = s111->GetFirstImage()) == 0 )
      {
         std::cout << "missing image S1111" << std::endl;
         errorFound = true;
         break;
      } 

      if ( (s1112 = s111->GetNextImage()) == 0 )
      {
         std::cout << "missing image S1112" << std::endl;
         errorFound = true;
         break;
      }

      break; // No error found. Stop looping
   }

   if ( errorFound )
   {
      std::cout << "MissWritting / MissReading " << std::endl;
   }

   std::cout<<std::flush;
   newDicomDir->Delete();

   return errorFound;
}
