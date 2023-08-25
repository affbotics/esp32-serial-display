import os
from io import BytesIO
from dotenv import load_dotenv
import requests

import threading

import spotipy  
from spotipy.oauth2 import SpotifyOAuth

from pprint import pprint
from time import sleep

import SerialCommand_sendJPEG

from PIL import Image

def resize_image(image_bytes, width):
    # Open the image from bytes
    image = Image.open(BytesIO(image_bytes))

    # Calculate the aspect ratio
    aspect_ratio = image.width / image.height

    # Determine the new height based on the width while preserving the aspect ratio
    height = int(width / aspect_ratio)

    # Resize the image
    resized_image = image.resize((width, height))

    # Create an in-memory byte stream to save the resized image
    output_bytes = BytesIO()
    resized_image.save(output_bytes, format='JPEG')
    output_bytes.seek(0)

    return output_bytes.read()

def findPoint(x, y, rect_x, rect_y, rect_width, rect_height):
    if x >= rect_x and x <= rect_x + rect_width:
        if y >= rect_y and y <= rect_y + rect_height:
            return True
    return False
5495

def getTouch():
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

timer = threading.Timer(0.1, getTouch)
timer.start()  # after 0.1 seconds, 'callback' will be called


load_dotenv("./.env")

CLIENT_ID = os.getenv('CLIENT_ID')
CLIENT_SECRET = os.getenv('CLIENT_SECRET')
REDIRECT_URI = os.getenv('REDIRECT_URI')

SCOPE = "user-read-playback-state,user-modify-playback-state"


sp = spotipy.Spotify(auth_manager=SpotifyOAuth(client_id=CLIENT_ID,
                                               client_secret=CLIENT_SECRET,
                                               redirect_uri=REDIRECT_URI,
                                               scope=SCOPE))


prv_track = ""
handle_pos = 0

# Shows playing devices
res = sp.devices()
pprint(res)

# Change track
# sp.start_playback(uris=['spotify:track:6gdLoMygLsgktydTQ71b15'])

trackDetails = sp.currently_playing()

print("#######")

pprint(trackDetails)

print("#######")

coverArt   = trackDetails['item']['album']['images'][1]['url']
trackName  = trackDetails['item']['name']
artistName = trackDetails['item']['artists']
trackLength = trackDetails['item']['duration_ms']
progress = trackDetails['progress_ms']

completedRatio = (progress/trackLength)


response = requests.get(coverArt)
print(type(response.content))

resizedImg = resize_image(response.content, 150)

bar_Y_pos = 215

SerialCommand_sendJPEG.clearScreen(0)

SerialCommand_sendJPEG.sendCommand(10, [1]) # set font
SerialCommand_sendJPEG.sendCommand(8, [0xff, 0xff, 0x00, 0x00]) # Set font color
SerialCommand_sendJPEG.printText(10, 5, trackName)

SerialCommand_sendJPEG.sendRaw(resizedImg, 10, 45)
SerialCommand_sendJPEG.sendJPEG("control_icon_rgb.jpg",170 , 40)

SerialCommand_sendJPEG.setColor(0xa554)
SerialCommand_sendJPEG.drawRectangle(40, bar_Y_pos, 240, 7)
SerialCommand_sendJPEG.drawCircle(280, bar_Y_pos + 3, 3)

handle_pos = 40 + int(completedRatio*240)

SerialCommand_sendJPEG.setColor(0xffff)
SerialCommand_sendJPEG.drawRectangle(40, bar_Y_pos, int(completedRatio*240), 7)
SerialCommand_sendJPEG.drawCircle(handle_pos, bar_Y_pos + 4, 8)

SerialCommand_sendJPEG.setColor(0xffff)
SerialCommand_sendJPEG.drawCircle(40, bar_Y_pos + 3, 3)

prv_track = trackName



while True:
    sleep(1)
    trackDetails = sp.currently_playing()
    coverArt   = trackDetails['item']['album']['images'][1]['url']
    trackName  = trackDetails['item']['name']
    artistName = trackDetails['item']['artists']
    trackLength = trackDetails['item']['duration_ms']
    progress = trackDetails['progress_ms']
    completedRatio = (progress/trackLength)

    if prv_track != trackName:
        prv_track = trackName
        response = requests.get(coverArt)
        resizedImg = resize_image(response.content, 150)
        SerialCommand_sendJPEG.clearScreen(0)

        SerialCommand_sendJPEG.sendCommand(10, [1]) # set font
        SerialCommand_sendJPEG.sendCommand(8, [0xff, 0xff, 0x00, 0x00]) # Set font color
        SerialCommand_sendJPEG.printText(10, 5, trackName)

        SerialCommand_sendJPEG.sendRaw(resizedImg, 10, 45)
        SerialCommand_sendJPEG.sendJPEG("control_icon_rgb.jpg",170 , 40)

        # Draw the base bar
        SerialCommand_sendJPEG.setColor(0xa554)
        SerialCommand_sendJPEG.drawRectangle(40, bar_Y_pos, 240, 7)
        SerialCommand_sendJPEG.drawCircle(280, bar_Y_pos + 3, 3)

        SerialCommand_sendJPEG.setColor(0xff) # white end point at start of the bar
        SerialCommand_sendJPEG.drawCircle(40, bar_Y_pos + 3, 3)
        handle_pos = 0

    SerialCommand_sendJPEG.setColor(0) # erase the old handle
    SerialCommand_sendJPEG.drawCircle(handle_pos, bar_Y_pos + 4, 8)
    # SerialCommand_sendJPEG.drawRectangle(30, bar_Y_pos - 5, 260, 20)

    handle_pos = 40 + int(completedRatio*240)
    SerialCommand_sendJPEG.setColor(0xffff)
    SerialCommand_sendJPEG.drawCircle(handle_pos, bar_Y_pos + 4, 8)
    SerialCommand_sendJPEG.drawRectangle(40, bar_Y_pos, int(completedRatio*240), 7)
    SerialCommand_sendJPEG.drawCircle(40, bar_Y_pos + 3, 3)# white end point at start of the bar