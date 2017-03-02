#include "RainbowFill.h"
#include "LampAnimation.h"
#include "Lamp.h"

RainbowFill::RainbowFill(Lamp* thelamp, String name) : LampAnimation (thelamp, name)
{
  //ctor
  hue_step = 255 / lamp->getNumLeds();
  setFPS(5);
}

int RainbowFill::itterate()
{
  if(tick()) {
    for(int i=0; i < num_leds; i++) {
      uint8_t tmp_hue = i * hue_step + hue;
      lamp->setLed(i, CHSV(tmp_hue, 255, 255));
    }
    
    lamp->render();
    hue+=hue_step;
  }
  return 1;
}

void RainbowFill::reset() {
  cur_led = -1;
  hue = 0;
  hue_step = 0;
}
