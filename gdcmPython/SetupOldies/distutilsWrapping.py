from distutils.command.build_ext import build_ext
from distutils.core import Extension
from distutils.file_util import copy_file
from types import ListType
import os
import string

class build_extWrap(build_ext):
	"""
	This distutils command is meant to be used with all wrapper defined for
	this format.
	To realize it,we can't have command-line parameters
	"""
	def build_extension(self,ext):
		# command-line arguments prevail over extension arguments
		# but if no command-line argument is defined,extension argument is
		# taken into account
		self.__ext=ext
		build_ext.build_extension(self,ext)

		if(os.name!='posix'):
			# Copy the .lib file
			fullname = self.get_ext_fullname(ext.name)
			modpath = string.split(fullname, '.')
			package = string.join(modpath[0:-1], '.')
			base = modpath[-1]
			if self.inplace:
				# ignore build-lib -- put the compiled extension into
				# the source tree along with pure Python modules
				build_py = self.get_finalized_command('build_py')
				package_dir = build_py.get_package_dir(package)
				dstLib=package_dir
			else:
				dstLib=self.build_lib
	
			srcLib=os.path.join(self.build_temp,base+".lib")
			dstLib=os.path.join(dstLib,package)
	
			copy_file(srcLib,dstLib)

	def swig_sources(self,sources):
		"""Walk the list of source files in 'sources',looking for SWIG
		interface(.i) files.  Run SWIG on all that are found,and
		return a modified 'sources' list with SWIG source files replaced
		by the generated C(or C++) files.
		"""
		try:
			return(self.__ext.wrapper.WrapSources(self,self.__ext,sources))
		except Exception,e:
			print Exception,e
			self.announce("Warning: wrapping error")
			return(sources)

class Wrapper:
	def WrapSources(self,distutil,extWrap,sources):
		pass

class ExtensionWrap(Extension):
	"""
	This class extends basic distutils Extension class,adding two keyword
	arguments :
		* swig_cpp,which triggers -c++ mode when swigging
		* swig_include,which specifies -I flag when swigging
	This class is meant to be build with mybuild_ext distutils command.
	"""
	def __init__(self,wrapper=None,**args):
		Extension.__init__(self,**args)

		self.wrapper=wrapper

"""
Example of use of these classes in distutils setup method :

from Transfert.tcDistUtils import mybuild_ext,MyExtension
setup(name="xxx",
		version="X.Y",
		description="blah blah blah",
		author="John Doe",
		author_email="i.hate@spam.com",
		url="http://www.fakeurl.com",
		packages=["yyy"],
		cmdclass={'build_ext':build_extWrap},# redirects default build_ext
		ext_modules=[ExtensionWrap(name="src/xxx,# instance of our Extension class
										   sources=["src/xxx.cpp",
									  				 "src/xxx.i"],
										   include_dirs=["/usr/include/python2.1",
										 					"/usr/include/vtk"],
										   libraries=["vtkGraphics"],
										  )
						]
		)
		
and then run "python setup.py build"...
"""
