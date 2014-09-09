/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: exExtractCSATag.cxx,v $
  Language:  C++
  Date:      $Date: 2010/08/30 15:23:23 $
  Version:   $Revision: 1.4 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmCommon.h"
#include "gdcmDebug.h"
#include "gdcmDocEntry.h"
#include "gdcmDataEntry.h"
#include "gdcmSeqEntry.h"
#include "gdcmSQItem.h"

#include <iomanip>

//namespace gdcm
//{

   static const char *lookupTable1[] = {
      "UsedPatientWeight",
      "NumberOfPrescans",
      "TransmitterCalibration",
      "PhaseGradientAmplitude",
      "ReadoutGradientAmplitude",
      "SelectionGradientAmplitude",
      "GradientDelayTime",
      "RfWatchdogMask",
      "RfPowerErrorIndicator",
      "SarWholeBody",
      "Sed",
      "SequenceFileOwner",
      "Stim_mon_mode",
      "Operation_mode_flag",
      "dBdt_max",
      "t_puls_max",
      "dBdt_thresh",
      "dBdt_limit",
      "SW_korr_faktor",
      "Stim_max_online",
      "Stim_max_ges_norm_online",
      "Stim_lim",
      "Stim_faktor",
      "CoilForGradient",
      "CoilTuningReflection",
      "CoilId",
      "MiscSequenceParam",
      "MrProtocolVersion",
      "MrProtocol",
      "DataFileName",
      "RepresentativeImage",
      "PositivePCSDirections",
      "RelTablePosition",
      "ReadoutOS",
      "LongModelName",
      "SliceArrayConcatenations",
      "SliceResolution",
      "MrEvaProtocol",
      "AbsTablePosition",
      "AutoAlignMatrix",
      "MeasurementIndex",
      "CoilString",
      "PATModeText",
      "PatReinPattern",
      NULL
   };

   static const char *lookupTable2[] = {
      "EchoLinePosition",
      "EchoColumnPosition",
      "EchoPartitionPosition",
      "UsedChannelMask",
      "Actual3DImaPartNumber",
      "ICE_Dims",
      "B_value",
      "Filter1",
      "Filter2",
      "ProtocolSliceNumber",
      "RealDwellTime",
      "PixelFile",
      "PixelFileName",
      "SliceMeasurementDuration",
      "SequenceMask",
      "AcquisitionMatrixText",
      "MeasuredFourierLines",
      "FlowEncodingDirection",
      "FlowVenc",
      "PhaseEncodingDirectionPositive",
      "NumberOfImagesInMosaic",
      "DiffusionGradientDirection",
      "ImageGroup",
      "SliceNormalVector",
      "DiffusionDirectionality",
      "TimeAfterStart",
      "FlipAngle",
      "SequenceName",
      "RepetitionTime",
      "EchoTime",
      "NumberOfAverages",
      "VoxelThickness",
      "VoxelPhaseFOV",
      "VoxelReadoutFOV",
      "VoxelPositionSag",
      "VoxelPositionCor",
      "VoxelPositionTra",
      "VoxelNormalSag",
      "VoxelNormalCor",
      "VoxelNormalTra",
      "VoxelInPlaneRot",
      "ImagePositionPatient",
      "ImageOrientationPatient",
      "PixelSpacing",
      "SliceLocation",
      "SliceThickness",
      "SpectrumTextRegionLabel",
      "Comp_Algorithm",
      "Comp_Blended",
      "Comp_ManualAdjusted",
      "Comp_AutoParam",
      "Comp_AdjustedParam",
      "Comp_JobID",
      "FMRIStimulInfo",
      "FlowEncodingDirectionString",
      "RepetitionTimeEffective",
    NULL
   };

   static const char *lookupTable3[] = {
      "ImageNumber",
      "ImageComments",
      "ReferencedImageSequence",
      "PatientOrientation",
      "ScanningSequence",
      "SequenceName",
      "RepetitionTime",
      "EchoTime",
      "InversionTime",
      "NumberOfAverages",
      "ImagingFrequency",
      "ImagedNucleus",
      "EchoNumbers",
      "MagneticFieldStrength",
      "NumberOfPhaseEncodingSteps",
      "EchoTrainLength",
      "PercentSampling",
      "PercentPhaseFieldOfView",
      "TriggerTime",
      "ReceivingCoil",
      "TransmittingCoil",
      "AcqusitionMatrix",
      "PhaseEncodingDirection",
      "FlipAngle",
      "VariableFlipAngleFlag",
      "SAR",
      "dBdt",
      "Rows",
      "Columns",
      "SliceThickness",
      "ImagePositionPatient",
      "ImageOrientationPatient",
      "SliceLocation",
      "EchoLinePosition",
      "EchoColumnPosition",
      "EchoPartitionPosition",
      "Actual3DImaPartNumber",
      "RealDwellTime",
      "ProtocolSliceNumber",
      "DataFile",
      "DataFileName",
      "ICE_Dims",
      "PixelSpacing",
      "SourceImageSequence",
      "PixelBandwidth",
      "SliceMeasurementDuration",
      "SequenceMask",
      "AcquisitionMatrixText",
      "MeasuredFourierLines",
      "CsiGridshiftVector",
      NULL
   };

   /*
    * What if SIEMENS decide to add another entry in this table, all the offsets are completely off ...
    * Also we do not respect the ordering anymore.
    * TODO: Need to check if elements always comes in the same order
    */
   unsigned int GetLookupCSAIndex(const char *csa_name, const char **lookuptable = lookupTable1)
      {
     const char **p = lookuptable;
      while( *p && strcmp(*p, csa_name) != 0 )
         {
      ++p;
         }
    assert( strcmp(*p, csa_name) == 0 );
    return p - lookuptable + 1;  // Start at 1 to avoid being on the 0000 position
      }

      // Looks like there is mapping in between syngodt and vr...
      // O <=> UN
      // 3 <=> DS
      // 4 <=> FD
      // 5 <=> FL
      // 6 <=> IS
      // 9 <=> UL
      // 10 <=> US
      // 16 <=> CS
      // 19 <=> LO
      // 20 <=> LT
      // 22 <=> SH
      // 25 <=> UI

struct equ
{ 
   uint32_t syngodt;
   char vr[2+1];
};

static equ mapping[] = {
   {     0 , "UN" },
   {     3 , "DS" },
   {     4 , "FD" },
   {     5 , "FL" },
   {     6 , "IS" },
   {     7 , "SL" },
   {     8 , "SS" },
   {     9 , "UL" },
   {    10 , "US" },
   {    16 , "CS" },
   {    19 , "LO" },
   {    20 , "LT" },
   {    22 , "SH" },
   {    23 , "ST" },
   {    25 , "UI" },
   {    27 , "UT" }
};

bool check_mapping(uint32_t syngodt, const char *vr)
{
  static const unsigned int max = sizeof(mapping) / sizeof(equ);
  unsigned int s = 0;
  const equ *p = mapping;
  assert( syngodt <= mapping[max-1].syngodt );
   while(p->syngodt < syngodt )
      {
    ++p;
      }
  assert( p->syngodt == syngodt ); // or else need to update mapping
  const char* lvr = p->vr;
  int check = strcmp(vr, lvr) == 0;
  assert( check );
  return true;
}

uint32_t readCSAHeader(std::istream &is)
{
   char dummy[4+1];
   dummy[4] = 0;
   is.read(dummy, 4);
   std::cout << dummy << std::endl;
   if( strcmp( dummy, "SV10" )  ) 
      {
      std::cerr << "Either not a SV10 header or filled with 0..." << std::endl;
      return 1;
      }
   // wotsit ?
   is.read(dummy, 4);
   if( strcmp( dummy, "\4\3\2\1" )  ) 
      {
      std::cerr << "Either not a SV10 header or filled with 0..." << std::endl;
      return 1;
      }
   std::cout << dummy << std::endl;
   uint32_t n;
   is.read((char*)&n, sizeof(n));
   std::cout << n << std::endl;
   uint32_t unused;
   is.read((char*)&unused, sizeof(unused));
   std::cout << unused << std::endl;
   assert( unused == 77 ); // 'M' character...

  return n;
}

GDCM_NAME_SPACE::DataEntry *readCSAElement(std::istream &is)
{
      char name[64+1];
      name[64] = 0; // security
      //std::cout << "Pos 0x" << std::hex << is.tellg() << std::dec << std::endl;
      is.read(name, 64);
      std::cout << "Name=" << name << std::endl;
     unsigned int element = GetLookupCSAIndex(name,lookupTable2);
      uint32_t vm;
      is.read((char*)&vm, sizeof(vm));
      std::cout << "vm=" << vm << std::endl;
      char vr[4];
      is.read(vr, 4);
    assert( vr[2] == vr[3] && vr[2] == 0 );
      std::cout << "vr=" << vr << std::endl;

      GDCM_NAME_SPACE::DataEntry *de = GDCM_NAME_SPACE::DataEntry::New(0x0029, element, vr);

      uint32_t syngodt;
      is.read((char*)&syngodt, sizeof(syngodt));
      check_mapping(syngodt, vr);
      
      std::cout << "syngodt=" << syngodt << std::endl;
      uint32_t nitems;
      is.read((char*)&nitems, sizeof(nitems));
      std::cout << "nitems=" << nitems<< std::endl;
      uint32_t xx;
      is.read((char*)&xx, sizeof(xx));
      //std::cout << "xx=" << xx<< std::endl;
      assert( xx == 77 || xx == 205 );
      for( uint32_t j = 0; j < nitems; ++j)
         {
         uint32_t item_xx[4];
         is.read((char*)&item_xx, 4*sizeof(uint32_t));
         std::cout << "item_xx=" << item_xx[0] << " " << item_xx[1] << " " << item_xx[2] << " " << item_xx[3] << std::endl;
         //std::cout << "0x" << std::hex << is.tellg() << std::dec << std::endl;
         assert( item_xx[2] == 77 || item_xx[2] == 205 );
         uint32_t len = item_xx[1]; // 2nd element
         std::cout << "len=" << len << std::endl;
         assert( item_xx[0] == item_xx[1] && item_xx[1] == item_xx[3] );
         char *val = new char[len+1];
         val[len] = 0; // security
         is.read(val,len);
         // WARNING vr does not means anything AFAIK, simply print the value as if it was IS/DS or LO (ASCII)
         std::cout << "val=" << val << std::endl;
      if( !j )
      de->SetString( std::string(val,len) );
         delete[] val;
         char dummy[4];
         uint32_t dummy_len = (4 - len % 4) % 4;
         is.read(dummy, dummy_len );
         for(uint32_t d= 0; d < dummy_len; ++d)
            {
            // I think dummy should always be 0
            if( dummy[d] )
               {
               std::cout << "dummy=" << (int)dummy[d] << std::endl;
               }
            }
         }


  return de;
}

int convertCSA(std::istream &is, GDCM_NAME_SPACE::File *f)
{
   f->RemoveEntry( f->GetDataEntry(0X0029,0x1010) );

    GDCM_NAME_SPACE::SeqEntry *sq = GDCM_NAME_SPACE::SeqEntry::New(0x0029,0x1010);
    GDCM_NAME_SPACE::SQItem *sqi = GDCM_NAME_SPACE::SQItem::New(1);
//    GDCM_NAME_SPACE::DataEntry *e_0008_1150 = DataEntry::New(0x0008, 0x1150, "UI");
//    e_0008_1150->SetString( "coucou" );
//    sqi->AddEntry(e_0008_1150);
//    e_0008_1150->Delete();
//
//    GDCM_NAME_SPACE::DataEntry *e_0008_1155 = DataEntry::New(0x0008, 0x1155, "UI");
//    e_0008_1155->SetString( "mathieu" );
//    sqi->AddEntry(e_0008_1155);
//    e_0008_1155->Delete();



  uint32_t n = readCSAHeader(is);

   for(uint32_t i = 0; i < n; ++i)
      {
    GDCM_NAME_SPACE::DataEntry *de = readCSAElement(is);
    sqi->AddEntry(de);
    de->Delete();
      }

    sq->AddSQItem(sqi,1);
    sqi->Delete();
    //sq->Print( std::cout );
   f->AddEntry( sq );
    sq->Delete();
   f->Print( std::cout );

   GDCM_NAME_SPACE::FileHelper *fh = GDCM_NAME_SPACE::FileHelper::New(f);
         fh->SetWriteTypeToDcmExplVR();
         fh->Write("/tmp/csa2.dcm");

   return 0;
}
//} // end namespace gdcm

int main(int argc, char *argv[])
{  
   GDCM_NAME_SPACE::File *f;
 
   if( argc < 5 )
   {
      std::cerr << "Usage :" << argv[0] << " input.dcm  group element outputfile" << std::endl;
      std::cerr << "  Ex: " << argv[0] << " /tmp/bla.dcm 0029 2110 /tmp/out.raw" << std::endl;
      return 1;
   }
   std::string fileName = argv[1];

   std::cout << fileName << std::endl;
// ============================================================
//   Read the input image.
// ============================================================

   f = GDCM_NAME_SPACE::File::New( );

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

   // Find the dicom tag, and extract the string
   uint16_t group, elem;
   std::istringstream convert;
   convert.str( argv[2] );
   convert >> std::hex >> group;
   convert.clear(); //important
   convert.str( argv[3] );
   convert >> std::hex >> elem;
   std::cout << "Extracting tag: (0x" << std::hex << std::setw(4) << std::setfill('0')
     << group << ",0x" << std::setw(4) << std::setfill('0') << elem << ")" << std::endl;
   std::string dicom_tag_value = f->GetEntryString(group, elem);
   if (dicom_tag_value == GDCM_NAME_SPACE::GDCM_UNFOUND)
   {
     GDCM_NAME_SPACE::DictEntry *dictEntry = f->GetPubDict()->GetEntry( group, elem);
     std::cerr << "Image doesn't contain any tag: " << dictEntry->GetName() << std::endl;
     f->Delete();
     return 1;
   }

   GDCM_NAME_SPACE::DocEntry *dicom_tag_doc = f->GetDocEntry(group, elem);
   GDCM_NAME_SPACE::DataEntry *dicom_tag = dynamic_cast<GDCM_NAME_SPACE::DataEntry *>(dicom_tag_doc);
   if( !dicom_tag )
   {
      std::cerr << "Sorry DataEntry only please" << std::endl;
      f->Delete();
      return 1;
   }

   // Write out the data as a file:
   std::ofstream o(argv[4]);
   if( !o )
   {
      std::cerr << "Problem opening file: " << argv[4] << std::endl;
      f->Delete();
      return 1;
   }
   o.write((char*)dicom_tag->GetBinArea(), dicom_tag->GetLength());
   o.close();

   std::istringstream is;
   is.str( std::string( (char*)dicom_tag->GetBinArea(), dicom_tag->GetLength()) );

   convertCSA(is, f);

   f->Delete();



   return 0;
}


