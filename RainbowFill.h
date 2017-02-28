#ifndef RAINBOWFILL_H
#define RAINBOWFILL_H

#include "LampAnimation.h"
#include "Lamp.h"
#include "FastLED.h"

class RainbowFill : public LampAnimation
{
  public:
    RainbowFill(Lamp* lamp, String name);
    int itterate();
    void reset();
  protected:
    int cur_led = -1;
    uint8_t hue = 0;
    int hue_step = 0;
    CRGB prev_col = CRGB::Black;
  private:
};

#endif // RAINBOWFILL_H
