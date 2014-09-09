/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: VTKTestReadSeq.cxx,v $
  Language:  C++
  Date:      $Date: 2007/09/18 07:58:38 $
  Version:   $Revision: 1.12 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/
#include "vtkGdcmReader.h"
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
#include "gdcmDataSeqImages.h"

#ifndef vtkFloatingPointType
#define vtkFloatingPointType float
#endif

int VTKReadSeqTest(vtkTesting *t, vtkImageViewer *viewer,
                   std::string const &filename, 
                   std::string const &referenceFileName)
{
   int retVal; // = 0;  (to avoid bcc 5.5 warnings)

   // Set the reader   
   vtkGdcmReader *reader = vtkGdcmReader::New();

   char *newFileDcm = new char[filename.size()+1];
   int fileCount = 0;
   for(int i=0;i<9;i++)
   {
      fileCount = i;
      sprintf(newFileDcm,filename.c_str(),i);

      // Test the existance of the file
      ifstream opened(newFileDcm,std::ios::in | std::ios::binary);
      if(opened)
      {
         reader->AddFileName(newFileDcm);
         opened.close();
      }
      else
         break;
   }
   delete[] newFileDcm;
   reader->Update();

   double range[2];
   reader->GetOutput()->GetScalarRange(range);

   // Show
   if( viewer )
   {
      viewer->SetInput ( reader->GetOutput() );
      viewer->OffScreenRenderingOff();

      viewer->SetColorWindow (range[1] - range[0]);
      viewer->SetColorLevel (0.5 * (range[1] + range[0]));

      int dim[3];
      reader->GetOutput()->GetDimensions( dim );
      viewer->SetSize(dim[0], dim[1]);
      viewer->SetZSlice( 0 );
      viewer->Render();
      viewer->SetInput(NULL);
   }

   // make test
   int ret = 0;
   std::ostringstream str;
   char *newFilePng = new char[referenceFileName.size()+1];
   for(int j=0;j<fileCount;j++)
   {
      sprintf(newFilePng,referenceFileName.c_str(),j);

      t->CleanArguments();
      t->AddArgument("-D");
      t->AddArgument( GDCM_DATA_ROOT );
      t->AddArgument("-V");
      t->AddArgument( newFilePng );
      t->AddArgument("-T");
      t->AddArgument( "." );

      //----------------------------------------------------------------------
      // Transform the image to be RGB unsigned char, due to the requests in
      // vtkTesting processing
      // The pipeline is broken after each process to keep maximum of memory
      vtkImageData *image=reader->GetOutput();
      image->Update();
      image->Register(NULL);

      // Get the middle slice in the image
      int *ext=image->GetExtent();
      vtkImageClip *clip=vtkImageClip::New();
      clip->SetInput(image);
      clip->SetOutputWholeExtent(ext[0],ext[1],ext[2],ext[3],j,j);
      clip->ClipDataOn();
      vtkImageTranslateExtent *translat=vtkImageTranslateExtent::New();
      translat->SetInput(clip->GetOutput());
      translat->SetTranslation(0,0,-j);

      image->UnRegister(NULL);
      image=translat->GetOutput();
      image->Update();
      image->Register(NULL);
      translat->SetOutput(NULL);
      clip->Delete();
      translat->Delete();

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
      retVal = t->RegressionTest(image,2.0,str);
      image->UnRegister(NULL);

      if( retVal != vtkTesting::PASSED )
      {
         std::cerr << str.str();
      }

      if( retVal == vtkTesting::PASSED )
      {
         std::cerr << "       ...Slice " << j << ": OK" << std::endl;
      }
      else
      {
         std::cerr << "       ...Slice " << j << ": Failed" << std::endl;
         ret++;
      }
   }

   delete[] newFilePng;
   reader->Delete();

   return ret;
}

int VTKTestReadSeq(int argc, char *argv[])
{
   bool show = false;
   if( argc >= 2 )
   {
      if( std::string(argv[1]) == "-V" )
      {
         show = true;
      }
   }

   int ret = 0;
   vtkTesting *t = vtkTesting::New();
   vtkImageViewer *viewer;
   if( show )
      viewer = vtkImageViewer::New();
   else
      viewer = NULL;

   if( argc < 3+show )
   {
      std::cerr << "Usage: " << argv[0] << " [-V] image%d.dcm ref%d.png\n";
      std::cerr << "   -V : to view images to the screen... \n"
                << "        this mode can generate errors in the test\n";
      std::cerr << "   %d : this will be replaced by a number at execution.\n"
                << "        It will be from 0 to 9 only with a step of 1\n\n";
   }
   else
   {
      ret = VTKReadSeqTest(t,viewer,argv[1+show],argv[2+show]);
      t->Delete();
      if( viewer )
         viewer->Delete();

      return ret;
   }

   // Test for all images
   int i = 0;
   while( gdcmDataSeqImages[i] != 0 )
   {
      std::string filename = GDCM_DATA_ROOT;
      filename += "/";  //doh!
      filename += gdcmDataSeqImages[i];
      std::cerr << "Filename: " << filename << std::endl;

      //Extract name to find the png file matching:
      std::string pngfile = gdcmDataSeqImages[i++];
      std::string::size_type dot_pos = pngfile.rfind( "." );
      pngfile = pngfile.substr(0, dot_pos).append( ".png" );
      pngfile.insert( 0, "Baseline/");
      
      ret += VTKReadSeqTest(t,viewer,filename,pngfile);
   }
   t->Delete();
   if( viewer )
      viewer->Delete();

   return ret;
}
