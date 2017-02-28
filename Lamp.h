#ifndef LAMP_H
#define LAMP_H

//fastled crap
#include <bitswap.h>
#include <chipsets.h>
#include <color.h>
#include <colorpalettes.h>
#include <colorutils.h>
#include <controller.h>
#include <cpp_compat.h>
#include <dmx.h>
#include <FastLED.h>
#include <fastled_config.h>
#include <fastled_delay.h>
#include <fastled_progmem.h>
#include <fastpin.h>
#include <fastspi.h>
#include <fastspi_bitbang.h>
#include <fastspi_dma.h>
#include <fastspi_nop.h>
#include <fastspi_ref.h>
#include <fastspi_types.h>
#include <hsv2rgb.h>
#include <led_sysdefs.h>
#include <lib8tion.h>
#include <noise.h>
#include <pixelset.h>
#include <pixeltypes.h>
#include <platforms.h>
#include <power_mgt.h>

class Lamp
{
  public:
    Lamp(int pin, int numberofleds, int cols);
    ~Lamp();
    //run in setup!
    void begin();
    //render the current assigned colors to the lamp
    void render();
    //is the lamp on?
    int on();
    //turn the lamp off
    void turnOff();
    //turn the lamp on
    void turnOn();
    int getRows();
    void setRows(int rows);
    int getCols();
    void setCols(int cols);
    int getNumLeds();
    void setNumLeds(int numberofleds);
    void setLed(int led, CRGB led_value);
    CRGB getLed(int led);
    CRGB* getLeds();
    void setLeds(CRGB (* ledarr));
    //these probably don't belong here, but whatever
    void fill_color(int start, int end, CRGB color);
    void fill_row(int row, CRGB color);
    void fill_column(int col, CRGB color);
    uint16_t XY( uint8_t x, uint8_t y);
    void setColorShift(CRGB color);
    int8_t getColorShift();
  protected:
    //pin to write data to
    int pin;
    //type of led strip
    
    
    //fastled led array
    CRGB* leds;
    //whether lamp is on
    int is_on = 1;
    //how many leds
    int num_leds = 50;
    //how many cols in a matrix of the leds
    int cols = 5;
    //how many rows in a matrix of the leds
    int rows = 10;
    //the current color shift off the norm
    CRGB color_shift = CRGB(0, 0, 0);
  private:
    //method to apply the color shift
    void applyColorShift();
};

void setupLamp();

Lamp* getLamp();

#endif // LAMP_H
