### This simple code illustrates how the behavior of the parsing of
### directory in order to build a DicomDir can be controlled by
### the caller. By caller control we mean two things:
###  1/ watch the advancement of the parsing
###  2/ abort it on caller demand
### In this example we arbitrarily stop the parsing (see progessMethod() )
### when the parsing reaches 20% of it's duty. A typical GUI usage of
### this mecanism would be to offer a "Cancel" button to the user...
from gdcmPython import *
import sys

### Get the pathname from command line or default it to .
try:
   DirPathName = sys.argv[1]
except IndexError:
   DirPathName = "."

### The python defined methods that we wan't to call back
def startMethod():
   print "Start"
def progressMethod():
   print "Progress", dicomDir.GetProgress()
   if( dicomDir.GetProgress() > 0.2 ):
      dicomDir.AbortProgress()
def endMethod():
   print "End"

dicomDir=gdcm.DicomDir( DirPathName )
print dicomDir.IsReadable()
### Set up the call backs:
dicomDir.SetStartMethod(startMethod)
dicomDir.SetProgressMethod(progressMethod)
dicomDir.SetEndMethod(endMethod)
### Launch the parsing of the directory
dicomDir.ParseDirectory()
