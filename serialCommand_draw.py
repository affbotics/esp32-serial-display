import serial
from cobs import cobs
import time
import random

ser = serial.Serial('/dev/ttyUSB1', 2000000)

def sendCommand(commandByte, dataBytes):
    # Build command message
    encoded_data = cobs.encode(bytearray([commandByte] + dataBytes))
    ser.write(encoded_data + b'\x00')

def setTextColor(fgColor, bgColor):
    fgColorH = fgColor >> 8
    fgColorL = fgColor & 0x00FF

    bgColorH = bgColor >> 8
    bgColorL = bgColor & 0x00FF

    sendCommand(8, [fgColorH, fgColorL, bgColorH, bgColorL])

def printText(x, y, textString):
    xH = x >> 8
    xL = x & 0x00FF
    yH = y >> 8
    yL = y & 0x00FF

    dataBytes = [xH, xL, yH, yL]

    textByte = [ord(x) for x in textString]
    # textByte = ord(textString) 
    dataBytes += textByte

    sendCommand(12, dataBytes)

def clearScreen(color):
    colorH = color >> 8
    colorL = color & 0x00FF
    sendCommand(4,  [colorH, colorL])

def setColor(color):
    colorH = color >> 8
    colorL = color & 0x00FF
    sendCommand(7, [colorH, colorL])

def drawRectangle(x, y, w, h):
    xH = x >> 8
    xL = x & 0x00FF
    yH = y >> 8
    yL = y & 0x00FF

    wH = w >> 8
    wL = w & 0x00FF
    hH = h >> 8
    hL = h & 0x00FF
    sendCommand(17, [xH, xL, yH, yL, wH, wL, hH, hL])

def drawCircle(x, y, r):
    xH = x >> 8
    xL = x & 0x00FF
    yH = y >> 8
    yL = y & 0x00FF

    rH = r >> 8
    rL = r & 0x00FF

    sendCommand(19, [xH, xL, yH, yL, rH, rL])


data = bytearray()
touched = False
t_x = 0
t_y = 0

def getTouch():
    global touched
    global t_x 
    global t_y

    if touched:
        touched = False
        touch = (t_x, t_y)
        return touch
    return False

def readCobs():
    global touched
    global t_x 
    global t_y
    global data
    curr_value = ser.read()
    # print(curr_value)
    if(curr_value != b'\x00'):
        data += curr_value
        # print(data)
    else:
        try:
            decoded_data = cobs.decode(data)
            # print(f'Decoded: {decoded_data}')
            data = bytearray()

            if(decoded_data[0] == 20):
                touched = True
                t_x = (decoded_data[1]<<8) | decoded_data[2]
                t_y = (decoded_data[3]<<8) | decoded_data[4]
                # print(f'touch: {t_x}, {t_y}')

        except Exception as err:
            print(f"Decode error: {err}")
            data = bytearray()

def findPoint(x, y, rect_x, rect_y, rect_width, rect_height):
    if x >= rect_x and x <= rect_x + rect_width:
        if y >= rect_y and y <= rect_y + rect_height:
            return True
    return False



def startDrawBook():
    clearScreen(0)

    setColor(0x27e0)
    drawRectangle(0, 0, 60, 25)

    sendCommand(10, [1]) # set font
    setTextColor(0x00, 0x27e0)
    printText(2, 2, "Clear")

    setColor(0x713f) # Pen color

def drawBookMainLoop():
    while True:
        readCobs()
        touch = getTouch()
        if touch:
            touch_x, touch_y = touch
            if(findPoint(touch_x, touch_y, 0, 0, 60, 25)):
                startDrawBook()
                continue
            drawCircle(touch_x, touch_y, 3)


startDrawBook()
drawBookMainLoop()