#ifndef SEAHAWKS_H
#define SEAHAWKS_H

#include "FastLED.h"
#include "Lamp.h"
#include "LampAnimation.h"

DEFINE_GRADIENT_PALETTE( seahawks_gp ) {
  0, 3, 38, 58, //dk blue
  63, 54, 87, 140,   //blue
  127, 78, 167, 1, //green
  191, 144, 145, 140,   //gray
  255, 255, 255, 255, //white
};

class Seahawks : public LampAnimation
{
  public: 
    Seahawks(Lamp* lamp, String name) : LampAnimation(lamp, name) {
      setFPS(1);  
    }
    int itterate() {
        if(tick()) {
            //logic...
            int num = lamp->getNumLeds() - 1;
//            lamp->fill_color();
            
            for(int i = 0; i <= num; i++) {
                uint8_t val = map(i, 0, num, 0, 255);
//                CRGB color = ColorFromPalette(pallet, val);
                CRGB color = CRGB(3, 38, 58);
                lamp->setLed(i, color);
            }
            lamp->render();
            step++;
        }
        return 1;
    }
  protected:
    uint8_t step = 0;
    CRGBPalette256 pallet = seahawks_gp;
};



#endif // SEAHAWKS_H
