import serial
from cobs import cobs
import time
import random


class Display:
    def __init__(self, port, baudrate):
        self.port = port
        self.baudrate = baudrate
        self.packetSize = 1024*60 #256
        self.ser = serial.Serial(self.port, self.baudrate)

    def start(self, port, baudrate):
        self.port = port
        self.baudrate = baudrate
        self.ser = serial.Serial(self.port, self.baudrate)
    
    def end(self):
        self.ser.close()

    def sendCommand(self, commandByte, dataBytes):
        # Build command message
        encoded_data = cobs.encode(bytearray([commandByte] + dataBytes))
        self.ser.write(encoded_data + b'\x00')

    def drawToSprite(self):
        self.sendCommand(202, [])
    
    def pushSprite(self):
        self.sendCommand(203, [])
    
    def sendJPEG(self, image_data, pos_x, pos_y):
        totalSize = len(image_data)
        print(f"Total size: {totalSize} bytes")

        pos_xH = pos_x >> 8
        pos_xL = pos_x & 0x00FF

        pos_yH = pos_y >> 8
        pos_yL = pos_y & 0x00FF

        fileSize_H = totalSize >> 8
        fileSize_L = totalSize & 0x00FF

        self.sendCommand(200, [pos_xH, pos_xL, pos_yH, pos_yL, fileSize_H, fileSize_L]) # Stream Starts

        # Split the file data into packets
        packets = [image_data[i:i+ (self.packetSize -2)] for i in range(0, len(image_data), (self.packetSize -2))]
        print(f"packets: {len(packets)}, len: {len(packets[0])}")

        ser = serial.Serial('/dev/ttyUSB1', 2000000)

        for i in packets:
            encoded_data = cobs.encode(i)
            ser.write(encoded_data + b'\x00')
            while True:
                ret = ser.read_until(b'\x00')
                ret = ret[:-1]
                if(cobs.decode(ret) == b'OK'):
                    break
        self.sendCommand(201, [0xff, 0xff, 0xff]) # Stream Ends 
    
    def reset(self):
        self.sendCommand(1, [])

    def setBacklight(self, value):
        self.sendCommand(2, [value])
    
    def setRotation(self, value):
        self.sendCommand(3, [value])
    
    def fillScreen(self, color):
        colorH = color >> 8
        colorL = color & 0x00FF
        self.sendCommand(4, [colorH, colorL])
    
    def setColor(self, color):
        colorH = color >> 8
        colorL = color & 0x00FF
        self.sendCommand(7, [colorH, colorL])
    
    def setTextColor(self, fgColor, bgColor, fill=False):
        fgColorH = fgColor >> 8
        fgColorL = fgColor & 0x00FF
        bgColorH = bgColor >> 8
        bgColorL = bgColor & 0x00FF
        self.sendCommand(8, [fgColorH, fgColorL, bgColorH, bgColorL, fill])
    
    def setTextSize(self, size):
        self.sendCommand(9, [size])

    def setFont(self, font_no):
        self.sendCommand(10, [font_no])

    def printText(self, x, y, textString):
        xH = x >> 8
        xL = x & 0x00FF
        yH = y >> 8
        yL = y & 0x00FF

        dataBytes = [xH, xL, yH, yL]

        textByte = [ord(x) for x in textString]
        # textByte = ord(textString) 
        dataBytes += textByte

        self.sendCommand(12, dataBytes)
    
    def drawVline(self, x, y, height):
        xH = x >> 8
        xL = x & 0x00FF
        yH = y >> 8
        yL = y & 0x00FF
        heightH = height >> 8
        heightL = height & 0x00FF
        self.sendCommand(13, [xH, xL, yH, yL, heightH, heightL])

    def drawHline(self, x, y, width):
        xH = x >> 8
        xL = x & 0x00FF
        yH = y >> 8
        yL = y & 0x00FF
        widthH = width >> 8
        widthL = width & 0x00FF
        self.sendCommand(14, [xH, xL, yH, yL, widthH, widthL])

    def drawLine(self, x1, y1, x2, y2):
        x1H = x1 >> 8
        x1L = x1 & 0x00FF
        y1H = y1 >> 8
        y1L = y1 & 0x00FF
        x2H = x2 >> 8
        x2L = x2 & 0x00FF
        y2H = y2 >> 8
        y2L = y2 & 0x00FF
        self.sendCommand(15, [x1H, x1L, y1H, y1L, x2H, x2L, y2H, y2L])
    
    def drawRectangle(self, x, y, w, h):
        xH = x >> 8
        xL = x & 0x00FF
        yH = y >> 8
        yL = y & 0x00FF

        wH = w >> 8
        wL = w & 0x00FF
        hH = h >> 8
        hL = h & 0x00FF
        self.sendCommand(16, [xH, xL, yH, yL, wH, wL, hH, hL])
    
    def fillRectangle(self, x, y, w, h):
        xH = x >> 8
        xL = x & 0x00FF
        yH = y >> 8
        yL = y & 0x00FF

        wH = w >> 8
        wL = w & 0x00FF
        hH = h >> 8
        hL = h & 0x00FF
        self.sendCommand(17, [xH, xL, yH, yL, wH, wL, hH, hL])

    def drawCircle(self, x, y, r):
        xH = x >> 8
        xL = x & 0x00FF
        yH = y >> 8
        yL = y & 0x00FF

        rH = r >> 8
        rL = r & 0x00FF
        self.sendCommand(18, [xH, xL, yH, yL, rH, rL])

    def fillCircle(self, x, y, r):
        xH = x >> 8
        xL = x & 0x00FF
        yH = y >> 8
        yL = y & 0x00FF

        rH = r >> 8
        rL = r & 0x00FF
        self.sendCommand(19, [xH, xL, yH, yL, rH, rL])

    def drawTriangle(self, x1, y1, x2, y2, x3, y3):
        x1H = x1 >> 8
        x1L = x1 & 0x00FF
        y1H = y1 >> 8
        y1L = y1 & 0x00FF

        x2H = x2 >> 8
        x2L = x2 & 0x00FF
        y2H = y2 >> 8
        y2L = y2 & 0x00FF

        x3H = x3 >> 8
        x3L = x3 & 0x00FF
        y3H = y3 >> 8
        y3L = y3 & 0x00FF

        self.sendCommand(20, [x1H, x1L, y1H, y1L, x2H, x2L, y2H, y2L, x3H, x3L, y3H, y3L])

    def fillTriangle(self, x1, y1, x2, y2, x3, y3):
        x1H = x1 >> 8
        x1L = x1 & 0x00FF
        y1H = y1 >> 8
        y1L = y1 & 0x00FF

        x2H = x2 >> 8
        x2L = x2 & 0x00FF
        y2H = y2 >> 8
        y2L = y2 & 0x00FF

        x3H = x3 >> 8
        x3L = x3 & 0x00FF
        y3H = y3 >> 8
        y3L = y3 & 0x00FF

        self.sendCommand(21, [x1H, x1L, y1H, y1L, x2H, x2L, y2H, y2L, x3H, x3L, y3H, y3L])

    def drawArc(self, x, y, radius, innerRadius, startAngle, endAngle, roundEnds):
        xH = x >> 8
        xL = x & 0x00FF
        yH = y >> 8
        yL = y & 0x00FF

        rH = radius >> 8
        rL = radius & 0x00FF

        irH = innerRadius >> 8
        irL = innerRadius & 0x00FF

        saH = startAngle >> 8
        saL = startAngle & 0x00FF

        eaH = endAngle >> 8
        eaL = endAngle & 0x00FF

        self.sendCommand(22, [xH, xL, yH, yL, rH, rL, irH, irL, saH, saL, eaH, eaL, roundEnds])

    def setBgColor(self, color):
        colorH = color >> 8
        colorL = color & 0x00FF
        self.sendCommand(23, [colorH, colorL])

    def drawSmoothCircle(self, x, y, r):
        xH = x >> 8
        xL = x & 0x00FF
        yH = y >> 8
        yL = y & 0x00FF

        rH = r >> 8
        rL = r & 0x00FF
        self.sendCommand(24, [xH, xL, yH, yL, rH, rL])

    def fillSmoothCircle(self, x, y, r):
        xH = x >> 8
        xL = x & 0x00FF
        yH = y >> 8
        yL = y & 0x00FF

        rH = r >> 8
        rL = r & 0x00FF
        self.sendCommand(25, [xH, xL, yH, yL, rH, rL])

    def drawSmoothRoundRect(self, x, y, radius, innerRadius, width, height):
        xH = x >> 8
        xL = x & 0x00FF
        yH = y >> 8
        yL = y & 0x00FF

        rH = radius >> 8
        rL = radius & 0x00FF

        irH = innerRadius >> 8
        irL = innerRadius & 0x00FF

        wH = width >> 8
        wL = width & 0x00FF

        hH = height >> 8
        hL = height & 0x00FF

        self.sendCommand(26, [xH, xL, yH, yL, rH, rL, irH, irL, wH, wL, hH, hL])

    def fillSmoothRoundRect(self, x, y, width, height, radius):
        xH = x >> 8
        xL = x & 0x00FF
        yH = y >> 8
        yL = y & 0x00FF

        wH = width >> 8
        wL = width & 0x00FF

        hH = height >> 8
        hL = height & 0x00FF

        rH = radius >> 8
        rL = radius & 0x00FF

        self.sendCommand(27, [xH, xL, yH, yL, wH, wL, hH, hL, rH, rL])

    def drawSmoothArc(self, x, y, radius, innerRadius, startAngle, endAngle, roundEnds):
        xH = x >> 8
        xL = x & 0x00FF
        yH = y >> 8
        yL = y & 0x00FF

        rH = radius >> 8
        rL = radius & 0x00FF

        irH = innerRadius >> 8
        irL = innerRadius & 0x00FF

        saH = startAngle >> 8
        saL = startAngle & 0x00FF

        eaH = endAngle >> 8
        eaL = endAngle & 0x00FF

        self.sendCommand(28, [xH, xL, yH, yL, rH, rL, irH, irL, saH, saL, eaH, eaL, roundEnds])

    def drawWideLine(self, x1, y1, x2, y2, width):
        x1H = x1 >> 8
        x1L = x1 & 0x00FF
        y1H = y1 >> 8
        y1L = y1 & 0x00FF

        x2H = x2 >> 8
        x2L = x2 & 0x00FF
        y2H = y2 >> 8
        y2L = y2 & 0x00FF

        wH = width >> 8
        wL = width & 0x00FF

        self.sendCommand(29, [x1H, x1L, y1H, y1L, x2H, x2L, y2H, y2L, wH, wL])

    def drawWedgeLine(self, x1, y1, x2, y2, startWidth, endWidth):
        x1H = x1 >> 8
        x1L = x1 & 0x00FF
        y1H = y1 >> 8
        y1L = y1 & 0x00FF

        x2H = x2 >> 8
        x2L = x2 & 0x00FF
        y2H = y2 >> 8
        y2L = y2 & 0x00FF

        swH = startWidth >> 8
        swL = startWidth & 0x00FF
        ewH = endWidth >> 8
        ewL = endWidth & 0x00FF

        self.sendCommand(30, [x1H, x1L, y1H, y1L, x2H, x2L, y2H, y2L, swH, swL, ewH, ewL])
