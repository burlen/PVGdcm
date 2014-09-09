from distutilsWrapping import *
import types
import string
import os

class VTKWrapper(Wrapper):
	"""
	This distutils command is meant to be used with MyExtension extension, which
	defines a swig_include attribute.
	"""
	def WrapSources(self,distutil,extWrap,sources):
		"""
		Walk the list of source files in 'sources', looking for VTK
		interface (vtk*.cxx) files. Compile vtkWrapPythonInit.
		Run vtkWrapPython on all that are found, and
		return a modified 'sources' list with SWIG source files replaced
		by the generated C (or C++) files.
		
		FIXME nierk
		"""
		self.__extWrap=extWrap

		newSources=[]
		vtkSources=[]
		vtkTargets={}

		# Wrapping of sources
		target_ext='Python.cxx'

		for source in sources:
			(base,ext)=os.path.splitext(source)
			fileName=os.path.split(base)
			if((ext==".cxx")and(fileName[-1][0:3]=="vtk")
			   and(fileName[-1][-6:]!="Python")):
				newSources.append(source)
				newSources.append(base+target_ext)
				vtkSources.append(base+'.h')
				vtkTargets[base+'.h']=newSources[-1]
			else:
				newSources.append(source)

		# Find vtkWrapPython
		wrapper=self.FindvtkWrapPython()
		if(not self.__extWrap.vtkHints):
			self.__extWrap.vtkHints="dummyHints"

		wrapCmd=[wrapper]
		for source in vtkSources:
			target=vtkTargets[source]
			distutil.announce("VTK wrapping %s to %s" % (source,target))
			distutil.spawn([wrapper,source,self.__extWrap.vtkHints,target])

		# Compilation of vtkWrapPythonInit
		vtkWrapInit=self.__extWrap.vtkModule+"Init"+target_ext
		distutil.announce("VTK init wrapping to %s" % vtkWrapInit)
		self.WrapInit(vtkSources,vtkWrapInit)
		newSources.append(vtkWrapInit)

		return newSources

	def FindvtkWrapPython(self):
		assert(os.path.isfile(self.__extWrap.vtkWrapper),
		       "Write an heuristic in FindvtkWrapPython")
		return(self.__extWrap.vtkWrapper)

	def WrapInit(self,vtkSource,target):
		dllName=string.split(self.__extWrap.vtkModule,'.')[-1]
		f=open(target,"w")

		f.write('#include <string.h>\n')
		f.write('#include "Python.h"\n\n')

		for src in vtkSource:
			src=os.path.split(src)[-1]
			(src,_)=os.path.splitext(src)
			f.write('extern "C" { ')
			if(os.name!="posix"):
				f.write('__declspec( dllexport ) ')
			f.write('PyObject *PyVTKClass_%sNew(char *); }\n'% src)

		# Lib Init
		f.write('\nstatic PyMethodDef Py%s_ClassMethods[] = {\n'% dllName)
		f.write('{NULL, NULL}};\n\n')

		f.write('extern "C" { ')
		if(os.name!="posix"):
			f.write('__declspec( dllexport ) ')
		f.write('void init%s();}\n\n'% dllName)

		f.write('void init%s()\n{\n'% dllName)
		f.write('  PyObject *m, *d, *c;\n\n')
		f.write('  static char modulename[] = "%s";\n'% dllName)
		f.write('  m = Py_InitModule(modulename, Py%s_ClassMethods);\n'% dllName)

		f.write('  d = PyModule_GetDict(m);\n')
		f.write('  if (!d) Py_FatalError("can''t get dictionary for module %s!");\n\n'% dllName)

		# New function
		for src in vtkSource:
			src=os.path.split(src)[-1]
			(src,_)=os.path.splitext(src)
			f.write('  if ((c = PyVTKClass_%sNew(modulename)))\n'% src)
			f.write('    if (-1 == PyDict_SetItemString(d, "%s", c))\n'% src)
			f.write('      Py_FatalError("can''t add class %s to dictionary!");\n\n'% src)
		f.write('}\n\n')

		f.close()

class VTKExtension(ExtensionWrap):
	"""
	This class extends basic distutils Extension class, adding two keyword
	arguments :
		* swig_cpp, which triggers -c++ mode when swigging
		* swig_include, which specifies -I flag when swigging
	This class is meant to be build with mybuild_ext distutils command.
	"""
	def __init__(self,name,vtkHints=None,
	             vtkWrapper=None,**args):
		ExtensionWrap.__init__(self,name=name,wrapper=VTKWrapper(),**args)

		assert(type(name)==types.StringType,"vtk Module must be a string")

		self.vtkHints=vtkHints
		self.vtkModule=name
		self.vtkWrapper=vtkWrapper

