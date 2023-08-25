import serial
from cobs import cobs
import time
import random

ser = serial.Serial('/dev/ttyUSB1', 2000000)

def sendCommand(commandByte, dataBytes):
    # Build command message
    encoded_data = cobs.encode(bytearray([commandByte] + dataBytes))
    ser.write(encoded_data + b'\x00')

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

def findPoint(x, y, rect_x, rect_y, rect_width, rect_height):
    if x >= rect_x and x <= rect_x + rect_width:
        if y >= rect_y and y <= rect_y + rect_height:
            return True
    return False

# set rotation
# sendCommand(3, [3])

# # # fill screen
# sendCommand(4, [0x0A, 0xF0])

# # time.sleep(0.1)
# # # # set color
# sendCommand(7, [0x07, 0xE0])
# # time.sleep(0.1)
# # # # draw rect
# sendCommand(17, [0, 70, 0, 50, 0, 40, 0, 54])
# # time.sleep(0.1)
# # # # set font
# sendCommand(10, [1])
# time.sleep(0.1)
# # # # set text color
# sendCommand(8, [0x07, 0xE0, 0x02, 0xF0])
# time.sleep(0.1)
# # # # print
# sendCommand(12, [0, 0, 0, 0, ord('a'), ord('p'), ord('p'), ord('l'), ord('e')])

# sendCommand(4, [0x0A, 0xF0])
# time.sleep(0.2)

# sendCommand(7, [0xea, 0x0d])
# time.sleep(0.2)

# sendCommand(17, [0, 50, 0, 150, 0, 70, 0, 50])
# time.sleep(0.2)
# sendCommand(7, [0x9f, 0x91])
# time.sleep(0.2)
# sendCommand(17, [0, 150, 0, 250, 0, 50, 0, 50])

# ser = serial.Serial('/dev/ttyUSB1', 115200)
# data = bytearray()

# decoded_data = bytearray()

score = 0

# fill screen
sendCommand(4, [0x00, 0x00])

# set color
sendCommand(7, [0x71, 0x3f])

# # draw rect
# drawRectangle(50, 80, 30, 30)

# set font
sendCommand(10, [1])

# set font color
sendCommand(8, [0x07, 0xE0, 0x00, 0x00])

printText(0, 0, f"Score : 0")

touched = False
def getTouch():
    global touched
    global t_x 
    global t_y

    if touched:
        touched = False
        touch = (t_x, t_y)
        return touch
    return False

data = bytearray()
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




def draw_random_rectangle():
    score = 0
    # Generate random coordinates for the rectangle
    x = random.randint(0, 300)  # Adjusted to fit within (0, 320)
    y = random.randint(30, 220)  # Adjusted to fit within (0, 240)

    # Generate random width and height for the rectangle
    width = random.randint(30, 100)
    height = random.randint(30, 100)

    # Draw the initial rectangle
    drawRectangle(x, y, width, height)

    while True:
        readCobs()
        touch = getTouch()
        if touch:
            print(touch)
            touch_x, touch_y = touch
            # Check if the touch is within the rectangle
            if x <= touch_x <= x + width and y <= touch_y <= y + height:
                # Draw a black rectangle in place of the previous one
                setColor(0)
                drawRectangle(x, y, width, height)
                
                # Erase text on top left
                setColor(100)
                drawRectangle(0, 0, 150, 20)

                # Generate new random coordinates for the next rectangle
                x = random.randint(0, 300)  # Adjusted to fit within (0, 320)
                y = random.randint(30, 220)  # Adjusted to fit within (0, 240)

                # Generate new random width and height for the next rectangle
                width = random.randint(30, 100)
                height = random.randint(30, 100)

                # Draw the next rectangle
                setColor(random.randint(300, 65535))
                drawRectangle(x, y, width, height)
                # print(f'x:{x}, y:{y}, w:{width}, h:{height}')
                score +=1
                printText(0, 0, f"Score : {score}")
                print("\n")
                time.sleep(0.5)

    
draw_random_rectangle()

ser.close()