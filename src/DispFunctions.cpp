#include "DispFunctions.h"
#include "config.h"

DispFunction::DispFunction(TFT_eSPI* tft, TFT_eSprite* spr) {
  tft_ = tft;
  spr_ = spr;
}

void DispFunction:: createSprite() {
  spr_->setAttribute(PSRAM_ENABLE, true);
  
  // Fix for rotation
  if(tft_->getRotation() %2){
    spr_->createSprite(tft_->width(), tft_->height());
  }else{
    spr_->createSprite(tft_->height(), tft_->width());
  }
}

void DispFunction::drawToSprite(uint8_t count, uint8_t* data) {
  if(spr_->created())
    targetSprite_ = true;
}

void DispFunction::pushSprite(uint8_t count, uint8_t* data) {
  targetSprite_ = false;
  spr_->pushSprite(0, 0);
  Serial.print("width: ");
  Serial.println(spr_->width());
  Serial.print("height: ");
  Serial.println(spr_->height());
  // spr_->pushRotated(90);
}

void DispFunction::test(uint8_t count, uint8_t* data) {
    // do something for TEST
}

void DispFunction::reset(uint8_t count, uint8_t* data) {
    // do something for RESET
}

void DispFunction::setBacklight(uint8_t count, uint8_t* data) {
  const uint8_t numArgsRequired = 1;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }
  ledcWrite(BACKLIGHT_PWM, data[0]);
  
}

void DispFunction::setRotation(uint8_t count, uint8_t* data) {
  const uint8_t numArgsRequired = 1;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }
  tft_->setRotation(data[0]);
}

void DispFunction::fillScreen(uint8_t count, byte * data){
  const uint8_t numArgsRequired = 2;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute fillScreen");
    return;
  }
  (targetSprite_) ? spr_->fillSprite((data[0]<<8) | data[1]):
                    tft_->fillScreen((data[0]<<8) | data[1]);

  Serial.println((data[0]<<8) | data[1], HEX);

}

void DispFunction::readCursor(uint8_t count, uint8_t* data) {
    // do something for READ_CURSOR
}

void DispFunction::setCursor(uint8_t count, uint8_t* data) {
  const uint8_t numArgsRequired = 4;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }

  uint8_t i = 0;
  if(targetSprite_){
    spr_->setCursor((data[i++]<<8) | data[i++],
                    (data[i++]<<8) | data[i++]);
  }else{
    tft_->setCursor((data[i++]<<8) | data[i++],
                    (data[i++]<<8) | data[i++]);
  }
}

void DispFunction::setColor(uint8_t count, uint8_t* data) {
  const uint8_t numArgsRequired = 2;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }
  setColor_ = (data[0]<<8) | data[1];

}

void DispFunction::setTextColor(uint8_t count, uint8_t* data) {
  const uint8_t numArgsRequired = 5;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }
  uint8_t i =0;

  if(targetSprite_){
    spr_->setTextColor((data[i++]<<8) | data[i++], (data[i++]<<8) | data[i++], data[i++]);
  }else{
    tft_->setTextColor((data[i++]<<8) | data[i++], (data[i++]<<8) | data[i++], data[i++]);
  }

}

void DispFunction::setTextSize(uint8_t count, uint8_t* data) {
  const uint8_t numArgsRequired = 1;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }

  textSize_ = data[0];

  if(targetSprite_){
    spr_->setTextSize(textSize_);
  }else{
    tft_->setTextSize(textSize_);
  }

}

const GFXfont * getFont(uint8_t font_no){
  switch(font_no){
    case 0:
      return(&FreeSans9pt7b);
      break;
    case 1:
      return(&FreeSans12pt7b);
      break;
    case 2:
      return(&FreeSans18pt7b);
      break;
    case 3:
      return(&FreeSans24pt7b);
      break;
    default:
      Serial.println("Invalid font");
      return(&TomThumb);
      break;
  }
}

void DispFunction::setFont(uint8_t count, uint8_t* data) {
  const uint8_t numArgsRequired = 1;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }
  
  if(targetSprite_){
    spr_->setFreeFont(getFont(data[0]));
  }else{
    tft_ ->setFreeFont(getFont(data[0]));
  }
  
}

void DispFunction::printLn(uint8_t count, uint8_t* data) {
    const uint8_t minArgsRequired = 2;
  
  if(count < minArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }

  char str[count - 2];
  for(uint8_t i=2; i < count; i++){
    str[i - 2] = data[i];
  }

  if(targetSprite_){
    spr_->drawString(str, data[0], data[1]);
  }else{
    tft_->drawString(str, data[0], data[1]);
  }
  
}

void DispFunction::print(uint8_t count, uint8_t* data) {
  const uint8_t minArgsRequired = 4;
  
  if(count < minArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }

  char str[count - 4 + 1]; // +1 for '\0'

  for(uint8_t i=4; i < count; i++){
    str[i - 4] = data[i];
  }
  str[count - 4 ] = '\0'; // NULL terminator

  Serial.print(" String :");
  Serial.println(str);

  uint8_t i=0;

  if(targetSprite_){
    spr_->drawString(str, (data[i++]<<8) | data[i++], (data[i++]<<8) | data[i++]);
  }else{
    tft_->drawString(str, (data[i++]<<8) | data[i++], (data[i++]<<8) | data[i++]);
  }
  
}

void DispFunction::drawVLine(uint8_t count, uint8_t* data) {
  const uint8_t numArgsRequired = 6;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }
  uint8_t i = 0;
  if(targetSprite_){
    spr_->drawFastVLine((data[i++]<<8) | data[i++], // x
                      (data[i++]<<8) | data[i++], // y
                      (data[i++]<<8) | data[i++], // height
                      setColor_);// color
  }else{
    tft_->drawFastVLine((data[i++]<<8) | data[i++], // x
                      (data[i++]<<8) | data[i++], // y
                      (data[i++]<<8) | data[i++], // height
                      setColor_);// color
  }
}

void DispFunction::drawHLine(uint8_t count, uint8_t* data) {
  const uint8_t numArgsRequired = 6;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }
  uint8_t i = 0;
  if(targetSprite_){
    spr_->drawFastHLine((data[i++]<<8) | data[i++], // x
                      (data[i++]<<8) | data[i++], // y
                      (data[i++]<<8) | data[i++], // width
                      setColor_);// color
  }else{
    tft_->drawFastHLine((data[i++]<<8) | data[i++], // x
                      (data[i++]<<8) | data[i++], // y
                      (data[i++]<<8) | data[i++], // width
                      setColor_);// color
  }
}


void DispFunction::drawLine(uint8_t count, uint8_t* data) {
  const uint8_t numArgsRequired = 8;

  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }

  uint8_t i = 0;
  if(targetSprite_){
    spr_->drawLine((data[i++]<<8) | data[i++], // x1
                 (data[i++]<<8) | data[i++], // y1
                 (data[i++]<<8) | data[i++], // x2
                 (data[i++]<<8) | data[i++], // y2
                 setColor_);// color
  }else{
    tft_->drawLine((data[i++]<<8) | data[i++], // x1
                 (data[i++]<<8) | data[i++], // y1
                 (data[i++]<<8) | data[i++], // x2
                 (data[i++]<<8) | data[i++], // y2
                 setColor_);// color
  }

}

void DispFunction::drawRect(uint8_t count, uint8_t* data) {
  const uint8_t numArgsRequired = 8;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }

  uint8_t i = 0;
  if(targetSprite_){
    spr_->drawRect((data[i++]<<8) | data[i++], // x
                 (data[i++]<<8) | data[i++], // y
                 (data[i++]<<8) | data[i++], // width
                 (data[i++]<<8) | data[i++], // height
                 setColor_);// color
  }else{
    tft_->drawRect((data[i++]<<8) | data[i++], // x
                 (data[i++]<<8) | data[i++], // y
                 (data[i++]<<8) | data[i++], // width
                 (data[i++]<<8) | data[i++], // height
                 setColor_);// color
  }
}

void DispFunction::fillRect(uint8_t count, uint8_t* data) {
  const uint8_t numArgsRequired = 8;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }
  
  uint8_t i = 0;

  if(targetSprite_){
    spr_->fillRect((data[i++]<<8) | data[i++], // x
                  (data[i++]<<8) | data[i++], // y
                  (data[i++]<<8) | data[i++], // width
                  (data[i++]<<8) | data[i++], // height
                  setColor_);// color
  }else{
    tft_->fillRect((data[i++]<<8) | data[i++], // x
                  (data[i++]<<8) | data[i++], // y
                  (data[i++]<<8) | data[i++], // width
                  (data[i++]<<8) | data[i++], // height
                  setColor_);// color
  }

}

void DispFunction::drawCircle(uint8_t count, uint8_t* data){
  const uint8_t numArgsRequired = 6;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }
  uint8_t i=0;
  if(targetSprite_){
    spr_->drawCircle((data[i++]<<8) | data[i++], // x
                     (data[i++]<<8) | data[i++], // y
                     (data[i++]<<8) | data[i++], // r
                     setColor_);  //color
  }else{
    tft_->drawCircle((data[i++]<<8) | data[i++], // x
                     (data[i++]<<8) | data[i++], // y
                     (data[i++]<<8) | data[i++], // r
                     setColor_);  //color
  }

}

void DispFunction::fillCircle(uint8_t count, uint8_t* data){
  const uint8_t numArgsRequired = 6;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }
  uint8_t i=0;
  if(targetSprite_){
    spr_->fillCircle((data[i++]<<8) | data[i++], // x
                     (data[i++]<<8) | data[i++], // y
                     (data[i++]<<8) | data[i++], // r
                     setColor_);  //color
  }else{
    tft_->fillCircle((data[i++]<<8) | data[i++], // x
                     (data[i++]<<8) | data[i++], // y
                     (data[i++]<<8) | data[i++], // r
                    setColor_);  //color
  }

}

void DispFunction::drawTriangle(uint8_t count, uint8_t* data){
  const uint8_t numArgsRequired = 12;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }
  uint8_t i=0;
  if(targetSprite_){
    spr_->drawTriangle((data[i++]<<8) | data[i++], // x1
                     (data[i++]<<8) | data[i++], // y1
                     (data[i++]<<8) | data[i++], // x2
                     (data[i++]<<8) | data[i++], // y2
                     (data[i++]<<8) | data[i++], // x3
                     (data[i++]<<8) | data[i++], // y3
                     setColor_);  //color
  }else{
    tft_->drawTriangle((data[i++]<<8) | data[i++], // x1
                      (data[i++]<<8) | data[i++], // y1
                      (data[i++]<<8) | data[i++], // x2
                      (data[i++]<<8) | data[i++], // y2
                      (data[i++]<<8) | data[i++], // x3
                      (data[i++]<<8) | data[i++], // y3
                      setColor_);  //color
  }
}

void DispFunction::fillTriangle(uint8_t count, uint8_t* data){
  const uint8_t numArgsRequired = 12;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }
  uint8_t i=0;
  if(targetSprite_){
    spr_->fillTriangle((data[i++]<<8) | data[i++], // x1
                     (data[i++]<<8) | data[i++], // y1
                     (data[i++]<<8) | data[i++], // x2
                     (data[i++]<<8) | data[i++], // y2
                     (data[i++]<<8) | data[i++], // x3
                     (data[i++]<<8) | data[i++], // y3
                     setColor_);  //color
  }else{
    tft_->fillTriangle((data[i++]<<8) | data[i++], // x1
                      (data[i++]<<8) | data[i++], // y1
                      (data[i++]<<8) | data[i++], // x2
                      (data[i++]<<8) | data[i++], // y2
                      (data[i++]<<8) | data[i++], // x3
                      (data[i++]<<8) | data[i++], // y3
                      setColor_);  //color
  }
}

void DispFunction::drawArc(uint8_t count, uint8_t* data){
  const uint8_t numArgsRequired = 13;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }
  uint8_t i=0;
  if(targetSprite_){
    spr_->drawArc((data[i++]<<8) | data[i++], // x
                (data[i++]<<8) | data[i++], // y
                (data[i++]<<8) | data[i++], // radius
                (data[i++]<<8) | data[i++], // inner radius
                (data[i++]<<8) | data[i++], // start angle
                (data[i++]<<8) | data[i++], // end angle
                setColor_,                  // color
                set_bg_color_,              // bg color
                data[i++]);                 // antialiased
  }else{
    tft_->drawArc((data[i++]<<8) | data[i++], // x
                (data[i++]<<8) | data[i++], // y
                (data[i++]<<8) | data[i++], // radius
                (data[i++]<<8) | data[i++], // inner radius
                (data[i++]<<8) | data[i++], // start angle
                (data[i++]<<8) | data[i++], // end angle
                setColor_,                  // color
                set_bg_color_,              // bg color
                data[i++]);                 // antialiased
  }
}

void DispFunction::setBgColor(uint8_t count, uint8_t* data){
  const uint8_t numArgsRequired = 2;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }
  set_bg_color_ = (data[0]<<8) | data[1];
}

void DispFunction::drawSmoothCircle(uint8_t count, uint8_t* data){
  const uint8_t numArgsRequired = 6;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }
  uint8_t i=0;

  if(targetSprite_){
    spr_->drawSmoothCircle((data[i++]<<8) | data[i++], // x
                         (data[i++]<<8) | data[i++], // y
                         (data[i++]<<8) | data[i++], // r
                         setColor_,                  // color
                         set_bg_color_);             // bg color
  }else{
    tft_->drawSmoothCircle((data[i++]<<8) | data[i++], // x
                         (data[i++]<<8) | data[i++], // y
                         (data[i++]<<8) | data[i++], // r
                         setColor_,                  // color
                         set_bg_color_);             // bg color 
  }
  
}

void DispFunction::fillSmoothCircle(uint8_t count, uint8_t* data){
  const uint8_t numArgsRequired = 6;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }
  uint8_t i=0;

  if(targetSprite_){
    spr_->fillSmoothCircle((data[i++]<<8) | data[i++], // x
                         (data[i++]<<8) | data[i++], // y
                         (data[i++]<<8) | data[i++], // r
                         setColor_,                  // color
                         set_bg_color_);             // bg color
  }else{
    tft_->fillSmoothCircle((data[i++]<<8) | data[i++], // x
                         (data[i++]<<8) | data[i++], // y
                         (data[i++]<<8) | data[i++], // r
                         setColor_,                  // color
                         set_bg_color_);             // bg color
  }
  
}

void DispFunction::drawSmoothRoundRect(uint8_t count, uint8_t* data){
  const uint8_t numArgsRequired = 12;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }
  uint8_t i=0;

  if(targetSprite_){
    spr_->drawSmoothRoundRect((data[i++]<<8) | data[i++], // x
                            (data[i++]<<8) | data[i++], // y
                            (data[i++]<<8) | data[i++], // radius
                            (data[i++]<<8) | data[i++], // inner radius
                            (data[i++]<<8) | data[i++], // width
                            (data[i++]<<8) | data[i++], // height
                            setColor_,                  // color
                            set_bg_color_,              // bg color
                            0xff);                      // all 4 quadrant
  }else{
    tft_->drawSmoothRoundRect((data[i++]<<8) | data[i++], // x
                            (data[i++]<<8) | data[i++], // y
                            (data[i++]<<8) | data[i++], // radius
                            (data[i++]<<8) | data[i++], // inner radius
                            (data[i++]<<8) | data[i++], // width
                            (data[i++]<<8) | data[i++], // height
                            setColor_,                  // color
                            set_bg_color_,              // bg color
                            0xff);                      // all 4 quadrant
  }
  
}

void DispFunction::fillSmoothRoundRect(uint8_t count, uint8_t* data){
  const uint8_t numArgsRequired = 12;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }
  uint8_t i=0;
  
  if(targetSprite_){
    spr_->fillSmoothRoundRect((data[i++]<<8) | data[i++], // x
                            (data[i++]<<8) | data[i++], // y
                            (data[i++]<<8) | data[i++], // width
                            (data[i++]<<8) | data[i++], // height
                            (data[i++]<<8) | data[i++], // radius
                            setColor_,                  // color
                            set_bg_color_);             // bg color
  }else{
    tft_->fillSmoothRoundRect((data[i++]<<8) | data[i++], // x
                            (data[i++]<<8) | data[i++], // y
                            (data[i++]<<8) | data[i++], // width
                            (data[i++]<<8) | data[i++], // height
                            (data[i++]<<8) | data[i++], // radius
                            setColor_,                  // color
                            set_bg_color_);             // bg color
  }
}

void DispFunction::drawSmoothArc(uint8_t count, uint8_t* data){
  const uint8_t numArgsRequired = 13;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }
  uint8_t i=0;

  if(targetSprite_){
    spr_->drawSmoothArc((data[i++]<<8) | data[i++], // x
                      (data[i++]<<8) | data[i++], // y
                      (data[i++]<<8) | data[i++], // radius
                      (data[i++]<<8) | data[i++], // inner radius
                      (data[i++]<<8) | data[i++], // start angle
                      (data[i++]<<8) | data[i++], // end angle
                      setColor_,                  // color
                      set_bg_color_,              // bg color
                      data[i++]);                 // Round ends
  }else{
    tft_->drawSmoothArc((data[i++]<<8) | data[i++], // x
                      (data[i++]<<8) | data[i++], // y
                      (data[i++]<<8) | data[i++], // radius
                      (data[i++]<<8) | data[i++], // inner radius
                      (data[i++]<<8) | data[i++], // start angle
                      (data[i++]<<8) | data[i++], // end angle
                      setColor_,                  // color
                      set_bg_color_,              // bg color
                      data[i++]);                 // Round ends
  }
  
}

void DispFunction::drawWideLine(uint8_t count, uint8_t* data){
  const uint8_t numArgsRequired = 10;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }
  uint8_t i=0;

  if(targetSprite_){
    spr_->drawWideLine((data[i++]<<8) | data[i++], // x1
                     (data[i++]<<8) | data[i++], // y1
                     (data[i++]<<8) | data[i++], // x2
                     (data[i++]<<8) | data[i++], // y2
                     (data[i++]<<8) | data[i++], // width
                     setColor_,                  // color
                     set_bg_color_);             // bg color
  }else{
    tft_->drawWideLine((data[i++]<<8) | data[i++], // x1
                     (data[i++]<<8) | data[i++], // y1
                     (data[i++]<<8) | data[i++], // x2
                     (data[i++]<<8) | data[i++], // y2
                     (data[i++]<<8) | data[i++], // width
                     setColor_,                  // color
                     set_bg_color_);             // bg color
  }
  
}

void DispFunction::drawWedgeLine(uint8_t count, uint8_t* data){
  const uint8_t numArgsRequired = 12;
  
  if(count != numArgsRequired){
    Serial.println("Unable to execute ");
    return;
  }
  uint8_t i=0;

  if(targetSprite_){
    spr_->drawWedgeLine((data[i++]<<8) | data[i++], // x1
                      (data[i++]<<8) | data[i++], // y1
                      (data[i++]<<8) | data[i++], // x2
                      (data[i++]<<8) | data[i++], // y2
                      (data[i++]<<8) | data[i++], // start width
                      (data[i++]<<8) | data[i++], // end width
                      setColor_,                  // color
                      set_bg_color_);             // bg color
  }else{
    tft_->drawWedgeLine((data[i++]<<8) | data[i++], // x1
                      (data[i++]<<8) | data[i++], // y1
                      (data[i++]<<8) | data[i++], // x2
                      (data[i++]<<8) | data[i++], // y2
                      (data[i++]<<8) | data[i++], // start width
                      (data[i++]<<8) | data[i++], // end width
                      setColor_,                  // color
                      set_bg_color_);             // bg color
  }
  
}