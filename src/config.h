#ifndef CONFIG_H
#define CONFIG_H

#define SERIAL_SIZE_RX 1024*60

#define SERIAL_BAUD_DEFAULT 115200
// #define SERIAL_BAUD_DEFAULT 2000000

#define CALIBRATION_FILE "/TouchCalData_3"
#define BAUD_FILE "/SavedBaud"

#define BACKLIGHT_PWM  1
#define BACKLIGHT_PIN 48
#define POWER_EN_PIN  9
#define TOUCH_CS_PIN  2

#define RXD2 18
#define TXD2 17

#define PIXEL_LED 38

static const int baudRateList[] = {
    9600, 19200, 115200, 460800, 2000000
};
enum baudrates{
    BAUD_9600,
    BAUD_19200,
    BAUD_115200,
    BAUD_460800,
    BAUD_2000000
};

#endif // CONFIG_H