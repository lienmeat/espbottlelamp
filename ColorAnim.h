#ifndef COLORANIM_H
#define COLORANIM_H

#include "LampAnimation.h"
#include "Lamp.h"
#include "FastLED.h"

class ColorAnim : public LampAnimation
{
  public:
    ColorAnim(Lamp* thelamp, CRGB color, String name) : LampAnimation (thelamp, name) {
      this->color = color;
      setFPS(5);
    }
    void setColor(CRGB c) {
      color = c;
    }
    int itterate() {
        if(tick()) {
          lamp->fill_color(0, (lamp->getNumLeds() - 1), color);
          lamp->render();
        }
    };
    void reset() {
      color = CRGB::Black;
    };
  protected:
    CRGB color = CRGB::Black;
  private:
};

#endif // COLORANIM_H
