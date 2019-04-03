from qtpy.QtWidgets import QApplication, QLineEdit, QPushButton
from qtpy.QtGui import QKeyEvent, QMouseEvent
import sys
from qtpy.QtCore import Qt
from logging import Logger


class NosyQApplication(QApplication):
    def __init__(self, args):
        super(NosyQApplication, self).__init__(args)
        # self.view = view
        self.logger = Logger("NosyQApplication")

    def notify(self, receiver, event):
        # QLineEdit receiving QKeyEvent
        # Note: there are 3 events:
        #     - key down, up and ? TODO figure that out laterrz
        if isinstance(receiver, QLineEdit) and isinstance(event, QKeyEvent):
            print(receiver, "object name:", receiver.objectName(), event.key(), event.type() == QKeyEvent.KeyPress,
                  event.modifiers() == Qt.NoModifier)
        if isinstance(receiver, QPushButton) and isinstance(event, QMouseEvent) and (
                event.type() == QMouseEvent.MouseButtonPress or event.type() == QMouseEvent.MouseButtonRelease):
            # TODO map to actual enum strings rather than just ints, PyQt will Error with ints!
            self.logger.critical(
                "event = QMouseEvent({},{},{})\napp.notify('{}', event)\napp.processEvents()".format(event.type(),
                                                                                                     event.pos(),
                                                                                                     event.button(),
                                                                                                     receiver.objectName()))
            sys.stdout.flush()
            # print(receiver, "object name:", receiver.objectName(), event.button(), event.type() == QKeyEvent.KeyPress, event.modifiers()==Qt.NoModifier)
        return super(NosyQApplication, self).notify(receiver, event)
