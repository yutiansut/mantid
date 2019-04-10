from qtpy.QtWidgets import QApplication, QLineEdit, QPushButton
from qtpy.QtGui import QKeyEvent, QMouseEvent
import sys
from qtpy.QtCore import Qt
import logging


class NosyQApplication(QApplication):
    def __init__(self, args):
        super(NosyQApplication, self).__init__(args)
        # self.view = view
        self.map_cache = {}

    def str_from_enum(self, cls, enum_val):
        key = "{}_{}".format(str(cls), enum_val)
        if enum_val in self.map_cache:
            return self.map_cache[key]
        else:
            for attr in dir(cls):
                if enum_val == getattr(cls, attr):
                    self.map_cache[key] = attr
                    return attr

    def str_from_button(self, enum_val):
        if enum_val == Qt.LeftButton:
            return "LeftButton"
        else:
            return ""

    def notify(self, receiver, event):
        # QLineEdit receiving QKeyEvent
        # Note: there are 3 events:
        #     - key down, up and ? TODO figure that out laterrz
        if isinstance(receiver, QLineEdit) and isinstance(event, QKeyEvent):
            print(receiver, "object name:", receiver.objectName(), event.key(), event.type() == QKeyEvent.KeyPress,
                  event.modifiers() == Qt.NoModifier)
        if isinstance(receiver, QPushButton) and isinstance(event, QMouseEvent) and (
                event.type() == QMouseEvent.MouseButtonPress or event.type() == QMouseEvent.MouseButtonRelease):
            import sys
            sys.path.append("c:\\users\\qbr77747\\apps\\miniconda3\\lib\\site-packages")
            import pydevd
            pydevd.settrace('localhost', port=44445, stdoutToServer=True, stderrToServer=True)

            # TODO map to actual enum strings rather than just ints, PyQt will Error with ints!
            print(
                "event = QMouseEvent(QMouseEvent.{},{}, Qt.{}, Qt.{}, Qt.NoModifier)\napp.notify('{}', event)\napp.processEvents()".format(
                    self.str_from_enum(QMouseEvent, event.type()), event.pos(),
                    self.str_from_button(event.button()),
                    self.str_from_button(event.button()), receiver.objectName()))
            sys.stdout.flush()
            # print(receiver, "object name:", receiver.objectName(), event.button(), event.type() == QKeyEvent.KeyPress, event.modifiers()==Qt.NoModifier)
        return super(NosyQApplication, self).notify(receiver, event)
