#ifndef AUDIO_H

#include "AL/al.h"
#include "AL/alext.h"

struct Audio
{
    ALCdevice *device;
    ALCcontext *context;
};

#define AUDIO_H
#endif