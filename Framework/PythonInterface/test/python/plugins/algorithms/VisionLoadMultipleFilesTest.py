# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
from __future__ import (absolute_import, division, print_function)

import unittest
# from mantid.api import
from mantid.simpleapi import VisionLoadMultipleFiles, config, mtd


class VisionLoadMultipleFilesTest(unittest.TestCase):
    out = VisionLoadMultipleFiles(Run_numbers='37760-',Weighting_method='estimate from error', File_structure='Flat')


if __name__ == "__main__":
    unittest.main()
