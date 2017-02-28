#ifndef LAMPANIMATION_H
#define LAMPANIMATION_H

#include "Lamp.h"

class LampAnimation
{
public:
    LampAnimation(Lamp* thelamp, String name);
    virtual int itterate();
    virtual void reset();
    void setSpeed(uint8_t percent);
    uint8_t getSpeed();
    void setName(String name);
    String getName();
    Lamp* getLamp();
protected:
    Lamp* lamp;
    uint8_t num_leds;
    uint8_t rows;
    uint8_t cols;
    //how many updates per second this animation does by default
    void startTimer();
    uint8_t tick();
    uint8_t getFPS();
    void setFPS(uint8_t fps);
    String name;
private:
  uint8_t fps = 60;
  unsigned long timer_start = 0;
  unsigned long timer_end = 0;
  //animation speed in percent
  uint8_t animation_speed = 100;
};

#endif // LAMPANIMATION_H
