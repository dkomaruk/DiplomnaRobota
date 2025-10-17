#include "timer.h"

Timer StartTimer(float seconds)
{
    Timer timer = {};

    timer.start = SDL_GetPerformanceCounter();
    timer.duration = seconds;
    timer.isFinished = false;

    return timer;
}

void PauseTimer(Timer *timer)
{
    timer->isPaused = true;
}

void ResumeTimer(Timer *timer)
{
    timer->isPaused = false;
}

float UpdateTimer(Timer *timer, float deltaTime)
{
    if(timer->isFinished || timer->isPaused)
    {
        return 0.0f;
    }

    float remaining = timer->duration - timer->elapsed;
    float timerDeltaTime = deltaTime;

    if(timer->elapsed + deltaTime >= timer->duration)
    {
        timerDeltaTime = remaining;
        timer->isFinished = true;
    }

    timer->elapsed += timerDeltaTime;

    return timerDeltaTime;
}

float RemainingTime(Timer *timer)
{
    if(timer->isFinished)
    {
        return 0.0f;
    }

    return timer->duration - timer->elapsed;
}