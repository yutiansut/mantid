# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
#  This file is part of the mantidqt package.
from __future__ import (absolute_import, division, print_function)

from qtpy.QtWidgets import QLineEdit, QPushButton, QDialog, QVBoxLayout


class SillyInterfaceView(QDialog):
    LINEEDIT_OBJECTNAME = "SillyInterfaceView_LineEdit"
    TEST_STR = "someranoiemfaoiemflaksejdvoadjfgoakr"

    def __init__(self, parent=None):
        super(SillyInterfaceView, self).__init__(parent)
        _layout = QVBoxLayout()
        self.setLayout(_layout)
        self.button = QPushButton('wow')
        self.button.clicked.connect(self.changelineedit)
        _layout.addWidget(self.button)
        self.lineedit = QLineEdit()
        self.lineedit.setObjectName(self.LINEEDIT_OBJECTNAME)
        _layout.addWidget(self.lineedit)
        # self.lineedit.cha

    def changelineedit(self, _):
        self.lineedit.setText(self.TEST_STR)
