
from PyQt6 import QtWidgets
from PyQt6.QtWidgets import QMainWindow, QLabel, QFileDialog, QPushButton, QListWidget
from PyQt6.QtCore import Qt, QThread, pyqtSignal
from PyQt6.QtGui import QKeySequence

# import pyqt graphing stuff
import pyqtgraph as pg
from pyqtgraph import PlotWidget, plot
import numpy as np

import serial as ser
import os


class scan_thread(QThread):
    finished = pyqtSignal(dict)

    def __init__(self):
        super().__init__()

    def run(self):

        if os.name == 'nt':
            ports = ['COM%s' % i for i in range(1,256)]
        elif os.name == 'posix':
            ports = glob.glob('/dev/tty[A-Za-z]*')
        else:
            raise EnvironmentError('Unsupported platform')
        
        result = {}
        for port in reversed(ports):

            try:
                s = ser.Serial(port, 115200, timeout=1.0)
                s.write("M100\n".encode())

                response = s.read_until("\n".encode()).decode()
                assert response.strip() == "3Dprinter"
    
            except (ser.SerialException):
                pass
            except (AssertionError):
                s.close()
            else:
                print("port {} is a printer".format(port))
                result[port] = s
                break
        else:
            pass
            # no printer found
        self.finished.emit(result)

class data_collector(QThread):
    data_point_update = pyqtSignal(list)


    def __init__(self, serial):
        super().__init__()

        self.cancelled = False
        self.serial = serial

    def run(self):
        pass

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("PID Autotune")
        self.resize(800, 600)

    def graph_update(self):
        pass



if __name__ == "__main__":
    app = QtWidgets.QApplication([])

    window = MainWindow()
    window.show()

    app.exec()

