from distutils.core import setup
import glob, os, sys, shutil
from distutilsWrapping import *
from WrapSwig import *
from WrapVTK import *

ThisModule      ="gdcmPython"
gdcmPythonSrcDir=ThisModule
gdcmSrcDir      ="src"
gdcmJpeg8SrcDir =os.path.join('src', 'jpeg', 'libijg8')
gdcmJpeg12SrcDir=os.path.join('src', 'jpeg', 'libijg12')
gdcmJpgSrcDir   =os.path.join('src', 'jpeg', 'ljpg')
gdcmvtkSrcDir   ="vtk"
gdcmDictsDir    ="Dicts"
gdcmTestDir     ="Test"

# Due to a distutils oddity on Unices : see
# http://aspn.activestate.com/ASPN/Mail/Message/distutils-sig/588325
if(os.name=='posix'):
	targetDir=os.path.join('lib','python'+sys.version[:3],'site-packages')
	libraries=["stdc++"]
	macros   =[('__STDC_LIMIT_MACROS', '1')]

	VTKPATH="/usr"
	vtkWrapper="vtkWrapPython"
else:
	targetDir=os.path.join('lib','site-packages')
	libraries=["WSOCK32"]
	macros   =[]

	try:
		VTKPATH=os.environ['VTKPATH']
	except KeyError,e:
		err=str(e)
		print "Environment variable",err[err.rfind(':')+1:],'not defined, '\
		       'please fix it!'
		VTKPATH="/usr"
	vtkWrapper=os.path.join(VTKPATH,"bin","vtkWrapPython")

targetDir=os.path.join(targetDir, ThisModule)

### Sources section: determination of sources for the extensions:
# Sources 1a/ The kernel of gdcm itself (which wrapped with Swig)
#             defines the first extension
Sources = []
Sources.extend(glob.glob(os.path.join(gdcmSrcDir,"*.cxx")))
Sources.append(os.path.join(gdcmPythonSrcDir,"gdcm.i"))
# Sources 1b/ The kernel of gdcm depends on a jpeg library whose sources are
#             contained in subdir gdcmJpeg8SrcDir. But within this subdir
#             some of the C files should not be compiled (refer to
#             gdcmJpeg8SrcDir/Makefile.am) !

Jpeg8Sources = glob.glob(os.path.join(gdcmJpeg8SrcDir,"j*.c"))
Jpeg8SourcesToRemove = ['jmemansi.c', 'jmemname.c', 'jmemdos.c', 'jmemmac.c']
for Remove in Jpeg8SourcesToRemove:
   ### Because setup.py is a multiple pass process we need to trap
   ### the case where the files were already wed out on a previous pass.
   try:
      Jpeg8Sources.remove(os.path.join(gdcmJpeg8SrcDir, Remove))
   except ValueError:
      continue
Sources.extend(Jpeg8Sources)

Jpeg12Sources = glob.glob(os.path.join(gdcmJpeg12SrcDir,"j*.c"))
Jpeg12SourcesToRemove = ['jmemansi12.c', 'jmemname12.c', 'jmemdos12.c', 'jmemmac12.c']
for Remove in Jpeg12SourcesToRemove:
   ### Because setup.py is a multiple pass process we need to trap
   ### the case where the files were already wed out on a previous pass.
   try:
      Jpeg12Sources.remove(os.path.join(gdcmJpeg12SrcDir, Remove))
   except ValueError:
      continue
Sources.extend(Jpeg12Sources)

#For 'xmedcon' Jpeg Lossless
JpgSources =glob.glob(os.path.join(gdcmJpgSrcDir,"*.c"))
Sources.extend(JpgSources)  

# Sources 2/ The second extension contains the VTK classes (which we wrap
#            with the vtk wrappers):
VTK_INCLUDE_DIR=os.path.join(VTKPATH,"include","vtk")
VTK_LIB_DIR=os.path.join(VTKPATH,"lib","vtk")
vtkSources = []
vtkSources.extend(glob.glob(os.path.join(gdcmvtkSrcDir,'vtk*.cxx')))
vtkSources.extend(glob.glob(os.path.join(gdcmSrcDir,'*.cxx')))
vtkSources.extend(Jpeg8Sources)
vtkSources.extend(Jpeg12Sources)
vtkSources.extend(JpgSources)

vtkLibraries=["vtkCommon","vtkCommonPython",
              "vtkIO","vtkIOPython",
              "vtkFiltering","vtkFilteringPython"]

##### 
setup(name=ThisModule,
      version="0.4",
      description="...",
      author="frog",
      author_email="frog@creatis.insa-lyon.fr",
      url="http://www.creatis.insa-lyon.fr/Public/Gdcm/",
      packages=[ '.',
                 gdcmPythonSrcDir,
                 gdcmPythonSrcDir + '.demo' ],
      cmdclass={'build_ext':build_extWrap}, # redirects default build_ext
      ext_modules=[SwigExtension(name='_gdcm',
                                 sources=Sources,
                                 include_dirs=[gdcmSrcDir,gdcmJpeg8SrcDir,
                                               gdcmJpeg12SrcDir,gdcmJpgSrcDir],
                                 libraries=libraries,
                                 define_macros=macros,
                                 swig_cpp=1,
                                 swig_include=[gdcmSrcDir]
                                ),
                                VTKExtension(name='gdcmPython.vtkgdcmPython',
                                sources=vtkSources,
                                include_dirs=[gdcmSrcDir,gdcmvtkSrcDir,
                                              VTK_INCLUDE_DIR],
                                libraries=libraries+vtkLibraries,
                                define_macros=macros,
                                library_dirs=[VTK_LIB_DIR],
                                vtkWrapper=vtkWrapper,
                               ),
						],
      data_files=[(os.path.join(targetDir,gdcmTestDir),
                   glob.glob(os.path.join(gdcmTestDir,"*.acr"))),
                  (os.path.join(targetDir,"Dicts"),
                   glob.glob(os.path.join(gdcmDictsDir,"*.*"))),
                ]
     )
