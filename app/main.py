
from PyQt6 import QtWidgets
from PyQt6.QtWidgets import QMainWindow, QLabel, QFileDialog, QPushButton, QListWidget
from PyQt6.QtCore import Qt, QThread, pyqtSignal
from PyQt6.QtGui import QKeySequence


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
    
class print_thread(QThread):
    line_update = pyqtSignal(int)

    def __init__(self, serial, gcode):
        super().__init__()

        self.serial = serial
        self.gcode = gcode

        self.cancelled = False


    def run(self):
        
        i = 0
        for line in self.gcode:

            if self.cancelled:
                self.statusBar().showMessage("Print cancelled")
                break
            
            if line[0] == ';':
                continue # skip comment lines

            # write line to serial port and wait for it to finish being sent
            
            self.serial.write(line.encode())
            self.serial.flush()

            # now the command is fully sent, wait for a second response
            while True:
                response = self.serial.readline()
                if response == b"OK\r\n":
                    break

            i+=1
            self.line_update.emit(i)

        else: 
            self.statusBar().showMessage("Print complete")


class MainWindow(QMainWindow):

    def __init__(self):
        super().__init__()

        self.setWindowTitle("Printer Control")

        layout = QtWidgets.QVBoxLayout()

        label = QLabel("3D printer control!")
        label.setAlignment(Qt.AlignmentFlag.AlignCenter)


        # create a list widget of lines of text

        self.printer_found = False
        self.scan_ports_button = QPushButton("Scan Ports")
        self.scan_ports_thread = scan_thread()
        self.scan_ports_thread.finished.connect(self.scan_ports_finished)

        load_gcode_button = QPushButton("Load GCode")
        self.start_print_button = QPushButton("Start Print")
        self.cancel_print_button = QPushButton("Cancel Print")
        self.cancel_print_button.setEnabled(False)

        self.scan_ports_button.clicked.connect(self.scan_ports)
        load_gcode_button.clicked.connect(self.load_gcode)
        self.start_print_button.clicked.connect(self.start_print)
        self.cancel_print_button.clicked.connect(self.cancel_print)
        

        self.list_widget = QListWidget()

        # make list widget uneditable
        self.list_widget.setEditTriggers(QtWidgets.QAbstractItemView.EditTrigger.NoEditTriggers)
        # make list widget unselectable
        self.list_widget.setSelectionMode(QtWidgets.QAbstractItemView.SelectionMode.NoSelection)

        layout.addWidget(label)
        layout.addWidget(self.scan_ports_button)
        layout.addWidget(load_gcode_button)
        layout.addWidget(self.start_print_button)
        layout.addWidget(self.cancel_print_button)
        layout.addWidget(self.list_widget)

        # set the central widget of the Window

        widget = QtWidgets.QWidget()
        widget.setLayout(layout)

        self.setCentralWidget(widget)

        self.show()

    def scan_ports(self):
        self.scan_ports_button.setEnabled(False)
        self.scan_ports_thread.start()

    def scan_ports_finished(self, result):
        self.scan_ports_button.setEnabled(True)

        if len(result) == 0:
            self.statusBar().showMessage("No printer found")
            return

        for key,val in result.items():
            self.statusBar().showMessage("Printer found on port {}".format(key))
            self.serial = val

        self.printer_found = True


    def load_gcode(self):
        ## open file dialog
        file_name = QFileDialog.getOpenFileName(self, "Open GCode file", "", "GCode files (*.gcode)")
        if file_name[0] == "":
            self.statusBar().showMessage("No file selected")
            return
        
        ## read file
        with open(file_name[0], 'r') as f:
            self.gcode = f.readlines()

        ## display files lines in list widget
        self.list_widget.clear()
        self.list_widget.addItems(self.gcode)

        self.statusBar().showMessage("GCode loaded")

    
    def start_print(self):
    
        ## send gcode to printer
        if self.list_widget.count() == 0:
            self.statusBar().showMessage("No GCode loaded")
            return
        if not self.printer_found:
            self.statusBar().showMessage("No printer found")
            return
        
        self.serial.timeout = 1000.0

        self.statusBar().showMessage("Starting print")
        self.start_print_button.setEnabled(False)
        self.cancel_print_button.setEnabled(True)

        self.print_thread = print_thread(self.serial, self.gcode)
        self.print_thread.line_update.connect(self.line_update)
        self.print_thread.finished.connect(self.on_print_complete)
        self.print_thread.start()
 
    def cancel_print(self):
        self.print_thread.terminate()

        self.serial.flush()
        self.serial.close()
        QThread.msleep(100)
        self.serial.open()

    def line_update(self, i):
        self.list_widget.setCurrentRow(i)
        # highlight current line
        self.list_widget.item(i).setSelected(True)

    def on_print_complete(self):
        self.cancel_print_button.setEnabled(False)
        self.start_print_button.setEnabled(True)

    def on_exit(self):
        if self.printer_found:
            self.serial.close()


if __name__ == "__main__":
    app = QtWidgets.QApplication([])

    window = MainWindow()
    app.aboutToQuit.connect(window.on_exit)

    app.exec()
