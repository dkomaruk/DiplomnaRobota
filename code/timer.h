#ifndef TIMER_H

#include <SDL3/SDL.h>

struct Timer
{
    Uint64 start;

    float duration;
    float elapsed = 0.0f;

    bool isFinished = true;
    bool isPaused = false;
};

Timer StartTimer(float seconds);

void PauseTimer(Timer *timer);
void ResumeTimer(Timer *timer);

float UpdateTimer(Timer *timer, float deltaTime);

float RemainingTime(Timer *timer);

#define TIMER_H
#endif