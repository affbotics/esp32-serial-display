// 🟡 TODO: Move `touch_calibrate()`, `jpegInfo()`, `renderJPEG()`, and `drawArrayJpeg()` into DispFunction class

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <JPEGDecoder.h>
#include <SPI.h>
#include <PacketSerial.h>
#include "FS.h"
#include "LittleFS.h"

#include "commands.h"
#include "DispFunctions.h"

#include "config.h"

#include <FastLED.h>

// #include <HardwareSerial.h>
// #include "USB.h"
// #define HWSerial Serial0
// #define USBSerial Serial
// #define HWSerial Serial
// USBCDC USBSerial;


// #include <Wire.h>
// TwoWire i2cWire = TwoWire(1);


TFT_eSPI    tft  = TFT_eSPI();
TFT_eSprite spr  = TFT_eSprite(&tft);

DispFunction disp = DispFunction(&tft, &spr);

CRGB pixel[1];
// Initialize PacketSerial library
// PacketSerial packetSerial;
PacketSerial_<COBS, 0, SERIAL_SIZE_RX> packetSerial;

// Global variables to store incoming command
bool commandReceived = false;
byte commandByte = 0;
byte numDataBytes = 0;
byte dataBytes[253];

int getBaudRate(){
  int baud = 0; 
  if (LittleFS.exists(BAUD_FILE)){
    fs::File f = LittleFS.open(BAUD_FILE, "r");
    if (f) {
      if (f.read((uint8_t*)&baud, sizeof(baud))){
        f.close();
        return baud;
      }
    }
    f.close();
  }
  return 0;
}

void saveBaud(int baud){
// store data
    fs::File f = LittleFS.open(BAUD_FILE, "w");
    if (f) {
      f.write((uint8_t*)&baud, sizeof(baud));
      f.close();
    }
}

void chagneBaud(uint8_t count, uint8_t* data){
  if(count != 1)
    return;
  int baudRate = baudRateList[data[0]];
  saveBaud(baudRate);

  Serial1.flush();
  Serial1.end();
  Serial1.begin(baudRate, SERIAL_8N1, RXD2, TXD2);
  delay(100);
  // Clear the serial buffer
  while(Serial1.available()){
    Serial1.read();
  }
}

void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!LittleFS.begin()) {
    Serial.println("Formating file system");
    LittleFS.format();
    LittleFS.begin();
  }

  // check if calibration file exists and size is correct
  if (LittleFS.exists(CALIBRATION_FILE)) {
    fs::File f = LittleFS.open(CALIBRATION_FILE, "r");
    if (f) {
      if (f.readBytes((char *)calData, 14) == 14)
        calDataOK = 1;
      f.close();
    }
  }

  if (calDataOK) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    fs::File f = LittleFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

bool     streamData = false;
uint16_t pos_x, pos_y = 0;
uint     fileSize = 0;

uint8_t* dataBuffer;

//####################################################################################################
// Return the minimum of two values a and b
#define minimum(a,b)  (((a) < (b)) ? (a) : (b))

// Print image information to the serial port (optional)
void jpegInfo() {
  Serial.println(F("==============="));
  Serial.println(F("JPEG image info"));
  Serial.println(F("==============="));
  Serial.print(F(  "Width      :")); Serial.println(JpegDec.width);
  Serial.print(F(  "Height     :")); Serial.println(JpegDec.height);
  Serial.print(F(  "Components :")); Serial.println(JpegDec.comps);
  Serial.print(F(  "MCU / row  :")); Serial.println(JpegDec.MCUSPerRow);
  Serial.print(F(  "MCU / col  :")); Serial.println(JpegDec.MCUSPerCol);
  Serial.print(F(  "Scan type  :")); Serial.println(JpegDec.scanType);
  Serial.print(F(  "MCU width  :")); Serial.println(JpegDec.MCUWidth);
  Serial.print(F(  "MCU height :")); Serial.println(JpegDec.MCUHeight);
  Serial.println(F("==============="));
}

// Draw a JPEG on the TFT, images will be cropped on the right/bottom sides if they do not fit
// This function assumes xpos,ypos is a valid screen coordinate. For convenience images that do not
// fit totally on the screen are cropped to the nearest MCU size and may leave right/bottom borders.
void renderJPEG(int xpos, int ypos) {

  // retrieve infomration about the image
  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
  // Typically these MCUs are 16x16 pixel blocks
  // Determine the width and height of the right and bottom edge image blocks
  uint32_t min_w = minimum(mcu_w, max_x % mcu_w);
  uint32_t min_h = minimum(mcu_h, max_y % mcu_h);

  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  // record the current time so we can measure how long it takes to draw an image
  uint32_t drawTime = millis();

  // save the coordinate of the right and bottom edges to assist image cropping
  // to the screen size
  max_x += xpos;
  max_y += ypos;

  // read each MCU block until there are no more
  while (JpegDec.readSwappedBytes()) {
	  
    // save a pointer to the image block
    pImg = JpegDec.pImage ;

    // calculate where the image block should be drawn on the screen
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;  // Calculate coordinates of top left corner of current MCU
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
    else win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
    else win_h = min_h;

    // copy pixels into a contiguous block
    if (win_w != mcu_w)
    {
      uint16_t *cImg;
      int p = 0;
      cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++)
      {
        p += mcu_w;
        for (int w = 0; w < win_w; w++)
        {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }

    // draw image MCU block only if it will fit on the screen
    if (( mcu_x + win_w ) <= tft.width() && ( mcu_y + win_h ) <= tft.height())
    {
      tft.pushRect(mcu_x, mcu_y, win_w, win_h, pImg);
    }
    else if ( (mcu_y + win_h) >= tft.height()) JpegDec.abort(); // Image has run off bottom of screen so abort decoding
  }

  // calculate how long it took to draw the image
  drawTime = millis() - drawTime;

  // print the results to the serial port
  Serial.print(F(  "Total render time was    : ")); Serial.print(drawTime); Serial.println(F(" ms"));
  Serial.println(F(""));
}

// Draw a JPEG on the TFT pulled from a program memory array
void drawArrayJpeg(const uint8_t arrayname[], uint32_t array_size, int xpos, int ypos) {

  int x = xpos;
  int y = ypos;

  JpegDec.decodeArray(arrayname, array_size);
  
  jpegInfo(); // Print information from the JPEG file (could comment this line out)
  
  renderJPEG(x, y);
  
  Serial.println("#########################");
}
//####################################################################################################

unsigned long start_Time = 0;

void startStream(uint8_t count, uint8_t* data){
  if(count < 2){
    Serial.println("Not enough data, Unable to execute ");
    return;
  }

  streamData = true;
  pos_x        = (data[0]<<8) | data[1];
  pos_y        = (data[2]<<8) | data[3];
  fileSize     = (data[4]<<8) | data[5];
  // dataBuffer = (uint8_t*)calloc(totalPackets, 254); // One packet size is 254 bytes
  // dataBuffer = (uint8_t*)calloc(fileSize, 1);
  
  Serial.println("Stream starting");
  Serial.print(", File Size: ");
  Serial.print(fileSize);
  Serial.println("");
  start_Time = millis();
}

void processStream(const uint8_t* dataBuffer, uint size){

  packetSerial.send((const uint8_t*)"OK", 2); // Send Acknowledgement

  Serial.print("Time to transmit: ");
  Serial.println(millis() - start_Time);

  Serial.println("Stream End");

  // Serial.println("Raw data:");
  // for(uint i=0; i<fileSize; i++){
  //   if(i%16 == 0) Serial.println("");
    
  //   Serial.print(dataBuffer[i], HEX);
  //   Serial.print(" ");
  // }
  // Serial.println("");

  drawArrayJpeg(dataBuffer, fileSize, pos_x, pos_y);
  
  // Deleting the buffer and resetting the values
  // free(dataBuffer);
  streamData   = false;  // Stop the stream
  pos_x = pos_y = 0;

}

void processCommand(byte CommandByte, byte numDataBytes, byte *dataBytes){
  for(uint8_t i = 0; i < numDataBytes; i++){
    Serial.print(dataBytes[i], HEX);
    if(numDataBytes - i != 1)
      Serial.print(", ");
    else
      Serial.println("");
  }

  switch (commandByte) {
    case TEST:
        // code to execute when TEST command is received
        break;
    case RESET:
        // code to execute when RESET command is received
        ESP.restart();
        break;
    case SET_BACKLIGHT:
        disp.setBacklight(numDataBytes, dataBytes);
        break;
    case SET_ROTATION:
        disp.setRotation(numDataBytes, dataBytes);
        break;
    case FILL_SCREEN:
        Serial.println("Filling screen");
        disp.fillScreen(numDataBytes, dataBytes);
        break;
    case READ_CURSOR:
        // code to execute when READ_CURSOR command is received
        break;
    case SET_CURSOR:
        disp.setCursor(numDataBytes, dataBytes);
        break;
    case SET_COLOR:
        Serial.println("color set");
        disp.setColor(numDataBytes, dataBytes);
        break;
    case SET_TEXT_COLOR:
        Serial.println("Text color set");
        disp.setTextColor(numDataBytes, dataBytes);
    case SET_TEXT_SIZE:
        disp.setTextSize(numDataBytes, dataBytes);
        break;
    case SET_FONT:
        Serial.println("setting font");
        disp.setFont(numDataBytes, dataBytes);
        break;
    case PRINT_LN:
        // code to execute when PRINT_LN command is received
        // ? is this really needed? 🤔
        break;
    case PRINT:
        Serial.println("printing");
        disp.print(numDataBytes, dataBytes);
        break;
    case DRAW_V_LINE:
        Serial.println("v line");
        disp.drawVLine(numDataBytes, dataBytes);
        break;
    case DRAW_H_LINE:
        Serial.println("h line");
        disp.drawHLine(numDataBytes, dataBytes);
        break;
    case DRAW_LINE:
        disp.drawLine(numDataBytes, dataBytes);
        break;
    case DRAW_RECT:
        Serial.println("rect");
        disp.drawRect(numDataBytes, dataBytes);
        break;
    case FILL_RECT:
        Serial.println("fill rect");
        disp.fillRect(numDataBytes, dataBytes);
        break;
    case DRAW_CIRCLE:
        Serial.println("Draw circle");
        disp.drawCircle(numDataBytes, dataBytes);
        break;
    case FILL_CIRCLE:
        Serial.println("fill circle");
        disp.fillCircle(numDataBytes, dataBytes);
        break;
    case DRAW_TRIANGLE:
        disp.drawTriangle(numDataBytes, dataBytes);
        break;
    case FILL_TRIANGLE:
        disp.fillTriangle(numDataBytes, dataBytes);
        break;
    case DRAW_ARC:
        disp.drawArc(numDataBytes, dataBytes);
        break;
    case SET_BG_COLOR:
        disp.setBgColor(numDataBytes, dataBytes);
        break;
    case DRAW_SMOOTH_CIRCLE:
        disp.drawSmoothCircle(numDataBytes, dataBytes);
        break;
    case FILL_SMOOTH_CIRCLE:
        disp.fillSmoothCircle(numDataBytes, dataBytes);
        break;
    case DRAW_SMOOTH_ROUND_RECT:
        disp.drawSmoothRoundRect(numDataBytes, dataBytes);
        break;
    case FILL_SMOOTH_ROUND_RECT:
        disp.fillSmoothRoundRect(numDataBytes, dataBytes);
        break;
    case DRAW_SMOOTH_ARC:
        disp.drawSmoothArc(numDataBytes, dataBytes);
        break;
    case DRAW_WIDE_LINE:
        disp.drawWideLine(numDataBytes, dataBytes);
        break;
    case DRAW_WEDGE_LINE:
        disp.drawWedgeLine(numDataBytes, dataBytes);
        break;

    case STREAM_START:
        Serial.println("Incoming Stream");
        startStream(numDataBytes, dataBytes);
        break;
    case STREAM_END:
        Serial.println("201 Recieved");
        break;
    case DRAWTOSPRITE:
        disp.drawToSprite(numDataBytes, dataBytes);
        break;
    case PUSHSPRITE:
        disp.pushSprite(numDataBytes, dataBytes);
    
    case CHANGEBAUD:
        chagneBaud(numDataBytes, dataBytes);
        break;

    default:
        // code to execute when an unknown command is received
        Serial.print("Unknown Command: ");
        Serial.println(commandByte, HEX);
        break;
  }

}

void onPacketReceived(const uint8_t* buffer, size_t size) {
  // Check if the packet size is at least 1 byte

  // Serial.println("a packet recieved");

  if(size < 1){
    Serial.println("Invalid Data Packet Recieved!");
    Serial1.flush();
    return;
  }
  
  if(streamData){
    // If the incoming data is a serial file stream
    processStream(buffer, size);
    return;
  }
  Serial.println("");
  
  // Extract the command byte from the packet
  commandByte = buffer[0];
  
  // Calculate the number of data bytes in the packet
  numDataBytes = size - 1;
  
  // Check if the packet contains any data bytes
  if (numDataBytes > 0) {
    // Copy the data bytes into the dataBytes array
    for (int i = 0; i < numDataBytes; i++) {
      dataBytes[i] = buffer[i + 1];
    }
  }

  // Set the commandReceived flag to true
  commandReceived = true;
  Serial.print(size);
  Serial.println(" Packet recieved !");

  processCommand(commandByte, numDataBytes, dataBytes);
}

bool contains(int16_t x, int16_t y, int16_t _x1, int16_t _y1, int16_t _w, int16_t _h) {
  return ((x >= _x1) && (x < (_x1 + _w)) &&
          (y >= _y1) && (y < (_y1 + _h)));
}

/* this code will scan for all possible i2c address and 
try to find the device i2c id and print it*/

// void i2c_scan(){
//   i2cWire.begin(5, 19, 5000);
//   pinMode(20, OUTPUT);
//   digitalWrite(20, HIGH);
  
//   while(1){
//     byte error, address;
//   int nDevices;

//   Serial.println("Scanning...");

//   nDevices = 0;
//   for(address = 1; address < 127; address++ ) 
//   {
//     // The i2c_scanner uses the return value of
//     // the Write.endTransmisstion to see if
//     // a device did acknowledge to the address.
//     i2cWire.beginTransmission(address);
//     error = i2cWire.endTransmission();

//     if (error == 0)
//     {
//       Serial.print("I2C device found at address 0x");
//       if (address<16) 
//         Serial.print("0");
//       Serial.print(address,HEX);
//       Serial.println("  !");

//       nDevices++;
//     }
//     else if (error==4) 
//     {
//       Serial.print("Unknown error at address 0x");
//       if (address<16) 
//         Serial.print("0");
//       Serial.println(address,HEX);
//     }    
//   }
//   if (nDevices == 0)
//     Serial.println("No I2C devices found\n");
//   else
//     Serial.println("done\n");

//   delay(5000);           // wait 5 seconds for next scan
//   }

// }

void setup() {
  // Serial.setRxBufferSize(1024);   

  Serial.begin(115200);
  pinMode(POWER_EN_PIN, OUTPUT);
  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(POWER_EN_PIN, LOW);
  FastLED.addLeds<WS2812B, PIXEL_LED, GRB>(pixel, 1); 
  ledcSetup(BACKLIGHT_PWM, 5000, 8);
  ledcAttachPin(BACKLIGHT_PIN, BACKLIGHT_PWM);
  ledcWrite(BACKLIGHT_PWM, 200);

  // USBSerial.begin(115200);

  tft.begin();
  tft.setRotation(3);
  touch_calibrate();
  tft.fillScreen(0);

  disp.createSprite();

  Serial.println("getting the baud");

  int baudRate = getBaudRate();
  if(baudRate == 0){
    baudRate = SERIAL_BAUD_DEFAULT;
  }

  Serial1.setRxBufferSize(SERIAL_SIZE_RX);
  Serial1.begin(baudRate, SERIAL_8N1, RXD2, TXD2);

  // i2c_scan();

  packetSerial.setStream(&Serial1);
  packetSerial.setPacketHandler(&onPacketReceived);
  Serial.println("Packet serial init done.");
  Serial.println("Waiting for command in serial 2.");
  // tft.setCursor(13, 20);
  // tft.setFreeFont(&FreeSans9pt7b);
  // tft.print("Waiting for command.");

  char buff[50];
  sprintf(buff, "RX: G_%d, Tx: G_%d, Baud rate:%d", RXD2, TXD2, baudRate);
  Serial.println(buff);
  // tft.setCursor(13, 48);
  // tft.print(buff);
  // tft.setCursor(13, 86);
  // tft.print("Baud rate:");

  // tft.drawRect(105, 65, 100, 31, 0xffa0); 
  // // tft.drawSmoothRoundRect(105, 65, 6, 4, 90, 30, 0xffa0, 0, 0xff);
  // tft.setCursor(112, 86);
  // tft.print(baudRate);


  // tft.setTextFont(4);
  // tft.print("Hey there");

  // uint16_t t_x = 0, t_y = 0;
  // bool pressed = tft.getTouch(&t_x, &t_y);

}

uint32_t color_value;
int redValue = 0;
int greenValue = 0;
int blueValue = 0;

int redTarget = 255;
int greenTarget = 0;
int blueTarget = 0;

void ledUpdate(){
  if (redValue != redTarget) {
    if (redValue < redTarget) {
      redValue++;
    } else {
      redValue--;
    }
  }

  if (greenValue != greenTarget) {
    if (greenValue < greenTarget) {
      greenValue++;
    } else {
      greenValue--;
    }
  }

  if (blueValue != blueTarget) {
    if (blueValue < blueTarget) {
      blueValue++;
    } else {
      blueValue--;
    }
  }

  pixel[0]= CRGB(redValue, greenValue, blueValue);
  FastLED.show();

  if (redValue == redTarget && greenValue == greenTarget && blueValue == blueTarget) {
    // Randomly select the next target color
    redTarget = random(255);
    greenTarget = random(255);
    blueTarget = random(255);
  }

}

void loop() {
  // ledUpdate();
  pixel[0]= CRGB(0);
  FastLED.show();

  uint16_t t_x = 0, t_y = 0; // To store the touch coordinates
  bool pressed = tft.getTouch(&t_x, &t_y);
  tft.drawPixel(0, 0, 0xffff);

  if(pressed){
    Serial.print("Touch: ");
    Serial.print(t_x); Serial.print(", ");
    Serial.println(t_y);

    const uint8_t touch_command = 20;
    const uint8_t size = 5;
    uint8_t xH, xL = 0;
    uint8_t yH, yL = 0;

    xH = t_x >> 8;
    xL = t_x & 0x00FF;
    yH = t_y >> 8;
    yL = t_y & 0x00FF;

    uint8_t data[size] = {touch_command, xH, xL, yH, yL};

    packetSerial.send(data, size);
  }

  packetSerial.update();

  if (packetSerial.overflow())
  {
    Serial.println("Packet serial overflow");
  }

}
