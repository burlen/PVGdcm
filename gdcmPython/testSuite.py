import unittest
import os
from gdcmPython import *
if os.name == 'posix':
   from libvtkgdcmPython import *
else:
   from vtkgdcmPython import *

class gdcmTestCase(unittest.TestCase):
   # The files whose name starts with a modality (e.g. CR-MONO1-10-chest.dcm)
   # come from Sebastien Barre's Dicom2 highly recommendable site
   # http://www.barre.nom.fr/medical/samples/index.html

   MultiFrameFiles = [

   GdcmFiles = [
      # FOLLOWING FILE NOT IN GDCMDATA ANYMORE !?!?!?!
      ["gdcm-MR-PHILIPS-16.dcm",
         # Interest: - possesses a sequence
         #           - dicom file, with a recognition code of ACR-NEMA1
         [ ["Transfer Syntax UID", "1.2.840.10008.1.2"],  # Implicit VR, LE
           ["Recognition Code (RET)", "ACR-NEMA 1.0"],
           ["Modality", "MR"],
           ["Photometric Interpretation", "MONOCHROME2"],
           ["Rows", "256"],
           ["Columns", "256"],
           ["Bits Stored", "8"],
           ["Bits Allocated", "16"],
           ["High Bit", "7"],
           ["Pixel Representation", "0"],
           ["Manufacturer", "Philips Medical Systems"],
           ["Manufacturer's Model Name", "Gyroscan Intera"],
           ["Sequence Variant", "OTHER"],
           ["Pixel Data", "gdcm::NotLoaded. Address:6584 Length:131072 x(20000)"]
          ] ],
   ]

   def _BaseTest(self, FileSet):
      for entry in FileSet:
         fileName = os.path.join(GDCM_TEST_DATA_PATH, entry[0])
         reader = gdcmFile(fileName)
         assert reader.IsReadable(),\
                "File '%s' is not readable by gdcmFile" % fileName

         valDict = reader.GetEntryValue()
         for subEntry in entry[1]:
            element = subEntry[0]
            value   = subEntry[1]
            self.assertEqual(valDict[element], value,
                             ("Wrong %s for file %s (got %s, shoud be %s)"
                             % (element,fileName, valDict[element], value)) )

   def testFiles(self):
      gdcmTestCase._BaseTest(self, gdcmTestCase.GdcmFiles)

if __name__ == '__main__':
   if not GDCM_TEST_DATA_PATH:
      print "GDCM_TEST_DATA_PATH (internal variable) is not setup properly."
      print "   This test suite requires that some Dicom reference files be "
      print "   installed."
      print "   For further details on installation of gdcmData, please"
      print "   refer to the developper's section of page "
      print "       http://www.creatis.insa-lyon.fr/Public/Gdcm"
      print ""
      print "gdcmData directory (used in the test suite) must be placed in"
      print "the gdcm directory. The gdcm tree must be :"
      print "   gdcm"
      print "    |____Dicts"
      print "    |____Doc"
      print "    |____gdcmData      (not in gdcm by default)"
      print "    |____gdcmPython"
      print "    |____Test"
   else:
      unittest.main()

