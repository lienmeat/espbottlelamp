#ifndef ANIMATIONMANAGER_H
#define ANIMATIONMANAGER_H

#include "LampAnimation.h"
#include "Lamp.h"

class AnimationManager
{
  public:
    AnimationManager();
    void begin();
    void loop();
    //adds an animation to the manager
    int8_t addAnimation(LampAnimation* anim);
//    void setAnimations(LampAnimation* animations[], int8_t numb);
    //gets an animation at an index
    LampAnimation* getAnimation(int8_t idx);
    //gets the current animation
    LampAnimation* getCurrentAnimation();
    void switchAnimation(int8_t idx);
    //switch to an animation by name & return index or -1
    int switchAnimationByName(String name);
    //gets number of animations
    int8_t getNumAnimations();
    //gets an array of animation names
    String getAnimationNames();
    //itterates the current animation
    int itterate();
    //resets the current animation
    void reset();
    void setSpeed(uint8_t percent);
    Lamp* getLamp();
  protected:
    int8_t num_animations = 0;
    int8_t max_animations = 100;
    int8_t cur_animation = 0;
    uint8_t speed = 100;
    LampAnimation* anims[100];
  private:
};
#endif // ANIMATIONMANAGER_H
