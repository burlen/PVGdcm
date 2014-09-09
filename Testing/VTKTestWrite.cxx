/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: VTKTestWrite.cxx,v $
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
#include "vtkGdcmReader.h"
#include "vtkGdcmWriter.h"
#include "vtkImageViewer.h"
#include "vtkImageData.h"
#include "vtkRegressionTestImage.h"
#include "vtkImageClip.h"
#include "vtkImageTranslateExtent.h"
#include "vtkImageAppendComponents.h"
#include "vtkImageShiftScale.h"

#include <iostream>
#include <sstream>

//Generated file:
#include "gdcmDataImages.h"

#include "gdcmDebug.h"

#ifndef vtkFloatingPointType
#define vtkFloatingPointType float
#endif

int VTKWriteTest(vtkTesting *t,vtkImageViewer *viewer,
                 std::string const &filename, 
                 std::string const &referenceFileName)
{
   int retVal;

   t->CleanArguments();
   t->AddArgument("-D");
   t->AddArgument( GDCM_DATA_ROOT );
   t->AddArgument("-V");
   t->AddArgument( referenceFileName.c_str() );
   t->AddArgument("-T");
   t->AddArgument( "." );

   // Read the image   
   vtkGdcmReader *origin = vtkGdcmReader::New();
   origin->SetFileName( filename.c_str() );
   origin->Update();
  
   std::cout << "1 ...";
   
   // Write the image
   vtkGdcmWriter *writer = vtkGdcmWriter::New();
   writer->SetFileName( "TestWrite.dcm" );
   writer->SetInput(origin->GetOutput());
   std::cout << "2' ...",
   writer->Write();

   std::cout << "2'' ...";
   origin->Delete();
   writer->Delete();

   // Read the written image   
   vtkGdcmReader *reader = vtkGdcmReader::New();
   reader->SetFileName( "TestWrite.dcm" );
   reader->Update();

   std::cout << "3 ...";
   double range[2];
   reader->GetOutput()->GetScalarRange(range);
   int dim[3];
   reader->GetOutput()->GetDimensions( dim );

   // Show
   if( viewer )
   {
      viewer->SetInput ( reader->GetOutput() );

      viewer->SetColorWindow (range[1] - range[0]);
      viewer->SetColorLevel (0.5 * (range[1] + range[0]));

      viewer->SetSize(dim[0], dim[1]);
      if(dim[2] != 1)
      {
         //For multiframe dicom, take a snapshot of the center slice (+/- 1)
         viewer->SetZSlice( dim[2] / 2 );
      }
      else
      {
         viewer->SetZSlice( 0 );
      }

      viewer->OffScreenRenderingOff();
      viewer->Render();
      viewer->SetInput(NULL);
   }

   std::cout << std::endl;
   //----------------------------------------------------------------------
   // Transform the image to be RGB unsigned char, due to the requests in
   // vtkTesting processing
   // The pipeline is broken after each process to keep maximum of memory
   vtkImageData *image=reader->GetOutput();
   image->Update();
   image->Register(NULL);
   reader->Delete();

   // Get the middle slice in the image
   if(dim[2] != 1)
   {
      int *ext=image->GetExtent();
      vtkImageClip *clip=vtkImageClip::New();
      clip->SetInput(image);
      clip->SetOutputWholeExtent(ext[0],ext[1],ext[2],ext[3],
                                 ext[4]+dim[2] / 2,ext[4]+dim[2] / 2);
      clip->ClipDataOn();
      vtkImageTranslateExtent *translat=vtkImageTranslateExtent::New();
      translat->SetInput(clip->GetOutput());
      translat->SetTranslation(0,0,-ext[4]-dim[2] / 2);

      image->UnRegister(NULL);
      image=translat->GetOutput();
      image->Update();
      image->Register(NULL);
      translat->SetOutput(NULL);
      clip->Delete();
      translat->Delete();
   }

   // Set an unsigned char image
   // Shift/Scale the image 
   image->Update();
   vtkImageShiftScale *iss=vtkImageShiftScale::New();
   iss->SetInput(image);
   iss->SetOutputScalarTypeToUnsignedChar();
   iss->SetShift(-range[0]);
   iss->SetScale(255.0/(range[1]-range[0]));
   iss->ClampOverflowOn();

   image->UnRegister(NULL);
   image=iss->GetOutput();
   image->Update();
   image->Register(NULL);
   iss->Delete();

   // Set 3 components to the image
   if(image->GetNumberOfScalarComponents()==1)
   {
      vtkImageAppendComponents *x3=vtkImageAppendComponents::New();
      x3->AddInput(image);
      x3->AddInput(image);
      x3->AddInput(image);

      image->UnRegister(NULL);
      image=x3->GetOutput();
      image->Update();
      image->Register(NULL);
      x3->SetOutput(NULL);
      x3->Delete();
   }
   // End of transform
   //----------------------------------------------------------------------

   // make test
   std::ostringstream str;
   retVal = t->RegressionTest(image,0.0,str);
   image->UnRegister(NULL);

   if( retVal != vtkTesting::PASSED )
   {
      std::cerr << str.str();
   }

   if( retVal == vtkTesting::PASSED )
   {
      std::cerr << "       ... OK" << std::endl;
      return 0;
   }
   else
   {
      std::cerr << "       ... Failed" << std::endl;
      return 1;
   }
}

int VTKTestWrite(int argc, char *argv[])
{
   bool show = false;
   if( argc >= 2 )
   {
      if( std::string(argv[1]) == "-V" )
      {
         show = true;
      }
   }

//   GDCM_NAME_SPACE::Debug::DebugOn();

   int ret = 0;
   vtkTesting *t = vtkTesting::New();
   vtkImageViewer *viewer;
   if( show )
      viewer = vtkImageViewer::New();
   else
      viewer = NULL;

   if( argc < 3+show )
   {
      std::cerr << "Usage: " << argv[0] << " [-V] image.dcm ref.png\n";
      std::cerr << "   -V : to view images to the screen... \n"
                << "        this mode can generate errors in the test\n\n";
   }
   else
   {
      ret = VTKWriteTest(t,viewer,argv[1+show],argv[2+show]);
      t->Delete();
      if( viewer )
         viewer->Delete();

      return ret;
   }

   // Test for all images
   int i = 0;
   while( gdcmDataImages[i] != 0 )
   {
      std::string filename = GDCM_DATA_ROOT;
      filename += "/";  //doh!
      filename += gdcmDataImages[i];
      std::cerr << "Filename: " << filename << std::endl;

      //Extract name to find the png file matching:
      std::string pngfile = gdcmDataImages[i++];
      //pngfile.replace(pngfile.size()-3, 3, "png");
      //More robust approach:
      std::string::size_type dot_pos = pngfile.rfind( "." );
      pngfile = pngfile.substr(0, dot_pos).append( ".png" );
      pngfile.insert( 0, "Baseline/");
      //std::cerr << "PNG file: " << pngfile << std::endl;

      ret += VTKWriteTest(t,viewer,filename,pngfile);
   }
   t->Delete();
   if( viewer )
      viewer->Delete();

   return ret;
}
