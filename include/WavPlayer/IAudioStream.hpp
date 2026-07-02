#pragma once
#include <windef.h>
#include "Typedefs.hpp"

class IAudioStream{
public:
    virtual AudioFrameUnit ReadFrames(BYTE* buf, AudioFrameUnit frameCountNeeded) = 0;
    virtual AudioFramePos FramePosition() = 0;
    virtual void Seek(AudioFramePos frame) = 0;
    virtual bool Eof() = 0;
};