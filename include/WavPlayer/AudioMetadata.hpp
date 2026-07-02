#pragma once

#include <string>
#include <chrono>
#include <windef.h>
#include "Utility.hpp"

struct AudioMetadata{
    std::string name;
    std::string artist;
    DWORD frameCount;
    std::chrono::milliseconds totalDuration;
    DWORD audioDataSize;
    DWORD audioFrameCount;
};