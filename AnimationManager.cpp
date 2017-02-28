#include "AnimationManager.h"

AnimationManager::AnimationManager() {
  
}

void AnimationManager::begin() {
  //what do we have to do here?  
}

int8_t AnimationManager::addAnimation(LampAnimation* anim) {
  if(num_animations < max_animations) {
    anims[num_animations] = anim;
    int8_t ret = num_animations;
    num_animations++;
    return ret;
  }
}

LampAnimation* AnimationManager::getAnimation(int8_t idx) {
  if(idx < num_animations) {
    return anims[idx];
  }
}

LampAnimation* AnimationManager::getCurrentAnimation() {
  if(cur_animation < num_animations) {
    return anims[cur_animation];
  }
}
void AnimationManager::switchAnimation(int8_t idx) {
  if(idx < num_animations) {
    cur_animation = idx;  
  }
}

int8_t AnimationManager::getNumAnimations() {
  return num_animations;  
}

//gets an array of animation names
String AnimationManager::getAnimationNames() {
  String names;
  for(int8_t i = 0; i < num_animations; i++) {
    if(i > 0) {
      names+=",";  
    }
    names += (String) anims[i]->getName();
  }
  return names;
}

//itterates the current animation
int AnimationManager::itterate() {
  return anims[cur_animation]->itterate();  
}

//resets the current animation
void AnimationManager::reset() {
  anims[cur_animation]->reset();  
}

void AnimationManager::setSpeed(uint8_t percent) {
  speed = percent;
  anims[cur_animation]->setSpeed(percent);
}

Lamp* AnimationManager::getLamp() {
  return anims[cur_animation]->getLamp();
}

void AnimationManager::loop() {
  setSpeed(speed);
  itterate();
}
