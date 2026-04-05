# Importing Libraries
import serial
import time
import numpy as np

startMarker = 60
endMarker = 62


class Arduino(object):
    def __init__(self, Serial):

        self.Serial = Serial

    def write(self, string):
        self.Serial.write(bytes(string, "UTF-8"))

    def read(self):
        global startMarker, endMarker

        ck = ""
        x = "z"  # any value that is not an end- or startMarker
        byteCount = -1  # to allow for the fact that the last increment will be one too many

        # wait for the start character
        while ord(x) != startMarker:
            x = self.Serial.read()


        # save data until the end marker is found
        while ord(x) != endMarker:
            if ord(x) != startMarker:
                ck = ck + x.decode("UTF-8")
                byteCount += 1
            x = self.Serial.read()

        return (ck)

    def waitForArduino(self):

        # wait until the Arduino sends 'Arduino Ready' - allows time for Arduino reset
        # it also ensures that any bytes left over from a previous message are discarded

        global startMarker, endMarker

        msg = ""
        while msg.find("ready") == -1:

            while self.Serial.inWaiting() == 0:
                pass

            msg = self.read()





S = serial.Serial(port='COM5', baudrate=57600, timeout=None)
A = Arduino(S)

a = 0

with open("penguin4_0005.ngc", "r") as file:
    lines = file.readlines()  # get gcode lines

def createGap(string, before):
    ns = ""
    for s in string.split(before):
        ns += s + " " + before

    return ns

def removeNotations(line):
    inbrackets = False
    cleanline = ""
    for char in line:
        if char == "(":
            inbrackets = True
        elif char == ")":
            inbrackets = False
            continue

        if not inbrackets:
            cleanline += char
    return cleanline

method = ""

x = 0
y = 0
px = 0
py = 0
i = 0
j = 0

for line in lines:

    line = removeNotations(line)
    if line == "": continue

    print(line)
    A.waitForArduino()
    time.sleep(0.1)
    A.write(line + "\n")

A.waitForArduino()
A.write("G01 X00 Y00 Z1\n") # return home
A.waitForArduino()
A.write("M18\n") # disable steppers

