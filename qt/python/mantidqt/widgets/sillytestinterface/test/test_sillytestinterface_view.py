# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
#  This file is part of the mantid workbench.

import os
from qtpy.QtCore import QPointF, Qt
from qtpy.QtGui import QMouseEvent
from qtpy.QtWidgets import QApplication

from mantidqt.utils.qt.testing import GuiTest
from mantidqt.utils.qt.testing.qt_widget_finder import QtWidgetFinder
from mantidqt.widgets.sillytestinterface.view import SillyInterfaceView


class SillyInterfaceViewTest(GuiTest, QtWidgetFinder):
    @classmethod
    def setUpClass(cls, be_nosy=False):
        # creates the QApplication to use for this test
        # import sys
        # sys.path.append("c:\\users\\qbr77747\\apps\\miniconda3\\lib\\site-packages")
        # import pydevd
        # pydevd.settrace('localhost', port=44444, stdoutToServer=True, stderrToServer=True)
        if os.environ.get("WORKBENCH_MAKE_GUI_TEST", None) is None:
            super(SillyInterfaceViewTest, cls).setUpClass()
        else:
            super(SillyInterfaceViewTest, cls).setUpClass(be_nosy=True)
            # TODO figure out how to skip suite
            view = SillyInterfaceView()
            view.exec_()

    def test_lineedit_press_a(self):
        view = SillyInterfaceView()
        app = QApplication.instance()
        # event = QKeyEvent(QKeyEvent.KeyPress, 69, Qt.NoModifier, chr(69))

        # These 2 events constitute a single click
        event = QMouseEvent(QMouseEvent.MouseButtonPress, QPointF(10, 10), Qt.LeftButton, Qt.LeftButton, Qt.NoModifier)
        app.notify(view.button, event)
        event = QMouseEvent(QMouseEvent.MouseButtonRelease, QPointF(10, 10), Qt.LeftButton, Qt.LeftButton,
                            Qt.NoModifier)
        app.notify(view.button, event)

        app.processEvents()

        self.assertEqual(view.TEST_STR, view.lineedit.text())
        # self.assertEqual(chr(69), view.lineedit.text())
