#ifndef DISP_FUNCTIONS_H
#define DISP_FUNCTIONS_H

#include <TFT_eSPI.h>


class DispFunction {
public:
  DispFunction(TFT_eSPI* tft, TFT_eSprite* spr);

  void fillScreen(uint8_t count, byte * data);

  void drawToSprite(uint8_t count, uint8_t* data);
  void pushSprite(uint8_t count, uint8_t* data);
  void createSprite();

  void test(uint8_t count, uint8_t* data);
  void reset(uint8_t count, uint8_t* data);
  void setBacklight(uint8_t count, uint8_t* data);
  void setRotation(uint8_t count, uint8_t* data);
  // void fillScreen(uint8_t count, uint8_t* data);
  void readCursor(uint8_t count, uint8_t* data);
  void setCursor(uint8_t count, uint8_t* data);
  void setColor(uint8_t count, uint8_t* data);
  void setTextColor(uint8_t count, uint8_t* data);
  void setTextSize(uint8_t count, uint8_t* data);
  void setFont(uint8_t count, uint8_t* data);
  void printLn(uint8_t count, uint8_t* data);
  void print(uint8_t count, uint8_t* data);
  void drawVLine(uint8_t count, uint8_t* data);
  void drawHLine(uint8_t count, uint8_t* data);
  void drawLine(uint8_t count, uint8_t* data);
  void drawRect(uint8_t count, uint8_t* data);
  void fillRect(uint8_t count, uint8_t* data);
  void drawCircle(uint8_t count, uint8_t* data);
  void fillCircle(uint8_t count, uint8_t* data);

  void drawTriangle(uint8_t count, uint8_t* data);
  void fillTriangle(uint8_t count, uint8_t* data);
  void drawArc(uint8_t count, uint8_t* data);

  void setBgColor(uint8_t count, uint8_t* data);
  void drawSmoothCircle(uint8_t count, uint8_t* data);
  void fillSmoothCircle(uint8_t count, uint8_t* data);
  void drawSmoothRoundRect(uint8_t count, uint8_t* data);
  void fillSmoothRoundRect(uint8_t count, uint8_t* data);
  void drawSmoothArc(uint8_t count, uint8_t* data);

  void drawWideLine(uint8_t count, uint8_t* data);
  void drawWedgeLine(uint8_t count, uint8_t* data);

private:
  TFT_eSPI* tft_;
  TFT_eSprite* spr_;

  bool targetSprite_ = false;

  uint16_t cursor_x_ = 0;
  uint16_t cursor_y_ = 0;

  uint16_t setColor_ = 0;
  uint16_t set_bg_color_ = 0;
  // uint16_t text_bg_color_ = 0;
  uint8_t textSize_   = 0;
  uint8_t textFont_   = 0;

};

#endif // DISP_FUNCTIONS_H