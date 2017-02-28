#include "LampAnimation.h"

LampAnimation::LampAnimation(Lamp* thelamp, String name)
{
  lamp = thelamp;
  setName(name);
  //needed often
  num_leds = lamp->getNumLeds();
  cols = lamp->getCols();
  rows = lamp->getRows();
}

int LampAnimation::itterate() {
  //noop, override
  return 1;
}

void LampAnimation::reset() {
  //noop, override
}

void LampAnimation::startTimer() {
    timer_start = millis();
    int ms = (1000.0 / (float(fps) * (float(animation_speed) / 100.0)));
    this->timer_end = timer_start + ms;
}

uint8_t LampAnimation::tick() {
  if(millis() < timer_end) {
    return 0;
  }
  startTimer();
  return 1;
}

void LampAnimation::setSpeed(uint8_t percent) {
  this->animation_speed = percent;
}

uint8_t LampAnimation::getSpeed() {
  return animation_speed;
}

void LampAnimation::setFPS(uint8_t fps) {
  this->fps = fps;  
}

uint8_t LampAnimation::getFPS() {
  return fps;  
}

void LampAnimation::setName(String name) {
  this->name = name;
}

String LampAnimation::getName() {
  return name;
}

Lamp* LampAnimation::getLamp() {
  return lamp;  
}
