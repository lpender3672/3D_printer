#include "display.h"


#define LOGO_HEIGHT   8
#define LOGO_WIDTH    8

static const unsigned char PROGMEM hotend_logo_bmp[] =
{ 0b00000000,
  0b00000001,
  0b00000001,
  0b00000011,
  0b11110011,
  0b11111110,
  0b01111110,
  0b00110011
};

void printer_display::begin()
{
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }
  display.display();
}

void printer_display::draw_nozzle_icon(float x, float y) {
    display.drawBitmap(x, y, hotend_logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, SSD1306_WHITE);
}

void printer_display::update(float px, float py, float pz, float pe, float Tnozzle, float Tbed) {
    // do some updating here


}
