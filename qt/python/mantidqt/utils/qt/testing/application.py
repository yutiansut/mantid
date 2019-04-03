# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2017 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
#  This file is part of the mantid workbench.
#
#
# flake8: noqa
from __future__ import absolute_import
from qtpy.QtWidgets import QApplication

from mantidqt.utils.qt.plugins import setup_library_paths

# Hold on to QAPP reference to avoid garbage collection
from mantidqt.utils.qt.testing.nosy_qapplication import NosyQApplication

_QAPP = None


def get_application(name='', be_nosy=False):
    """
    Initialise and return the global application object
    :param name: Optional application name
    :param be_nosy:
    :return: Global application object
    """
    global _QAPP
    if _QAPP is None:
        setup_library_paths()
        _QAPP = QApplication([name]) if not be_nosy else NosyQApplication([name])
    return _QAPP
