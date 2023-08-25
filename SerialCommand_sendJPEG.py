import serial
from cobs import cobs
import time

import datetime

packetSize = 1024*60 #256


def sendCommand(commandByte, dataBytes):
    ser = serial.Serial('/dev/ttyUSB1', 2000000)
    # Build command message
    encoded_data = cobs.encode(bytearray([commandByte] + dataBytes))
    ser.write(encoded_data + b'\x00')
    ser.close()

def clearScreen(color):
    colorH = color >> 8
    colorL = color & 0x00FF
    sendCommand(4,  [colorH, colorL])

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


def printTimeDiff(name, start_time, end_time):
    delta = end_time - start_time
    delta_time = int(delta.total_seconds() * 1000) # milliseconds
    print(f"{name} time : {delta_time}ms")

def sendJPEG(filePath, pos_x, pos_y):
    with open(filePath, 'rb') as file:
        image_data = file.read()

    totalSize = len(image_data)
    print(f"Total size: {totalSize} bytes")

    pos_xH = pos_x >> 8
    pos_xL = pos_x & 0x00FF

    pos_yH = pos_y >> 8
    pos_yL = pos_y & 0x00FF

    fileSize_H = totalSize >> 8
    fileSize_L = totalSize & 0x00FF

    sendCommand(200, [pos_xH, pos_xL, pos_yH, pos_yL, fileSize_H, fileSize_L]) # Stream Starts

    # Split the file data into packets
    packets = [image_data[i:i+ (packetSize -2)] for i in range(0, len(image_data), (packetSize -2))]
    print(f"packets: {len(packets)}, len: {len(packets[0])}")

    ser = serial.Serial('/dev/ttyUSB1', 2000000)

    for i in packets:
        start_time = datetime.datetime.now() #🟡

        encoded_data = cobs.encode(i)

        enc_time = datetime.datetime.now() #🟡

        ser.write(encoded_data + b'\x00')
        
        write_time = datetime.datetime.now() #🟡
        
        while True:
            ret = ser.read_until(b'\x00')
            ret = ret[:-1]
            if(cobs.decode(ret) == b'OK'):
                break
        end_time = datetime.datetime.now() #🟡
    
    printTimeDiff("Total", start_time, end_time)
    printTimeDiff("Enc", start_time, enc_time)
    printTimeDiff("Write", enc_time, write_time)
    printTimeDiff("Ack wait", write_time, end_time)
    ser.close()


    sendCommand(201, [0xff, 0xff, 0xff]) # Stream Ends 


def sendRaw(image_data, pos_x, pos_y):

    totalSize = len(image_data)
    print(f"Total size: {totalSize} bytes")

    pos_xH = pos_x >> 8
    pos_xL = pos_x & 0x00FF

    pos_yH = pos_y >> 8
    pos_yL = pos_y & 0x00FF

    fileSize_H = totalSize >> 8
    fileSize_L = totalSize & 0x00FF

    sendCommand(200, [pos_xH, pos_xL, pos_yH, pos_yL, fileSize_H, fileSize_L]) # Stream Starts

    # Split the file data into packets
    packets = [image_data[i:i+ (packetSize -2)] for i in range(0, len(image_data), (packetSize -2))]
    print(f"packets: {len(packets)}, len: {len(packets[0])}")

    ser = serial.Serial('/dev/ttyUSB1', 2000000)

    for i in packets:
        start_time = datetime.datetime.now() #🟡

        encoded_data = cobs.encode(i)

        enc_time = datetime.datetime.now() #🟡

        ser.write(encoded_data + b'\x00')
        
        write_time = datetime.datetime.now() #🟡
        
        while True:
            ret = ser.read_until(b'\x00')
            ret = ret[:-1]
            if(cobs.decode(ret) == b'OK'):
                break
        end_time = datetime.datetime.now() #🟡
    
    printTimeDiff("Total", start_time, end_time)
    printTimeDiff("Enc", start_time, enc_time)
    printTimeDiff("Write", enc_time, write_time)
    printTimeDiff("Ack wait", write_time, end_time)
    ser.close()

    sendCommand(201, [0xff, 0xff, 0xff]) # Stream Ends 


# clearScreen(0xff)
# sendJPEG('image1.jpg', 0, 0)
# sendJPEG('test.txt')
# sendJPEG('test2.txt')

# sendCommand(201, [0xff, 0xff, 0xff])

# ser.close()