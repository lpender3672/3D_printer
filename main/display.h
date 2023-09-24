#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT_Macros.h>
#include <Adafruit_SPITFT.h>
#include <gfxfont.h>

#include <Adafruit_SSD1306.h>
#include <splash.h>

#include <SPI.h>
#include <Wire.h>



#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C

class printer_display : public Adafruit_SSD1306
{

private:

    Adafruit_SSD1306 display;

public:

    printer_display() : Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET)
    {
        display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    }

    void begin();

    void draw_nozzle_icon(float x, float y);

    void update(float px, float py, float pz, float pe, float Tnozzle, float Tbed);

};