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
from mantidqt.utils.qt.testing.nosy_qapplication import NosyQApplication
from mantidqt.utils.qt.testing.qt_widget_finder import QtWidgetFinder
from mantidqt.widgets.sillytestinterface.view import SillyInterfaceView

if os.environ.get("WORKBENCH_MAKE_GUI_TEST", None):

    app = NosyQApplication(["SillyInterfaceView_MakeGuiTest"])
    view = SillyInterfaceView()
    view.show()
    app.exec_()
else:

    class SillyInterfaceViewTest(GuiTest, QtWidgetFinder):
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
